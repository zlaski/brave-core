// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "chrome/browser/ui/webui/chrome_web_ui_configs.h"

#include <memory>

#include "brave/components/ai_chat/core/common/buildflags/buildflags.h"

#if BUILDFLAG(ENABLE_AI_CHAT)
#include "brave/browser/ui/webui/ai_chat/ai_chat_ui.h"
#endif  // BUILDFLAG(ENABLE_AI_CHAT)

#define RegisterChromeWebUIConfigs RegisterChromeWebUIConfigs_ChromiumImpl

#include "src/chrome/browser/ui/webui/chrome_web_ui_configs.cc"

#undef RegisterChromeWebUIConfigs

void RegisterChromeWebUIConfigs() {
  RegisterChromeWebUIConfigs_ChromiumImpl();

#if BUILDFLAG(ENABLE_AI_CHAT)
  auto& map = content::WebUIConfigMap::GetInstance();
  map.AddWebUIConfig(std::make_unique<AIChatUIConfig>());
#endif
}
