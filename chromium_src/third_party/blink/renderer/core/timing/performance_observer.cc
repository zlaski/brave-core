/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/timing/performance_observer.h"
#include "third_party/blink/renderer/core/timing/performance_entry.h"

#include "src/third_party/blink/renderer/core/timing/performance_observer.cc"

namespace blink {

PerformanceObserver* PerformanceObserver::RoundOffTimes(
    PerformanceEntry& entry,
    bool allow_fingerprinting) {
  if (!allow_fingerprinting) {
    entry.RoundOffTimes();
  }
  return this;
}

}  // namespace blink
