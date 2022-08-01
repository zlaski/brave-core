/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/node/edge_node_insert.h"

#include <libxml/tree.h>
#include <sstream>
#include <string>

#include "third_party/blink/renderer/core/dom/dom_node_ids.h"

#include "brave/third_party/blink/renderer/core/brave_page_graph/graphml.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/logging.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/page_graph.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/types.h"

#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/actor/node_actor.h"

#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/html/node_html.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/html/node_html_element.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/node/html/node_html_text.h"

using ::blink::DOMNodeId;

namespace brave_page_graph {

EdgeNodeInsert::EdgeNodeInsert(PageGraph* const graph,
                               NodeActor* const out_node,
                               NodeHTML* const in_node,
                               NodeHTMLElement* parent_node,
                               NodeHTML* prior_sibling_node)
    : EdgeNode(graph, out_node, in_node),
      parent_node_(parent_node),
      prior_sibling_node_(prior_sibling_node) {}

EdgeNodeInsert::~EdgeNodeInsert() {}

NodeHTMLElement* EdgeNodeInsert::GetParentNode() const {
  return parent_node_;
}

NodeHTML* EdgeNodeInsert::GetPriorSiblingNode() const {
  return prior_sibling_node_;
}

ItemName EdgeNodeInsert::GetItemName() const {
  return "insert node";
}

ItemDesc EdgeNodeInsert::GetItemDesc() const {
  const Node* const parent_node = GetParentNode();
  PG_LOG_ASSERT(parent_node);

  const Node* const prior_sibling_node = GetPriorSiblingNode();

  std::stringstream builder;
  builder << EdgeNode::GetItemDesc();
  builder << " [parent: " << parent_node->GetItemDesc() << "]";

  if (prior_sibling_node) {
    builder << " [prior sibling: " << prior_sibling_node->GetItemDesc() << "]";
  }

  return builder.str();
}

void EdgeNodeInsert::AddGraphMLAttributes(xmlDocPtr doc,
                                          xmlNodePtr parent_node) const {
  EdgeNode::AddGraphMLAttributes(doc, parent_node);
  if (parent_node_) {
    GraphMLAttrDefForType(kGraphMLAttrDefParentNodeId)
        ->AddValueNode(doc, parent_node, parent_node_->GetNodeId());
  }
  if (prior_sibling_node_) {
    GraphMLAttrDefForType(kGraphMLAttrDefBeforeNodeId)
        ->AddValueNode(doc, parent_node, prior_sibling_node_->GetNodeId());
  }
}

bool EdgeNodeInsert::IsEdgeNodeInsert() const {
  return true;
}

}  // namespace brave_page_graph
