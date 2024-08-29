/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ios/chrome/browser/tabs/model/tab_helper_util.h"

// Comment here to stop formatter form moving it up
#include "src/ios/web_view/internal/cwv_web_view.mm"

@implementation CWVWebView (Internal)

- (web::WebState*)webState {
  return _webState.get();
}

@end
