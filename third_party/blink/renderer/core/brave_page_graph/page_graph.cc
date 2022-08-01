/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/third_party/blink/renderer/core/brave_page_graph/page_graph.h"

#include <signal.h>
#include <climits>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>

#include <libxml/tree.h>

#include "base/json/json_string_value_serializer.h"
#include "base/no_destructor.h"
#include "brave/components/brave_shields/common/brave_shield_constants.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/attribute/edge_attribute_delete.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/attribute/edge_attribute_set.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/binding/edge_binding.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/binding/edge_binding_event.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/edge_cross_dom.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/edge_filter.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/edge_resource_block.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/edge_shield.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/edge_structure.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/edge_text_change.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/event_listener/edge_event_listener_add.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/event_listener/edge_event_listener_remove.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/execute/edge_execute.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/execute/edge_execute_attr.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/js/edge_js_call.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/js/edge_js_result.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/node/edge_node_create.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/node/edge_node_delete.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/node/edge_node_insert.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/node/edge_node_remove.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/request/edge_request.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/request/edge_request_complete.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/request/edge_request_error.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/request/edge_request_start.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/storage/edge_storage_bucket.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/storage/edge_storage_clear.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/storage/edge_storage_delete.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/storage/edge_storage_read_call.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/storage/edge_storage_read_result.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/storage/edge_storage_set.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/actor/node_actor.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/actor/node_parser.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/actor/node_script.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/binding/node_binding.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/binding/node_binding_event.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/filter/node_ad_filter.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/filter/node_fingerprinting_filter.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/filter/node_tracker_filter.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/html/node_dom_root.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/html/node_frame_owner.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/html/node_html.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/html/node_html_element.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/html/node_html_text.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/js/node_js.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/js/node_js_builtin.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/js/node_js_webapi.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/node_extensions.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/node_remote_frame.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/node_resource.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/shield/node_shield.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/shield/node_shields.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/storage/node_storage_cookiejar.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/storage/node_storage_localstorage.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/storage/node_storage_root.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/storage/node_storage_sessionstorage.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graphml.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/logging.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/requests/request_tracker.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/requests/tracked_request.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/scripts/script_tracker.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/types.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/utilities/response_metadata.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/utilities/urls.h"
#include "brave/v8/include/v8-isolate-page-graph-utils.h"
#include "third_party/blink/public/mojom/script/script_type.mojom-shared.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/renderer/bindings/core/v8/js_based_event_listener.h"
#include "third_party/blink/renderer/bindings/core/v8/referrer_script_info.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_binding_for_core.h"
#include "third_party/blink/renderer/core/core_probe_sink.h"
#include "third_party/blink/renderer/core/dom/character_data.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
#include "third_party/blink/renderer/core/dom/element.h"
#include "third_party/blink/renderer/core/dom/events/event_listener.h"
#include "third_party/blink/renderer/core/dom/events/event_target.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/html/html_frame_owner_element.h"
#include "third_party/blink/renderer/core/html_names.h"
#include "third_party/blink/renderer/core/inspector/console_message.h"
#include "third_party/blink/renderer/core/inspector/identifiers_factory.h"
#include "third_party/blink/renderer/core/inspector/protocol/Protocol.h"
#include "third_party/blink/renderer/core/loader/document_loader.h"
#include "third_party/blink/renderer/core/loader/modulescript/module_script_creation_params.h"
#include "third_party/blink/renderer/core/loader/resource/script_resource.h"
#include "third_party/blink/renderer/core/page/page.h"
#include "third_party/blink/renderer/core/script/classic_pending_script.h"
#include "third_party/blink/renderer/core/script/classic_script.h"
#include "third_party/blink/renderer/core/script/module_pending_script.h"
#include "third_party/blink/renderer/core/script/script_element_base.h"
#include "third_party/blink/renderer/platform/bindings/string_resource.h"
#include "third_party/blink/renderer/platform/crypto.h"
#include "third_party/blink/renderer/platform/graphics/dom_node_id.h"
#include "third_party/blink/renderer/platform/loader/fetch/fetch_initiator_type_names.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource_loader_options.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource_request.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"
#include "third_party/blink/renderer/platform/wtf/casting.h"
#include "third_party/blink/renderer/platform/wtf/text/base64.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "url/gurl.h"
#include "v8/include/v8.h"

#include "base/debug/stack_trace.h"

using ::blink::ClassicScript;
using ::blink::Document;
using ::blink::DOMNodeId;
using ::blink::DynamicTo;
using ::blink::ExecutionContext;
using ::blink::IsA;
using ::blink::KURL;
using ::blink::LocalDOMWindow;
using ::blink::ResourceType;
using ::blink::To;
using ::blink::ToExecutionContext;
using ::blink::protocol::Array;

namespace brave_page_graph {

namespace {

constexpr char kPageGraphVersion[] = "0.2.3";
constexpr char kPageGraphUrl[] =
    "https://github.com/brave/brave-browser/wiki/PageGraph";

PageGraph* GetPageGraphFromIsolate(v8::Isolate* isolate) {
  blink::LocalDOMWindow* window = blink::CurrentDOMWindow(isolate);
  if (!window) {
    return nullptr;
  }
  blink::LocalFrame* frame = window->GetFrame();
  if (!frame) {
    return nullptr;
  }

  return blink::PageGraph::From(*frame);
}

class V8PageGraphDelegate : public v8::page_graph::PageGraphDelegate {
 public:
  void OnEvalScriptCompiled(
      v8::Isolate* isolate,
      const std::vector<v8::page_graph::ExecutingScript>& executing_scripts,
      const ScriptId script_id,
      v8::Local<v8::String> source) override {
    PageGraph* const page_graph = GetPageGraphFromIsolate(isolate);
    if (page_graph) {
      int parent_script_id = 0;
      if (!executing_scripts.empty())
        parent_script_id = executing_scripts[0].script_id;
      ScriptData script_data{
          .code = blink::ToBlinkString<String>(source, blink::kExternalize),
          .source = {
              .parent_script_id = parent_script_id,
              .is_eval = true,
          }};
      page_graph->RegisterScriptCompilationFromEval(
          blink::ToExecutionContext(isolate->GetCurrentContext()), script_id,
          script_data);
    }
  }

  void OnBuiltinCall(v8::Isolate* isolate,
                     const char* built_in,
                     const std::vector<std::string>& args,
                     const std::string& result) override {
    PageGraph* const page_graph = GetPageGraphFromIsolate(isolate);
    blink::ExecutionContext* execution_context =
        blink::ToExecutionContext(isolate->GetCurrentContext());
    if (page_graph) {
      page_graph->RegisterJSBuiltInCall(execution_context, built_in, args);
      page_graph->RegisterJSBuiltInResponse(execution_context, built_in,
                                            result);
    }
  }
};

static v8::Local<v8::Function> GetInnermostFunction(
    v8::Local<v8::Function> function) {
  while (true) {
    v8::Local<v8::Value> bound_function = function->GetBoundFunction();
    if (bound_function->IsFunction()) {
      function = bound_function.As<v8::Function>();
    } else {
      break;
    }
  }

  return function;
}

static int GetListenerScriptId(blink::EventTarget* event_target,
                               blink::EventListener* listener) {
  blink::JSBasedEventListener* const js_listener =
      DynamicTo<blink::JSBasedEventListener>(listener);
  if (!js_listener) {
    return 0;
  }

  v8::HandleScope handle_scope(
      event_target->GetExecutionContext()->GetIsolate());
  v8::Local<v8::Value> maybe_listener_function =
      js_listener->GetEffectiveFunction(*event_target);
  if (!maybe_listener_function->IsFunction()) {
    return 0;
  }

  v8::Local<v8::Function> listener_function =
      GetInnermostFunction(maybe_listener_function.As<v8::Function>());
  return listener_function->ScriptId();
}

}  // namespace

PageGraph::PageGraph(blink::LocalFrame* local_frame)
    : local_frame_(local_frame),
      frame_id_(blink::IdentifiersFactory::FrameId(local_frame_).Utf8()),
      shields_node_(new NodeShields(this)),
      ad_shield_node_(new NodeShield(this, brave_shields::kAds)),
      tracker_shield_node_(new NodeShield(this, brave_shields::kTrackers)),
      js_shield_node_(new NodeShield(this, brave_shields::kJavaScript)),
      fingerprinting_shield_node_(
          new NodeShield(this, brave_shields::kFingerprintingV2)),
      storage_node_(new NodeStorageRoot(this)),
      cookie_jar_node_(new NodeStorageCookieJar(this)),
      local_storage_node_(new NodeStorageLocalStorage(this)),
      session_storage_node_(new NodeStorageSessionStorage(this)),
      start_(base::TimeTicks::Now()) {
  DCHECK(local_frame_->IsLocalRoot());

  AddNode(shields_node_);
  AddShieldNode(ad_shield_node_);
  AddShieldNode(tracker_shield_node_);
  AddShieldNode(js_shield_node_);
  AddShieldNode(fingerprinting_shield_node_);

  AddNode(storage_node_);
  AddStorageNode(cookie_jar_node_);
  AddStorageNode(local_storage_node_);
  AddStorageNode(session_storage_node_);
}

PageGraph::~PageGraph() = default;

void PageGraph::Trace(blink::Visitor* visitor) const {
  visitor->Trace(local_frame_);
  visitor->Trace(execution_context_nodes_);
}

void PageGraph::DidCommitLoad(blink::LocalFrame* local_frame,
                              blink::DocumentLoader* loader) {
  blink::Document* document = local_frame->GetDocument();
  DCHECK(document);

  if (!document->IsHTMLDocument()) {
    VLOG(1) << "Skipping DidCommitLoad. !IsHTMLDocument()";
    return;
  }

  const KURL normalized_url = NormalizeUrl(document->Url());
  const std::string local_url(normalized_url.GetString().Utf8());
  static_cast<NodeDOMRoot*>(
      GetHTMLElementNode(blink::DOMNodeIds::IdForNode(document)))
      ->SetURL(local_url);
}

NodeHTML* PageGraph::GetHTMLNode(const DOMNodeId node_id) const {
  PG_LOG_ASSERT(element_nodes_.count(node_id) + text_nodes_.count(node_id) ==
                1);
  if (element_nodes_.count(node_id) == 1) {
    return element_nodes_.at(node_id);
  }
  return text_nodes_.at(node_id);
}

NodeHTMLElement* PageGraph::GetHTMLElementNode(const DOMNodeId node_id) const {
  VLOG(1) << "GetHTMLElementNode) node id: " << node_id;
  PG_LOG_ASSERT(element_nodes_.count(node_id) == 1);
  return element_nodes_.at(node_id);
}

NodeHTMLText* PageGraph::GetHTMLTextNode(const DOMNodeId node_id) const {
  PG_LOG_ASSERT(text_nodes_.count(node_id) == 1);
  return text_nodes_.at(node_id);
}

NodeScript* PageGraph::GetScriptNode(blink::ExecutionContext* execution_context,
                                     const ScriptId script_id) {
  VLOG(1) << "GetScriptNode) script id: " << script_id;
  auto& script_nodes = GetScriptNodes(execution_context);
  PG_LOG_ASSERT(script_nodes.count(script_id) == 1);
  return script_nodes.at(script_id);
}

void PageGraph::RegisterRemoteFrameCreated(blink::Node* parent_node,
                                           const String& frame_id) {
  /*const blink::DOMNodeId parent_node_id =
      blink::DOMNodeIds::IdForNode(parent_node);
  const std::string local_frame_id(frame_id.Utf8().data());

  VLOG(1) << "RegisterRemoteFrameCreated) parent node id: " << parent_node_id
          << ", frame id: " << local_frame_id;

  SpeculativelyRegisterCurrentlyConstructedNode(parent_node_id);
  PG_LOG_ASSERT(element_nodes_.count(parent_node_id) == 1);

  // Create the new remote frame node.
  NodeRemoteFrame* const remote_frame =
      new NodeRemoteFrame(this, local_frame_id);
  AddNode(remote_frame);

  // Add the cross-DOM edge.
  NodeFrameOwner* const parent_graph_node =
      To<NodeFrameOwner>(element_nodes_.at(parent_node_id));
  AddEdge(new EdgeCrossDOM(this, parent_graph_node, remote_frame));*/
}

void PageGraph::RegisterDocumentNodeCreated(blink::Document* node) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  VLOG(1) << "RegisterDocumentNodeCreated) node id: " << node_id
          << " execution context: " << node->GetExecutionContext();
  v8::Isolate* const isolate = node->GetExecutionContext()->GetIsolate();
  if (isolate) {
    static base::NoDestructor<V8PageGraphDelegate> page_graph_delegate;
    v8::page_graph::SetPageGraphDelegate(isolate, page_graph_delegate.get());
  }
  const std::string local_tag_name(
      static_cast<blink::Node*>(node)->nodeName().Utf8());
  auto* dom_root = new NodeDOMRoot(this, node_id, local_tag_name);
  AddNode(dom_root);
  PG_LOG_ASSERT(element_nodes_.count(node_id) == 0);
  element_nodes_.emplace(node_id, dom_root);

