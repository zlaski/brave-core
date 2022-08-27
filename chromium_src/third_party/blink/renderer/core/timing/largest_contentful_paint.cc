/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/timing/largest_contentful_paint.h"
#include "base/time/time.h"

#include "src/third_party/blink/renderer/core/timing/largest_contentful_paint.cc"

namespace blink {

void LargestContentfulPaint::RoundOffTimes() {
  const base::TimeDelta one_millisecond = base::Milliseconds(1);
  render_time_ = render_time_.RoundToMultiple(one_millisecond);
  load_time_ = load_time_.RoundToMultiple(one_millisecond);
  first_animated_frame_time_ =
      first_animated_frame_time_.RoundToMultiple(one_millisecond);
  PerformanceEntry::RoundOffTimes();
}

}  // namespace blink
