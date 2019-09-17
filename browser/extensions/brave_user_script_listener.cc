/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/extensions/brave_user_script_listener.h"

#include "brave/browser/profiles/profile_util.h"
#include "chrome/browser/chrome_notification_types.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_source.h"
#include "extensions/browser/extension_registry.h"

namespace extensions {

void BraveUserScriptListener::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  switch (type) {
    case chrome::NOTIFICATION_PROFILE_ADDED: {
      // The dummy regular profile we created for Tor shares the same extension
      // registry with its parent profile, since the registry is already being
      // observed when parent profile is added, we will not add the registry to
      // the observer again when the dummy regular profile is added. Without
      // this, we would hit DCHECK by trying to add the same registry to the
      // observer again.
      Profile* profile = content::Source<Profile>(source).ptr();
      if (brave::IsTorProfile(profile)) {
        return;
      }
      break;
    }
    default:
      break;
  }

  return UserScriptListener::Observe(type, source, details);
}

}  // namespace extensions
