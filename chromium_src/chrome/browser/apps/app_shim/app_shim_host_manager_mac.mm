/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/apps/app_shim/brave_extension_app_shim_handler_mac.h"

#define ExtensionAppShimHandler BraveExtensionAppShimHandler
#include "../../../../../../chrome/browser/apps/app_shim/app_shim_host_manager_mac.mm"
#undef ExtensionAppShimHandler
