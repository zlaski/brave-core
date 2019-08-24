/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/profile_util.h"
#include "../../../../../chrome/browser/profiles/profile.cc"  // NOLINT

// Return the parent profile which used to create Tor profile. This method
// returns this if the profile is not Tor profile.
Profile* Profile::GetParentProfile() {
  if (!brave::IsTorProfile(this))
    return this;

  if (!parent_profile_) {
    parent_profile_ = brave::GetTorParentProfile(GetPath());
  }

  return parent_profile_;
}
