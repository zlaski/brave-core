/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/probe/core_probes.h"

#define BRAVE_DYNAMIC_MODULE_RESOLVER_RESOLVE_DYNAMICALLY          \
  if (CoreProbeSink::HasAgentsGlobal(CoreProbeSink::kPageGraph)) { \
    options.SetDOMNodeId(referrer_info.GetDOMNodeId());            \
    options.SetParentScriptId(modulator_->GetScriptState()         \
                                  ->GetIsolate()                   \
                                  ->GetExecutingScript()           \
                                  .script_id);                     \
    probe::RegisterPageGraphModuleScriptForDescendant(             \
        ExecutionContext::From(modulator_->GetScriptState()),      \
        options.GetParentScriptId(), url);                         \
  }

#include "src/third_party/blink/renderer/core/script/dynamic_module_resolver.cc"

#undef BRAVE_DYNAMIC_MODULE_RESOLVER_RESOLVE_DYNAMICALLY
