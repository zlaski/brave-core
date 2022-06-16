/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "src/third_party/blink/renderer/bindings/core/v8/v8_script_runner.cc"

namespace blink {

// static
v8::MaybeLocal<v8::Script> V8ScriptRunner::CompileScript(
    ScriptState* script_state,
    const ClassicScript& classic_script,
    v8::ScriptCompiler::CompileOptions compile_options,
    v8::ScriptCompiler::NoCacheReason no_cache_reason,
    v8::Local<v8::Data> host_defined_options) {
  v8::MaybeLocal<v8::Script> result = V8ScriptRunner_CompileScript_ChromiumImpl(
      script_state, classic_script, compile_options, no_cache_reason,
      host_defined_options);
  if (CoreProbeSink::HasAgentsGlobal(CoreProbeSink::kPageGraph)) {
    v8::Local<v8::Script> script;
    if (result.ToLocal(&script)) {
      probe::RegisterPageGraphScriptCompilation(
          ExecutionContext::From(script_state), classic_script, script);
    }
  }
  return result;
}

// static
v8::MaybeLocal<v8::Module> V8ScriptRunner::CompileModule(
    v8::Isolate* isolate,
    const ModuleScriptCreationParams& params,
    const TextPosition& start_position,
    v8::ScriptCompiler::CompileOptions compile_options,
    v8::ScriptCompiler::NoCacheReason no_cache_reason,
    const ReferrerScriptInfo& referrer_info) {
  v8::MaybeLocal<v8::Module> result = V8ScriptRunner_CompileModule_ChromiumImpl(
      isolate, params, start_position, compile_options, no_cache_reason,
      referrer_info);
  if (CoreProbeSink::HasAgentsGlobal(CoreProbeSink::kPageGraph)) {
    v8::Local<v8::Module> script;
    if (result.ToLocal(&script)) {
      probe::RegisterPageGraphModuleCompilation(
          ExecutionContext::From(isolate->GetCurrentContext()), params, script);
    }
  }
  return result;
}

}  // namespace blink
