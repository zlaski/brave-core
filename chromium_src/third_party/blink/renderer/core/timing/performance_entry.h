/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_ENTRY_H_

#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_ENTRY_H_

#define IsResource                 \
  dummy_ [[maybe_unused]] = false; \
  virtual void RoundOffTimes();    \
  bool isResource

// Remove constness of start_time_.
#define start_time_                 \
  dummy2_ [[maybe_unused]] = false; \
  double start_time_

#include "src/third_party/blink/renderer/core/timing/performance_entry.h"

#undef IsResource
#undef start_time_

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_ENTRY_H_
