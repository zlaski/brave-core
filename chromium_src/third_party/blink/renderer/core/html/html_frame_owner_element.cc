/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/probe/core_probes.h"

#define BRAVE_HTML_FRAME_OWNER_ELEMENT_SET_UP_CONTENT_FRAME \
  probe::RegisterPageGraphRemoteFrameCreated(GetExecutionContext(), this);

#include "src/third_party/blink/renderer/core/html/html_frame_owner_element.cc"

#undef BRAVE_HTML_FRAME_OWNER_ELEMENT_SET_UP_CONTENT_FRAME
