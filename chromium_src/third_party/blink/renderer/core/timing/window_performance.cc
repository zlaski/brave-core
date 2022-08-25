/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/timing/window_performance.h"

#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/timing/performance.h"
#include "third_party/blink/renderer/core/timing/performance_entry.h"

#define MonotonicTimeToDOMHighResTimeStamp(...) \
  RoundDOMHighResTimeStamp(                     \
      GetExecutionContext(),                    \
      Performance::MonotonicTimeToDOMHighResTimeStamp(__VA_ARGS__))

#include "src/third_party/blink/renderer/core/timing/window_performance.cc"

#undef MonotonicTimeToDOMHighResTimeStamp
