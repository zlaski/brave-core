/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_BACKGROUND_BRAVE_BACKGROUND_MODE_MANAGER_H_
#define BRAVE_BROWSER_BACKGROUND_BRAVE_BACKGROUND_MODE_MANAGER_H_

#include "chrome/browser/background/background_mode_manager.h"

class BraveBackgroundModeManager : public BackgroundModeManager {
 public:
  BraveBackgroundModeManager(const base::CommandLine& command_line,
                             ProfileAttributesStorage* profile_storage);
  ~BraveBackgroundModeManager() override;

  void RegisterProfile(Profile* profile) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(BraveBackgroundModeManager);
};

#endif  // BRAVE_BROWSER_BACKGROUND_BRAVE_BACKGROUND_MODE_MANAGER_H_
