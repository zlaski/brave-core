/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/internal/orchard_shard_tree_manager.h"

#include "brave/components/brave_wallet/common/zcash_utils.h"

namespace brave_wallet {

// static
std::unique_ptr<OrchardShardTreeManager> OrchardShardTreeManager::Create(
    std::unique_ptr<OrchardShardTreeDelegate> delegate) {
  auto shard_tree = orchard::OrchardShardTree::Create(std::move(delegate));
  if (!shard_tree) {
    // NOTREACHED
    return nullptr;
  }
  return std::make_unique<OrchardShardTreeManager>(std::move(shard_tree));
}

OrchardShardTreeManager::OrchardShardTreeManager(
    std::unique_ptr<::brave_wallet::orchard::OrchardShardTree> shard_tree) {
  orchard_shard_tree_ = std::move(shard_tree);
}

OrchardShardTreeManager::~OrchardShardTreeManager() {}

bool OrchardShardTreeManager::InsertCommitments(
    std::vector<OrchardCommitment> commitments) {
  return orchard_shard_tree_->InsertCommitments(std::move(commitments));
}

bool OrchardShardTreeManager::InsertSubtreeRoots(
    std::vector<OrchardShard> roots) {
  return orchard_shard_tree_->InsertSubtreeRoots(std::move(roots));
}

}  // namespace brave_wallet
