/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_PROFILES_BRAVE_PROFILE_IMPL_H_
#define BRAVE_BROWSER_PROFILES_BRAVE_PROFILE_IMPL_H_

#include "chrome/browser/profiles/profile_impl.h"

#include "base/memory/weak_ptr.h"

class PrefStore;

class BraveProfileImpl : public ProfileImpl {
 public:
  BraveProfileImpl(const base::FilePath& path,
                   Delegate* delegate,
                   CreateMode create_mode,
                   scoped_refptr<base::SequencedTaskRunner> io_task_runner);

  ~BraveProfileImpl() override;

  bool IsSameProfile(Profile* profile) override;

  // We need to access chromium's Profile::CreateExtensionPrefStore which is a
  // protected method from pref_service_builder_utils.cc when creating prefs
  // for Tor profile. Instead of patching profile.h directly, we define this
  // public method in this derived class to access it.
  static PrefStore* CreateExtensionPrefStore(Profile* profile,
                                             bool incognito_pref_store);

 private:
  base::WeakPtrFactory<BraveProfileImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(BraveProfileImpl);
};

#endif  // BRAVE_BROWSER_PROFILES_BRAVE_PROFILE_IMPL_H_
