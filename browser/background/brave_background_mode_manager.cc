/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/background/brave_background_mode_manager.h"

#include "brave/browser/profiles/profile_util.h"

BraveBackgroundModeManager::BraveBackgroundModeManager(
    const base::CommandLine& command_line,
    ProfileAttributesStorage* profile_storage)
    : BackgroundModeManager(command_line, profile_storage) {}

BraveBackgroundModeManager::~BraveBackgroundModeManager() {}

// Chromium does not register off-the-record profile with background mode
// manager, similarly, we don't register the dummy Tor regular profile with
// background mode manager either. Things should be taken care of by its parent
// profile which should have already been registered.
void BraveBackgroundModeManager::RegisterProfile(Profile* profile) {
  if (brave::IsTorProfile(profile))
    return;
  BackgroundModeManager::RegisterProfile(profile);
}
