// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/webui/ai_chat/ai_chat_ui_page_handler.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "base/notreached.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "brave/browser/ai_chat/ai_chat_service_factory.h"
#include "brave/browser/ui/side_panel/ai_chat/ai_chat_side_panel_utils.h"
#include "brave/components/ai_chat/core/browser/ai_chat_service.h"
#include "brave/components/ai_chat/core/browser/constants.h"
#include "brave/components/ai_chat/core/common/mojom/ai_chat.mojom-shared.h"
#include "brave/components/ai_chat/core/common/mojom/ai_chat.mojom.h"
#include "brave/components/ai_chat/core/common/pref_names.h"
#include "chrome/browser/favicon/favicon_service_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/singleton_tabs.h"
#include "components/favicon/core/favicon_service.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/visibility.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/url_constants.h"
#include "ui/base/l10n/l10n_util.h"

#if BUILDFLAG(IS_ANDROID)
#include "brave/browser/ui/android/ai_chat/brave_leo_settings_launcher_helper.h"
#endif

namespace {
constexpr char kURLRefreshPremiumSession[] =
    "https://account.brave.com/?intent=recover&product=leo";
constexpr char kURLLearnMoreBraveSearchLeo[] =
    "https://support.brave.com/hc/en-us/categories/20990938292237-Brave-Leo";
#if !BUILDFLAG(IS_ANDROID)
constexpr char kURLGoPremium[] =
    "https://account.brave.com/account/?intent=checkout&product=leo";
constexpr char kURLManagePremium[] = "https://account.brave.com/";
#endif
}  // namespace

