/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/ai_chat/core/browser/associated_content_driver.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "base/containers/contains.h"
#include "base/containers/fixed_flat_set.h"
#include "base/functional/bind.h"
#include "base/memory/weak_ptr.h"
#include "base/one_shot_event.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_util.h"
#include "brave/components/ai_chat/core/browser/constants.h"
#include "brave/components/ai_chat/core/browser/conversation_handler.h"

namespace ai_chat {

AssociatedContentDriver::AssociatedContentDriver()
    : on_page_text_fetch_complete_(std::make_unique<base::OneShotEvent>()) {}

AssociatedContentDriver::~AssociatedContentDriver() {
  for (auto& conversation : associated_conversations_) {
    if (conversation) {
      conversation->OnAssociatedContentDestroyed(cached_text_content_,
                                                 is_video_);
    }
  }
}

void AssociatedContentDriver::AddRelatedConversation(
    ConversationHandler* conversation) {
  associated_conversations_.insert(conversation);
}

void AssociatedContentDriver::OnRelatedConversationDestroyed(
    ConversationHandler* conversation) {
  associated_conversations_.erase(conversation);
}

void AssociatedContentDriver::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void AssociatedContentDriver::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

int AssociatedContentDriver::GetContentId() const {
  return current_navigation_id_;
}

GURL AssociatedContentDriver::GetURL() const {
  return GetPageURL();
}

std::u16string AssociatedContentDriver::GetTitle() const {
  return GetPageTitle();
}

void AssociatedContentDriver::GetContent(
    ConversationHandler::GetPageContentCallback callback) {
  // Register callback to fire when the event is complete
  auto handle_existing_fetch_complete = base::BindOnce(
      &AssociatedContentDriver::OnExistingGeneratePageContentComplete,
      weak_ptr_factory_.GetWeakPtr(), std::move(callback),
      current_navigation_id_);
  on_page_text_fetch_complete_->Post(FROM_HERE,
                                     std::move(handle_existing_fetch_complete));
  // Only perform a fetch once at a time, and then use the results from
  // an in-progress operation.
  if (is_page_text_fetch_in_progress_) {
    DVLOG(1) << "A page content fetch is in progress, waiting for the existing "
                "operation to complete";
  } else {
    is_page_text_fetch_in_progress_ = true;
    GetPageContent(
        base::BindOnce(&AssociatedContentDriver::OnGeneratePageContentComplete,
                       weak_ptr_factory_.GetWeakPtr(), current_navigation_id_),
        content_invalidation_token_);
  }
}

void AssociatedContentDriver::OnExistingGeneratePageContentComplete(
    ConversationHandler::GetPageContentCallback callback,
    int64_t navigation_id) {
  if (navigation_id != current_navigation_id_) {
    return;
  }
  std::move(callback).Run(cached_text_content_, is_video_,
                          content_invalidation_token_);
}

void AssociatedContentDriver::OnGeneratePageContentComplete(
    int64_t navigation_id,
    std::string contents_text,
    bool is_video,
    std::string invalidation_token) {
  DVLOG(1) << "OnGeneratePageContentComplete";
  DVLOG(4) << "Contents(is_video=" << is_video
           << ", invalidation_token=" << invalidation_token
           << "): " << contents_text;
  if (navigation_id != current_navigation_id_) {
    return;
  }

  is_page_text_fetch_in_progress_ = false;

  // If invalidation token matches existing token, then
  // content was not re-fetched and we can use our existing cache.
  if (invalidation_token.empty() ||
      (invalidation_token != content_invalidation_token_)) {
    is_video_ = is_video;
    // Cache page content on instance so we don't always have to re-fetch
    // if the content fetcher knows the content won't have changed and the fetch
    // operation is expensive (e.g. network).
    cached_text_content_ = contents_text;
    content_invalidation_token_ = invalidation_token;
    if (contents_text.empty()) {
      DVLOG(1) << __func__ << ": No data";
    }
  }

  on_page_text_fetch_complete_->Signal();
  on_page_text_fetch_complete_ = std::make_unique<base::OneShotEvent>();
}

std::string_view AssociatedContentDriver::GetCachedTextContent() {
  return cached_text_content_;
}

bool AssociatedContentDriver::GetCachedIsVideo() {
  return is_video_;
}

void AssociatedContentDriver::OnFaviconImageDataChanged() {
  for (auto& conversation : associated_conversations_) {
    conversation->OnFaviconImageDataChanged();
  }
}

void AssociatedContentDriver::OnNewPage(int64_t navigation_id) {
  // Tell the associated_conversations_ that we're breaking up
  for (auto& conversation : associated_conversations_) {
    conversation->OnAssociatedContentDestroyed(cached_text_content_, is_video_);
  }
  // Tell the observer how to find the next conversation
  for (auto& observer : observers_) {
    observer.OnAssociatedContentNavigated(navigation_id);
  }

  // Reset state for next navigated Page
  associated_conversations_.clear();
  current_navigation_id_ = navigation_id;
  cached_text_content_.clear();
  content_invalidation_token_.clear();
  is_video_ = false;
}

}  // namespace ai_chat
