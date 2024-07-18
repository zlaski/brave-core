// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_AI_CHAT_CORE_BROWSER_CONVERSATION_OBSERVER_H_
#define BRAVE_COMPONENTS_AI_CHAT_CORE_BROWSER_CONVERSATION_OBSERVER_H_

#include <string>
#include <vector>

#include "base/observer_list.h"
#include "brave/components/ai_chat/core/common/mojom/ai_chat.mojom.h"

namespace ai_chat {

// TODO(petemill): This is temporary to avoid circular dependency until
// ConversationDriver is fully replaced by ConversationHandler.
class ConversationObserver : public base::CheckedObserver {
 public:
  ~ConversationObserver() override {}

  virtual void OnHistoryUpdate() {}
  virtual void OnAPIRequestInProgress(bool in_progress) {}
  virtual void OnAPIResponseError(mojom::APIError error) {}
  virtual void OnModelDataChanged(
      const std::string& model_key,
      const std::vector<mojom::ModelPtr>& model_list) {}
  virtual void OnSuggestedQuestionsChanged(
      std::vector<std::string> questions,
      mojom::SuggestionGenerationStatus suggestion_generation_status) {}
  virtual void OnFaviconImageDataChanged() {}
  virtual void OnPageHasContent(mojom::SiteInfoPtr site_info) {}
  virtual void OnPrintPreviewRequested(bool is_pdf) {}
};

}  // namespace ai_chat

#endif  // BRAVE_COMPONENTS_AI_CHAT_CORE_BROWSER_CONVERSATION_OBSERVER_H_
