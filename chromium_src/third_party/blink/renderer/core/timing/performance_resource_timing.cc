/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/timing/performance.h"
#include "third_party/blink/renderer/core/timing/performance_entry.h"
#include "third_party/blink/renderer/core/timing/performance_resource_timing.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"

#define is_secure_transport_(...) \
  is_secure_transport_(__VA_ARGS__),             \
  context(brave::AllowFingerprinting(context))

#define MonotonicTimeToDOMHighResTimeStamp(... ) \
  RoundDOMHighResTimeStamp(context,                  \
    Performance::MonotonicTimeToDOMHighResTimeStamp(__VA_ARGS__))

#include "src/third_party/blink/renderer/core/timing/performance_resource_timing.cc"

#undef MonotonicTimeToDOMHighResTimeStamp

#undef is_secure_transport_
