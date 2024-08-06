// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_UI_AI_CHAT_PRINT_PREVIEW_EXTRACTOR_H_
#define BRAVE_BROWSER_UI_AI_CHAT_PRINT_PREVIEW_EXTRACTOR_H_

#include "brave/components/ai_chat/content/browser/ai_chat_tab_helper.h"
#include "printing/buildflags/buildflags.h"

static_assert(BUILDFLAG(ENABLE_PRINT_PREVIEW));

namespace content {
class WebContents;
}  // namespace content

namespace ai_chat {

void ExtractPrintPreviewContent(
    content::WebContents* web_contents,
    bool is_pdf,
    AIChatTabHelper::ExtractPrintPreviewCallback callback);

}  // namespace ai_chat

#endif  // BRAVE_BROWSER_UI_AI_CHAT_PRINT_PREVIEW_EXTRACTOR_H_
