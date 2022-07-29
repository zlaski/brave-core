/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_SCRIPTS_SCRIPT_TRACKER_H_
#define BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_SCRIPTS_SCRIPT_TRACKER_H_

#include <map>
#include <vector>

#include "brave/third_party/blink/renderer/core/brave_page_graph/types.h"

namespace brave_page_graph {

class ScriptTracker {
 public:
  ScriptTracker();
  ~ScriptTracker();

  void AddScript(ScriptId script_id, const ScriptData& script_data);

  const ScriptData& GetScriptData(ScriptId script_id) const;
  DOMNodeIdList GetElmsForScriptId(ScriptId script_id) const;
  ScriptIdList GetModuleScriptParentsForScriptId(ScriptId script_id) const;

 private:
  std::map<ScriptId, ScriptData> scripts_;
  std::map<ScriptId, DOMNodeIdList> script_dom_nodes_;
  std::map<ScriptId, ScriptIdList> script_parent_scritps_;
};

}  // namespace brave_page_graph

#endif  // BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_SCRIPTS_SCRIPT_TRACKER_H_
