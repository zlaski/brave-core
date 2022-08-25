/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/modules/gamepad/gamepad.h"

#define UpdateFromDeviceState UpdateFromDeviceState_ChromiumImpl

#include "src/third_party/blink/renderer/modules/gamepad/gamepad.cc"

#undef UpdateFromDeviceState

namespace blink {

void Gamepad::UpdateFromDeviceState(const device::Gamepad& device_gamepad,
                                    bool cross_origin_isolated_capability,
                                    bool allow_fingerprinting) {
  UpdateFromDeviceState_ChromiumImpl(device_gamepad,
                                     cross_origin_isolated_capability);
  if (!allow_fingerprinting) {
    timestamp_ = floor(timestamp_ + 0.5);
  }
}

}  // namespace blink
