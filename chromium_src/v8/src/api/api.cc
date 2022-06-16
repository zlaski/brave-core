/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "src/v8/src/api/api.cc"

namespace v8 {

Isolate::ExecutingScript Isolate::GetExecutingScript(bool include_position) {
  i::Isolate* internal_isolate = reinterpret_cast<i::Isolate*>(this);
  return internal_isolate->GetExecutingScript(include_position);
}

std::vector<Isolate::ExecutingScript> Isolate::GetAllExecutingScripts() {
  i::Isolate* internal_isolate = reinterpret_cast<i::Isolate*>(this);
  return internal_isolate->GetAllExecutingScripts();
}

void Isolate::SetPageGraphBackend(PageGraphBackend* page_graph_backend) {
  i::Isolate* internal_isolate = reinterpret_cast<i::Isolate*>(this);
  internal_isolate->SetPageGraphBackend(page_graph_backend);
}

}  // namespace v8
