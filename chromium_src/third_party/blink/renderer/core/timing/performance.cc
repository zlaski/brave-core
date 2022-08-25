/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/timing/performance.h"

#define now() now_ChromiumImpl()

#define cross_origin_isolated_capability_(...)    \
  cross_origin_isolated_capability_(__VA_ARGS__), \
      allow_fingerprinting_(brave::AllowFingerprinting(context))

#include "src/third_party/blink/renderer/core/timing/performance.cc"

#undef now
#undef cross_origin_isolated_capability_

#include "third_party/blink/renderer/core/execution_context/execution_context.h"

namespace blink {

// static
DOMHighResTimeStamp Performance::RoundDOMHighResTimeStamp(
    bool allow_fingerprinting,
    DOMHighResTimeStamp time_stamp) {
  //  printf("allowFingerprinting: %d\n", allowFingerprinting);
  return allow_fingerprinting ? time_stamp : floor(timeStamp + 0.5);
}

// static
DOMHighResTimeStamp Performance::RoundDOMHighResTimeStamp(
    ExecutionContext* context,
    DOMHighResTimeStamp time_stamp) {
  return RoundDOMHighResTimeStamp(brave::AllowFingerprinting(context),
                                  time_stamp);
}

DOMHighResTimeStamp Performance::now() const {
  return RoundDOMHighResTimeStamp(allow_fingerprinting_, now_ChromiumImpl());
}

}  // namespace blink
