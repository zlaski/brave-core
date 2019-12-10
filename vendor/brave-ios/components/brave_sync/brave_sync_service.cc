/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 3.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/vendor/brave-ios/components/brave_sync/brave_sync_service.h"

#include "base/task/post_task.h"
#include "components/prefs/pref_service.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/sync_preferences/pref_service_syncable.h"

namespace {

bool GetUserDataDir(base::FilePath* result) {
  bool success = base::mac::GetUserDirectory(NSApplicationSupportDirectory,
                                             result);

  // On IOS, this directory does not exist unless it is created explicitly.
  if (success && !base::PathExists(*result))
    success = base::CreateDirectory(*result);

  if (!success)
    return false;

  result->Append(FILE_PATH_LITERAL("BraveBrowser")); // TODO(bridiver) - dev, etc..

  if (!base::PathExists(*result))
    success = base::CreateDirectory(*result);

  return success;
}

}  // namespace

BraveSyncService::BraveSyncService()
    : pref_registry_(new user_prefs::PrefRegistrySyncable) {
  bookmarks::RegisterProfilePrefs(pref_registry_.get());

  io_task_runner_ =
      base::CreateSequencedTaskRunner(
          {base::ThreadPool(), base::TaskShutdownBehavior::BLOCK_SHUTDOWN,
           base::MayBlock()});

  base::FilePath browser_state_path;
  if (!GetUserDataDir(&browser_state_path))
    NOTREACHED();

  sync_preferences::PrefServiceSyncableFactory factory;
  factory->set_user_prefs(base::MakeRefCounted<JsonPrefStore>(
      browser_state_path.Append(kPreferencesFilename),
      std::unique_ptr<PrefFilter>(), io_task_runner.get()));

  prefs_ =
      factory.CreateSyncable(pref_registry_.get());

  // Register on BrowserState.
  // user_prefs::UserPrefs::Set(this, prefs_.get());

  bookmarks_api_ = std::make_unique<BookmarksAPI>(prefs_.get(),
      browser_state_path,
      io_task_runner_.get(),
      nullptr); //std::unique_ptr<BookmarkClient>
}

BraveSyncService::~BraveSyncService() {}

BookmarksAPI* BraveSyncService::bookmarks_api() {

}
