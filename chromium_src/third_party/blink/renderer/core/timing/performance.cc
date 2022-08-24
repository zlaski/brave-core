/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "src/third_party/blink/renderer/core/timing/performance.cc"

namespace blink {

DOMHighResTimeStamp Performance::RoundDOMHighResTimeStamp(
    ExecutionContext* context,
    DOMHighResTimeStamp timeStamp) {
  return floor(timeStamp + 0.5);
}

}  // namespace blink