  NodeParser* parser_node = nullptr;
  auto execution_context_nodes_it =
      execution_context_nodes_.find(node->GetExecutionContext());
  if (execution_context_nodes_it == execution_context_nodes_.end()) {
    ExecutionContextNodes nodes{
        .parser_node = new NodeParser(this),
        .extensions_node = new NodeExtensions(this),
    };
    parser_node = nodes.parser_node;
    AddNode(nodes.parser_node);
    AddNode(nodes.extensions_node);
    AddEdge(new EdgeStructure(this, nodes.parser_node, nodes.extensions_node));
    execution_context_nodes_.insert(node->GetExecutionContext(),
                                    std::move(nodes));
  } else {
    parser_node = execution_context_nodes_it->value.parser_node;
  }

  NodeActor* acting_node = nullptr;
  if (blink::HTMLFrameOwnerElement* owner = node->LocalOwner()) {
    acting_node = GetCurrentActingNode(owner->GetExecutionContext());
    if (parser_node) {
      AddEdge(new EdgeCrossDOM(this, acting_node, parser_node));
      acting_node = parser_node;
    } else {
      auto* owner_graph_node =
          GetHTMLElementNode(blink::DOMNodeIds::IdForNode(owner));
      AddEdge(new EdgeCrossDOM(
          this, DynamicTo<NodeFrameOwner>(owner_graph_node), dom_root));
    }
  } else if (blink::Document* parent_document = node->ParentDocument()) {
    acting_node = GetCurrentActingNode(parent_document->GetExecutionContext());
    if (parser_node) {
      AddEdge(new EdgeCrossDOM(this, acting_node, parser_node));
      acting_node = parser_node;
    } else {
      auto* parent_document_graph_node =
          GetHTMLElementNode(blink::DOMNodeIds::IdForNode(parent_document));
      AddEdge(new EdgeCrossDOM(
          this, DynamicTo<NodeDOMRoot>(parent_document_graph_node), dom_root));
    }
  } else {
    acting_node = parser_node;
  }
  AddEdge(new EdgeNodeCreate(this, acting_node, dom_root));

  VLOG(1) << "Add document ID: " << node_id;
}

void PageGraph::RegisterHTMLTextNodeCreated(blink::CharacterData* node) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  std::string local_text(node->data().Utf8());

  VLOG(1) << "RegisterHTMLTextNodeCreated) node id: " << node_id
          << ", text"; /*: '" + local_text + "'"*/
  ;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  PG_LOG_ASSERT(text_nodes_.count(node_id) == 0);
  NodeHTMLText* const new_node = new NodeHTMLText(this, node_id, local_text);
  AddNode(new_node);
  text_nodes_.emplace(node_id, new_node);

  AddEdge(new EdgeNodeCreate(this, acting_node, new_node));
}

void PageGraph::RegisterHTMLElementNodeCreated(blink::Node* node) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  std::string local_tag_name(node->nodeName().Utf8());

  VLOG(1) << "RegisterHTMLElementNodeCreated) node id: " << node_id << " ("
          << local_tag_name << ")";
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  PG_LOG_ASSERT(element_nodes_.count(node_id) == 0);

  NodeHTMLElement* new_node = nullptr;
  if (node->IsFrameOwnerElement()) {
    new_node = new NodeFrameOwner(this, node_id, local_tag_name);
    VLOG(1) << "(type = FrameOwnerElement";
  } else {
    new_node = new NodeHTMLElement(this, node_id, local_tag_name);
  }

  AddNode(new_node);
  element_nodes_.emplace(node_id, new_node);

  AddEdge(new EdgeNodeCreate(this, acting_node, new_node));
}

void PageGraph::SpeculativelyRegisterCurrentlyConstructedNode(
    blink::DOMNodeId node_id) {
  auto node_it = currently_constructed_nodes_.find(node_id);
  if (node_it == currently_constructed_nodes_.end()) {
    return;
  }
  if (blink::Node* node = node_it->second.Get()) {
    // Speculative call.
    RegisterPageGraphNodeFullyCreated(node);
    // Node should be removed from currently_constructed_nodes_.
    DCHECK(!base::Contains(currently_constructed_nodes_, node_id));
    // Mark as already registered for the upcoming real
    // RegisterPageGraphNodeFullyCreated call.
    currently_constructed_nodes_.emplace(node_id, nullptr);
  }
}

void PageGraph::RegisterHTMLElementNodeInserted(
    blink::Node* node,
    blink::Node* parent_node,
    const DOMNodeId before_sibling_id) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  const blink::DOMNodeId parent_node_id =
      blink::DOMNodeIds::IdForNode(parent_node);

  VLOG(1) << "RegisterHTMLElementNodeInserted) node id: " << node_id
          << ", parent id: " << parent_node_id
          << ", prev sibling id: " << before_sibling_id;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  PG_LOG_ASSERT(element_nodes_.count(node_id) == 1);
  SpeculativelyRegisterCurrentlyConstructedNode(parent_node_id);
  NodeHTMLElement* parent_graph_node = nullptr;
  auto parent_node_it = element_nodes_.find(parent_node_id);
  if (parent_node_it != element_nodes_.end()) {
    parent_graph_node = parent_node_it->second;
  }
  NodeHTML* prior_graph_sibling_node = nullptr;
  if (before_sibling_id) {
    auto element_node_it = element_nodes_.find(before_sibling_id);
    if (element_node_it != element_nodes_.end()) {
      prior_graph_sibling_node = element_node_it->second;
    } else {
      auto text_node_it = text_nodes_.find(before_sibling_id);
      if (text_node_it != text_nodes_.end()) {
        prior_graph_sibling_node = text_node_it->second;
      }
    }
  }
  PG_LOG_ASSERT(element_nodes_.count(parent_node_id) == 1);
  PG_LOG_ASSERT(before_sibling_id == 0 || prior_graph_sibling_node);
  NodeHTMLElement* const inserted_node = element_nodes_.at(node_id);

  AddEdge(new EdgeNodeInsert(this, acting_node, inserted_node,
                             parent_graph_node, prior_graph_sibling_node));
}

void PageGraph::RegisterHTMLTextNodeInserted(
    blink::Node* node,
    blink::Node* parent_node,
    const DOMNodeId before_sibling_id) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  const blink::DOMNodeId parent_node_id =
      blink::DOMNodeIds::IdForNode(parent_node);

  VLOG(1) << "RegisterHTMLTextNodeInserted) node id: " << node_id
          << ", parent id: " << parent_node_id
          << ", prev sibling id: " << before_sibling_id;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  PG_LOG_ASSERT(text_nodes_.count(node_id) == 1);
  SpeculativelyRegisterCurrentlyConstructedNode(parent_node_id);
  NodeHTMLElement* parent_graph_node = nullptr;
  auto parent_node_it = element_nodes_.find(parent_node_id);
  if (parent_node_it != element_nodes_.end()) {
    parent_graph_node = parent_node_it->second;
  }
  NodeHTML* prior_graph_sibling_node = nullptr;
  if (before_sibling_id) {
    auto element_node_it = element_nodes_.find(before_sibling_id);
    if (element_node_it != element_nodes_.end()) {
      prior_graph_sibling_node = element_node_it->second;
    } else {
      auto text_node_it = text_nodes_.find(before_sibling_id);
      if (text_node_it != text_nodes_.end()) {
        prior_graph_sibling_node = text_node_it->second;
      }
    }
  }
  NodeHTMLText* const inserted_node = text_nodes_.at(node_id);

  AddEdge(new EdgeNodeInsert(this, acting_node, inserted_node,
                             parent_graph_node, prior_graph_sibling_node));
}

