/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_RESOURCE_TIMING_H_

#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_RESOURCE_TIMING_H_

// Adding a "context" member here. This member has no trailing underscore
// because we are taking advantage of a name collision with one of the
// PerformanceResourceTiming constructor arguments, which has type
// ExecutionContext.
#define is_secure_transport_    \
  is_secure_transport_ = false; \
  const bool context

#include "src/third_party/blink/renderer/core/timing/performance_resource_timing.h"

#undef is_secure_transport_

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_RESOURCE_TIMING_H_
