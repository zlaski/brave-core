/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/extensions/brave_extensions_browser_client.h"

#include "brave/browser/extensions/brave_extensions_browser_api_provider.h"
#include "brave/browser/profiles/profile_util.h"
#include "chrome/browser/profiles/profile.h"

namespace extensions {

BraveExtensionsBrowserClient::BraveExtensionsBrowserClient() {
  AddAPIProvider(std::make_unique<BraveExtensionsBrowserAPIProvider>());
}

content::BrowserContext* BraveExtensionsBrowserClient::GetOriginalContext(
    content::BrowserContext* context) {
  if (brave::IsTorProfile(context)) {
    return brave::GetTorParentProfile(context);
  }

  return ChromeExtensionsBrowserClient::GetOriginalContext(context);
}

}  // namespace extensions
