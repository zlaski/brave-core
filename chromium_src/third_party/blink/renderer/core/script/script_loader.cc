/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_page_graph/common/buildflags.h"
#include "third_party/blink/renderer/core/probe/core_probes.h"

#define BRAVE_SCRIPT_LOADER_PREPARE_SCRIPT                           \
  IF_BUILDFLAG(ENABLE_BRAVE_PAGE_GRAPH, {                            \
    if (CoreProbeSink::HasAgentsGlobal(CoreProbeSink::kPageGraph)) { \
      options.SetDOMNodeId(element_->GetDOMNodeId());                \
    }                                                                \
  })

#define BRAVE_SCRIPT_LOADER_PREPARE_SCRIPT_IF_HAS_SRC_ATTRIBUTE          \
  IF_BUILDFLAG(ENABLE_BRAVE_PAGE_GRAPH, {                                \
    probe::RegisterPageGraphElmForRemoteScript(                          \
        element_->GetExecutionContext(), element_->GetDOMNodeId(), url); \
  })

#define BRAVE_SCRIPT_LOADER_PREPARE_SCRIPT_IF_NO_SRC_ATTRIBUTE                 \
  IF_BUILDFLAG(ENABLE_BRAVE_PAGE_GRAPH, {                                      \
    probe::RegisterPageGraphElmForLocalScript(element_->GetExecutionContext(), \
                                              element_->GetDOMNodeId(),        \
                                              source_text);                    \
  })

#include "src/third_party/blink/renderer/core/script/script_loader.cc"

#undef BRAVE_SCRIPT_LOADER_PREPARE_SCRIPT_IF_NO_SRC_ATTRIBUTE
#undef BRAVE_SCRIPT_LOADER_PREPARE_SCRIPT_IF_HAS_SRC_ATTRIBUTE
#undef BRAVE_SCRIPT_LOADER_PREPARE_SCRIPT
