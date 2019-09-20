/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/extensions/brave_extensions_browser_client_impl.h"

#include <memory>

#include "brave/browser/extensions/brave_extensions_browser_api_provider.h"
#include "brave/browser/profiles/profile_util.h"
#include "components/prefs/pref_service.h"

namespace extensions {

BraveExtensionsBrowserClientImpl::BraveExtensionsBrowserClientImpl() {
  BraveExtensionsBrowserClient::Set(this);
  AddAPIProvider(std::make_unique<BraveExtensionsBrowserAPIProvider>());
}

bool BraveExtensionsBrowserClientImpl::AreExtensionsDisabledOverride(
    const base::CommandLine& command_line,
    content::BrowserContext* context) {
  return ExtensionsBrowserClient::Get()->AreExtensionsDisabled(
      command_line, context);
}

void BraveExtensionsBrowserClientImpl::GetEarlyExtensionPrefsObserversOverride(
      content::BrowserContext* context,
      std::vector<EarlyExtensionPrefsObserver*>* observers) const {
  ExtensionsBrowserClient::Get()->GetEarlyExtensionPrefsObservers(
      context, observers);
}

PrefService* BraveExtensionsBrowserClientImpl::GetPrefServiceForContextOverride(
      content::BrowserContext* context) {
  return ExtensionsBrowserClient::Get()->GetPrefServiceForContext(context);
}

content::BrowserContext*
BraveExtensionsBrowserClientImpl::GetOriginalContextOverride(
    content::BrowserContext* context) {
  context = ExtensionsBrowserClient::Get()->GetOriginalContext(context);
  if (brave::IsSessionProfile(context))
    context = brave::GetParentProfile(context);

  return context;
}

}  // namespace extensions
