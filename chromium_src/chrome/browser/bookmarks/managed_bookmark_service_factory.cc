/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/profile_util.h"

#define BRAVE_GET_BROWSER_CONTEXT_TO_USE_       \
  if (brave::IsTorProfile(context)) {           \
    return brave::GetTorParentProfile(context); \
  }
// define BRAVE_GET_BROWSER_CONTEXT_TO_USE_

#include "../../../../../chrome/browser/bookmarks/managed_bookmark_service_factory.cc"

#undef BRAVE_GET_BROWSER_CONTEXT_TO_USE_
