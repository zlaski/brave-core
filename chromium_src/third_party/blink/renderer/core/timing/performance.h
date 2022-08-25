/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_H_

#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_H_

#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/timing/performance_resource_timing.h"

#define now()               \
  now_ChromiumImpl() const; \
  DOMHighResTimeStamp now()

#define cross_origin_isolated_capability_ \
  cross_origin_isolated_capability_;      \
  bool allow_fingerprinting_

#define ClampTimeResolution                                       \
  RoundDOMHighResTimeStamp(bool allow_fingerprinting,             \
                           DOMHighResTimeStamp time_stamp);       \
  static DOMHighResTimeStamp RoundDOMHighResTimeStamp(            \
      ExecutionContext* context, DOMHighResTimeStamp time_stamp); \
  static DOMHighResTimeStamp ClampTimeResolution

#include "src/third_party/blink/renderer/core/timing/performance.h"

#undef now
#undef cross_origin_isolated_capability_
#undef ClampTimeResolution

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_H_
