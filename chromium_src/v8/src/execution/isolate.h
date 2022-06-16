/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_V8_SRC_EXECUTION_ISOLATE_H_
#define BRAVE_CHROMIUM_SRC_V8_SRC_EXECUTION_ISOLATE_H_

#include "v8/include/v8-isolate.h"

#define SetRAILMode                                                       \
  NotUsed();                                                              \
  v8::Isolate::ExecutingScript GetExecutingScript(bool include_position); \
  std::vector<v8::Isolate::ExecutingScript> GetAllExecutingScripts();     \
  v8::Isolate::PageGraphBackend* GetPageGraphBackend() const {            \
    return page_graph_backend_;                                           \
  }                                                                       \
  void SetPageGraphBackend(                                               \
      v8::Isolate::PageGraphBackend* page_graph_backend) {                \
    page_graph_backend_ = page_graph_backend;                             \
  }                                                                       \
                                                                          \
  void SetRAILMode

#define rail_mutex_ \
  rail_mutex_;      \
  v8::Isolate::PageGraphBackend* page_graph_backend_ = nullptr

#include "src/v8/src/execution/isolate.h"

#undef rail_mutex_
#undef SetRAILMode

#endif  // BRAVE_CHROMIUM_SRC_V8_SRC_EXECUTION_ISOLATE_H_
