/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/timing/performance.h"

#define cross_origin_isolated_capability_(...)    \
  cross_origin_isolated_capability_(__VA_ARGS__), \
      allow_fingerprinting_(brave::AllowFingerprinting(context))

#define MonotonicTimeToDOMHighResTimeStamp \
  MonotonicTimeToDOMHighResTimeStamp_ChromiumImpl

#include "src/third_party/blink/renderer/core/timing/performance.cc"

#undef cross_origin_isolated_capability_
#undef MonotonicTimeToDOMHighResTimeStamp

#include "third_party/blink/renderer/core/execution_context/execution_context.h"

namespace blink {

// static
DOMHighResTimeStamp Performance::RoundDOMHighResTimeStamp(
    bool allow_fingerprinting,
    DOMHighResTimeStamp time_stamp) {
  //  printf("allowFingerprinting: %d\n", allowFingerprinting);
  return allow_fingerprinting ? time_stamp : floor(time_stamp + 0.5);
}

// static
DOMHighResTimeStamp Performance::RoundDOMHighResTimeStamp(
    ExecutionContext* context,
    DOMHighResTimeStamp time_stamp) {
  return RoundDOMHighResTimeStamp(brave::AllowFingerprinting(context),
                                  time_stamp);
}

// static
DOMHighResTimeStamp Performance::MonotonicTimeToDOMHighResTimeStamp(
    base::TimeTicks time_origin,
    base::TimeTicks monotonic_time,
    bool allow_negative_value,
    bool cross_origin_isolated_capability) {
  return MonotonicTimeToDOMHighResTimeStamp_ChromiumImpl(
      time_origin, monotonic_time, allow_negative_value,
      cross_origin_isolated_capability);
}

// static
DOMHighResTimeStamp Performance::MonotonicTimeToDOMHighResTimeStamp(
  base::TimeTicks time_origin,
  base::TimeTicks monotonic_time,
  bool allow_negative_value,
  bool cross_origin_isolated_capability,
  bool allow_fingerprinting) {
return RoundDOMHighResTimeStamp(allow_fingerprinting,
                         MonotonicTimeToDOMHighResTimeStamp_ChromiumImpl(time_origin,
monotonic_time, allow_negative_value, cross_origin_isolated_capability));
}

// static
DOMHighResTimeStamp Performance::MonotonicTimeToDOMHighResTimeStamp(
  base::TimeTicks time_origin,
  base::TimeTicks monotonic_time,
  bool allow_negative_value,
  bool cross_origin_isolated_capability,
  ExecutionContext* context) {
return RoundDOMHighResTimeStamp(context,
                         MonotonicTimeToDOMHighResTimeStamp_ChromiumImpl(time_origin,
monotonic_time, allow_negative_value, cross_origin_isolated_capability));
}

DOMHighResTimeStamp Performance::MonotonicTimeToDOMHighResTimeStamp(
    base::TimeTicks monotonic_time) const {
  return RoundDOMHighResTimeStamp(
      allow_fingerprinting_,
      MonotonicTimeToDOMHighResTimeStamp_ChromiumImpl(monotonic_time));
}

}  // namespace blink
