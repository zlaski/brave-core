/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/profile_util.h"

#include "base/files/file_path.h"
#include "brave/browser/tor/buildflags.h"
#include "brave/common/tor/tor_constants.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_impl.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/prefs/pref_service.h"

namespace brave {

bool IsTorProfile(const Profile* profile) {
  DCHECK(profile);
  return IsTorProfile(profile->GetPath());
}

bool IsTorProfile(content::BrowserContext* context) {
  DCHECK(context);
  return IsTorProfile(Profile::FromBrowserContext(context));
}

bool IsTorProfile(base::FilePath path) {
#if BUILDFLAG(ENABLE_TOR)
  return path.BaseName() == base::FilePath(tor::kTorProfileDir);
#else
  return false;
#endif
}

bool IsGuestProfile(Profile* profile) {
  DCHECK(profile);
  return profile->GetOriginalProfile()->IsGuestSession();
}

Profile* GetTorParentProfile(content::BrowserContext* context) {
  DCHECK(context);
  DCHECK(IsTorProfile(context));
  return Profile::FromBrowserContext(context)->GetOriginalProfile()
      ->GetParentProfile();
}

Profile* GetTorParentProfile(base::FilePath path) {
  DCHECK(IsTorProfile(path));

  ProfileManager* profile_manager = g_browser_process->profile_manager();
  DCHECK(profile_manager);

  base::FilePath parent_profile_path = path.DirName().DirName();
  Profile* parent_profile =
      profile_manager->GetProfileByPath(parent_profile_path);
  DCHECK(parent_profile);

  return parent_profile;
}

}  // namespace brave
