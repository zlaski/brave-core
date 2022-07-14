/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_PAGE_GRAPH_PAGE_GRAPH_H_
#define BRAVE_COMPONENTS_BRAVE_PAGE_GRAPH_PAGE_GRAPH_H_

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/inspector/protocol/Protocol.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "third_party/blink/renderer/platform/heap/member.h"

#include "brave/third_party/blink/brave_page_graph/page_graph_agent.h"
#include "brave/third_party/blink/brave_page_graph/requests/request_tracker.h"
#include "brave/third_party/blink/brave_page_graph/scripts/script_tracker.h"
#include "brave/third_party/blink/brave_page_graph/types.h"

namespace blink {

class ExecutionContext;
class LocalFrame;

}  // namespace blink

namespace v8 {

class Context;
class Isolate;
template <class T>
class Local;

}

namespace brave_page_graph {

class Edge;
class EdgeEventListenerAction;
class EdgeRequestStart;
class EdgeNodeInsert;
class GraphItem;
class Node;
class NodeActor;
class NodeAdFilter;
class NodeBinding;
class NodeDOMRoot;
class NodeExtensions;
class NodeFingerprintingFilter;
class NodeFrame;
class NodeHTML;
class NodeHTMLElement;
class NodeHTMLText;
class NodeParser;
class NodeResource;
class NodeScript;
class NodeShields;
class NodeShield;
class NodeStorage;
class NodeStorageCookieJar;
class NodeStorageLocalStorage;
class NodeStorageRoot;
class NodeStorageSessionStorage;
class NodeTrackerFilter;
class NodeJSBuiltIn;
class NodeJSWebAPI;
class ResponseMetadata;
class RequestTracker;
class ScriptTracker;
struct TrackedRequestRecord;

::std::chrono::milliseconds NowInMs();

class CORE_EXPORT PageGraph : public blink::PageGraphAgent {
  // Needed so that graph items can assign themself the next graph id.
  friend GraphItem;
  // Needed so that edges between HTML nodes can find their siblings and
  // parents.
  friend EdgeNodeInsert;
  // Needed so that HTML element nodes can find the script nodes for their event
  // listeners during GraphML generation.
  friend NodeHTMLElement;

 public:
  static PageGraph* GetFromIsolate(v8::Isolate& isolate);
  static PageGraph* GetFromContext(v8::Local<v8::Context> context);
  static PageGraph* GetFromExecutionContext(
      blink::ExecutionContext& exec_context);

  explicit PageGraph(blink::LocalFrame* local_frame);
  ~PageGraph() override;

  void Shutdown();

  void Trace(blink::Visitor*) const override;

  blink::ExecutionContext* GetExecutionContext() const;

  // PageGraphAgent:
  void DidInsertDOMNode(blink::Node* node) override;
  void WillRemoveDOMNode(blink::Node* node) override;
  void NodeCreated(blink::Node* node) override;
  void RegisterPageGraphNodeFullyCreated(blink::Node* node) override;
  void RegisterPageGraphElmForLocalScript(blink::DOMNodeId dom_node_id,
                                          const String& source_text) override;
  void RegisterPageGraphElmForRemoteScript(blink::DOMNodeId dom_node_id,
                                           const blink::KURL& url) override;
  void RegisterPageGraphBindingEvent(const char* name,
                                     const char* type,
                                     const char* event) override;
  void RegisterPageGraphWebAPICallWithResult(
      const char* name,
      const blink::PageGraphBlinkReceiverData& receiver_data,
      const blink::PageGraphBlinkArgs& args,
      const blink::ExceptionState* exception_state,
      const absl::optional<String>& result) override;
  void RegisterPageGraphUrlForScriptSource(
      const blink::KURL& base_url,
      const String& source_string) override;
  void RegisterPageGraphModuleScriptForDescendant(
      int script_id,
      const blink::KURL& url) override;
  void RegisterPageGraphScriptCompilation(
      const blink::ClassicScript& classic_script,
      v8::Local<v8::Script> script) override;
  void RegisterPageGraphModuleCompilation(
      const blink::ModuleScriptCreationParams& params,
      v8::Local<v8::Module> script) override;
  void RegisterPageGraphScriptCompilationFromAttr(
      blink::EventTarget* event_target,
      const String& function_name,
      const String& script_body,
      v8::Local<v8::Function> compiled_function) override;
  void ConsoleMessageAdded(blink::ConsoleMessage* console_message) override;
  void DidModifyDOMAttr(blink::Element* element,
                        const blink::QualifiedName& name,
                        const AtomicString& value) override;
  void DidRemoveDOMAttr(blink::Element* element,
                        const blink::QualifiedName& name) override;
  void PrepareRequest(blink::DocumentLoader*,
                      blink::ResourceRequest&,
                      blink::ResourceLoaderOptions&,
                      blink::ResourceType) override;
  void DidReceiveResourceResponse(
      uint64_t identifier,
      blink::DocumentLoader* loader,
      const blink::ResourceResponse& response,
      const blink::Resource* cached_resource) override;
  void DidReceiveData(uint64_t identifier,
                      blink::DocumentLoader*,
                      const char* data,
                      uint64_t data_length) override;
  void DidReceiveBlob(uint64_t identifier,
                      blink::DocumentLoader*,
                      blink::BlobDataHandle*) override;
  void DidFinishLoading(uint64_t identifier,
                        blink::DocumentLoader*,
                        base::TimeTicks finish_time,
                        int64_t encoded_data_length,
                        int64_t decoded_body_length,
                        bool should_report_corb_blocking) override;
  void DidFailLoading(
      blink::CoreProbeSink* sink,
      uint64_t identifier,
      blink::DocumentLoader*,
      const blink::ResourceError&,
      const base::UnguessableToken& devtools_frame_or_worker_token) override;
  void RegisterPageGraphEventListenerAdd(
      blink::EventTarget* event_target,
      const String& event_type,
      blink::RegisteredEventListener* registered_listener) override;
  void RegisterPageGraphEventListenerRemove(
      blink::EventTarget* event_target,
      const String& event_type,
      blink::RegisteredEventListener* registered_listener) override;
  void RegisterPageGraphRemoteFrameCreated(
      blink::HTMLFrameOwnerElement* frame_owner_element) override;

