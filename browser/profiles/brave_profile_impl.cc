/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/brave_profile_impl.h"

#include "base/task/post_task.h"
#include "brave/browser/profiles/profile_util.h"

BraveProfileImpl::BraveProfileImpl(
    const base::FilePath& path,
    Delegate* delegate,
    CreateMode create_mode,
    scoped_refptr<base::SequencedTaskRunner> io_task_runner)
    : ProfileImpl(path, delegate, create_mode, io_task_runner),
      weak_ptr_factory_(this) {
  // In Tor profile, prefs are created from the original profile like how
  // incognito profile works. By the time chromium start to observe prefs
  // initialization in ProfileImpl constructor, prefs are already initialized
  // and it's too late for the observer to get the notification, so we manually
  // trigger the OnPrefsLoaded here.
  //
  // This need to be posted instead of running directly here because we need to
  // finish the construction and register this profile first in ProfileManager
  // before OnPrefsLoaded is called.
  if (brave::IsTorProfile(path))
    base::PostTaskAndReply(
        FROM_HERE, base::DoNothing(),
        base::BindOnce(&ProfileImpl::OnPrefsLoaded,
                       weak_ptr_factory_.GetWeakPtr(), create_mode, true));
}

BraveProfileImpl::~BraveProfileImpl() {}

// This is needed so things like shields events will be dispatched to Tor
// context as well.
bool BraveProfileImpl::IsSameProfile(Profile* profile) {
  if (brave::IsTorProfile(profile) &&
      brave::GetTorParentProfile(profile) == this) {
    return true;
  }

  return ProfileImpl::IsSameProfile(profile);
}

PrefStore* BraveProfileImpl::CreateExtensionPrefStore(
    Profile* profile,
    bool incognito_pref_store) {
  return Profile::CreateExtensionPrefStore(profile, incognito_pref_store);
}
