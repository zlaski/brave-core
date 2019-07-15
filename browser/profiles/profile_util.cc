/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/profile_util.h"

#include "brave/browser/tor/buildflags.h"
#include "brave/common/tor/tor_constants.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/prefs/pref_service.h"

namespace brave {

bool IsTorProfile(const Profile* profile) {
#if BUILDFLAG(ENABLE_TOR)
  DCHECK(profile);
  return profile->GetPath().BaseName() == base::FilePath(tor::kTorProfileDir);
#else
  return false;
#endif
}

bool IsTorProfile(content::BrowserContext* context) {
  return IsTorProfile(Profile::FromBrowserContext(context));
}

bool IsGuestProfile(Profile* profile) {
  DCHECK(profile);
  return profile->GetOriginalProfile()->IsGuestSession() &&
         !IsTorProfile(profile);
}

Profile* GetTorParentProfile(content::BrowserContext* context) {
  return GetTorParentProfile(context->GetPath());
}

Profile* GetTorParentProfile(base::FilePath tor_path) {
  ProfileManager* profile_manager = g_browser_process->profile_manager();
  base::FilePath parent_profile_path = tor_path.DirName().DirName();
  Profile* parent_profile =
    profile_manager->GetProfileByPath(parent_profile_path);
  return parent_profile;
}

ProfileKey* GetTorParentProfileKey(base::FilePath tor_path) {
  return GetTorParentProfile(tor_path)->GetProfileKey();
}

bool IsTorProfilePath(base::FilePath path) {
  return path.BaseName() == base::FilePath(tor::kTorProfileDir);
}

}  // namespace brave
