/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_SCRIPT_PENDING_SCRIPT_H_
#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_SCRIPT_PENDING_SCRIPT_H_

#include <type_traits>

#define StopWatchingForLoad \
  PendingScriptConstructed(); \
  void StopWatchingForLoad

#include "src/third_party/blink/renderer/core/script/pending_script.h"

#undef StopWatchingForLoad

namespace cppgc {

template <typename T, typename>
struct PostConstructionCallbackTrait;

template <typename T>
struct PostConstructionCallbackTrait<
    T,
    typename std::enable_if<std::is_base_of<blink::PendingScript, T>::value,
                            void>::type> {
  static void Call(blink::PendingScript* object) {
    object->PendingScriptConstructed();
  }
};

}  // namespace cppgc

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_SCRIPT_PENDING_SCRIPT_H_
