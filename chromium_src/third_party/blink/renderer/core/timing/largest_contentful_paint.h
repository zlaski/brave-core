/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_LARGEST_CONTENTFUL_PAINT_H_

#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_LARGEST_CONTENTFUL_PAINT_H_

#define loadTime                 \
  dummy() const { return 0; }    \
  void RoundOffTimes() override; \
  DOMHighResTimeStamp loadTime

#include "src/third_party/blink/renderer/core/timing/largest_contentful_paint.h"

#undef loadTime

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_LARGEST_CONTENTFUL_PAINT_H_
