/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/extensions/browser/brave_extensions_browser_client.h"
#include "extensions/browser/extensions_browser_client.h"

#undef ExtensionsBrowserClient
#define ExtensionsBrowserClient BraveExtensionsBrowserClient
#define GetOriginalContext GetOriginalContextOverride
#include "../../../extensions/browser/extension_pref_value_map_factory.cc"
