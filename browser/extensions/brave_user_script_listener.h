/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_EXTENSIONS_BRAVE_USER_SCRIPT_LISTENER_H_
#define BRAVE_BROWSER_EXTENSIONS_BRAVE_USER_SCRIPT_LISTENER_H_

#include "chrome/browser/extensions/user_script_listener.h"

namespace extensions {

class BraveUserScriptListener : public UserScriptListener {
 public:
  BraveUserScriptListener() = default;
  ~BraveUserScriptListener() override = default;

 private:
  // content::NotificationObserver
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  DISALLOW_COPY_AND_ASSIGN(BraveUserScriptListener);
};

}  // namespace extensions

#endif  // BRAVE_BROWSER_EXTENSIONS_BRAVE_USER_SCRIPT_LISTENER_H_
