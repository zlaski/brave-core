/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "src/v8/src/execution/isolate.cc"

namespace v8 {
namespace internal {

static std::vector<v8::Isolate::ExecutingScript>
GetExecutingScriptsImpl(Isolate* isolate, bool all, bool include_position) {
  std::vector<v8::Isolate::ExecutingScript> result;
  JavaScriptFrameIterator it(isolate);
  while (!it.done()) {
    JavaScriptFrame* frame = it.frame();
    std::vector<SharedFunctionInfo> functions;
    frame->GetFunctions(&functions);
    for (const auto& shared : functions) {
      Object maybe_script = shared.script();
      if (!maybe_script.IsScript()) {
        continue;
      }

      const int script_id = Script::cast(maybe_script).id();
      if (script_id <= 0) {
        continue;
      }

      int script_position = 0;
      if (include_position && !isolate->has_pending_exception()) {
        Handle<SharedFunctionInfo> shared_handle(shared, isolate);
        SharedFunctionInfo::EnsureSourcePositionsAvailable(isolate,
                                                           shared_handle);
        script_position = frame->position();
      }

      result.emplace_back(
          v8::Isolate::ExecutingScript{script_id, script_position});
      if (!all) {
        return result;
      }
    }
    it.Advance();
  }
  return result;
}

v8::Isolate::ExecutingScript Isolate::GetExecutingScript(
    bool include_position) {
  auto executing_scripts =
      GetExecutingScriptsImpl(this, false, include_position);
  if (!executing_scripts.empty()) {
    return std::move(executing_scripts[0]);
  }
  return v8::Isolate::ExecutingScript();
}

std::vector<v8::Isolate::ExecutingScript> Isolate::GetAllExecutingScripts() {
  return GetExecutingScriptsImpl(this, true, true);
}

}  // namespace internal
}  // namespace v8
