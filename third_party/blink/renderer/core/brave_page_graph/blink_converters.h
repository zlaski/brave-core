/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_BLINK_CONVERTERS_H_
#define BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_BLINK_CONVERTERS_H_

#include <ostream>
#include <sstream>
#include <type_traits>

#include "brave/third_party/blink/renderer/core/brave_page_graph/blink_probe_types.h"
#include "third_party/blink/renderer/bindings/core/v8/native_value_traits_impl.h"
#include "third_party/blink/renderer/bindings/core/v8/script_value.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/platform/wtf/text/atomic_string.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class ScriptValue;
class TextMetrics;
class CanvasRenderingContext;

namespace internal {

// https://gist.github.com/szymek156/9b1b90fe474277be4641e9ef4666f472
template <class Class>
struct has_ostream_operator_impl {
  template <class V>
  static auto test(V*)
      -> decltype(std::declval<std::ostream>() << std::declval<V>());
  template <typename>
  static auto test(...) -> std::false_type;

  using type = typename std::is_same<std::ostream&,
                                     decltype(test<Class>(nullptr))>::type;
};

template <class Class>
struct has_ostream_operator : has_ostream_operator_impl<Class>::type {};

}  // namespace internal

// Unknown class handler.
template <typename T>
typename std::enable_if<!internal::has_ostream_operator<T>::value, String>::type
ToPageGraphBlinkArg(const T&) {
  std::stringstream result;
  result << "[" << typeid(T).name() << "]";
  return String(result.str());
}

// ostream-supported handler.
template <typename T>
typename std::enable_if<internal::has_ostream_operator<T>::value, String>::type
ToPageGraphBlinkArg(const T& value) {
  std::stringstream result;
  result << value;
  return String(result.str());
}

template <typename T>
String ToPageGraphBlinkArg(const Vector<T>& values) {
  std::stringstream result;
  result << "{";
  for (const T& value : values) {
    if (result.tellp() != std::streampos(0)) {
      result << ", ";
    }
    result << ToPageGraphBlinkArg(value);
  }
  result << "}";
  return String(result.str());
}

template <>
inline String ToPageGraphBlinkArg(const String& string) {
  return string;
}

template <>
inline String ToPageGraphBlinkArg(const AtomicString& string) {
  return string.GetString();
}

template <>
CORE_EXPORT String
ToPageGraphBlinkArg(const bindings::NativeValueTraitsStringAdapter& adapter);
template <>
CORE_EXPORT String ToPageGraphBlinkArg(const ScriptValue& script_value);

CORE_EXPORT String ToPageGraphBlinkArg(TextMetrics* result);
CORE_EXPORT String ToPageGraphBlinkArg(CanvasRenderingContext* context);

template <typename T>
PageGraphBlinkReceiverData ToPageGraphBlinkReceiverData(T*) {
  return {};
}

CORE_EXPORT PageGraphBlinkReceiverData
ToPageGraphBlinkReceiverData(Document* document);

}  // namespace blink

#endif  // BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_BLINK_CONVERTERS_H_