void PageGraph::RegisterHTMLElementNodeRemoved(blink::Node* node) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  VLOG(1) << "RegisterHTMLElementNodeRemoved) node id: " << node_id;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  PG_LOG_ASSERT(element_nodes_.count(node_id) == 1);
  NodeHTMLElement* const removed_node = element_nodes_.at(node_id);

  AddEdge(new EdgeNodeRemove(this, static_cast<NodeScript*>(acting_node),
                             removed_node));
}

void PageGraph::RegisterHTMLTextNodeRemoved(blink::Node* node) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  VLOG(1) << "RegisterHTMLTextNodeRemoved) node id: " << node_id;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  PG_LOG_ASSERT(text_nodes_.count(node_id) == 1);
  NodeHTMLText* const removed_node = text_nodes_.at(node_id);

  AddEdge(new EdgeNodeRemove(this, static_cast<NodeScript*>(acting_node),
                             removed_node));
}

void PageGraph::RegisterEventListenerAdd(blink::Node* node,
                                         const String& event_type,
                                         const EventListenerId listener_id,
                                         ScriptId listener_script_id) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  std::string local_event_type(event_type.Utf8().data());

  VLOG(1) << "RegisterEventListenerAdd) node id: " << node_id
          << ", event_type: " + local_event_type
          << ", listener_id: " << listener_id
          << ", listener_script_id: " << listener_script_id;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  SpeculativelyRegisterCurrentlyConstructedNode(node_id);
  PG_LOG_ASSERT(element_nodes_.count(node_id) == 1);
  NodeHTMLElement* const element_node = element_nodes_.at(node_id);

  AddEdge(new EdgeEventListenerAdd(
      this, acting_node, element_node, local_event_type, listener_id,
      GetScriptNode(node->GetExecutionContext(), listener_script_id)));
}

void PageGraph::RegisterEventListenerRemove(blink::Node* node,
                                            const String& event_type,
                                            const EventListenerId listener_id,
                                            ScriptId listener_script_id) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  std::string local_event_type(event_type.Utf8().data());

  VLOG(1) << "RegisterEventListenerRemove) node id: " << node_id
          << ", event_type: " + local_event_type
          << ", listener_id: " << listener_id
          << ", listener_script_id: " << listener_script_id;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  SpeculativelyRegisterCurrentlyConstructedNode(node_id);
  PG_LOG_ASSERT(element_nodes_.count(node_id) == 1);
  NodeHTMLElement* const element_node = element_nodes_.at(node_id);

  AddEdge(new EdgeEventListenerRemove(
      this, acting_node, element_node, local_event_type, listener_id,
      GetScriptNode(node->GetExecutionContext(), listener_script_id)));
}

void PageGraph::RegisterInlineStyleSet(blink::Node* node,
                                       const String& attr_name,
                                       const String& attr_value) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  std::string local_attr_name(attr_name.Utf8().data());
  std::string local_attr_value(attr_value.Utf8().data());

  VLOG(1) << "RegisterInlineStyleSet) node id: " << node_id
          << ", attr: " << local_attr_name << ", value: " << local_attr_value;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  SpeculativelyRegisterCurrentlyConstructedNode(node_id);
  PG_LOG_ASSERT(element_nodes_.count(node_id) == 1);
  NodeHTMLElement* const target_node = element_nodes_.at(node_id);

  AddEdge(new EdgeAttributeSet(this, acting_node, target_node, local_attr_name,
                               local_attr_value, true));
}

void PageGraph::RegisterInlineStyleDelete(blink::Node* node,
                                          const String& attr_name) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  std::string local_attr_name(attr_name.Utf8().data());

  VLOG(1) << "RegisterInlineStyleDelete) node id: " << node_id
          << ", attr: " << local_attr_name;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  SpeculativelyRegisterCurrentlyConstructedNode(node_id);
  PG_LOG_ASSERT(element_nodes_.count(node_id) == 1);
  NodeHTMLElement* const target_node = element_nodes_.at(node_id);

  AddEdge(new EdgeAttributeDelete(this, acting_node, target_node,
                                  local_attr_name, true));
}

void PageGraph::RegisterAttributeSet(blink::Node* node,
                                     const String& attr_name,
                                     const String& attr_value) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  std::string local_attr_name(attr_name.Utf8().data());
  std::string local_attr_value(attr_value.Utf8().data());

  VLOG(1) << "RegisterAttributeSet) node id: " << node_id
          << ", attr: " << local_attr_name << ", value: " << local_attr_value;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  SpeculativelyRegisterCurrentlyConstructedNode(node_id);
  PG_LOG_ASSERT(element_nodes_.count(node_id) == 1);
  NodeHTMLElement* const target_node = element_nodes_.at(node_id);

  AddEdge(new EdgeAttributeSet(this, acting_node, target_node, local_attr_name,
                               local_attr_value));
}

void PageGraph::RegisterAttributeDelete(blink::Node* node,
                                        const String& attr_name) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  std::string local_attr_name(attr_name.Utf8().data());

  VLOG(1) << "RegisterAttributeDelete) node id: " << node_id
          << ", attr: " << local_attr_name;
  NodeActor* const acting_node =
      GetCurrentActingNode(node->GetExecutionContext());

  SpeculativelyRegisterCurrentlyConstructedNode(node_id);
  PG_LOG_ASSERT(element_nodes_.count(node_id) == 1);
  NodeHTMLElement* const target_node = element_nodes_.at(node_id);

  AddEdge(
      new EdgeAttributeDelete(this, acting_node, target_node, local_attr_name));
}

void PageGraph::RegisterTextNodeChange(blink::Node* node,
                                       const String& new_text) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  VLOG(1) << "RegisterNewTextNodeText) node id: " << node_id;
  NodeScript* const acting_node = static_cast<NodeScript*>(
      GetCurrentActingNode(node->GetExecutionContext()));

  SpeculativelyRegisterCurrentlyConstructedNode(node_id);
  PG_LOG_ASSERT(text_nodes_.count(node_id) == 1);
  NodeHTMLText* const text_node = text_nodes_.at(node_id);

  std::string local_new_text(new_text.Utf8().data());

  AddEdge(new EdgeTextChange(this, acting_node, text_node, local_new_text));
}

void PageGraph::DoRegisterRequestStart(const InspectorId request_id,
                                       Node* const requesting_node,
                                       const std::string& local_url,
                                       const blink::ResourceType resource_type,
                                       const RequestType type) {
  NodeResource* const requested_node = GetResourceNodeForUrl(local_url);

  scoped_refptr<const TrackedRequestRecord> request_record =
      request_tracker_.RegisterRequestStart(
          request_id, requesting_node, requested_node, resource_type, type);

  PossiblyWriteRequestsIntoGraph(std::move(request_record));
}

void PageGraph::RegisterRequestStartFromElm(
    const DOMNodeId node_id,
    const InspectorId request_id,
    const KURL& url,
    const blink::ResourceType resource_type,
    const RequestType type) {
  const KURL normalized_url = NormalizeUrl(url);
  const std::string local_url(normalized_url.GetString().Utf8());

  // For now, explode if we're getting duplicate requests for the same
  // URL in the same document.  This might need to be changed.
  VLOG(1) << "RegisterRequestStartFromElm) node id: " << node_id
          << ", request id: " << request_id << ", url: " << local_url
          << ", type: " << type;

  // We should know about the node thats issuing the request.
  PG_LOG_ASSERT(element_nodes_.count(node_id) == 1);

  NodeHTMLElement* const requesting_node = element_nodes_.at(node_id);
  DoRegisterRequestStart(request_id, requesting_node, local_url, resource_type,
                         type);
}

void PageGraph::RegisterRequestStartFromCurrentScript(
    blink::ExecutionContext* execution_context,
    const InspectorId request_id,
    const KURL& url,
    const blink::ResourceType resource_type,
    const RequestType type) {
  VLOG(1) << "RegisterRequestStartFromCurrentScript)";
  const ScriptId script_id = GetExecutingScriptId(execution_context);
  RegisterRequestStartFromScript(execution_context, script_id, request_id, url,
                                 resource_type, type);
}

void PageGraph::RegisterRequestStartFromScript(
    blink::ExecutionContext* execution_context,
    const ScriptId script_id,
    const InspectorId request_id,
    const blink::KURL& url,
    const blink::ResourceType resource_type,
    const RequestType type) {
  const KURL normalized_url = NormalizeUrl(url);
  const std::string local_url(normalized_url.GetString().Utf8().data());

  VLOG(1) << "RegisterRequestStartFromScript) script id: " << script_id
          << " request id: " << request_id << ", url: " << local_url
          << ", type: " << type;
  NodeActor* const acting_node =
      GetNodeActorForScriptId(execution_context, script_id);

  DoRegisterRequestStart(request_id, acting_node, local_url, resource_type,
                         type);
}

// This is basically the same as |RegisterRequestStartFromCurrentScript|,
// except we don't require the acting node to be a script (CSS fetches
// can be initiated by the parser).
void PageGraph::RegisterRequestStartFromCSSOrLink(
    blink::DocumentLoader* loader,
    const InspectorId request_id,
    const blink::KURL& url,
    const blink::ResourceType resource_type,
    const RequestType type) {
  NodeActor* const acting_node = GetCurrentActingNode(
      loader->GetFrame()->GetDocument()->GetExecutionContext());
  const KURL normalized_url = NormalizeUrl(url);
  const std::string local_url(normalized_url.GetString().Utf8().data());

  if (IsA<NodeParser>(acting_node)) {
    VLOG(1) << "RegisterRequestStartFromCSSOrLink) request id: " << request_id
            << ", url: " << local_url << ", type: " << type;
  } else {
    VLOG(1) << "RegisterRequestStartFromCSSOrLink) script id: "
            << static_cast<NodeScript*>(acting_node)->GetScriptId()
            << ", request id: " << request_id << ", url: " << local_url
            << ", type: " << type;
  }

  DoRegisterRequestStart(request_id, acting_node, local_url, resource_type,
                         type);
}

void PageGraph::RegisterRequestComplete(const InspectorId request_id) {
  VLOG(1) << "RegisterRequestComplete) request id: " << request_id;

  scoped_refptr<const TrackedRequestRecord> request_record =
      request_tracker_.RegisterRequestComplete(request_id);

  PossiblyWriteRequestsIntoGraph(std::move(request_record));
}

