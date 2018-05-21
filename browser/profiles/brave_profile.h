/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_PROFILES_BRAVE_PROFILE_H_
#define BRAVE_BROWSER_PROFILES_BRAVE_PROFILE_H_

#include "chrome/browser/profiles/profile_impl.h"

class BraveProfile : public ProfileImpl {
 public:
  static BraveProfile* CreateProfile(const base::FilePath& path,
                                Delegate* delegate,
                                CreateMode create_mode);
 private:
  BraveProfile(const base::FilePath& path,
              Delegate* delegate,
              CreateMode create_mode,
              scoped_refptr<base::SequencedTaskRunner> io_task_runner);
  DISALLOW_COPY_AND_ASSIGN(BraveProfile);
};

#endif  // BRAVE_BROWSER_PROFILES_BRAVE_PROFILE_H_
