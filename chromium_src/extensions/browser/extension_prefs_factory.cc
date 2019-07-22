/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/common/tor/tor_constants.h"
#include "content/public/browser/browser_context.h"

bool IsTorProfile(content::BrowserContext* context) {
// TODO: wrap it with Tor build flag, and return false when it's disabled.
  return context->GetPath().BaseName() == base::FilePath(tor::kTorProfileDir);
}

// Tor profile has a different profile dir from its parent profile, but shares
// the extension installation dir as its parent profile, so we need to
// explicitly change the installation dir from
// user_data_dir/Default/session_profiles/Tor Profile/Extensions to
// user_data_dir/Default/Extensions here.
#define BRAVE_CHANGE_INSTALL_DIR_FOR_TOR_PROFILE \
  IsTorProfile(context) ? context->GetPath().DirName().DirName().AppendASCII(extensions::kInstallDirectoryName) :

#include "../../../../extensions/browser/extension_prefs_factory.cc"  // NOLINT

#undef BRAVE_CHANGE_INSTALL_DIR_FOR_TOR_PROFILE