// Request start for root document and subdocument HTML
void PageGraph::RegisterRequestStartForDocument(blink::Document* document,
                                                const InspectorId request_id,
                                                const blink::KURL& url,
                                                const bool is_main_frame) {
  const blink::DOMNodeId frame_id = blink::DOMNodeIds::IdForNode(document);
  const base::TimeDelta timestamp = base::TimeTicks::Now() - start_;

  const KURL normalized_url = NormalizeUrl(url);
  const std::string local_url(normalized_url.GetString().Utf8().data());

  VLOG(1) << "RegisterRequestStartForDocument) frame id: " << frame_id
          << ", request id: " << request_id << ", url: " << local_url
          << ", is_main_frame: " << is_main_frame;

  request_tracker_.RegisterDocumentRequestStart(request_id, frame_id, local_url,
                                                is_main_frame, timestamp);
}

// TODO we still need some way to record request errors for documents.
// A good place for this is NavigationBodyLoader::OnConnectionClosed() in
// content/renderer/loader/navigation_body_loader.cc
void PageGraph::RegisterRequestCompleteForDocument(const InspectorId request_id,
                                                   const int64_t size) {
  VLOG(1) << "RegisterRequestCompleteForDocument) request id: " << request_id
          << ", size: " << size;

  const base::TimeDelta timestamp = base::TimeTicks::Now() - start_;
  request_tracker_.RegisterDocumentRequestComplete(request_id, size, timestamp);
}

void PageGraph::RegisterRequestError(const InspectorId request_id) {
  VLOG(1) << "RegisterRequestError) request id: " << request_id;

  scoped_refptr<const TrackedRequestRecord> request_record =
      request_tracker_.RegisterRequestError(request_id);

  PossiblyWriteRequestsIntoGraph(std::move(request_record));
}

void PageGraph::RegisterResourceBlockAd(const blink::WebURL& url,
                                        const std::string& rule) {
  const KURL normalized_url = NormalizeUrl(url);
  const std::string local_url(normalized_url.GetString().Utf8().data());

  VLOG(1) << "RegisterResourceBlockAd) url: " << local_url
          << ", rule: " << rule;

  NodeResource* const resource_node = GetResourceNodeForUrl(local_url);
  NodeAdFilter* const filter_node = GetAdFilterNodeForRule(rule);

  AddEdge(new EdgeResourceBlock(this, filter_node, resource_node));
}

void PageGraph::RegisterResourceBlockTracker(const blink::WebURL& url,
                                             const std::string& host) {
  const KURL normalized_url = NormalizeUrl(url);
  const std::string local_url(normalized_url.GetString().Utf8().data());

  VLOG(1) << "RegisterResourceBlockTracker) url: " << local_url
          << ", host: " << host;

  NodeResource* const resource_node = GetResourceNodeForUrl(local_url);
  NodeTrackerFilter* const filter_node = GetTrackerFilterNodeForHost(host);

  AddEdge(new EdgeResourceBlock(this, filter_node, resource_node));
}

void PageGraph::RegisterResourceBlockJavaScript(const blink::WebURL& url) {
  const KURL normalized_url = NormalizeUrl(url);
  const std::string local_url(normalized_url.GetString().Utf8().data());

  VLOG(1) << "RegisterResourceBlockJavaScript) url: " << local_url;

  NodeResource* const resource_node = GetResourceNodeForUrl(local_url);

  AddEdge(new EdgeResourceBlock(this, js_shield_node_, resource_node));
}

void PageGraph::RegisterResourceBlockFingerprinting(
    const blink::WebURL& url,
    const FingerprintingRule& rule) {
  const KURL normalized_url = NormalizeUrl(url);
  const std::string local_url(normalized_url.GetString().Utf8().data());

  VLOG(1) << "RegisterResourceBlockFingerprinting) url: " << local_url
          << ", rule: " << rule.ToString();

  NodeResource* const resource_node = GetResourceNodeForUrl(local_url);
  NodeFingerprintingFilter* const filter_node =
      GetFingerprintingFilterNodeForRule(rule);

  AddEdge(new EdgeResourceBlock(this, filter_node, resource_node));
}

void PageGraph::RegisterScriptCompilation(
    blink::ExecutionContext* execution_context,
    const ScriptId script_id,
    const ScriptData& script_data) {
  VLOG(1) << "RegisterScriptCompilation) script id: " << script_id
          << " script: "
          << (VLOG_IS_ON(2) ? script_data.code : String("<VLOG(2)>"));

  auto& script_tracker = GetScriptTracker(execution_context);
  script_tracker.AddScript(script_id, script_data);

  NodeScript* const code_node = new NodeScript(this, script_id, script_data);
  AddNode(code_node);
  GetScriptNodes(execution_context).emplace(script_id, code_node);

  if (script_data.source.is_module) {
    // Module scripts are pulled by URL from a parent module script.
    ScriptIdList parent_script_ids =
        script_tracker.GetModuleScriptParentsForScriptId(script_id);
    for (const ScriptId parent_script_id : parent_script_ids) {
      NodeScript* const parent_node =
          GetScriptNode(execution_context, parent_script_id);
      AddEdge(new EdgeExecute(this, parent_node, code_node));
    }

    // If this is a root-level module script, it can still be associated with
    // an HTML script element
    if (parent_script_ids.empty()) {
      DOMNodeIdList node_ids = script_tracker.GetElmsForScriptId(script_id);
      for (const DOMNodeId node_id : node_ids) {
        NodeHTMLElement* const script_elm_node = GetHTMLElementNode(node_id);
        AddEdge(new EdgeExecute(this, script_elm_node, code_node));
      }
    }

    return;
  }

  DOMNodeIdList node_ids = script_tracker.GetElmsForScriptId(script_id);
  if (!node_ids.empty()) {
    for (const DOMNodeId node_id : node_ids) {
      NodeHTMLElement* const script_elm_node = GetHTMLElementNode(node_id);
      AddEdge(new EdgeExecute(this, script_elm_node, code_node));
    }
  } else {
    PG_LOG_ASSERT(execution_context_nodes_.Contains(execution_context));
    AddEdge(new EdgeExecute(
        this, execution_context_nodes_.at(execution_context).extensions_node,
        code_node));
  }
}

void PageGraph::RegisterScriptCompilationFromAttr(
    blink::ExecutionContext* execution_context,
    const ScriptId script_id,
    const ScriptData& script_data) {
  std::string local_attr_name(script_data.source.function_name.Utf8());
  VLOG(1) << "RegisterScriptCompilationFromAttr) script id: " << script_id
          << ", node id: " << script_data.source.node_id << ", attr name: ";
  GetScriptTracker(execution_context).AddScript(script_id, script_data);

  NodeScript* const code_node = new NodeScript(this, script_id, script_data);
  AddNode(code_node);
  GetScriptNodes(execution_context).emplace(script_id, code_node);

  NodeHTMLElement* const html_node =
      GetHTMLElementNode(script_data.source.node_id);
  AddEdge(new EdgeExecuteAttr(this, html_node, code_node, local_attr_name));
}

void PageGraph::RegisterScriptCompilationFromEval(
    blink::ExecutionContext* execution_context,
    const ScriptId script_id,
    const ScriptData& script_data) {
  VLOG(1) << "RegisterScriptCompilationFromEval) script id: " << script_id
          << ", parent script id: " << script_data.source.parent_script_id;
  GetScriptTracker(execution_context).AddScript(script_id, script_data);

  NodeScript* const code_node = new NodeScript(this, script_id, script_data);
  AddNode(code_node);
  GetScriptNodes(execution_context).emplace(script_id, code_node);

  if (script_data.source.parent_script_id) {
    NodeScript* const parent_node =
        GetScriptNode(execution_context, script_data.source.parent_script_id);
    AddEdge(new EdgeExecute(this, parent_node, code_node));
  }
}

// Functions for handling storage read, write, and deletion
void PageGraph::RegisterStorageRead(blink::ExecutionContext* execution_context,
                                    const String& key,
                                    const String& value,
                                    const StorageLocation location) {
  std::string local_key(key.Utf8().data());
  std::string local_value(value.Utf8().data());

  VLOG(1) << "RegisterStorageRead) key: " << local_key
          << ", value: " << local_value
          << ", location: " << StorageLocationToString(location);
  NodeActor* const acting_node = GetCurrentActingNode(execution_context);

  // Optimized(?) calls sometimes generate script_id == 0.
  // PG_LOG_ASSERT(IsA<NodeScript>(acting_node));

  NodeStorage* storage_node = nullptr;
  switch (location) {
    case kStorageLocationCookie:
      storage_node = cookie_jar_node_;
      break;
    case kStorageLocationLocalStorage:
      storage_node = local_storage_node_;
      break;
    case kStorageLocationSessionStorage:
      storage_node = session_storage_node_;
      break;
  }

  AddEdge(new EdgeStorageReadCall(this, static_cast<NodeScript*>(acting_node),
                                  storage_node, local_key));
  AddEdge(new EdgeStorageReadResult(this, storage_node,
                                    static_cast<NodeScript*>(acting_node),
                                    local_key, local_value));
}

void PageGraph::RegisterStorageWrite(blink::ExecutionContext* execution_context,
                                     const String& key,
                                     const String& value,
                                     const StorageLocation location) {
  std::string local_key(key.Utf8().data());
  std::string local_value(value.Utf8().data());

  VLOG(1) << "RegisterStorageWrite) key: " << local_key
          << ", value: " << local_value
          << ", location: " << StorageLocationToString(location);
  NodeActor* const acting_node = GetCurrentActingNode(execution_context);

  // Optimized calls sometime generate script_id == 0.
  // PG_LOG_ASSERT(IsA<NodeScript>(acting_node));

  NodeStorage* storage_node = nullptr;
  switch (location) {
    case kStorageLocationCookie:
      storage_node = cookie_jar_node_;
      break;
    case kStorageLocationLocalStorage:
      storage_node = local_storage_node_;
      break;
    case kStorageLocationSessionStorage:
      storage_node = session_storage_node_;
      break;
  }

  AddEdge(new EdgeStorageSet(this, static_cast<NodeScript*>(acting_node),
                             storage_node, local_key, local_value));
}

