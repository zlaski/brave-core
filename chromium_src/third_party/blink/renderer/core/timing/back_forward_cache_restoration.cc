/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/timing/back_forward_cache_restoration.h"

#include "src/third_party/blink/renderer/core/timing/back_forward_cache_restoration.cc"

namespace blink {

void BackForwardCacheRestoration::RoundOffTimes() {
  page_show_event_start_ = round(page_show_event_start_);
  page_show_event_end_ = round(page_show_event_end_);
  PerformanceEntry::RoundOffTimes();
}

}  // namespace blink
