/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/chromium_src/components/sync/driver/profile_sync_service.h"

// For use_transport_only_mode
#define IsSyncFeatureEnabled IsBraveSyncEnabled
#include "../../../../components/sync/driver/profile_sync_service.cc"   // NOLINT

#include "base/bind.h"
#include "base/memory/weak_ptr.h"
#include "brave/components/brave_sync/brave_profile_sync_service.h"
#include "brave/components/brave_sync/jslib_messages.h"
#include "chrome/browser/sync/chrome_sync_client.h"
#include "content/public/browser/browser_thread.h"

using brave_sync::BraveProfileSyncService;

namespace syncer {

const int64_t kBraveDefaultPollIntervalSeconds = 60;

void OnNudgeSyncCycle(base::WeakPtr<ProfileSyncService> profile_sync_service,
                      brave_sync::RecordsListPtr records_list) {
  if (profile_sync_service.get() &&
      profile_sync_service->IsBraveSyncEnabled()) {
    static_cast<BraveProfileSyncService*>(
        profile_sync_service.get())->OnNudgeSyncCycle(std::move(records_list));
  }
}

void OnPollSyncCycle(base::WeakPtr<ProfileSyncService> profile_sync_service,
                     brave_sync::GetRecordsCallback cb,
                     base::WaitableEvent* wevent) {
  if (profile_sync_service.get() &&
      profile_sync_service->IsBraveSyncEnabled()) {
    static_cast<BraveProfileSyncService*>(
        profile_sync_service.get())->OnPollSyncCycle(cb, wevent);
  }
}

void ProfileSyncService::BraveEngineParamsInit(
    syncer::SyncEngine::InitParams* params) {
  DCHECK(params);
  params->nudge_sync_cycle_delegate_function =
      base::BindRepeating(&OnNudgeSyncCycle,
                          sync_enabled_weak_factory_.GetWeakPtr());
  params->poll_sync_cycle_delegate_function =
      base::BindRepeating(&OnPollSyncCycle,
                          sync_enabled_weak_factory_.GetWeakPtr());

  sync_prefs_.SetPollInterval(
    base::TimeDelta::FromSeconds(
      syncer::kBraveDefaultPollIntervalSeconds));
}

bool ProfileSyncService::IsBraveSyncEnabled() const {
  return false;
}

}   // namespace syncer
