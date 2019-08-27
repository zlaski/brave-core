/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/extensions/brave_extensions_browser_client.h"

#include <memory>

#include "brave/browser/extensions/brave_extensions_browser_api_provider.h"
#include "brave/browser/extensions/brave_user_script_listener.h"
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

UserScriptListener* BraveExtensionsBrowserClient::GetUserScriptListener() {
  // Create lazily since this accesses g_browser_process which may not be set up
  // when ChromeExtensionsBrowserClient is created.
  if (!user_script_listener_)
    user_script_listener_ = std::make_unique<BraveUserScriptListener>();
  return user_script_listener_.get();
}

}  // namespace extensions