namespace ai_chat {

using mojom::CharacterType;
using mojom::ConversationTurn;
using mojom::ConversationTurnVisibility;

AIChatUIPageHandler::ChatContextObserver::ChatContextObserver(
    content::WebContents* web_contents,
    AIChatUIPageHandler& page_handler)
    : content::WebContentsObserver(web_contents), page_handler_(page_handler) {}

AIChatUIPageHandler::ChatContextObserver::~ChatContextObserver() = default;

AIChatUIPageHandler::AIChatUIPageHandler(
    content::WebContents* owner_web_contents,
    content::WebContents* chat_context_web_contents,
    Profile* profile,
    mojo::PendingReceiver<ai_chat::mojom::AIChatUIHandler> receiver)
    : content::WebContentsObserver(owner_web_contents),
      profile_(profile),
      receiver_(this, std::move(receiver)) {
  // Standalone mode means Chat is opened as its own tab in the tab strip and
  // not a side panel. chat_context_web_contents is nullptr in that case
  const bool is_standalone = chat_context_web_contents == nullptr;
  if (!is_standalone) {
    active_chat_tab_helper_ =
        ai_chat::AIChatTabHelper::FromWebContents(chat_context_web_contents);
    chat_tab_helper_observation_.Observe(active_chat_tab_helper_);
    chat_context_observer_ =
        std::make_unique<ChatContextObserver>(chat_context_web_contents, *this);
  } else {
    // TODO(petemill): Enable conversation without the TabHelper now that
    // all conversation logic is extracted to ConversationHandler.
    NOTIMPLEMENTED();
  }

  favicon_service_ = FaviconServiceFactory::GetForProfile(
      profile_, ServiceAccessType::EXPLICIT_ACCESS);
}

AIChatUIPageHandler::~AIChatUIPageHandler() = default;

void AIChatUIPageHandler::HandleVoiceRecognition() {
#if BUILDFLAG(IS_ANDROID)
  // TODO(petemill): figure out why java needs access to conversation context
  // web contents and this can't be done in UI WebContents only. Taget
  // WebContents
  auto* context_web_contents = (active_chat_tab_helper_)
                                   ? active_chat_tab_helper_->web_contents()
                                   : web_contents();
  ai_chat::HandleVoiceRecognition(web_contents(), context_web_contents);
#endif
}

void AIChatUIPageHandler::OpenAIChatSettings() {
  auto* contents_to_navigate = (active_chat_tab_helper_)
                                   ? active_chat_tab_helper_->web_contents()
                                   : web_contents();
#if !BUILDFLAG(IS_ANDROID)
  const GURL url("brave://settings/leo-assistant");
  if (auto* browser = chrome::FindBrowserWithTab(contents_to_navigate)) {
    ShowSingletonTab(browser, url);
  } else {
    contents_to_navigate->OpenURL(
        {url, content::Referrer(), WindowOpenDisposition::NEW_FOREGROUND_TAB,
         ui::PAGE_TRANSITION_LINK, false},
        /*navigation_handle_callback=*/{});
  }
#else
  ai_chat::ShowBraveLeoSettings(contents_to_navigate);
#endif
}

void AIChatUIPageHandler::OpenURL(const GURL& url) {
  if (!url.SchemeIs(content::kChromeUIScheme) &&
      !url.SchemeIs(url::kHttpsScheme)) {
    return;
  }

#if !BUILDFLAG(IS_ANDROID)
  auto* contents_to_navigate = (active_chat_tab_helper_)
                                   ? active_chat_tab_helper_->web_contents()
                                   : web_contents();
  contents_to_navigate->OpenURL(
      {url, content::Referrer(), WindowOpenDisposition::NEW_FOREGROUND_TAB,
       ui::PAGE_TRANSITION_LINK, false},
      /*navigation_handle_callback=*/{});
#else
  // We handle open link different on Android as we need to close the chat
  // window because it's always full screen
  ai_chat::OpenURL(url.spec());
#endif
}

void AIChatUIPageHandler::GoPremium() {
#if !BUILDFLAG(IS_ANDROID)
  OpenURL(GURL(kURLGoPremium));
#else
  auto* contents_to_navigate = (active_chat_tab_helper_)
                                   ? active_chat_tab_helper_->web_contents()
                                   : web_contents();
  ai_chat::GoPremium(contents_to_navigate);
#endif
}

void AIChatUIPageHandler::RefreshPremiumSession() {
  OpenURL(GURL(kURLRefreshPremiumSession));
}

void AIChatUIPageHandler::ManagePremium() {
#if !BUILDFLAG(IS_ANDROID)
  OpenURL(GURL(kURLManagePremium));
#else
  auto* contents_to_navigate = (active_chat_tab_helper_)
                                   ? active_chat_tab_helper_->web_contents()
                                   : web_contents();
  ai_chat::ManagePremium(contents_to_navigate);
#endif
}

void AIChatUIPageHandler::OpenLearnMoreAboutBraveSearchWithLeo() {
  OpenURL(GURL(kURLLearnMoreBraveSearchLeo));
}

void AIChatUIPageHandler::OpenModelSupportUrl() {
  OpenURL(GURL(kLeoModelSupportUrl));
}

void AIChatUIPageHandler::ChatContextObserver::WebContentsDestroyed() {
  page_handler_->HandleWebContentsDestroyed();
}

void AIChatUIPageHandler::HandleWebContentsDestroyed() {
  chat_tab_helper_observation_.Reset();
  chat_context_observer_.reset();
}

void AIChatUIPageHandler::OnAssociatedContentNavigated(int new_navigation_id) {
  chat_ui_->OnNewDefaultConversation();
}

// void AIChatUIPageHandler::OnPrintPreviewRequested(bool is_pdf) {
// #if BUILDFLAG(ENABLE_PRINT_PREVIEW)
//   print_preview_extractor_ = std::make_unique<PrintPreviewExtractor>(
//       active_chat_tab_helper_->web_contents(), profile_, is_pdf);
//   print_preview_extractor_->CreatePrintPreview();
// #endif
// }

void AIChatUIPageHandler::OnVisibilityChanged(content::Visibility visibility) {
  // TODO(petemill): perhaps the ConversationHandler can check to see if
  // anything is bound to determine ConversationActive, instead of using
  // OnVisiblityChanged.

  // WebUI visibility changed (not target tab)
  // if (!active_chat_tab_helper_) {
  //   return;
  // }
  // bool is_visible = (visibility == content::Visibility::VISIBLE) ? true :
  // false; active_chat_tab_helper_->OnConversationActiveChanged(is_visible);
}

void AIChatUIPageHandler::CloseUI() {
#if !BUILDFLAG(IS_ANDROID)
  ai_chat::ClosePanel(web_contents());
#else
  ai_chat::CloseActivity(web_contents());
#endif
}

void AIChatUIPageHandler::SetChatUI(
    mojo::PendingRemote<mojom::ChatUI> chat_ui) {
  chat_ui_.Bind(std::move(chat_ui));
}

void AIChatUIPageHandler::BindRelatedConversation(
    mojo::PendingReceiver<mojom::ConversationHandler> receiver,
    mojo::PendingRemote<mojom::ConversationUI> conversation_ui_handler) {
  // TODO(petemill): This function and the service's BindConversation should
  // be the only things that actually create conversation instances.

  if (!active_chat_tab_helper_) {
    // No initial conversation for standalone page
    return;
  }

  ConversationHandler* conversation =
      AIChatServiceFactory::GetForBrowserContext(profile_)
          ->GetOrCreateConversationHandlerForPageContent(
              active_chat_tab_helper_->GetContentId(),
              active_chat_tab_helper_->GetWeakPtr());

  conversation->Bind(std::move(receiver), std::move(conversation_ui_handler));
}

void AIChatUIPageHandler::NewConversation(
    mojo::PendingReceiver<mojom::ConversationHandler> receiver,
    mojo::PendingRemote<mojom::ConversationUI> conversation_ui_handler) {
  ConversationHandler* conversation;
  if (active_chat_tab_helper_) {
    conversation = AIChatServiceFactory::GetForBrowserContext(profile_)
                       ->CreateConversationHandlerForPageContent(
                           active_chat_tab_helper_->GetContentId(),
                           active_chat_tab_helper_->GetWeakPtr());
  } else {
    conversation = AIChatServiceFactory::GetForBrowserContext(profile_)
                       ->CreateConversation();
  }

  conversation->Bind(std::move(receiver), std::move(conversation_ui_handler));
}

}  // namespace ai_chat
