/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_page_graph/common/buildflags.h"

#define BRAVE_COMPILER_GET_FUNCTION_FROM_EVAL                          \
  IF_BUILDFLAG(ENABLE_BRAVE_PAGE_GRAPH, {                              \
    auto* page_graph_backend = isolate->GetPageGraphBackend();         \
    if (V8_UNLIKELY(page_graph_backend)) {                             \
      Object maybe_script = result->shared().script();                 \
      if (maybe_script.IsScript()) {                                   \
        const int parent_script_id =                                   \
            isolate->GetExecutingScript(false).script_id;              \
        Script script = Script::cast(maybe_script);                    \
        page_graph_backend->eval_script_compiled_cb(                   \
            reinterpret_cast<v8::Isolate*>(isolate), parent_script_id, \
            script.id(), v8::Utils::ToLocal(source));                  \
      }                                                                \
    }                                                                  \
  })

#include "src/v8/src/codegen/compiler.cc"

#undef BRAVE_COMPILER_GET_FUNCTION_FROM_EVAL
