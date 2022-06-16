/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "src/third_party/blink/renderer/core/html/parser/html_document_parser.cc"

namespace blink {

void HTMLDocumentParser::HTMLDocumentParserConstructed() {
  if (CoreProbeSink::HasAgentsGlobal(CoreProbeSink::kPageGraph)) {
    preloader_ = nullptr;
  }
}

}  // namespace blink