  bool IsActive() const;

  void DidCommitLoad(blink::LocalFrame*, blink::DocumentLoader*) override;

  void RegisterDocumentRootCreated(const blink::DOMNodeId node_id,
                                   const blink::DOMNodeId parent_node_id,
                                   const WTF::String& tag_name,
                                   const blink::KURL& url);
  void RegisterRemoteFrameCreated(const blink::DOMNodeId parent_node_id,
                                  const WTF::String& frame_id);

  void RegisterHTMLElementNodeCreated(
      const blink::DOMNodeId node_id,
      const WTF::String& tag_name,
      const ElementType element_type = kElementTypeDefault);
  void RegisterHTMLTextNodeCreated(const blink::DOMNodeId node_id,
                                   const WTF::String& text);
  void RegisterHTMLElementNodeInserted(
      const blink::DOMNodeId node_id,
      const blink::DOMNodeId parent_node_id,
      const blink::DOMNodeId before_sibling_id);
  void RegisterHTMLTextNodeInserted(const blink::DOMNodeId node_id,
                                    const blink::DOMNodeId parent_node_id,
                                    const blink::DOMNodeId before_sibling_id);
  void RegisterHTMLElementNodeRemoved(const blink::DOMNodeId node_id);
  void RegisterHTMLTextNodeRemoved(const blink::DOMNodeId node_id);

  void RegisterEventListenerAdd(const blink::DOMNodeId,
                                const WTF::String& event_type,
                                const EventListenerId listener_id,
                                ScriptId listener_script_id);
  void RegisterEventListenerRemove(const blink::DOMNodeId,
                                   const WTF::String& event_type,
                                   const EventListenerId listener_id,
                                   ScriptId listener_script_id);

  void RegisterInlineStyleSet(const blink::DOMNodeId node_id,
                              const WTF::String& attr_name,
                              const WTF::String& attr_value);
  void RegisterInlineStyleDelete(const blink::DOMNodeId node_id,
                                 const WTF::String& attr_name);
  void RegisterAttributeSet(const blink::DOMNodeId node_id,
                            const WTF::String& attr_name,
                            const WTF::String& attr_value);
  void RegisterAttributeDelete(const blink::DOMNodeId node_id,
                               const WTF::String& attr_name);

  void RegisterTextNodeChange(const blink::DOMNodeId node_id,
                              const WTF::String& new_text);

  void RegisterRequestStartFromElm(const blink::DOMNodeId node_id,
                                   const InspectorId request_id,
                                   const blink::KURL& url,
                                   const RequestType type);
  void RegisterRequestStartFromCurrentScript(const InspectorId request_id,
                                             const blink::KURL& url,
                                             const RequestType type);
  void RegisterRequestStartFromScript(const ScriptId script_id,
                                      const InspectorId request_id,
                                      const blink::KURL& url,
                                      const RequestType type);
  void RegisterRequestStartFromCSS(const InspectorId request_id,
                                   const blink::KURL& url,
                                   const RequestType type);
  void RegisterRequestStartForDocument(const blink::DOMNodeId node_id,
                                       const InspectorId request_id,
                                       const blink::KURL& url,
                                       const bool is_main_frame);
  void RegisterRequestComplete(const InspectorId request_id,
                               const blink::ResourceType type,
                               const ResponseMetadata& metadata,
                               const std::string& resource_hash);
  void RegisterRequestCompleteForDocument(const InspectorId request_id,
                                          const int64_t size);
  void RegisterRequestError(const InspectorId request_id,
                            const ResponseMetadata& metadata);

