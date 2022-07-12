/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "src/third_party/blink/renderer/core/script/pending_script.cc"

#include "third_party/blink/renderer/core/probe/core_probes.h"

namespace blink {

void PendingScript::PendingScriptConstructed() {
  probe::RegisterPageGraphPendingScriptConstructed(
      GetElement()->GetExecutionContext(), this);
}

}  // namespace blink
