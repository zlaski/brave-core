/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/profile_util.h"
#include "chrome/browser/profiles/profile.h"

// Share the process manager between dummy Tor regular profile and its parent
// profile here because we don't need one for the dummy profile we created.
#define BRAVE_GET_BROWSER_CONTEXT_TO_USE                            \
  if (brave::IsTorProfile(context) && !context->IsOffTheRecord()) { \
    return brave::GetTorParentProfile(context);                     \
  }

#include "../../../../extensions/browser/process_manager_factory.cc"

#undef BRAVE_GET_BROWSER_CONTEXT_TO_USE
