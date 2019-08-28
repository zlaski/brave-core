/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_APPS_APP_SHIM_BRAVE_EXTENSION_APP_SHIM_HANDLER_MAC_H_
#define BRAVE_BROWSER_APPS_APP_SHIM_BRAVE_EXTENSION_APP_SHIM_HANDLER_MAC_H_

#include "chrome/browser/apps/app_shim/extension_app_shim_handler_mac.h"

#include "content/public/browser/notification_observer.h"

namespace apps {

class BraveExtensionAppShimHandler : public ExtensionAppShimHandler {
 public:
  BraveExtensionAppShimHandler() = default;
  ~BraveExtensionAppShimHandler() override = default;

  // content::NotificationObserver overrides:
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  DISALLOW_COPY_AND_ASSIGN(BraveExtensionAppShimHandler);
};

}  // namespace apps

#endif  // BRAVE_BROWSER_APPS_APP_SHIM_BRAVE_EXTENSION_APP_SHIM_HANDLER_MAC_H_
