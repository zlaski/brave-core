/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/profile_util.h"
#include "../../../../../../chrome/browser/web_applications/components/web_app_utils.cc"

namespace web_app {

content::BrowserContext* BraveGetBrowserContextForWebApps(
    content::BrowserContext* context) {
  if (!brave::IsTorProfile(context))
    return GetBrowserContextForWebApps(context);

  // For Tor profile case, both dummy regular Tor profile and the Tor OTR
  // profile shares one KeyedService instance with its parent profile.
  // This gets pass the DCHECK(browser_context == profile()) in
  // BookmarkAppRegistrar::OnExtensionUninstalled.
  Profile* parent_profile =
      Profile::FromBrowserContext(context)->GetParentProfile();
  return AreWebAppsEnabled(parent_profile) ? parent_profile : nullptr;
}

}  // namespace web_app
