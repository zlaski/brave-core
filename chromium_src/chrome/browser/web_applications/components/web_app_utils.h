/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_CHROME_BROWSER_WEB_APPLICATIONS_COMPONENTS_WEB_APP_UTILS_H_
#define BRAVE_CHROMIUM_SRC_CHROME_BROWSER_WEB_APPLICATIONS_COMPONENTS_WEB_APP_UTILS_H_

#include "../../../../../../chrome/browser/web_applications/components/web_app_utils.h"

namespace web_app {

// Get BrowserContext to use for a WebApp KeyedService creation.
content::BrowserContext* BraveGetBrowserContextForWebApps(
    content::BrowserContext* context);

}  // namespace web_app

#endif  // BRAVE_CHROMIUM_SRC_CHROME_BROWSER_WEB_APPLICATIONS_COMPONENTS_WEB_APP_UTILS_H_
