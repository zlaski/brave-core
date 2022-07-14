/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_THIRD_PARTY_BLINK_BRAVE_PAGE_GRAPH_PAGE_GRAPH_AGENT_H_
#define BRAVE_THIRD_PARTY_BLINK_BRAVE_PAGE_GRAPH_PAGE_GRAPH_AGENT_H_

#include <vector>

#include "base/unguessable_token.h"
#include "brave/third_party/blink/brave_page_graph/probe_types.h"
#include "brave/third_party/blink/brave_page_graph/types.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
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
class Document;
class ExecutionContext;
struct FetchInitiatorInfo;
class HTTPHeaderMap;
class KURL;
enum class RenderBlockingBehavior : uint8_t;
class ResourceError;
struct ResourceLoaderOptions;
class ResourceResponse;
enum class ResourceType : uint8_t;
class CoreProbeSink;
class ResourceRequest;
class Resource;
class BlobDataHandle;
class PendingScript;
class ClassicScript;
class EventTarget;
class HTMLFrameOwnerElement;

class CORE_EXPORT PageGraphAgent : public GarbageCollected<PageGraphAgent> {
 public:
  virtual ~PageGraphAgent() = default;

  virtual void Trace(Visitor*) const {}

  virtual void DidInsertDOMNode(Node*) = 0;
  virtual void WillRemoveDOMNode(Node*) = 0 ;
  virtual void DidCommitLoad(LocalFrame*, DocumentLoader*) = 0;
  virtual void NodeCreated(Node*) = 0;
  virtual void RegisterPageGraphNodeFullyCreated(Node*) = 0;
  virtual void RegisterPageGraphElmForLocalScript(
      DOMNodeId dom_node_id,
      const String& source_text) = 0;
  virtual void RegisterPageGraphElmForRemoteScript(DOMNodeId dom_node_id,
                                                   const KURL& url) = 0;
  virtual void RegisterPageGraphUrlForScriptSource(
      const KURL& base_url,
      const String& source_string) = 0;
  virtual void RegisterPageGraphModuleScriptForDescendant(int script_id,
                                                          const KURL& url) = 0;
  virtual void RegisterPageGraphScriptCompilation(const ClassicScript&,
                                                  v8::Local<v8::Script>) = 0;
  virtual void RegisterPageGraphModuleCompilation(
      const ModuleScriptCreationParams& params,
      v8::Local<v8::Module> script) = 0;
  virtual void RegisterPageGraphScriptCompilationFromAttr(
      EventTarget*,
      const String& function_name,
      const String& script_body,
      v8::Local<v8::Function> compiled_function) = 0;
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
  virtual void PrepareRequest(DocumentLoader*,
                              ResourceRequest&,
                              ResourceLoaderOptions&,
                              ResourceType) = 0;
  virtual void DidReceiveResourceResponse(uint64_t identifier,
                                          DocumentLoader* loader,
                                          const ResourceResponse& response,
                                          const Resource* cached_resource) = 0;
  virtual void DidReceiveData(uint64_t identifier,
                              DocumentLoader*,
                              const char* data,
                              uint64_t data_length) = 0;
  virtual void DidReceiveBlob(uint64_t identifier,
                              DocumentLoader*,
                              BlobDataHandle*) = 0;
  virtual void DidFinishLoading(uint64_t identifier,
                                DocumentLoader*,
                                base::TimeTicks finish_time,
                                int64_t encoded_data_length,
                                int64_t decoded_body_length,
                                bool should_report_corb_blocking) = 0;
  virtual void DidFailLoading(
      CoreProbeSink* sink,
      uint64_t identifier,
      DocumentLoader*,
      const ResourceError&,
      const base::UnguessableToken& devtools_frame_or_worker_token) = 0;
  virtual void RegisterPageGraphEventListenerAdd(
      EventTarget*,
      const String& event_type,
      RegisteredEventListener* registered_listener) = 0;
  virtual void RegisterPageGraphEventListenerRemove(
      EventTarget*,
      const String& event_type,
      RegisteredEventListener* registered_listener) = 0;
  virtual void RegisterPageGraphRemoteFrameCreated(
      HTMLFrameOwnerElement* frame_owner_element) = 0;
};

}  // namespace blink

#endif  // BRAVE_THIRD_PARTY_BLINK_BRAVE_PAGE_GRAPH_PAGE_GRAPH_AGENT_H_
