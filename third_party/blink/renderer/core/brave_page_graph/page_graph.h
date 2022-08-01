/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_PAGE_GRAPH_H_
#define BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_PAGE_GRAPH_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/time/time.h"
#include "brave/third_party/blink/public/web/web_page_graph.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/blink_probe_types.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/requests/request_tracker.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/scripts/script_tracker.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/types.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/inspector/console_message.h"
#include "third_party/blink/renderer/core/inspector/protocol/Protocol.h"
#include "third_party/blink/renderer/core/inspector/protocol/dom.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "third_party/blink/renderer/platform/heap/member.h"
#include "third_party/blink/renderer/platform/supplementable.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace base {
class UnguessableToken;
}  // namespace base

namespace blink {

class ExecutionContext;
class LocalFrame;
class Node;
class CharacterData;
class Element;
class DocumentLoader;
class ExceptionState;
class ConsoleMessage;
class KURL;
enum class RenderBlockingBehavior : uint8_t;
class ResourceError;
struct ResourceLoaderOptions;
class ResourceResponse;
class CoreProbeSink;
class ResourceRequest;
class Resource;
class BlobDataHandle;
class ClassicScript;
class EventTarget;
class HTMLFrameOwnerElement;
class EncodedFormData;
class ModuleScriptCreationParams;
class ScriptElementBase;
class ReferrerScriptInfo;

}  // namespace blink

namespace brave_page_graph {

class Edge;
class EdgeNodeInsert;
class GraphItem;
class Node;
class NodeActor;
class NodeAdFilter;
class NodeBinding;
class NodeDOMRoot;
class NodeExtensions;
class NodeFingerprintingFilter;
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
class RequestTracker;
class ScriptTracker;
struct TrackedRequestRecord;

class CORE_EXPORT PageGraph : public blink::WebPageGraph {
  // Needed so that graph items can assign themself the next graph id.
  friend GraphItem;
  // Needed so that edges between HTML nodes can find their siblings and
  // parents.
  friend EdgeNodeInsert;
  // Needed so that HTML element nodes can find the script nodes for their event
  // listeners during GraphML generation.
  friend NodeHTMLElement;

 public:
  explicit PageGraph(blink::LocalFrame* local_frame);
  ~PageGraph() override;

  virtual void Trace(blink::Visitor*) const;

  void DidInsertDOMNode(blink::Node* node);
  void WillRemoveDOMNode(blink::Node* node);
  void NodeCreated(blink::Node* node);
  void RegisterPageGraphNodeFullyCreated(blink::Node* node);
  void RegisterPageGraphBindingEvent(blink::ExecutionContext*,
                                     base::StringPiece name,
                                     blink::PageGraphBindingType type,
                                     blink::PageGraphBindingEvent event);
  void RegisterPageGraphWebAPICallWithResult(
      blink::ExecutionContext*,
      base::StringPiece name,
      const blink::PageGraphBlinkReceiverData& receiver_data,
      const blink::PageGraphBlinkArgs& args,
      const blink::ExceptionState* exception_state,
      const absl::optional<String>& result);
  void RegisterPageGraphScriptCompilation(
      blink::ExecutionContext* execution_context,
      const blink::ReferrerScriptInfo& referrer_info,
      const blink::ClassicScript& classic_script,
      v8::Local<v8::Script> script);
  void RegisterPageGraphModuleCompilation(
      blink::ExecutionContext* execution_context,
      const blink::ReferrerScriptInfo& referrer_info,
      const blink::ModuleScriptCreationParams& params,
      v8::Local<v8::Module> script);
  void RegisterPageGraphScriptCompilationFromAttr(
      blink::EventTarget* event_target,
      const String& function_name,
      const String& script_body,
      v8::Local<v8::Function> compiled_function);
  void ConsoleMessageAdded(blink::ConsoleMessage* console_message);
  void DidModifyDOMAttr(blink::Element* element,
                        const blink::QualifiedName& name,
                        const AtomicString& value);
  void DidRemoveDOMAttr(blink::Element* element,
                        const blink::QualifiedName& name);
  void WillSendNavigationRequest(uint64_t identifier,
                                 blink::DocumentLoader*,
                                 const blink::KURL&,
                                 const AtomicString& http_method,
                                 blink::EncodedFormData*);
  void PrepareRequest(blink::DocumentLoader*,
                      blink::ResourceRequest&,
                      blink::ResourceLoaderOptions&,
                      blink::ResourceType);
  void DidReceiveResourceResponse(uint64_t identifier,
                                  blink::DocumentLoader* loader,
                                  const blink::ResourceResponse& response,
                                  const blink::Resource* cached_resource);
  void DidReceiveData(uint64_t identifier,
                      blink::DocumentLoader*,
                      const char* data,
                      uint64_t data_length);
  void DidReceiveBlob(uint64_t identifier,
                      blink::DocumentLoader*,
                      blink::BlobDataHandle*);
  void DidFinishLoading(uint64_t identifier,
                        blink::DocumentLoader*,
                        base::TimeTicks finish_time,
                        int64_t encoded_data_length,
                        int64_t decoded_body_length,
                        bool should_report_corb_blocking);
  void DidFailLoading(
      blink::CoreProbeSink* sink,
      uint64_t identifier,
      blink::DocumentLoader*,
      const blink::ResourceError&,
      const base::UnguessableToken& devtools_frame_or_worker_token);
  void RegisterPageGraphEventListenerAdd(
      blink::EventTarget* event_target,
      const String& event_type,
      blink::RegisteredEventListener* registered_listener);
  void RegisterPageGraphEventListenerRemove(
      blink::EventTarget* event_target,
      const String& event_type,
      blink::RegisteredEventListener* registered_listener);
  void RegisterPageGraphRemoteFrameCreated(
      blink::ExecutionContext*,
      blink::HTMLFrameOwnerElement* frame_owner_element);

