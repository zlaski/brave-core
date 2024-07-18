// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/components/ai_chat/core/browser/ai_chat_service.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/uuid.h"
#include "brave/components/ai_chat/core/browser/ai_chat_credential_manager.h"
#include "brave/components/ai_chat/core/browser/conversation_handler.h"
#include "brave/components/ai_chat/core/browser/model_service.h"
#include "brave/components/ai_chat/core/browser/utils.h"
#include "brave/components/ai_chat/core/common/mojom/ai_chat.mojom-forward.h"

namespace ai_chat {

AIChatService::AIChatService(
    ModelService* model_service,
    std::unique_ptr<AIChatCredentialManager> ai_chat_credential_manager,
    PrefService* profile_prefs,
    AIChatMetrics* ai_chat_metrics,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    std::string_view channel_string)
    : model_service_(model_service),
      profile_prefs_(profile_prefs),
      ai_chat_metrics_(ai_chat_metrics),
      url_loader_factory_(url_loader_factory),
      credential_manager_(std::move(ai_chat_credential_manager)) {}

AIChatService::~AIChatService() {}

mojo::PendingRemote<mojom::Service> AIChatService::MakeRemote() {
  mojo::PendingRemote<mojom::Service> remote;
  receivers_.Add(this, remote.InitWithNewPipeAndPassReceiver());
  return remote;
}

void AIChatService::Bind(mojo::PendingReceiver<mojom::Service> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void AIChatService::Shutdown() {
  // Disconnect remotes
  receivers_.ClearWithReason(0, "Shutting down");
}

ConversationHandler* AIChatService::CreateConversation() {
  base::Uuid uuid = base::Uuid::GenerateRandomV4();
  std::string conversation_uuid = uuid.AsLowercaseString();

  // Create the conversation metadata
  {
    mojom::ConversationPtr conversation =
        mojom::Conversation::New(conversation_uuid, "", false);
    conversations_.insert_or_assign(conversation_uuid, std::move(conversation));
  }
  mojom::Conversation* conversation =
      conversations_.at(conversation_uuid).get();
  // Create the ConversationHandler. We don't persist it until it has data.
  std::unique_ptr<ConversationHandler> conversation_handler =
      std::make_unique<ConversationHandler>(conversation);
  conversation_observations_.AddObservation(conversation_handler.get());

  // Own it
  conversation_handlers_.insert_or_assign(conversation_uuid,
                                          std::move(conversation_handler));

  // TODO(petemill): Is this neccessary? This conversation won't be
  // considered visible until it has entries.
  OnConversationListChanged();

  // Return the raw pointer
  return GetConversation(conversation_uuid);
}

ConversationHandler* AIChatService::GetConversation(
    const std::string conversation_uuid) {
  auto conversation_handler_it = conversation_handlers_.find(conversation_uuid);
  if (conversation_handler_it == conversation_handlers_.end()) {
    return nullptr;
  }
  return conversation_handler_it->second.get();
}

ConversationHandler*
AIChatService::GetOrCreateConversationHandlerForPageContent(
    int associated_content_id,
    base::WeakPtr<ConversationHandler::AssociatedContentDelegate>
        associated_content) {
  LOG(ERROR) << __func__ << " " << associated_content_id;
  auto conversation_uuid_it =
      content_conversations_.find(associated_content_id);
  if (conversation_uuid_it != content_conversations_.end()) {
    auto conversation_uuid = conversation_uuid_it->second;
    LOG(ERROR) << __func__ << " got conversation: " << conversation_uuid;
    // Load from memory or database
    auto* item = GetConversation(conversation_uuid);
    if (item) {
      item->SetAssociatedContentDelegate(associated_content);
      return item;
    }
  }
  // New conversation needed
  auto* conversation_handler = CreateConversation();
  // Provide the content delegate
  conversation_handler->SetAssociatedContentDelegate(associated_content);
  // Record that this is the latest conversation for this content
  content_conversations_.insert_or_assign(
      associated_content_id, conversation_handler->get_conversation_uuid());

  return conversation_handler;
}

void AIChatService::MarkAgreementAccepted() {
  SetUserOptedIn(profile_prefs_, true);
}

void AIChatService::OnConversationEntriesChanged(
    ConversationHandler* handler,
    std::vector<mojom::ConversationTurnPtr> entries) {
  LOG(ERROR) << __func__ << " " << handler->get_conversation_uuid() << " "
             << entries.size();
  auto conversation_it = conversations_.find(handler->get_conversation_uuid());
  CHECK(conversation_it != conversations_.end());
  auto& conversation = conversation_it->second;
  if (!conversation->has_content && !entries.empty()) {
    // This conversation is now visible
    conversation->has_content = true;
    OnConversationListChanged();
  }
  // TODO(petemill): Persist the entries, but consider receiving finer grained
  // entry update events.
}

void AIChatService::GetVisibleConversations(
    GetVisibleConversationsCallback callback) {
  std::vector<mojom::ConversationPtr> conversations;
  for (const auto& conversation : FilterVisibleConversations()) {
    conversations.push_back(conversation->Clone());
  }
  std::move(callback).Run(std::move(conversations));
}

void AIChatService::BindConversation(
    const std::string& uuid,
    mojo::PendingReceiver<mojom::ConversationHandler> receiver,
    mojo::PendingRemote<mojom::ChatUIPage> conversation_ui_handler) {
  auto* conversation = GetConversation(uuid);
  CHECK(conversation);
  conversation->Bind(std::move(receiver), std::move(conversation_ui_handler));
}

void AIChatService::BindUI(mojo::PendingRemote<mojom::ChatUI> ui) {
  ui_handlers_.Add(std::move(ui));
}

std::vector<mojom::Conversation*> AIChatService::FilterVisibleConversations() {
  std::vector<mojom::Conversation*> conversations;
  for (const auto& kv : conversations_) {
    auto& conversation = kv.second;
    // Conversations are only visible if they have content
    if (!conversation->has_content) {
      continue;
    }
    conversations.push_back(conversation.get());
  }
  return conversations;
}

void AIChatService::OnConversationListChanged() {
  auto conversations = FilterVisibleConversations();
  for (auto& client : ui_handlers_) {
    std::vector<mojom::ConversationPtr> client_conversations;
    for (const auto& conversation : conversations) {
      client_conversations.push_back(conversation->Clone());
    }
    client->OnConversationListChanged(std::move(client_conversations));
  }
}

}  // namespace ai_chat