void PageGraph::RegisterStorageDelete(
    blink::ExecutionContext* execution_context,
    const String& key,
    const StorageLocation location) {
  std::string local_key(key.Utf8().data());

  VLOG(1) << "RegisterStorageDelete) key: " << local_key
          << ", location: " << StorageLocationToString(location);
  NodeActor* const acting_node = GetCurrentActingNode(execution_context);

  // Optimized calls sometime generate script_id == 0.
  // PG_LOG_ASSERT(IsA<NodeScript>(acting_node));

  NodeStorage* storage_node = nullptr;
  switch (location) {
    case kStorageLocationLocalStorage:
      storage_node = local_storage_node_;
      break;
    case kStorageLocationSessionStorage:
      storage_node = session_storage_node_;
      break;
    case kStorageLocationCookie:
      PG_LOG_ASSERT(location != kStorageLocationCookie);
  }

  AddEdge(new EdgeStorageDelete(this, static_cast<NodeScript*>(acting_node),
                                storage_node, local_key));
}

void PageGraph::RegisterStorageClear(blink::ExecutionContext* execution_context,
                                     const StorageLocation location) {
  VLOG(1) << "RegisterStorageClear) location: "
          << StorageLocationToString(location);
  NodeActor* const acting_node = GetCurrentActingNode(execution_context);

  PG_LOG_ASSERT(IsA<NodeScript>(acting_node));

  NodeStorage* storage_node = nullptr;
  switch (location) {
    case kStorageLocationLocalStorage:
      storage_node = local_storage_node_;
      break;
    case kStorageLocationSessionStorage:
      storage_node = session_storage_node_;
      break;
    case kStorageLocationCookie:
      PG_LOG_ASSERT(location != kStorageLocationCookie);
  }

  AddEdge(new EdgeStorageClear(this, static_cast<NodeScript*>(acting_node),
                               storage_node));
}

void PageGraph::RegisterWebAPICall(blink::ExecutionContext* execution_context,
                                   const MethodName& method,
                                   const std::vector<String>& arguments) {
  std::vector<std::string> local_args;
  std::stringstream buffer;
  buffer << "{";
  for (size_t i = 0; i < arguments.size(); ++i) {
    if (i != 0) {
      buffer << ", ";
    }
    buffer << arguments.at(i);
    local_args.push_back(arguments.at(i).Utf8().data());
  }
  buffer << "}";
  VLOG(2) << "RegisterWebAPICall) method: " << method
          << ", arguments: " << buffer.str();

  ScriptPosition script_position;
  NodeActor* const acting_node =
      GetCurrentActingNode(execution_context, &script_position);
  if (!IsA<NodeScript>(acting_node)) {
    // Ignore internal usage.
    return;
  }

  NodeJSWebAPI* webapi_node;
  if (webapi_nodes_.count(method) == 0) {
    webapi_node = new NodeJSWebAPI(this, method);
    AddNode(webapi_node);
    webapi_nodes_.emplace(method, webapi_node);
  } else {
    webapi_node = webapi_nodes_.at(method);
  }

  AddEdge(new EdgeJSCall(this, static_cast<NodeScript*>(acting_node),
                         webapi_node, local_args, script_position));
}

void PageGraph::RegisterWebAPIResult(blink::ExecutionContext* execution_context,
                                     const MethodName& method,
                                     const String& result) {
  const std::string local_result = result.Utf8().data();
  VLOG(2) << "RegisterWebAPIResult) method: " << method
          << ", result: " << local_result;

  NodeActor* const caller_node = GetCurrentActingNode(execution_context);
  if (!IsA<NodeScript>(caller_node)) {
    // Ignore internal usage.
    return;
  }

  PG_LOG_ASSERT(webapi_nodes_.count(method) != 0);
  NodeJSWebAPI* webapi_node = webapi_nodes_.at(method);

  AddEdge(new EdgeJSResult(
      this, webapi_node, static_cast<NodeScript*>(caller_node), local_result));
}

void PageGraph::RegisterJSBuiltInCall(
    blink::ExecutionContext* execution_context,
    const char* built_in,
    const std::vector<std::string>& arguments) {
  if (VLOG_IS_ON(2)) {
    std::stringstream buffer;
    buffer << "{";
    for (size_t i = 0; i < arguments.size(); ++i) {
      if (i != 0) {
        buffer << ", ";
      }
      buffer << arguments.at(i);
    }
    buffer << "}";
    VLOG(2) << "RegisterJSBuiltInCall) built in: " << built_in
            << ", arguments: " << buffer.str();
  }

  ScriptPosition script_position;
  NodeActor* const acting_node =
      GetCurrentActingNode(execution_context, &script_position);
  if (!IsA<NodeScript>(acting_node)) {
    // Ignore internal usage.
    return;
  }

  NodeJSBuiltIn* js_built_in_node;
  if (builtin_js_nodes_.count(built_in) == 0) {
    js_built_in_node = new NodeJSBuiltIn(this, built_in);
    AddNode(js_built_in_node);
    builtin_js_nodes_.emplace(built_in, js_built_in_node);
  } else {
    js_built_in_node = builtin_js_nodes_.at(built_in);
  }

  AddEdge(new EdgeJSCall(this, static_cast<NodeScript*>(acting_node),
                         js_built_in_node, arguments, script_position));
}

void PageGraph::RegisterJSBuiltInResponse(
    blink::ExecutionContext* execution_context,
    const char* built_in,
    const std::string& value) {
  const std::string local_result(value);
  VLOG(2) << "RegisterJSBuiltInResponse) built in: " << built_in
          << ", result: " << local_result;

  NodeActor* const caller_node = GetCurrentActingNode(execution_context);
  if (!IsA<NodeScript>(caller_node)) {
    // Ignore internal usage.
    return;
  }

  PG_LOG_ASSERT(builtin_js_nodes_.count(built_in) != 0);
  NodeJSBuiltIn* js_built_in_node = builtin_js_nodes_.at(built_in);

  AddEdge(new EdgeJSResult(this, js_built_in_node,
                           static_cast<NodeScript*>(caller_node),
                           local_result));
}

// void PageGraph::RegisterBindingEvent(const Binding binding,
//                                      const BindingType binding_type,
//                                      const BindingEvent binding_event) {
//   VLOG(2) << "RegisterBindingEvent) binding: " << binding
//           << ", event: " << binding_event;

//   NodeBinding* binding_node = nullptr;
//   NodeBindingEvent* binding_event_node = nullptr;

//   GetAllActingNodes([=](NodeActor* const acting_node,
//                         const ScriptPosition script_position) mutable {
//     NodeScript* const script_node = DynamicTo<NodeScript>(acting_node);
//     if (script_node) {
//       if (!binding_node) {
//         binding_node = GetBindingNode(binding, binding_type);
//       }

//       if (!binding_event_node) {
//         binding_event_node = new NodeBindingEvent(this, binding_event);
//         AddNode(binding_event_node);
//         AddEdge(new EdgeBinding(this, binding_event_node, binding_node));
//       }

//       AddEdge(new EdgeBindingEvent(this, script_node, binding_event_node,
//                                    script_position));
//     }
//   });
// }

void PageGraph::GenerateReportForNode(const blink::DOMNodeId node_id,
                                      Array<String>& report) {
  const Node* node;
  if (element_nodes_.count(node_id)) {
    node = element_nodes_.at(node_id);
  } else if (text_nodes_.count(node_id)) {
    node = text_nodes_.at(node_id);
  } else {
    return;
  }

  std::set<const Node*> predecessors;
  for (const std::unique_ptr<const Edge>& elm : Edges()) {
    if (elm->in_node_ == node)
      predecessors.insert(elm->out_node_);
  }

  std::set<const Node*> successors;
  for (const std::unique_ptr<const Edge>& elm : Edges()) {
    if (elm->out_node_ == node)
      successors.insert(elm->in_node_);
  }

  for (const Node* pred : predecessors) {
    if (IsA<NodeActor>(pred)) {
      for (const Edge* edge : pred->out_edges_) {
        if (edge->in_node_ == node) {
          std::string reportItem(edge->GetItemDesc() +
                                 "\r\n\r\nby: " + pred->GetItemDesc());
          report.push_back(String(reportItem.data()));
        }
      }
    }
  }

  for (const Node* succ : successors) {
    ItemName item_name = succ->GetItemName();
    if (item_name.find("resource #") == 0) {
      for (const Edge* edge : succ->in_edges_) {
        std::string reportItem(edge->GetItemDesc() +
                               "\r\n\r\nby: " + edge->out_node_->GetItemDesc());
        report.push_back(String(reportItem.data()));
      }
    }
  }
}

std::string PageGraph::ToGraphML() const {
  GraphItem::StartGraphMLExport(id_counter_);

  xmlDocPtr graphml_doc = xmlNewDoc(BAD_CAST "1.0");
  xmlNodePtr graphml_root_node = xmlNewNode(nullptr, BAD_CAST "graphml");
  xmlDocSetRootElement(graphml_doc, graphml_root_node);

  xmlNewNs(graphml_root_node, BAD_CAST "http://graphml.graphdrawing.org/xmlns",
           nullptr);
  xmlNsPtr xsi_ns = xmlNewNs(
      graphml_root_node, BAD_CAST "http://www.w3.org/2001/XMLSchema-instance",
      BAD_CAST "xsi");
  xmlNewNsProp(graphml_root_node, xsi_ns, BAD_CAST "schemaLocation",
               BAD_CAST
               "http://graphml.graphdrawing.org/xmlns "
               "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd");

  xmlNodePtr desc_container_node =
      xmlNewChild(graphml_root_node, nullptr, BAD_CAST "desc", nullptr);
  xmlNewTextChild(desc_container_node, nullptr, BAD_CAST "version",
                  BAD_CAST kPageGraphVersion);
  xmlNewTextChild(desc_container_node, nullptr, BAD_CAST "about",
                  BAD_CAST kPageGraphUrl);
  xmlNewTextChild(desc_container_node, nullptr, BAD_CAST "is_root",
                  BAD_CAST(IsRootFrame() ? "true" : "false"));
  xmlNewTextChild(desc_container_node, nullptr, BAD_CAST "frame_id",
                  BAD_CAST frame_id_.c_str());

  xmlNodePtr time_container_node =
      xmlNewChild(desc_container_node, nullptr, BAD_CAST "time", nullptr);

  xmlNewTextChild(time_container_node, nullptr, BAD_CAST "start",
                  BAD_CAST base::NumberToString(0).c_str());

  const base::TimeDelta end_time = base::TimeTicks::Now() - start_;
  xmlNewTextChild(
      time_container_node, nullptr, BAD_CAST "end",
      BAD_CAST base::NumberToString(end_time.InMilliseconds()).c_str());

  for (const auto& graphml_attr : GetGraphMLAttrs()) {
    graphml_attr.second->AddDefinitionNode(graphml_root_node);
  }

  xmlNodePtr graph_node =
      xmlNewChild(graphml_root_node, nullptr, BAD_CAST "graph", nullptr);
  xmlSetProp(graph_node, BAD_CAST "id", BAD_CAST "G");
  xmlSetProp(graph_node, BAD_CAST "edgedefault", BAD_CAST "directed");

  for (const std::unique_ptr<Node>& elm : Nodes()) {
    elm->AddGraphMLTag(graphml_doc, graph_node);
  }
  for (const std::unique_ptr<const Edge>& elm : Edges()) {
    elm->AddGraphMLTag(graphml_doc, graph_node);
  }

  xmlChar* xml_string;
  int size;
  xmlDocDumpMemoryEnc(graphml_doc, &xml_string, &size, "UTF-8");
  const std::string graphml_string(reinterpret_cast<const char*>(xml_string));

  xmlFree(xml_string);
  xmlFree(graphml_doc);

  return graphml_string;
}

