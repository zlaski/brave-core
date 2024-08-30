/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_AI_CHAT_CONTENT_BROWSER_AI_CHAT_TAB_HELPER_H_
#define BRAVE_COMPONENTS_AI_CHAT_CONTENT_BROWSER_AI_CHAT_TAB_HELPER_H_

#include <memory>
#include <optional>
#include <string>

#include "base/functional/callback_forward.h"
#include "base/memory/raw_ptr.h"
#include "brave/components/ai_chat/core/browser/associated_content_driver.h"
#include "brave/components/ai_chat/core/browser/conversation_handler.h"
#include "brave/components/ai_chat/core/common/mojom/page_content_extractor.mojom.h"
#include "components/favicon/core/favicon_driver_observer.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"

namespace content {
class ScopedAccessibilityMode;
}

class AIChatUIBrowserTest;
namespace ai_chat {
class AIChatMetrics;

// Provides context to an AI Chat conversation in the form of the Tab's content
class AIChatTabHelper : public content::WebContentsObserver,
                        public content::WebContentsUserData<AIChatTabHelper>,
                        public mojom::PageContentExtractorHost,
                        public favicon::FaviconDriverObserver,
                        public AssociatedContentDriver {
 public:
  static void BindPageContentExtractorHost(
      content::RenderFrameHost* rfh,
      mojo::PendingAssociatedReceiver<mojom::PageContentExtractorHost>
          receiver);

  using ExtractPrintPreviewCallback =
      base::OnceCallback<void(std::string text)>;
  using ExtractPrintPreviewContentFunction = base::RepeatingCallback<void(
      content::WebContents* web_contents,
      bool is_pdf,
      AIChatTabHelper::ExtractPrintPreviewCallback callback)>;

  AIChatTabHelper(const AIChatTabHelper&) = delete;
  AIChatTabHelper& operator=(const AIChatTabHelper&) = delete;
  ~AIChatTabHelper() override;

  void SetOnPDFA11yInfoLoadedCallbackForTesting(base::OnceClosure cb);

  // mojom::PageContentExtractorHost
  void OnInterceptedPageContentChanged() override;

 private:
  friend class content::WebContentsUserData<AIChatTabHelper>;
  friend class ::AIChatUIBrowserTest;

  // To observe PDF InnerWebContents for "Finished loading PDF" event which
  // means PDF content has been loaded to an accessibility tree.
  class PDFA11yInfoLoadObserver : public content::WebContentsObserver {
   public:
    ~PDFA11yInfoLoadObserver() override;
    explicit PDFA11yInfoLoadObserver(content::WebContents* web_contents,
                                     AIChatTabHelper* helper);

   private:
    void AccessibilityEventReceived(
        const ui::AXUpdatesAndEvents& details) override;
    raw_ptr<AIChatTabHelper> helper_;
  };

  AIChatTabHelper(content::WebContents* web_contents,
                  ExtractPrintPreviewContentFunction
                      extract_print_preview_content_function);

  void OnPDFA11yInfoLoaded();

  // content::WebContentsObserver
  void WebContentsDestroyed() override;
  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) override;
  void TitleWasSet(content::NavigationEntry* entry) override;
  void InnerWebContentsAttached(content::WebContents* inner_web_contents,
                                content::RenderFrameHost* render_frame_host,
                                bool is_full_page) override;
  void OnWebContentsFocused(
      content::RenderWidgetHost* render_widget_host) override;
  void OnWebContentsLostFocus(
      content::RenderWidgetHost* render_widget_host) override;

  // favicon::FaviconDriverObserver
  void OnFaviconUpdated(favicon::FaviconDriver* favicon_driver,
                        NotificationIconType notification_icon_type,
                        const GURL& icon_url,
                        bool icon_url_changed,
                        const gfx::Image& image) override;

  // ai_chat::AssociatedContentDriver
  GURL GetPageURL() const override;
  void GetPageContent(ConversationHandler::GetPageContentCallback callback,
                      std::string_view invalidation_token) override;
  std::u16string GetPageTitle() const override;

  void GetSearchSummarizerKey(
      mojom::PageContentExtractor::GetSearchSummarizerKeyCallback callback)
      override;

  void OnFetchPageContentComplete(
      ConversationHandler::GetPageContentCallback callback,
      std::string content,
      bool is_video,
      std::string invalidation_token);

  void OnExtractPrintPreviewContentComplete(
      ConversationHandler::GetPageContentCallback callback,
      std::string content);

  void BindPageContentExtractorReceiver(
      mojo::PendingAssociatedReceiver<mojom::PageContentExtractorHost>
          receiver);

  // Traverse through a11y tree to check existence of status node.
  void CheckPDFA11yTree();

  raw_ptr<AIChatMetrics> ai_chat_metrics_;

  bool is_same_document_navigation_ = false;
  int pending_navigation_id_;
  bool is_pdf_a11y_info_loaded_ = false;
  uint8_t check_pdf_a11y_tree_attempts_ = 0;

  // TODO(petemill): Use signal to allow for multiple callbacks
  ConversationHandler::GetPageContentCallback
      pending_get_page_content_callback_;

  ExtractPrintPreviewContentFunction extract_print_preview_content_function_;
  std::unique_ptr<PDFA11yInfoLoadObserver> pdf_load_observer_;
  base::OnceClosure on_pdf_a11y_info_loaded_cb_;

  // A scoper only used for PDF viewing.
  std::unique_ptr<content::ScopedAccessibilityMode> scoped_accessibility_mode_;

  mojo::AssociatedReceiver<mojom::PageContentExtractorHost>
      page_content_extractor_receiver_{this};

  base::WeakPtrFactory<AIChatTabHelper> weak_ptr_factory_{this};
  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace ai_chat

#endif  // BRAVE_COMPONENTS_AI_CHAT_CONTENT_BROWSER_AI_CHAT_TAB_HELPER_H_
