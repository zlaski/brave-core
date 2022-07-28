/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/frame/local_frame.h"

#include "brave/third_party/blink/renderer/core/brave_page_graph/page_graph.h"

#define BRAVE_LOCAL_FRAME_CONSTRUCTOR PageGraph::ProvideTo(*this);

#include "src/third_party/blink/renderer/core/frame/local_frame.cc"

#undef BRAVE_LOCAL_FRAME_CONSTRUCTOR
