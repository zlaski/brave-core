/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_H_

#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_H_

#include "third_party/blink/renderer/core/execution_context/execution_context.h"

#define ClampTimeResolution                                \
  RoundDOMHighResTimeStamp(bool allowFingerprinting,       \
                           DOMHighResTimeStamp timeStamp); \
  static DOMHighResTimeStamp RoundDOMHighResTimeStamp( \
      ExecutionContext* context,      \
                           DOMHighResTimeStamp timeStamp); \
  static DOMHighResTimeStamp ClampTimeResolution

#include "src/third_party/blink/renderer/core/timing/performance.h"

#undef ClampTimeResolution

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_H_
