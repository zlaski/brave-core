/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <utility>

#include "base/memory/weak_ptr.h"
#include "brave/components/brave_sync/jslib_messages.h"
#include "components/sync/driver/glue/sync_engine_impl.h"
#include "components/sync/engine/sync_engine_host.h"

using brave_sync::GetRecordsCallback;
using brave_sync::RecordsList;

namespace syncer {

void OnNudgeSyncCycleOnSyncThread(base::WeakPtr<SyncEngineImpl> sync_engine,
                                  SyncEngineHost* host,
                                  brave_sync::RecordsListPtr records_list) {
  if (sync_engine.get())
    host->OnNudgeSyncCycle(std::move(records_list));
}

void OnNudgeSyncCycle(WeakHandle<SyncEngineImpl> sync_engine_impl,
                      SyncEngineHost* host,
                      brave_sync::RecordsListPtr records_list) {
  sync_engine_impl.Call(FROM_HERE,
                        &OnNudgeSyncCycleOnSyncThread,
                        host,
                        base::Passed(&records_list));
}

void OnPollSyncCycleOnSyncThread(base::WeakPtr<SyncEngineImpl> sync_engine,
                                 SyncEngineHost* host,
                                 GetRecordsCallback cb,
                                 base::WaitableEvent* wevent) {
  if (sync_engine.get())
    host->OnPollSyncCycle(cb, wevent);
}

void OnPollSyncCycle(WeakHandle<SyncEngineImpl> sync_engine_impl,
                     SyncEngineHost* host,
                     GetRecordsCallback cb,
                     base::WaitableEvent* wevent) {
  sync_engine_impl.Call(FROM_HERE,
                        &OnPollSyncCycleOnSyncThread,
                        host,
                        cb,
                        wevent);
}

void DoDispatchGetRecordsCallback(GetRecordsCallback cb,
                                  std::unique_ptr<brave_sync::RecordsList> records) {
  cb.Run(std::move(records));
}

void BraveInit(WeakHandle<SyncEngineImpl> sync_engine_impl,
               SyncEngineHost* host,
               SyncManager::InitArgs* args) {
  DCHECK(args);
  args->nudge_sync_cycle_delegate_function =
      base::BindRepeating(&OnNudgeSyncCycle,
                          sync_engine_impl,
                          host);
  args->poll_sync_cycle_delegate_function =
      base::BindRepeating(&OnPollSyncCycle,
                          sync_engine_impl,
                          host);
}

}  // namespace syncer

#include "../../../../../../components/sync/driver/glue/sync_engine_backend.cc"   // NOLINT
