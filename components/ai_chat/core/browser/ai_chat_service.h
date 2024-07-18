// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_AI_CHAT_CORE_BROWSER_AI_CHAT_SERVICE_H_
#define BRAVE_COMPONENTS_AI_CHAT_CORE_BROWSER_AI_CHAT_SERVICE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_multi_source_observation.h"
#include "brave/components/ai_chat/core/browser/ai_chat_credential_manager.h"
#include "brave/components/ai_chat/core/browser/ai_chat_metrics.h"
#include "brave/components/ai_chat/core/browser/conversation_handler.h"
#include "brave/components/ai_chat/core/browser/model_service.h"
#include "brave/components/ai_chat/core/common/mojom/ai_chat.mojom.h"
#include "brave/components/skus/common/skus_sdk.mojom.h"
#include "components/keyed_service/core/keyed_service.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace ai_chat {

class AIChatService : public KeyedService,
                      public mojom::Service,
                      public ConversationHandler::Observer {
 public:
  using SkusServiceGetter =
      base::RepeatingCallback<mojo::PendingRemote<skus::mojom::SkusService>()>;

  AIChatService(
      ModelService* model_service,
      std::unique_ptr<AIChatCredentialManager> ai_chat_credential_manager,
      PrefService* profile_prefs,
      AIChatMetrics* ai_chat_metrics,
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
      std::string_view channel_string);

  ~AIChatService() override;
  AIChatService(const AIChatService&) = delete;
  AIChatService& operator=(const AIChatService&) = delete;

  mojo::PendingRemote<mojom::Service> MakeRemote();
  void Bind(mojo::PendingReceiver<mojom::Service> receiver);

  // KeyedService
  void Shutdown() override;

  // ConversationHandler::Observer
  void OnConversationEntriesChanged(
      ConversationHandler* handler,
      std::vector<mojom::ConversationTurnPtr> entries) override;

  // Adds new conversation and returns the handler
  ConversationHandler* CreateConversation();

  ConversationHandler* GetConversation(const std::string uuid);

  // Creates and owns a ConversationHandler if one hasn't been made for the
  // associated_content_id yet. associated_content_id should not be stored. It
  // is an ephemeral identifier for active browser content.
  // TODO(petemill): Maybe this should be a raw_ptr since we won't destroy it?
  ConversationHandler* GetOrCreateConversationHandlerForPageContent(
      int associated_content_id,
      base::WeakPtr<ConversationHandler::AssociatedContentDelegate>
          associated_content);

  // mojom::Service
  void MarkAgreementAccepted() override;
  void GetVisibleConversations(
      GetVisibleConversationsCallback callback) override;
  void BindConversation(
      const std::string& uuid,
      mojo::PendingReceiver<mojom::ConversationHandler> receiver,
      mojo::PendingRemote<mojom::ChatUIPage> conversation_ui_handler) override;
  void BindUI(mojo::PendingRemote<mojom::ChatUI> ui) override;

 private:
  std::vector<mojom::Conversation*> FilterVisibleConversations();
  void OnConversationListChanged();

  raw_ptr<ModelService> model_service_;
  raw_ptr<PrefService> profile_prefs_;
  raw_ptr<AIChatMetrics> ai_chat_metrics_;
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;

  std::unique_ptr<AIChatCredentialManager> credential_manager_;

  // All conversation metadata. Mainly just titles and uuids. Key is uuid
  std::map<std::string, mojom::ConversationPtr> conversations_;

  // TODO(petemill): Only keep ConversationHandlers around that are being
  // actively used.
  std::map<std::string, std::unique_ptr<ConversationHandler>>
      conversation_handlers_;

  // Map associated content id (a.k.a navigation id) to conversation uuid. This
  // acts as a cache for back-navigation to find the most recent conversation
  // for that navigation. This should be periodically cleaned up by removing any
  // keys where the ConversationHandler has had a destroyed
  // associated_content_delegate_ for some time.
  std::map<int, std::string> content_conversations_;

  base::ScopedMultiSourceObservation<ConversationHandler,
                                     ConversationHandler::Observer>
      conversation_observations_{this};
  mojo::ReceiverSet<mojom::Service> receivers_;
  mojo::RemoteSet<mojom::ChatUI> ui_handlers_;

  base::WeakPtrFactory<AIChatService> weak_ptr_factory_{this};
};

}  // namespace ai_chat

#endif  // BRAVE_COMPONENTS_AI_CHAT_CORE_BROWSER_AI_CHAT_SERVICE_H_