  void RegisterResourceBlockAd(const GURL& url, const std::string& rule);
  void RegisterResourceBlockTracker(const GURL& url, const std::string& host);
  void RegisterResourceBlockJavaScript(const GURL& url);
  void RegisterResourceBlockFingerprinting(const GURL& url,
                                           const FingerprintingRule& rule);

  void RegisterStorageRead(const WTF::String& key,
                           const WTF::String& value,
                           const StorageLocation location);
  void RegisterStorageWrite(const WTF::String& key,
                            const WTF::String& value,
                            const StorageLocation location);
  void RegisterStorageDelete(const WTF::String& key,
                             const StorageLocation location);
  void RegisterStorageClear(const StorageLocation location);

  void RegisterWebAPICall(const MethodName& method,
                          const std::vector<WTF::String>& arguments);
  void RegisterWebAPIResult(const MethodName& method,
                            const WTF::String& result);
  void RegisterJSBuiltInCall(const JSBuiltIn built_in,
                             const std::vector<const std::string>& args);
  void RegisterJSBuiltInResponse(const JSBuiltIn built_in,
                                 const std::string& result);

  void RegisterBindingEvent(const Binding binding,
                            const BindingType binding_type,
                            const BindingEvent binding_event);

  // Methods for handling the registration of script units in the document,
  // and v8 script executing.

  // Local scripts are scripts that define their code inline.
  void RegisterElmForLocalScript(const blink::DOMNodeId node_id,
                                 const WTF::String& code);
  // Remote scripts are scripts that reference remote code (eg src=...).
  void RegisterElmForRemoteScript(const blink::DOMNodeId node_id,
                                  const blink::KURL& url);
  // JavaScript URLs ("javascript:" schemes).
  void RegisterJavaScriptURL(const WTF::String& code);
  void RegisterUrlForScriptSource(const blink::KURL& url,
                                  const WTF::String& code);
  void RegisterUrlForExtensionScriptSource(const WTF::String& url,
                                           const WTF::String& code);
  void RegisterScriptCompilation(const WTF::String& code,
                                 const ScriptId script_id,
                                 const ScriptType type);
  void RegisterScriptCompilationFromAttr(const blink::DOMNodeId node_id,
                                         const String& attr_name,
                                         const String& attr_value,
                                         const ScriptId script_id);
  void RegisterScriptCompilationFromEval(const ScriptId parent_script_id,
                                         const ScriptId script_id,
                                         const String& source);

  void RegisterModuleScriptForDescendant(
      const blink::KURL& parent_location,
      const blink::KURL& descendant_location);
  void RegisterModuleScriptForDescendant(
      const ScriptId parent_id,
      const blink::KURL& descendant_location);

  void GenerateReportForNode(const blink::DOMNodeId node_id,
                             blink::protocol::Array<WTF::String>& report);

  std::string ToGraphML() const;

  std::chrono::milliseconds GetTimestamp() const;

 protected:
  void AddNode(Node* const node);
  void AddEdge(const Edge* const edge);

  void AddShieldNode(NodeShield* const shield_node);
  void AddStorageNode(NodeStorage* const storage_node);

  const NodeUniquePtrList& Nodes() const;
  const EdgeUniquePtrList& Edges() const;
  const GraphItemList& GraphItems() const;

  NodeHTML* GetHTMLNode(const blink::DOMNodeId node_id) const;
  NodeHTMLElement* GetHTMLElementNode(const blink::DOMNodeId node_id) const;
  NodeHTMLText* GetHTMLTextNode(const blink::DOMNodeId node_id) const;
  NodeScript* GetScriptNode(const ScriptId script_id) const;

  NodeActor* GetCurrentActingNode(
      ScriptPosition* out_script_position = nullptr) const;
  NodeActor* GetNodeActorForScriptId(const ScriptId script_id) const;
  ScriptId GetExecutingScriptId(
      ScriptPosition* out_script_position = nullptr) const;

  template <typename Callback>
  void GetAllActingNodes(Callback callback);

  template <typename Callback>
  void GetAllExecutingScripts(Callback callback);

  NodeResource* GetResourceNodeForUrl(const std::string& url);

