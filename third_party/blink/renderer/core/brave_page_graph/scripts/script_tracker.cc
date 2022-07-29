/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/third_party/blink/renderer/core/brave_page_graph/scripts/script_tracker.h"

#include <map>
#include <vector>

#include "base/containers/contains.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/logging.h"
#include "brave/third_party/blink/renderer/core/brave_page_graph/types.h"
#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"

namespace brave_page_graph {

ScriptTracker::ScriptTracker() = default;

ScriptTracker::~ScriptTracker() = default;

void ScriptTracker::AddScript(ScriptId script_id,
                              const ScriptData& script_data) {
  auto it = scripts_.find(script_id);
  if (it != scripts_.end()) {
    PG_LOG_ASSERT(it->second == script_data) << script_id;
    return;
  }
  scripts_.emplace(script_id, script_data);
  if (script_data.source.node_id != blink::kInvalidDOMNodeId) {
    script_dom_nodes_[script_id].push_back(script_data.source.node_id);
  }
  if (script_data.source.parent_script_id) {
    script_parent_scritps_[script_id].push_back(
        script_data.source.parent_script_id);
  }
}

const ScriptData& ScriptTracker::GetScriptData(ScriptId script_id) const {
  PG_LOG_ASSERT(base::Contains(scripts_, script_id)) << script_id;
  return scripts_.at(script_id);
}

DOMNodeIdList ScriptTracker::GetElmsForScriptId(ScriptId script_id) const {
  PG_LOG_ASSERT(base::Contains(scripts_, script_id)) << script_id;
  auto it = script_dom_nodes_.find(script_id);
  return it != script_dom_nodes_.end() ? it->second : DOMNodeIdList();
}

ScriptIdList ScriptTracker::GetModuleScriptParentsForScriptId(
    ScriptId script_id) const {
  PG_LOG_ASSERT(base::Contains(scripts_, script_id)) << script_id;
  auto it = script_parent_scritps_.find(script_id);
  return it != script_parent_scritps_.end() ? it->second : ScriptIdList();
}

}  // namespace brave_page_graph
