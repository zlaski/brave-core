/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_LOGGING_H_
#define BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_LOGGING_H_

#include "base/logging.h"

// Variant of LOG_ASSERT that tags the message with a *PageGraph* marker
#define PG_LOG_ASSERT(condition)                    \
  LOG_IF(FATAL, !(ANALYZER_ASSUME_TRUE(condition))) \
      << "*PageGraph* Assert failed: " #condition ". "

#endif  // BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_LOGGING_H_
