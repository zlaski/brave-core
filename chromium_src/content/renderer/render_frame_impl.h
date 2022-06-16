/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_CONTENT_RENDERER_RENDER_FRAME_IMPL_H_
#define BRAVE_CHROMIUM_SRC_CONTENT_RENDERER_RENDER_FRAME_IMPL_H_

#include "content/common/frame.mojom.h"

#define SetWantErrorMessageStackTrace                                         \
  NotUsed();                                                                  \
  void RegisterResourceBlockAd(const GURL& url, const std::string& rule)      \
      override;                                                               \
  void RegisterResourceBlockTracker(const GURL& url, const std::string& host) \
      override;                                                               \
  void SetWantErrorMessageStackTrace

#include "src/content/renderer/render_frame_impl.h"

#undef SetWantErrorMessageStackTrace

#endif  // BRAVE_CHROMIUM_SRC_CONTENT_RENDERER_RENDER_FRAME_IMPL_H_