  bool IsActive() const;

  void DidCommitLoad(blink::LocalFrame*, blink::DocumentLoader*);

  void RegisterRemoteFrameCreated(blink::Node* parent_node,
                                  const String& frame_id);

  void RegisterDocumentNodeCreated(blink::Document* node);
  void RegisterHTMLTextNodeCreated(blink::CharacterData* node);
  void RegisterHTMLElementNodeCreated(blink::Node* node);
  void RegisterHTMLElementNodeInserted(
      blink::Node* node,
      blink::Node* parent_node,
      const blink::DOMNodeId before_sibling_id);
  void RegisterHTMLTextNodeInserted(blink::Node* node,
                                    blink::Node* parent_node,
                                    const blink::DOMNodeId before_sibling_id);
  void RegisterHTMLElementNodeRemoved(blink::Node* node);
  void RegisterHTMLTextNodeRemoved(blink::Node* node);

  void RegisterEventListenerAdd(blink::Node* node,
                                const String& event_type,
                                const EventListenerId listener_id,
                                ScriptId listener_script_id);
  void RegisterEventListenerRemove(blink::Node* node,
                                   const String& event_type,
                                   const EventListenerId listener_id,
                                   ScriptId listener_script_id);

  void RegisterInlineStyleSet(blink::Node* node,
                              const String& attr_name,
                              const String& attr_value);
  void RegisterInlineStyleDelete(blink::Node* node,
                                 const String& attr_name);
  void RegisterAttributeSet(blink::Node* node,
                            const String& attr_name,
                            const String& attr_value);
  void RegisterAttributeDelete(blink::Node* node,
                               const String& attr_name);

  void RegisterTextNodeChange(blink::Node* node,
                              const String& new_text);

  void RegisterRequestStartFromElm(const blink::DOMNodeId node_id,
                                   const InspectorId request_id,
                                   const blink::KURL& url,
                                   const blink::ResourceType resource_type,
                                   const RequestType type);
  void RegisterRequestStartFromCurrentScript(
      blink::ExecutionContext* execution_context,
      const InspectorId request_id,
      const blink::KURL& url,
      const blink::ResourceType resource_type,
      const RequestType type);
  void RegisterRequestStartFromScript(
      blink::ExecutionContext* execution_context,
      const ScriptId script_id,
      const InspectorId request_id,
      const blink::KURL& url,
      const blink::ResourceType resource_type,
      const RequestType type);
  void RegisterRequestStartFromCSSOrLink(
      blink::DocumentLoader* loader,
      const InspectorId request_id,
      const blink::KURL& url,
      const blink::ResourceType resource_type,
      const RequestType type);
  void RegisterRequestStartForDocument(blink::Document* document,
                                       const InspectorId request_id,
                                       const blink::KURL& url,
                                       const bool is_main_frame);
  void RegisterRequestComplete(const InspectorId request_id);
  void RegisterRequestCompleteForDocument(const InspectorId request_id,
                                          const int64_t size);
  void RegisterRequestError(const InspectorId request_id);

  void RegisterResourceBlockAd(const blink::WebURL& url,
                               const std::string& rule) override;
  void RegisterResourceBlockTracker(const blink::WebURL& url,
                                    const std::string& host) override;
  void RegisterResourceBlockJavaScript(const blink::WebURL& url) override;
  void RegisterResourceBlockFingerprinting(const blink::WebURL& url,
                                           const FingerprintingRule& rule);

  void RegisterStorageRead(blink::ExecutionContext* execution_context,
                           const String& key,
                           const String& value,
                           const StorageLocation location);
  void RegisterStorageWrite(blink::ExecutionContext* execution_context,
                            const String& key,
                            const String& value,
                            const StorageLocation location);
  void RegisterStorageDelete(blink::ExecutionContext* execution_context,
                             const String& key,
                             const StorageLocation location);
  void RegisterStorageClear(blink::ExecutionContext* execution_context,
                            const StorageLocation location);

