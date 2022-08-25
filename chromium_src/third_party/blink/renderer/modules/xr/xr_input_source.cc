/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/execution_context/execution_context.h"

#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/modules/gamepad/gamepad.h"

#define UpdateFromDeviceState(...) \
  UpdateFromDeviceState(           \
      __VA_ARGS__, brave::AllowFingerprinting(window->GetExecutionContext()))

#include "src/third_party/blink/renderer/modules/xr/xr_input_source.cc"

#undef UpdateFromDeviceState
