/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_page_graph/common/buildflags.h"
#include "build/build_config.h"

#if BUILDFLAG(IS_ANDROID)
#define BRAVE_RENDER_FRAME_IMPL_SHOULD_USE_USER_AGENT_OVERRIDE
#else
// clang-format off
#define BRAVE_RENDER_FRAME_IMPL_SHOULD_USE_USER_AGENT_OVERRIDE \
    return true;                                               \
  if (false)                                                   \
// clang-format on
#endif

#include "src/content/renderer/render_frame_impl.cc"

#undef BRAVE_RENDER_FRAME_IMPL_SHOULD_USE_USER_AGENT_OVERRIDE

namespace content {

void RenderFrameImpl::RegisterResourceBlockAd(
    const GURL& url,
    const std::string& rule) {
#if BUILDFLAG(ENABLE_BRAVE_PAGE_GRAPH)
  if (auto* page_graph = GetWebFrame()->GetWebPageGraph()) {
    page_graph->RegisterResourceBlockAd(url, rule);
  }
#endif  // BUILDFLAG(ENABLE_BRAVE_PAGE_GRAPH)
}

void RenderFrameImpl::RegisterResourceBlockTracker(
    const GURL& url,
    const std::string& host) {
#if BUILDFLAG(ENABLE_BRAVE_PAGE_GRAPH)
  if (auto* page_graph = GetWebFrame()->GetWebPageGraph()) {
    page_graph->RegisterResourceBlockTracker(url, host);
  }
#endif  // BUILDFLAG(ENABLE_BRAVE_PAGE_GRAPH)
}

}  // namespace content