  void RegisterWebAPICall(blink::ExecutionContext* execution_context,
                          const MethodName& method,
                          const std::vector<String>& arguments);
  void RegisterWebAPIResult(blink::ExecutionContext* execution_context,
                            const MethodName& method,
                            const String& result);
  void RegisterJSBuiltInCall(blink::ExecutionContext* execution_context,
                             const char* built_in,
                             const std::vector<std::string>& args);
  void RegisterJSBuiltInResponse(blink::ExecutionContext* execution_context,
                                 const char* built_in,
                                 const std::string& result);

  //   void RegisterBindingEvent(const Binding binding,
  //                             const BindingType binding_type,
  //                             const BindingEvent binding_event);

  // Methods for handling the registration of script units in the document,
  // and v8 script executing.

  void RegisterScriptCompilation(blink::ExecutionContext* execution_context,
                                 const ScriptId script_id,
                                 const ScriptData& script_data);
  void RegisterScriptCompilationFromAttr(
      blink::ExecutionContext* execution_context,
      const ScriptId script_id,
      const ScriptData& script_data);
  void RegisterScriptCompilationFromEval(
      blink::ExecutionContext* execution_context,
      const ScriptId script_id,
      const ScriptData& script_data);

  void GenerateReportForNode(const blink::DOMNodeId node_id,
                             blink::protocol::Array<String>& report);

  std::string ToGraphML() const;

  base::TimeTicks GetStartTime() const;

 private:
  struct ExecutionContextNodes {
    NodeParser* parser_node;
    NodeExtensions* extensions_node;
  };

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
  NodeScript* GetScriptNode(blink::ExecutionContext* execution_context,
                            const ScriptId script_id);

  NodeActor* GetCurrentActingNode(
      blink::ExecutionContext* execution_context,
      ScriptPosition* out_script_position = nullptr);
  NodeActor* GetNodeActorForScriptId(blink::ExecutionContext* execution_context,
                                     const ScriptId script_id);
  ScriptId GetExecutingScriptId(
      blink::ExecutionContext* execution_context,
      ScriptPosition* out_script_position = nullptr) const;

  //   template <typename Callback>
  //   void GetAllActingNodes(Callback callback);

  //   template <typename Callback>
  //   void GetAllExecutingScripts(Callback callback);

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
                              const blink::ResourceType resource_type,
                              const RequestType type);
  void PossiblyWriteRequestsIntoGraph(
      scoped_refptr<const TrackedRequestRecord> record);
  void WriteDocumentRequestIntoGraph(const blink::DOMNodeId initiator,
                                     const DocumentRequest& request);

  // Return true if this PageGraph instance is instrumenting the top level
  // frame tree.
  bool IsRootFrame() const;

  void SpeculativelyRegisterCurrentlyConstructedNode(blink::DOMNodeId node_id);

  ScriptTracker& GetScriptTracker(blink::ExecutionContext*);
  std::map<ScriptId, NodeScript*>& GetScriptNodes(blink::ExecutionContext*);

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
  // base::flat_map<blink::ExecutionContext*, NodeParser*>
  // document_parser_nodes_;
  blink::HeapHashMap<blink::Member<ExecutionContext>, ExecutionContextNodes>
      execution_context_nodes_;

  NodeShields* const shields_node_;
  NodeShield* const ad_shield_node_;
  NodeShield* const tracker_shield_node_;
  NodeShield* const js_shield_node_;
  NodeShield* const fingerprinting_shield_node_;

  NodeStorageRoot* const storage_node_;
  NodeStorageCookieJar* const cookie_jar_node_;
  NodeStorageLocalStorage* const local_storage_node_;
  NodeStorageSessionStorage* const session_storage_node_;

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
  std::map<v8::Isolate*, ScriptTracker> script_trackers_;

  // Index structure for looking up script nodes.
  // This map does not own the references.
  std::map<v8::Isolate*, std::map<ScriptId, NodeScript*>>
      script_nodes_;

  // Makes sure we don't have more than one node in the graph representing
  // a single URL (not required for correctness, but keeps things tidier
  // and makes some kinds of queries nicer).
  std::map<RequestURL, NodeResource* const> resource_nodes_;

  // Data structure for keeping track of all the in-air requests that
  // have been made, but have not completed.
  RequestTracker request_tracker_;

  base::TimeTicks start_;
};

}  // namespace brave_page_graph

namespace blink {

class CORE_EXPORT PageGraph : public blink::GarbageCollected<PageGraph>,
                              public Supplement<LocalFrame>,
                              public brave_page_graph::PageGraph {
 public:
  static const char kSupplementName[];
  static PageGraph* From(LocalFrame&);
  static void ProvideTo(LocalFrame&);

  explicit PageGraph(LocalFrame& local_frame);

  void Trace(Visitor* visitor) const override;
};

}  // namespace blink

#endif  // BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_PAGE_GRAPH_H_
