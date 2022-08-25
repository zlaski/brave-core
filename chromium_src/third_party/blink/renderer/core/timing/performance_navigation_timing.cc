/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/timing/performance_navigation_timing.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/timing/performance.h"
#include "third_party/blink/renderer/core/timing/performance_entry.h"
#include "third_party/blink/renderer/core/timing/performance_resource_timing.h"

#define resource_timing_info_(...)    \
  resource_timing_info_(__VA_ARGS__), \
      allow_fingerprinting_(brave::AllowFingerprinting(window))

#define MonotonicTimeToDOMHighResTimeStamp(...)                \
  Performance::MonotonicTimeToDOMHighResTimeStamp(__VA_ARGS__, \
                                                  allow_fingerprinting_)

#include "src/third_party/blink/renderer/core/timing/performance_navigation_timing.cc"

#undef MonotonicTimeToDOMHighResTimeStamp

#undef resource_timing_info
