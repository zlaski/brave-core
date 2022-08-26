/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_OBSERVER_H_

#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_OBSERVER_H_

#define takeRecords()                                                 \
  takeRecords();                                                      \
  virtual PerformanceObserver* RoundOffTimes(PerformanceEntry& entry, \
                                             bool allow_fingerprinting)

#include "src/third_party/blink/renderer/core/timing/performance_observer.h"

#undef takeRecords

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_PERFORMANCE_OBSERVER_H_
