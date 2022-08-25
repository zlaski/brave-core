/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/timing/performance.h"

#define MonotonicTimeToDOMHighResTimeStamp(...) \
  MonotonicTimeToDOMHighResTimeStamp(           \
      __VA_ARGS__, performance_navigation_timing.GetExecutionContext())

#include "src/third_party/blink/renderer/core/timing/performance_navigation_timing_activation_start.cc"

#undef MonotonicTimeToDOMHighResTimeStamp