  NodeAdFilter* GetAdFilterNodeForRule(const std::string& rule);
  NodeTrackerFilter* GetTrackerFilterNodeForHost(const std::string& host);
  NodeFingerprintingFilter* GetFingerprintingFilterNodeForRule(
      const FingerprintingRule& rule);

  NodeBinding* GetBindingNode(const Binding binding,
                              const BindingType binding_type);

  void DoRegisterRequestStart(const InspectorId request_id,
                              Node* const requesting_node,
                              const std::string& local_url,
                              const RequestType type);
  void PossiblyWriteRequestsIntoGraph(
      const std::shared_ptr<const TrackedRequestRecord> record);
  void WriteDocumentRequestIntoGraph(const blink::DOMNodeId initiator,
                                     const DocumentRequest request);

  // Return true if this PageGraph instance is instrumenting the top level
  // frame tree.
  bool IsRootFrame() const;

  void SpeculativelyRegisterCurrentlyConstructedNode(blink::DOMNodeId node_id);

  // Monotonically increasing counter, used so that we can replay the
  // the graph's construction if needed.
  PageGraphId id_counter_ = 0;

  blink::WeakMember<blink::LocalFrame> local_frame_;

  // The blink assigned frame id for the local root's frame.
  const std::string frame_id_;

  // These vectors own all of the items that are shared and indexed across
  // the rest of the graph.  All the other pointers (the weak pointers)
  // do not own their data.
  NodeUniquePtrList nodes_;
  EdgeUniquePtrList edges_;

  // Vectors for tracking other ways of referencing graph elements, non-owning.
  GraphItemList graph_items_;

  // Non-owning references to singleton items in the graph. (the owning
  // references will be in the above vectors).
  NodeParser* const parser_node_;
  NodeExtensions* const extensions_node_;

  NodeShields* const shields_node_;
  NodeShield* const ad_shield_node_;
  NodeShield* const tracker_shield_node_;
  NodeShield* const js_shield_node_;
  NodeShield* const fingerprinting_shield_node_;

  NodeStorageRoot* const storage_node_;
  NodeStorageCookieJar* const cookie_jar_node_;
  NodeStorageLocalStorage* const local_storage_node_;
  NodeStorageSessionStorage* const session_storage_node_;

  // Non-owning reference to the HTML root of the document (i.e. <html>).
  NodeDOMRoot* html_root_node_ = nullptr;

  // Information about the network request to the HTML root of the document.
  // We need this separately from the RequestTracker because we create the
  // initial DOM Root node immediately at the initialization of PageGraph,
  // before the request occurs.
  DocumentRequest root_request_record_;

  // Index structure for storing and looking up webapi nodes.
  // This map does not own the references.
  std::map<MethodName, NodeJSWebAPI* const> webapi_nodes_;

  // Index structure for storing and looking up nodes representing built
  // in JS funcs and methods. This map does not own the references.
  std::map<JSBuiltIn, NodeJSBuiltIn* const> builtin_js_nodes_;

  // Keep track of currently constructed nodes to track not yet added nodes.
  base::flat_map<blink::DOMNodeId, blink::UntracedMember<blink::Node>>
      currently_constructed_nodes_;

  // Index structure for looking up HTML nodes.
  // This map does not own the references.
  std::map<blink::DOMNodeId, NodeHTMLElement* const> element_nodes_;
  std::map<blink::DOMNodeId, NodeHTMLText* const> text_nodes_;

  // Index structure for looking up script nodes.
  // This map does not own the references.
  std::map<ScriptId, NodeScript* const> script_nodes_;

  // Index structure for looking up filter nodes.
  // These maps do not own the references.
  std::map<std::string, NodeAdFilter* const> ad_filter_nodes_;
  std::map<std::string, NodeTrackerFilter* const> tracker_filter_nodes_;
  std::map<FingerprintingRule, NodeFingerprintingFilter* const>
      fingerprinting_filter_nodes_;

  // Index structure for looking up binding nodes.
  // This map does not own the references.
  std::map<Binding, NodeBinding* const> binding_nodes_;

  // Data structure used for mapping HTML script elements (and other
  // sources of script in a document) to v8 script units.
  ScriptTracker script_tracker_;

  // Makes sure we don't have more than one node in the graph representing
  // a single URL (not required for correctness, but keeps things tidier
  // and makes some kinds of queries nicer).
  std::map<RequestURL, NodeResource* const> resource_nodes_;

  // Data structure for keeping track of all the in-air requests that
  // have been made, but have not completed.
  RequestTracker request_tracker_;

  std::chrono::milliseconds start_;
};

}  // namespace brave_page_graph

#endif  // BRAVE_COMPONENTS_BRAVE_PAGE_GRAPH_PAGE_GRAPH_H_
