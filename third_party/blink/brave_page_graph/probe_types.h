/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_PAGE_GRAPH_PROBE_TYPES_H_
#define BRAVE_COMPONENTS_BRAVE_PAGE_GRAPH_PROBE_TYPES_H_

#include <vector>

#include "third_party/blink/renderer/platform/wtf/hash_map.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

using PageGraphBlinkArgs = std::vector<String>;
using PageGraphBlinkReceiverData = HashMap<String, String>;

}  // namespace blink

#endif  // BRAVE_COMPONENTS_BRAVE_PAGE_GRAPH_PROBE_TYPES_H_
