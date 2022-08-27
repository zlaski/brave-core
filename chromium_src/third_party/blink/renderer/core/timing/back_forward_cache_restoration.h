/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_BACK_FORWARD_CACHE_RESTORATION_H_

#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_BACK_FORWARD_CACHE_RESTORATION_H_

#define pageShowEventEnd         \
  dummy() const { return 0; }    \
  void RoundOffTimes() override; \
  DOMHighResTimeStamp pageShowEventEnd

#include "src/third_party/blink/renderer/core/timing/back_forward_cache_restoration.h"

#undef pageShowEventEnd

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_BACK_FORWARD_CACHE_RESTORATION_H_