base::TimeTicks PageGraph::GetStartTime() const {
  return start_;
}

NodeActor* PageGraph::GetCurrentActingNode(
    blink::ExecutionContext* execution_context,
    ScriptPosition* out_script_position) {
  const ScriptId current_script_id =
      GetExecutingScriptId(execution_context, out_script_position);

  static ScriptId last_reported_script_id = 0;
  const bool should_log = last_reported_script_id != current_script_id;
  last_reported_script_id = current_script_id;
  if (should_log) {
    VLOG(1) << "GetCurrentActingNode) script id: " << current_script_id;
  }

  if (current_script_id != 0) {
    return GetNodeActorForScriptId(execution_context, current_script_id);
  }

  PG_LOG_ASSERT(execution_context_nodes_.Contains(execution_context));
  return execution_context_nodes_.at(execution_context).parser_node;
}

// template <typename Callback>
// void PageGraph::GetAllActingNodes(Callback callback) {
//   GetAllExecutingScripts([=](const ScriptId script_id,
//                              const ScriptPosition script_position) mutable {
//     callback(GetNodeActorForScriptId(script_id), script_position);
//   });
// }

NodeActor* PageGraph::GetNodeActorForScriptId(
    blink::ExecutionContext* execution_context,
    const ScriptId script_id) {
  PG_LOG_ASSERT(GetScriptNodes(execution_context).count(script_id) == 1)
      << "script_id " << script_id;
  return GetScriptNodes(execution_context).at(script_id);
}

ScriptId PageGraph::GetExecutingScriptId(
    blink::ExecutionContext* execution_context,
    ScriptPosition* out_script_position) const {
  auto executing_script = v8::page_graph::GetExecutingScript(
      execution_context->GetIsolate(), out_script_position != nullptr);
  if (out_script_position) {
    *out_script_position = executing_script.script_position;
  }
  return executing_script.script_id;
}

// template <typename Callback>
// void PageGraph::GetAllExecutingScripts(Callback callback) {
//   for (const auto& executing_script : v8::page_graph::GetAllExecutingScripts(
//            GetExecutionContext()->GetIsolate())) {
//     callback(executing_script.script_id, executing_script.script_position);
//   }
// }

NodeResource* PageGraph::GetResourceNodeForUrl(const std::string& url) {
  if (resource_nodes_.count(url) == 0) {
    NodeResource* const resource_node = new NodeResource(this, url);
    AddNode(resource_node);
    resource_nodes_.emplace(url, resource_node);
    return resource_node;
  }

  return resource_nodes_.at(url);
}

NodeAdFilter* PageGraph::GetAdFilterNodeForRule(const std::string& rule) {
  if (ad_filter_nodes_.count(rule) == 0) {
    NodeAdFilter* const filter_node = new NodeAdFilter(this, rule);
    AddNode(filter_node);
    ad_filter_nodes_.emplace(rule, filter_node);

    AddEdge(new EdgeFilter(this, ad_shield_node_, filter_node));

    return filter_node;
  }

  return ad_filter_nodes_.at(rule);
}

NodeTrackerFilter* PageGraph::GetTrackerFilterNodeForHost(
    const std::string& host) {
  if (tracker_filter_nodes_.count(host) == 0) {
    NodeTrackerFilter* const filter_node = new NodeTrackerFilter(this, host);
    AddNode(filter_node);
    tracker_filter_nodes_.emplace(host, filter_node);

    AddEdge(new EdgeFilter(this, tracker_shield_node_, filter_node));

    return filter_node;
  }

  return tracker_filter_nodes_.at(host);
}

NodeFingerprintingFilter* PageGraph::GetFingerprintingFilterNodeForRule(
    const FingerprintingRule& rule) {
  if (fingerprinting_filter_nodes_.count(rule) == 0) {
    NodeFingerprintingFilter* const filter_node =
        new NodeFingerprintingFilter(this, rule);
    AddNode(filter_node);
    fingerprinting_filter_nodes_.emplace(rule, filter_node);

    AddEdge(new EdgeFilter(this, fingerprinting_shield_node_, filter_node));

    return filter_node;
  }

  return fingerprinting_filter_nodes_.at(rule);
}

NodeBinding* PageGraph::GetBindingNode(const Binding binding,
                                       const BindingType binding_type) {
  if (binding_nodes_.count(binding) == 0) {
    NodeBinding* const binding_node =
        new NodeBinding(this, binding, binding_type);
    AddNode(binding_node);
    binding_nodes_.emplace(binding, binding_node);
    return binding_node;
  }

  return binding_nodes_.at(binding);
}

void PageGraph::PossiblyWriteRequestsIntoGraph(
    scoped_refptr<const TrackedRequestRecord> record) {
  const TrackedRequest* const request = record->request.get();

  // Don't record anything into the graph if we've already recorded
  // this batch of requests (first condition) or if this batch of requests
  // hasn't finished yet (e.g. we don't have both a request and a response)
  // (second condition).
  if (!record->is_first_reply || !request->IsComplete()) {
    VLOG(1) << "Not (yet) writing request id: " << request->GetRequestId();
    return;
  }

  NodeResource* const resource = request->GetResource();
  const bool was_error = request->GetIsError();
  const RequestType request_type = request->GetRequestType();
  const InspectorId request_id = request->GetRequestId();

  if (was_error) {
    // Handling the case when the requests returned with errors.
    for (Node* const requester : request->GetRequesters()) {
      AddEdge(new EdgeRequestStart(this, requester, resource, request_id,
                                   request_type));
      AddEdge(new EdgeRequestError(this, resource, requester, request_id,
                                   request->GetResponseMetadata()));
    }
  } else {
    const ResourceType resource_type = request->GetResourceType();
    for (Node* const requester : request->GetRequesters()) {
      AddEdge(new EdgeRequestStart(this, requester, resource, request_id,
                                   request_type));
      AddEdge(new EdgeRequestComplete(
          this, resource, requester, request_id, resource_type,
          request->GetResponseMetadata(), request->GetResponseBodyHash()));
    }
    return;
  }
}

void PageGraph::WriteDocumentRequestIntoGraph(const DOMNodeId initiator,
                                              const DocumentRequest& request) {
  NodeResource* const resource_node = new NodeResource(this, request.url);
  NodeHTML* requester = GetHTMLNode(initiator);

  const RequestType request_type = request.is_main_frame
                                       ? RequestType::kRequestTypeDocument
                                       : RequestType::kRequestTypeSubdocument;

  AddNode(resource_node);

  AddEdge(new EdgeRequestStart(this, requester, resource_node,
                               request.request_id, request_type));
  // TODO blink::ResourceType does not include types for Document or
  // Subdocuments. We'll need to use something else here.
  // TODO for normal requests, we pass response metadata here. We don't get
  // access to that from DocumentLoader, so it's omitted. It may be possible
  // to get it from NavigationBodyLoader::ReadFromDataPipe.
  AddEdge(new EdgeRequestComplete(this, resource_node, requester,
                                  request.request_id,
                                  blink::ResourceType::kRaw /*TODO*/,
                                  ResponseMetadata() /*TODO*/, "" /*TODO*/));
}

const NodeUniquePtrList& PageGraph::Nodes() const {
  return nodes_;
}

const EdgeUniquePtrList& PageGraph::Edges() const {
  return edges_;
}

const GraphItemList& PageGraph::GraphItems() const {
  return graph_items_;
}

void PageGraph::AddNode(Node* const node) {
  nodes_.push_back(std::unique_ptr<Node>(node));
  graph_items_.push_back(node);
}

void PageGraph::AddEdge(const Edge* const edge) {
  edges_.push_back(std::unique_ptr<const Edge>(edge));
  graph_items_.push_back(edge);

  edge->GetInNode()->AddInEdge(edge);
  edge->GetOutNode()->AddOutEdge(edge);
}

void PageGraph::AddShieldNode(NodeShield* const shield_node) {
  AddNode(shield_node);
  AddEdge(new EdgeShield(this, shields_node_, shield_node));
}

void PageGraph::AddStorageNode(NodeStorage* const storage_node) {
  AddNode(storage_node);
  AddEdge(new EdgeStorageBucket(this, storage_node_, storage_node));
}

bool PageGraph::IsRootFrame() const {
  return local_frame_->IsLocalRoot();
}

void PageGraph::DidInsertDOMNode(blink::Node* node) {
  if (!IsA<blink::Document>(node)) {
    blink::Node* const parent = node->parentNode();

    if (parent) {
      blink::Node* const sibling = node->previousSibling();
      const blink::DOMNodeId sibling_node_id =
          (sibling) ? blink::DOMNodeIds::IdForNode(sibling) : 0;

      if (!IsA<blink::CharacterData>(node)) {
        RegisterHTMLElementNodeInserted(node, parent, sibling_node_id);
      } else {
        RegisterHTMLTextNodeInserted(node, parent, sibling_node_id);
      }
    }
  }
}

void PageGraph::WillRemoveDOMNode(blink::Node* node) {
  if (!IsA<blink::CharacterData>(node)) {
    RegisterHTMLElementNodeRemoved(node);
  } else {
    RegisterHTMLTextNodeRemoved(node);
  }
}

