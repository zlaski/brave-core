/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_EXTENSIONS_BROWSER_BRAVE_EXTENSIONS_BROWSER_CLIENT_H_
#define BRAVE_EXTENSIONS_BROWSER_BRAVE_EXTENSIONS_BROWSER_CLIENT_H_

#include <vector>

#include "base/command_line.h"
#include "base/macros.h"
#include "build/build_config.h"

namespace content {
class BrowserContext;
}

class PrefService;

namespace extensions {

class EarlyExtensionPrefsObserver;

class BraveExtensionsBrowserClient {
 public:
  BraveExtensionsBrowserClient();
  virtual ~BraveExtensionsBrowserClient();

  // Returns the single instance of |this|.
  static BraveExtensionsBrowserClient* Get();

  // Sets and initializes the single instance.
  static void Set(BraveExtensionsBrowserClient* client);

  virtual bool AreExtensionsDisabledOverride(
      const base::CommandLine& command_line,
      content::BrowserContext* context) = 0;

  virtual void GetEarlyExtensionPrefsObserversOverride(
      content::BrowserContext* context,
      std::vector<EarlyExtensionPrefsObserver*>* observers) const = 0;

  virtual PrefService* GetPrefServiceForContextOverride(
      content::BrowserContext* context) = 0;

  virtual content::BrowserContext* GetOriginalContextOverride(
      content::BrowserContext* context) = 0;
};

}  // namespace extensions

#endif  // BRAVE_EXTENSIONS_BROWSER_BRAVE_EXTENSIONS_BROWSER_CLIENT_H_
