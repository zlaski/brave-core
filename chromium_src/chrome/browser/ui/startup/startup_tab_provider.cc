/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/first_run/first_run.h"

// This effectively overrides GetDistributionFirstRunTabs by hooking its call to
// IsChromeFirstRun. We have to do it this way, because there are two
// occurrences of GetDistrubtionFirstRunTabs in the header file and two
// occurrences of GetInitialPrefsTabsForState (the next best candidate) in this
// source file.
#define IsChromeFirstRun                                  \
  IsChromeFirstRun(), browser_creator->first_run_tabs_);  \
  if (first_run::IsChromeFirstRun()) {                    \
    tabs.emplace_back(GURL(chrome::kChromeUIWelcomeURL)); \
  }                                                       \
  GetInitialPrefsTabsForState(first_run::IsChromeFirstRun

#include "src/chrome/browser/ui/startup/startup_tab_provider.cc"

#undef IsChromeFirstRun