void PageGraph::NodeCreated(blink::Node* node) {
  currently_constructed_nodes_.emplace(blink::DOMNodeIds::IdForNode(node),
                                       node);
}

void PageGraph::RegisterPageGraphNodeFullyCreated(blink::Node* node) {
  const blink::DOMNodeId node_id = blink::DOMNodeIds::IdForNode(node);
  auto node_it = currently_constructed_nodes_.find(node_id);
  if (node_it != currently_constructed_nodes_.end()) {
    const bool is_already_registered = node_it->second == nullptr;
    currently_constructed_nodes_.erase(node_it);
    if (is_already_registered) {
      return;
    }
  }

  if (auto* document_node = DynamicTo<blink::Document>(node)) {
    RegisterDocumentNodeCreated(document_node);
    return;
  }

  if (auto* character_data_node = DynamicTo<blink::CharacterData>(node)) {
    RegisterHTMLTextNodeCreated(character_data_node);
    return;
  }

  RegisterHTMLElementNodeCreated(node);
}

void PageGraph::RegisterPageGraphBindingEvent(
    blink::ExecutionContext*,
    base::StringPiece name,
    blink::PageGraphBindingType type,
    blink::PageGraphBindingEvent event) {
  // Disable RegisterBindingEvent in favor of
  // RegisterPageGraphWebAPICallWithResult.

  // RegisterBindingEvent(name.c_str(), type, event);
}

void PageGraph::RegisterPageGraphWebAPICallWithResult(
    blink::ExecutionContext* execution_context,
    base::StringPiece name,
    const blink::PageGraphBlinkReceiverData& receiver_data,
    const blink::PageGraphBlinkArgs& args,
    const blink::ExceptionState* exception_state,
    const absl::optional<String>& result) {
  if (name == "Document.cookie.get") {
    RegisterStorageRead(execution_context, receiver_data.at("cookie_url"),
                        *result, brave_page_graph::kStorageLocationCookie);
    return;
  }
  if (name == "Document.cookie.set") {
    String value = args[0];
    Vector<String> cookie_structure;
    value.Split("=", cookie_structure);
    String cookie_key = *(cookie_structure.begin());
    String cookie_value =
        value.Substring(cookie_key.length() + 1, value.length());
    RegisterStorageWrite(execution_context, cookie_key, cookie_value,
                         brave_page_graph::kStorageLocationCookie);
    return;
  }
  if (base::StartsWith(name, "Storage.")) {
    const String& storage_type = receiver_data.at("storage_type");
    DCHECK(storage_type == "localStorage" || storage_type == "sessionStorage");
    const auto storage = storage_type == "localStorage"
                             ? kStorageLocationLocalStorage
                             : kStorageLocationSessionStorage;
    if (name == "Storage.getItem") {
      DCHECK(result);
      RegisterStorageRead(execution_context, args[0], *result, storage);
      return;
    }
    if (name == "Storage.setItem") {
      RegisterStorageWrite(execution_context, args[0], args[1], storage);
      return;
    }
    if (name == "Storage.removeItem") {
      RegisterStorageDelete(execution_context, args[0], storage);
      return;
    }
    if (name == "Storage.clear") {
      RegisterStorageClear(execution_context, storage);
      return;
    }
  }
  RegisterWebAPICall(execution_context, name.data(), args);
  if (result)
    RegisterWebAPIResult(execution_context, name.data(), *result);
}

void PageGraph::ConsoleMessageAdded(blink::ConsoleMessage* console_message) {
  const auto source = console_message->Source();
  if (source == blink::mojom::ConsoleMessageSource::kJavaScript ||
      source == blink::mojom::ConsoleMessageSource::kConsoleApi) {
    const auto level = console_message->Level();
    base::Value dict{base::Value::Type::DICTIONARY};

    const char* source_name = nullptr;
    switch (source) {
      case blink::mojom::ConsoleMessageSource::kJavaScript:
        source_name = "JavaScript";
        break;
      case blink::mojom::ConsoleMessageSource::kConsoleApi:
        source_name = "ConsoleApi";
        break;
      default:
        source_name = "Unknown";
    }
    dict.SetStringKey("source", source_name);

    const char* level_name = "Unknown";
    switch (level) {
      case blink::mojom::ConsoleMessageLevel::kVerbose:
        level_name = "Verbose";
        break;
      case blink::mojom::ConsoleMessageLevel::kInfo:
        level_name = "Info";
        break;
      case blink::mojom::ConsoleMessageLevel::kWarning:
        level_name = "Warning";
        break;
      case blink::mojom::ConsoleMessageLevel::kError:
        level_name = "Error";
        break;
    }
    dict.SetStringKey("level", level_name);

    dict.SetStringKey("message", console_message->Message().Utf8());

    base::Value loc{base::Value::Type::DICTIONARY};
    loc.SetStringKey("url", console_message->Location()->Url().Utf8());
    loc.SetIntKey("line", console_message->Location()->LineNumber());
    loc.SetIntKey("column", console_message->Location()->ColumnNumber());
    loc.SetIntKey("scriptId", console_message->Location()->ScriptId());
    dict.SetKey("location", std::move(loc));

    std::string output;
    JSONStringValueSerializer serializer{&output};
    CHECK(serializer.Serialize(dict));

    RegisterWebAPICall(local_frame_->GetDocument()->GetExecutionContext(),
                       "console.log", {String::FromUTF8(output)});
  }
}

void PageGraph::DidModifyDOMAttr(blink::Element* element,
                                 const blink::QualifiedName& name,
                                 const AtomicString& value) {
  RegisterAttributeSet(element, name.ToString(), value);
}

void PageGraph::DidRemoveDOMAttr(blink::Element* element,
                                 const blink::QualifiedName& name) {
  RegisterAttributeDelete(element, name.ToString());
}

void PageGraph::WillSendNavigationRequest(uint64_t identifier,
                                          blink::DocumentLoader* loader,
                                          const blink::KURL& url,
                                          const AtomicString& http_method,
                                          blink::EncodedFormData*) {
  RegisterRequestStartForDocument(loader->GetFrame()->GetDocument(), identifier,
                                  url, loader->GetFrame()->IsMainFrame());
}

void PageGraph::PrepareRequest(blink::DocumentLoader* loader,
                               blink::ResourceRequest& request,
                               blink::ResourceLoaderOptions& options,
                               blink::ResourceType resource_type) {
  if (request.GetRedirectInfo()) {
    LOG(INFO) << "Skip redirect request prepare";
    return;
  }

  blink::ExecutionContext* execution_context =
      loader->GetFrame()->GetDocument()->GetExecutionContext();

  RequestType request_type = kRequestTypeUnknown;
  switch (resource_type) {
    case blink::ResourceType::kImage:
      request_type = kRequestTypeImage;
      break;
    case blink::ResourceType::kCSSStyleSheet:
      request_type = kRequestTypeCSS;
      break;
    case blink::ResourceType::kScript:
      request_type = kRequestTypeScriptClassic;
      break;
    case blink::ResourceType::kFont:
      request_type = kRequestTypeFont;
      break;
    case blink::ResourceType::kRaw:
      break;
    case blink::ResourceType::kSVGDocument:
      request_type = kRequestTypeSVG;
      break;
    case blink::ResourceType::kXSLStyleSheet:
      break;
    case blink::ResourceType::kLinkPrefetch:
      break;
    case blink::ResourceType::kTextTrack:
      break;
    case blink::ResourceType::kAudio:
      request_type = kRequestTypeAudio;
      break;
    case blink::ResourceType::kVideo:
      request_type = kRequestTypeVideo;
      break;
    case blink::ResourceType::kManifest:
    case blink::ResourceType::kMock:
      break;
  }

  // Unhandled page graph request types:
  // kRequestTypeDocument,
  // kRequestTypeScriptModule,
  // kRequestTypeSubdocument,

  if (options.initiator_info.dom_node_id != blink::kInvalidDOMNodeId) {
    RegisterRequestStartFromElm(options.initiator_info.dom_node_id,
                                request.InspectorId(), request.Url(),
                                resource_type, request_type);
    return;
  }

  if (options.initiator_info.name == blink::fetch_initiator_type_names::kCSS ||
      options.initiator_info.name ==
          blink::fetch_initiator_type_names::kUacss ||
      options.initiator_info.name == blink::fetch_initiator_type_names::kLink ||
      resource_type == blink::ResourceType::kLinkPrefetch) {
    RegisterRequestStartFromCSSOrLink(loader, request.InspectorId(),
                                      request.Url(), resource_type,
                                      request_type);
    return;
  }

  if (options.initiator_info.name ==
      blink::fetch_initiator_type_names::kFetch) {
    RegisterRequestStartFromCurrentScript(execution_context,
                                          request.InspectorId(), request.Url(),
                                          resource_type, request_type);
    return;
  }

  if (options.initiator_info.name ==
      blink::fetch_initiator_type_names::kXmlhttprequest) {
    request_type = kRequestTypeAJAX;
    RegisterRequestStartFromCurrentScript(execution_context,
                                          request.InspectorId(), request.Url(),
                                          resource_type, request_type);
    return;
  }

  if (options.initiator_info.name ==
      blink::fetch_initiator_type_names::kBeacon) {
    request_type = kRequestTypeAJAX;
    RegisterRequestStartFromCurrentScript(execution_context,
                                          request.InspectorId(), request.Url(),
                                          resource_type, request_type);
    return;
  }

  // TODO: Wire up HTMLMediaElement.
  if (options.initiator_info.name ==
          blink::fetch_initiator_type_names::kVideo ||
      options.initiator_info.name ==
          blink::fetch_initiator_type_names::kAudio) {
    RegisterRequestStartFromCSSOrLink(loader, request.InspectorId(),
                                      request.Url(), resource_type,
                                      request_type);
    return;
  }

  if (options.initiator_info.name.IsEmpty()) {
    LOG(INFO) << "Empty request initiator for request id: "
              << request.InspectorId();
    ScriptId script_id = options.initiator_info.parent_script_id;
    if (script_id == 0)
      script_id = GetExecutingScriptId(execution_context);
    if (script_id) {
      RegisterRequestStartFromScript(execution_context, script_id,
                                     request.InspectorId(), request.Url(),
                                     resource_type, request_type);
    } else {
      RegisterRequestStartFromCSSOrLink(loader, request.InspectorId(),
                                        request.Url(), resource_type,
                                        request_type);
    }
    return;
  }

  LOG(ERROR) << "Unhandled request id: " << request.InspectorId()
             << " resource type: " << ResourceTypeToString(resource_type)
             << " type: " << request_type
             << " initiator: " << options.initiator_info.name
             << " url: " << request.Url() << "\n"
             << base::debug::StackTrace().ToString();
}

