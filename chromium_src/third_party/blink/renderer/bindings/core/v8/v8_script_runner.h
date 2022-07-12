/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_BINDINGS_CORE_V8_V8_SCRIPT_RUNNER_H_
#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_BINDINGS_CORE_V8_V8_SCRIPT_RUNNER_H_

#include "v8/include/v8.h"

namespace blink {
namespace inspector_compile_script_event {
struct V8ConsumeCacheResult;
}  // namespace inspector_compile_script_event
}  // namespace blink

/*#define ReportExceptionForModule                                            \
  NotUsed();                                                                \
  static v8::MaybeLocal<v8::Script> CompileScript_ChromiumImpl(
      ScriptState*,
      const ClassicScript&,
      v8::ScriptCompiler::CompileOptions,
      v8::ScriptCompiler::NoCacheReason,
      v8::Local<v8::Data> host_defined_options);
  static v8::MaybeLocal<v8::Module> CompileScript_ChromiumImpl(
      v8::Isolate*,
      const ModuleScriptCreationParams&,
      const WTF::TextPosition&,
      v8::ScriptCompiler::CompileOptions,
      v8::ScriptCompiler::NoCacheReason,
      const ReferrerScriptInfo&);
  static void ReportExceptionForModule*/

#include "src/third_party/blink/renderer/bindings/core/v8/v8_script_runner.h"

//#undef ReportExceptionForModule

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_BINDINGS_CORE_V8_V8_SCRIPT_RUNNER_H_
