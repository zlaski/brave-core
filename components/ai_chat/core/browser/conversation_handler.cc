// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/components/ai_chat/core/browser/conversation_handler.h"

#include <utility>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "brave/components/ai_chat/core/browser/conversation_driver.h"
#include "brave/components/ai_chat/core/common/mojom/ai_chat.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"

namespace ai_chat {

namespace {
using AssociatedContentDelegate =
    ConversationHandler::AssociatedContentDelegate;
}  // namespace

ConversationHandler::ConversationHandler(
    const mojom::Conversation* conversation)
    : metadata_(conversation) {}

ConversationHandler::~ConversationHandler() = default;

void ConversationHandler::OnAssociatedContentDestroyed() {
  // The associated content delegate is destroyed, so we should not try to
  // fetch. It may be populated later, e.g. through back navigation.
  // If this conversation is allowed to be associated with content, we can keep
  // using our current cached content.
  conversation_driver_observation_.Reset();
  associated_content_delegate_.reset();
}

void ConversationHandler::SetAssociatedContentDelegate(
    base::WeakPtr<AssociatedContentDelegate> delegate) {
  // If this conversation is allowed to fetch content, this is the delegate
  // that can provide fresh content for the conversation.
  CHECK(delegate) << "Don't send a null delegate";

  if (associated_content_delegate_ &&
      (delegate.get() == associated_content_delegate_.get())) {
    return;
  }

  associated_content_delegate_ = delegate;

  // Temporary
  if (associated_content_delegate_) {
    conversation_driver_observation_.Observe(
        static_cast<ConversationDriver*>(associated_content_delegate_.get()));
  }
}

void ConversationHandler::GetConversationHistory(
    GetConversationHistoryCallback callback) {
  // Temporary pass through to the deprecated class
  if (!associated_content_delegate_) {
    std::move(callback).Run(std::vector<mojom::ConversationTurnPtr>());
    return;
  }
  ConversationDriver* driver =
      static_cast<ConversationDriver*>(associated_content_delegate_.get());
  std::move(callback).Run(driver->GetVisibleConversationHistory());
}

void ConversationHandler::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ConversationHandler::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void ConversationHandler::Bind(
    mojo::PendingReceiver<mojom::ConversationHandler> receiver,
    mojo::PendingRemote<mojom::ChatUIPage> conversation_ui_handler) {
  receivers_.Add(this, std::move(receiver));
  conversation_ui_handlers_.Add(std::move(conversation_ui_handler));
}

void ConversationHandler::OnHistoryUpdate() {
  for (auto& client : conversation_ui_handlers_) {
    client->OnConversationHistoryUpdate();
  }
  ConversationDriver* driver =
      static_cast<ConversationDriver*>(associated_content_delegate_.get());
  for (auto& observer : observers_) {
    observer.OnConversationEntriesChanged(
        this, driver->GetVisibleConversationHistory());
  }
}

}  // namespace ai_chat
