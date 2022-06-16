/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_HTML_PARSER_HTML_DOCUMENT_PARSER_H_
#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_HTML_PARSER_HTML_DOCUMENT_PARSER_H_

#include <type_traits>

#define AsHTMLParserScriptRunnerHostForTesting \
  NotUsed();                                   \
  void HTMLDocumentParserConstructed();        \
  HTMLParserScriptRunnerHost* AsHTMLParserScriptRunnerHostForTesting

#include "src/third_party/blink/renderer/core/html/parser/html_document_parser.h"

#undef AsHTMLParserScriptRunnerHostForTesting

namespace cppgc {

template <typename T, typename>
struct PostConstructionCallbackTrait;

template <typename T>
struct PostConstructionCallbackTrait<
    T,
    typename std::enable_if<
        std::is_base_of<blink::HTMLDocumentParser, T>::value,
        void>::type> {
  static void Call(blink::HTMLDocumentParser* object) {
    object->HTMLDocumentParserConstructed();
  }
};

}  // namespace cppgc

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_HTML_PARSER_HTML_DOCUMENT_PARSER_H_