void PageGraph::DidReceiveResourceResponse(
    uint64_t identifier,
    blink::DocumentLoader* loader,
    const blink::ResourceResponse& response,
    const blink::Resource* cached_resource) {
  TrackedRequestRecord* request_record =
      request_tracker_.GetTrackingRecord(identifier);
  if (request_record) {
    TrackedRequest* request = request_record->request.get();
    if (request) {
      request->SetResponseMetadata(ResponseMetadata(response));
    }
    return;
  }

  DocumentRequest* document_request =
      request_tracker_.GetDocumentRequestInfo(identifier);
  if (document_request) {
    document_request->response_metadata.emplace(response);
    return;
  }

  LOG(ERROR) << "DidReceiveResourceResponse) untracked request id: "
             << identifier;
}

void PageGraph::DidReceiveData(uint64_t identifier,
                               blink::DocumentLoader*,
                               const char* data,
                               uint64_t data_length) {
  TrackedRequestRecord* request_record =
      request_tracker_.GetTrackingRecord(identifier);
  if (request_record) {
    TrackedRequest* request = request_record->request.get();
    if (request) {
      request->UpdateResponseBodyHash(base::make_span(data, data_length));
    }
    return;
  }

  DocumentRequest* document_request =
      request_tracker_.GetDocumentRequestInfo(identifier);
  if (document_request) {
    // Track document data?
    return;
  }

  LOG(ERROR) << "DidReceiveData) untracked request id: " << identifier;
}

void PageGraph::DidReceiveBlob(uint64_t identifier,
                               blink::DocumentLoader*,
                               blink::BlobDataHandle*) {
  TrackedRequestRecord* request_record =
      request_tracker_.GetTrackingRecord(identifier);
  if (request_record) {
    // TODO: Track blob data.
    return;
  }

  // Document load cannot trigger this event.

  LOG(ERROR) << "DidReceiveBlob) untracked request id: " << identifier;
}

void PageGraph::DidFinishLoading(uint64_t identifier,
                                 blink::DocumentLoader*,
                                 base::TimeTicks finish_time,
                                 int64_t encoded_data_length,
                                 int64_t decoded_body_length,
                                 bool should_report_corb_blocking) {
  TrackedRequestRecord* request_record =
      request_tracker_.GetTrackingRecord(identifier);
  if (request_record) {
    RegisterRequestComplete(identifier);
    return;
  }

  DocumentRequest* document_request =
      request_tracker_.GetDocumentRequestInfo(identifier);
  if (document_request) {
    RegisterRequestCompleteForDocument(identifier, decoded_body_length);
    return;
  }

  LOG(ERROR) << "DidFinishLoading) untracked request id: " << identifier;
}

void PageGraph::DidFailLoading(
    blink::CoreProbeSink* sink,
    uint64_t identifier,
    blink::DocumentLoader*,
    const blink::ResourceError&,
    const base::UnguessableToken& devtools_frame_or_worker_token) {
  TrackedRequestRecord* request_record =
      request_tracker_.GetTrackingRecord(identifier);
  if (request_record) {
    RegisterRequestError(identifier);
    return;
  }

  DocumentRequest* document_request =
      request_tracker_.GetDocumentRequestInfo(identifier);
  if (document_request) {
    RegisterRequestCompleteForDocument(identifier, 0);
    return;
  }

  LOG(ERROR) << "DidFailLoading) untracked request id: " << identifier;
}

void PageGraph::RegisterPageGraphScriptCompilation(
    blink::ExecutionContext* execution_context,
    const blink::ReferrerScriptInfo& referrer_info,
    const blink::ClassicScript& classic_script,
    v8::Local<v8::Script> script) {
  blink::KURL script_url = classic_script.BaseUrl();
  if (script_url.IsEmpty() || script_url.ProtocolIsAbout()) {
    script_url = classic_script.SourceUrl();
  }

  const ScriptId script_id = script->GetUnboundScript()->GetId();
  ScriptData script_data{
      .code = classic_script.SourceText().ToString(),
      .source = {
          .node_id = referrer_info.GetDOMNodeId(),
          .parent_script_id = referrer_info.GetParentScriptId(),
          .url = script_url,
          .location_type = classic_script.SourceLocationType(),
      }};

  if (script_data.source.location_type ==
      blink::ScriptSourceLocationType::kEvalForScheduledAction) {
    RegisterScriptCompilationFromEval(execution_context, script_id,
                                      script_data);
  } else {
    RegisterScriptCompilation(execution_context, script_id, script_data);
  }
}

void PageGraph::RegisterPageGraphModuleCompilation(
    blink::ExecutionContext* execution_context,
    const blink::ReferrerScriptInfo& referrer_info,
    const blink::ModuleScriptCreationParams& params,
    v8::Local<v8::Module> module) {
  const ScriptId script_id = module->ScriptId();
  if (script_id == 0) {
    LOG(ERROR) << "Invalid script id after module compilation";
    return;
  }
  ScriptData script_data{
      .code = params.GetSourceText().ToString(),
      .source = {
          .node_id = referrer_info.GetDOMNodeId(),
          .parent_script_id = referrer_info.GetParentScriptId(),
          .url = params.BaseURL(),
          .is_module = true,
      }};

  RegisterScriptCompilation(execution_context, script_id, script_data);
}

void PageGraph::RegisterPageGraphScriptCompilationFromAttr(
    blink::EventTarget* event_target,
    const String& function_name,
    const String& script_body,
    v8::Local<v8::Function> compiled_function) {
  blink::Node* event_recipient = event_target->ToNode();
  if (!event_recipient && event_target->ToLocalDOMWindow() &&
      event_target->ToLocalDOMWindow()->document()) {
    event_recipient = event_target->ToLocalDOMWindow()->document()->body();
  }
  if (!event_recipient) {
    LOG(ERROR) << "No event_recipient";
    return;
  }
  const ScriptId script_id = compiled_function->ScriptId();
  ScriptData script_data{
      .code = script_body,
      .source =
          {
              .node_id = blink::DOMNodeIds::IdForNode(event_recipient),
              .function_name = function_name,
          },
  };
  RegisterScriptCompilationFromAttr(event_recipient->GetExecutionContext(),
                                    script_id, script_data);
}

void PageGraph::RegisterPageGraphEventListenerAdd(
    blink::EventTarget* event_target,
    const String& event_type,
    blink::RegisteredEventListener* registered_listener) {
  blink::Node* node = event_target->ToNode();
  if (!node || !node->IsHTMLElement()) {
    return;
  }
  const int listener_script_id =
      GetListenerScriptId(event_target, registered_listener->Callback());
  if (!listener_script_id) {
    return;
  }
  RegisterEventListenerAdd(node, event_type, registered_listener->Id(),
                           listener_script_id);
}

void PageGraph::RegisterPageGraphEventListenerRemove(
    blink::EventTarget* event_target,
    const String& event_type,
    blink::RegisteredEventListener* registered_listener) {
  blink::Node* node = event_target->ToNode();
  if (!node || !node->IsHTMLElement()) {
    return;
  }
  const int listener_script_id =
      GetListenerScriptId(event_target, registered_listener->Callback());
  if (!listener_script_id) {
    return;
  }
  RegisterEventListenerRemove(node, event_type, registered_listener->Id(),
                              listener_script_id);
}

void PageGraph::RegisterPageGraphRemoteFrameCreated(
    blink::ExecutionContext* execution_context,
    blink::HTMLFrameOwnerElement* frame_owner_element) {
  RegisterRemoteFrameCreated(
      frame_owner_element,
      blink::IdentifiersFactory::FrameId(frame_owner_element->ContentFrame()));
}

ScriptTracker& PageGraph::GetScriptTracker(
    blink::ExecutionContext* execution_context) {
  v8::Isolate* isolate = execution_context->GetIsolate();
  auto it = script_trackers_.find(isolate);
  if (it != script_trackers_.end()) {
    return it->second;
  }
  return script_trackers_.emplace(isolate, ScriptTracker()).first->second;
}

std::map<ScriptId, NodeScript*>& PageGraph::GetScriptNodes(
    blink::ExecutionContext* execution_context) {
  v8::Isolate* isolate = execution_context->GetIsolate();
  auto it = script_nodes_.find(isolate);
  if (it != script_nodes_.end()) {
    return it->second;
  }
  return script_nodes_.emplace(isolate, std::map<ScriptId, NodeScript*>())
      .first->second;
}

}  // namespace brave_page_graph

namespace blink {

// static
const char PageGraph::kSupplementName[] = "PageGraph";

// static
PageGraph* PageGraph::From(LocalFrame& frame) {
  return Supplement<LocalFrame>::From<PageGraph>(frame);
}

// static
void PageGraph::ProvideTo(LocalFrame& frame) {
  DCHECK(!PageGraph::From(frame));
  if (frame.IsLocalRoot()) {
    Supplement<LocalFrame>::ProvideTo(frame,
                                      MakeGarbageCollected<PageGraph>(frame));
  } else {
    Supplement<LocalFrame>::ProvideTo(frame,
                                      PageGraph::From(frame.LocalFrameRoot()));
  }
}

PageGraph::PageGraph(LocalFrame& local_frame)
    : Supplement<LocalFrame>(local_frame),
      brave_page_graph::PageGraph(&local_frame) {
  blink::Page* page = local_frame.GetPage();
  if (!page) {
    VLOG(1) << "No page";
    return;
  }
  if (!page->IsOrdinary()) {
    VLOG(1) << "Page type is not ordinary";
    return;
  }

  if (local_frame.IsLocalRoot()) {
    local_frame.GetProbeSink()->AddPageGraph(this);
  } else {
    // Non-root frames will inherit ProbeSink from the root frame.
  }
}

void PageGraph::Trace(Visitor* visitor) const {
  Supplement<LocalFrame>::Trace(visitor);
  brave_page_graph::PageGraph::Trace(visitor);
}

}  // namespace blink
