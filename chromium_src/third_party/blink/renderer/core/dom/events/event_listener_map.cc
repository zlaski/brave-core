/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "src/third_party/blink/renderer/core/dom/events/event_listener_map.cc"

#include "third_party/blink/renderer/core/probe/core_probes.h"

namespace blink {

// static
bool EventListenerMap::AddListenerToVector(
    EventListenerVector* vector,
    EventListener* listener,
    const AddEventListenerOptionsResolved* options,
    RegisteredEventListener* registered_listener) {
  const bool result = ::blink::AddListenerToVector(vector, listener, options,
                                                   registered_listener);
  if (result && CoreProbeSink::HasAgentsGlobal(CoreProbeSink::kPageGraph)) {
    DCHECK(registered_listener);
    DCHECK(vector && !vector->IsEmpty());
    DCHECK(*registered_listener == vector->back());
    const int id = RegisteredEventListener::GenerateId();
    registered_listener->SetId(id);
    vector->back().SetId(id);
  }
  return result;
}

}  // namespace blink
