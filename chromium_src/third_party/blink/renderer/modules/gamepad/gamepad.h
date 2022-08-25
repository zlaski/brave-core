/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_MODULES_GAMEPAD_GAMEPAD_H_
#define CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_MODULES_GAMEPAD_GAMEPAD_H_

#define UpdateFromDeviceState(...)                 \
  UpdateFromDeviceState_ChromiumImpl(__VA_ARGS__); \
  void UpdateFromDeviceState(__VA_ARGS__, bool allow_fingerprinting)

#include "src/third_party/blink/renderer/modules/gamepad/gamepad.h"

#undef UpdateFromDeviceState

#endif  // CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_MODULES_GAMEPAD_GAMEPAD_H_
