/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_THIRD_PARTY_BLINK_BRAVE_PAGE_GRAPH_PAGE_GRAPH_AGENT_H_
#define BRAVE_THIRD_PARTY_BLINK_BRAVE_PAGE_GRAPH_PAGE_GRAPH_AGENT_H_

#include <vector>

#include "brave/third_party/blink/brave_page_graph/probe_types.h"
#include "brave/third_party/blink/brave_page_graph/types.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/inspector/console_message.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class Node;
class Element;
class LocalFrame;
class DocumentLoader;
class ExceptionState;
class ConsoleMessage;

class CORE_EXPORT PageGraphAgent : public GarbageCollected<PageGraphAgent> {
 public:
  virtual ~PageGraphAgent() = default;

  virtual void Trace(Visitor*) const {}

  virtual void DidInsertDOMNode(Node*) {}
  virtual void DidCommitLoad(LocalFrame*, DocumentLoader*) {}
  virtual void RegisterPageGraphNodeConstructed(Node*) = 0;
  virtual void RegisterPageGraphBindingEvent(const char* name,
                                             const char* type,
                                             const char* event) = 0;
  virtual void RegisterPageGraphWebAPICallWithResult(
      const char* name,
      const PageGraphBlinkReceiverData& receiver_data,
      const PageGraphBlinkArgs& args,
      const ExceptionState* exception_state,
      const absl::optional<String>& result) = 0;
  virtual void ConsoleMessageAdded(ConsoleMessage*) = 0;
  virtual void DidModifyDOMAttr(Element*,
                                const QualifiedName& name,
                                const AtomicString& value) = 0;
  virtual void DidRemoveDOMAttr(Element*, const QualifiedName& name) = 0;
};

}  // namespace blink

#endif  // BRAVE_THIRD_PARTY_BLINK_BRAVE_PAGE_GRAPH_PAGE_GRAPH_AGENT_H_
