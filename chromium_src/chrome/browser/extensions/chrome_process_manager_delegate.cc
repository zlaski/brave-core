/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/profile_util.h"

// Override the original profile for Tor profile to pass
// DCHECK(original_profile, original_manager->browser_context()) in
// ChromeProcessManagerDelegate::OnBrowserOpened, the original_manager is the
// ProcessManager of Tor profile's parent profile, so we need to override the
// original_profile to the parent profile in order to pass the DCHECK.
#define BRAVE_ON_BROWSER_OPENED                             \
  if (brave::IsTorProfile(profile)) {                       \
    original_profile = brave::GetTorParentProfile(profile); \
  }

#include "../../../../../chrome/browser/extensions/chrome_process_manager_delegate.cc"
#undef BRAVE_ON_BROWSER_OPENED
