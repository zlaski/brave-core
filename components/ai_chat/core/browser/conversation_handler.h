// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_AI_CHAT_CORE_BROWSER_CONVERSATION_HANDLER_H_
#define BRAVE_COMPONENTS_AI_CHAT_CORE_BROWSER_CONVERSATION_HANDLER_H_

#include <string>
#include <vector>

#include "base/functional/callback_forward.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "brave/components/ai_chat/core/browser/conversation_observer.h"
#include "brave/components/ai_chat/core/common/mojom/ai_chat.mojom-forward.h"
#include "brave/components/ai_chat/core/common/mojom/ai_chat.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "url/gurl.h"

namespace ai_chat {

class ConversationDriver;

class ConversationHandler : public mojom::ConversationHandler,
                            public ConversationObserver {
 public:
  // |invalidation_token| is an optional parameter that will be passed back on
  // the next call to |GetPageContent| so that the implementer may determine if
  // the page content is static or if it needs to be fetched again. Most page
  // content should be fetched again, but some pages are known to be static
  // during their lifetime and may have expensive content fetching, e.g. videos
  // with transcripts fetched over the network.
  using GetPageContentCallback = base::OnceCallback<
      void(std::string content, bool is_video, std::string invalidation_token)>;
  // Supplements a conversation with active page content
  class AssociatedContentDelegate {
   public:
    // Unique ID for the content. For browser Tab content, this should be
    // a navigation ID that's re-used during back navigations.
    virtual int GetContentId() const = 0;
    // Get metadata about the current page
    virtual GURL GetURL() const = 0;
    virtual std::u16string GetTitle() const = 0;

    // Implementer should fetch content from the "page" associated with this
    // conversation.
    // |is_video| lets the conversation know that the content is focused on
    // video content so that various UI language can be adapted.
    // |invalidation_token| is an optional parameter received in a prior
    // callback response of this function against the same page. See
    // GetPageContentCallback for explanation.
    virtual void GetContent(GetPageContentCallback callback,
                            std::string_view invalidation_token) = 0;

    virtual void GetContentPrintPreviewFallback(
        GetPageContentCallback callback) = 0;
  };

  class Observer : public base::CheckedObserver {
   public:
    ~Observer() override {}

    virtual void OnConversationEntriesChanged(
        ConversationHandler* handler,
        std::vector<mojom::ConversationTurnPtr> entries) {}
  };

  explicit ConversationHandler(const mojom::Conversation* conversation);
  ~ConversationHandler() override;

  void Bind(mojo::PendingReceiver<mojom::ConversationHandler> receiver,
            mojo::PendingRemote<mojom::ChatUIPage> conversation_ui_handler);

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // Called when the associated content is destroyed or navigated away. If
  // it's a navigation, the AssociatedContentDelegate will set itself to a new
  // ConversationHandler.
  void OnAssociatedContentDestroyed();

  // This can be called multiple times, e.g. when the user navigates back to
  // content, this conversation can be reunited with the delegate.
  void SetAssociatedContentDelegate(
      base::WeakPtr<AssociatedContentDelegate> delegate);

  // mojom::ConversationHandler
  void GetConversationHistory(GetConversationHistoryCallback callback) override;

  base::WeakPtr<ConversationHandler> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  std::string get_conversation_uuid() const { return metadata_->uuid; }

  // TODO(petemill): Temporary until these move to here
  // ConversationObserver
  void OnHistoryUpdate() override;

 private:
  base::WeakPtr<AssociatedContentDelegate> associated_content_delegate_;

  // UUID for associated content so that back navigations can reunite the
  // AssociatedContentDelegate with the ConversationHandler, e.g. navigation id.
  std::string associated_content_uuid_;
  // Data store UUID for conversation
  raw_ptr<const mojom::Conversation> metadata_;

  // Temporary
  base::ScopedObservation<ConversationDriver, ConversationObserver>
      conversation_driver_observation_{this};

  base::ObserverList<Observer> observers_;
  mojo::ReceiverSet<mojom::ConversationHandler> receivers_;
  // TODO(petemill): Rename to ConversationUIHandler
  mojo::RemoteSet<mojom::ChatUIPage> conversation_ui_handlers_;

  base::WeakPtrFactory<ConversationHandler> weak_ptr_factory_{this};
};

}  // namespace ai_chat

#endif  // BRAVE_COMPONENTS_AI_CHAT_CORE_BROWSER_CONVERSATION_HANDLER_H_
