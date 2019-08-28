/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/apps/app_shim/brave_extension_app_shim_handler_mac.h"

#include "brave/browser/profiles/profile_util.h"
#include "chrome/browser/chrome_notification_types.h"
#include "content/public/browser/notification_source.h"

namespace apps {

// Same as off-the-record profiles, we return early for Tor profile for
// NOTIFICATION_PROFILE_CREATED and NOTIFICATION_PROFILE_DESTROYED cases
// since ExtensionAppShimHandler is observing AppLifetimeMonitorFactory through
// parent profiles.
void BraveExtensionAppShimHandler::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  switch (type) {
    case chrome::NOTIFICATION_PROFILE_CREATED: {
      Profile* profile = content::Source<Profile>(source).ptr();
      if (brave::IsTorProfile(profile))
        return;
      break;
    }
    case chrome::NOTIFICATION_PROFILE_DESTROYED: {
      Profile* profile = content::Source<Profile>(source).ptr();
      if (brave::IsTorProfile(profile))
        return;
      break;
    }
    default: {
      break;
    }
  }

  return ExtensionAppShimHandler::Observe(type, source, details);
}

}  // namespace apps
