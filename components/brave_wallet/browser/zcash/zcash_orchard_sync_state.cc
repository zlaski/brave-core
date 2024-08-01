/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/zcash/zcash_orchard_sync_state.h"

#include "brave/components/brave_wallet/common/zcash_utils.h"

namespace brave_wallet {

namespace {

OrchardShardTreeDelegate::Error From(ZCashOrchardStorage::Error) {
  return OrchardShardTreeDelegate::Error::kStorageError;
}

}  // namespace

class OrchardShardTreeDelegateImpl : public OrchardShardTreeDelegate {
 public:
  OrchardShardTreeDelegateImpl(mojom::AccountIdPtr account_id,
                               scoped_refptr<ZCashOrchardStorage> storage)
      : account_id_(std::move(account_id)), storage_(storage) {}

  ~OrchardShardTreeDelegateImpl() override {}

  base::expected<std::optional<OrchardCap>, Error> GetCap() const override {
    auto result = storage_->GetCap(account_id_.Clone());
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(*result);
  }

  base::expected<bool, Error> PutCap(OrchardCap cap) override {
    auto result = storage_->PutCap(account_id_.Clone(), cap);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return *result;
  }

  base::expected<bool, Error> Truncate(uint32_t block_height) override {
    auto result = storage_->TruncateShards(account_id_.Clone(), block_height);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return *result;
  }

  base::expected<std::optional<uint32_t>, Error> GetLatestShardIndex()
      const override {
    auto result = storage_->GetLatestShardIndex(account_id_.Clone());
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(*result);
  }

  base::expected<bool, Error> PutShard(OrchardShard shard) override {
    auto result = storage_->PutShard(account_id_.Clone(), shard);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return *result;
  }

  base::expected<std::optional<OrchardShard>, Error> GetShard(
      OrchardShardAddress address) const override {
    auto result = storage_->GetShard(account_id_.Clone(), address);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(*result);
  }

  base::expected<std::optional<OrchardShard>, Error> LastShard(
      uint8_t shard_height) const override {
    auto result = storage_->LastShard(account_id_.Clone(), shard_height);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(*result);
  }

  base::expected<size_t, Error> CheckpointCount() const override {
    auto result = storage_->CheckpointCount(account_id_.Clone());
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return *result;
  }

  base::expected<std::optional<uint32_t>, Error> MinCheckpointId()
      const override {
    auto result = storage_->MinCheckpointId(account_id_.Clone());
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(result.value());
  }

  base::expected<std::optional<uint32_t>, Error> MaxCheckpointId()
      const override {
    auto result = storage_->MaxCheckpointId(account_id_.Clone());
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(result.value());
  }

  base::expected<std::optional<uint32_t>, Error> GetCheckpointAtDepth(
      uint32_t depth) const override {
    auto result = storage_->GetCheckpointAtDepth(account_id_.Clone(), depth);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(result.value());
  }

  base::expected<std::optional<OrchardCheckpointBundle>, Error> GetCheckpoint(uint32_t checkpoint_id)
      const override {
    auto result = storage_->GetCheckpoint(account_id_.Clone(), checkpoint_id);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(result.value());
  }

  base::expected<std::vector<OrchardCheckpointBundle>, Error> GetCheckpoints(size_t limit)
      const override {
    auto result = storage_->GetCheckpoints(account_id_.Clone(), limit);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(result.value());
  }

  base::expected<bool, Error> TruncateCheckpoints(uint32_t checkpoint_id) override {
    auto result = storage_->TruncateCheckpoints(account_id_.Clone(), checkpoint_id);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(result.value());
  }

  base::expected<bool, Error> RemoveCheckpoint(
      uint32_t checkpoint_id) override {
    auto result = storage_->RemoveCheckpoint(account_id_.Clone(), checkpoint_id);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(result.value());
  }

  base::expected<bool, Error> RemoveCheckpointAt(uint32_t depth) override {
    auto result = storage_->RemoveCheckpointAt(account_id_.Clone(), depth);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(result.value());
  }

  base::expected<bool, Error> PutShardRoots(
      uint8_t shard_roots_height,
      uint32_t start_position,
      std::vector<OrchardShard> roots) override {
    auto result = storage_->PutShardRoots(
        account_id_.Clone(), shard_roots_height,
        start_position, std::move(roots));
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(result.value());
  }

  base::expected<std::vector<OrchardShardAddress>, Error> GetShardRoots(
      uint8_t shard_level) const override {
    auto result = storage_->GetShardRoots(
        account_id_.Clone(), shard_level);
    if (!result.has_value()) {
      return base::unexpected(From(result.error()));
    }
    return std::move(result.value());
  }

 private:
  mojom::AccountIdPtr account_id_;
  scoped_refptr<ZCashOrchardStorage> storage_;
};

ZCashOrchardSyncState::ZCashOrchardSyncState(base::FilePath path_to_database) {
  storage_ = base::MakeRefCounted<ZCashOrchardStorage>(path_to_database);
}

ZCashOrchardSyncState::~ZCashOrchardSyncState() {}

OrchardShardTreeManager* ZCashOrchardSyncState::GetOrCreateShardTreeManager(const mojom::AccountIdPtr& account_id) {
  if (shard_tree_managers_.find(account_id) == shard_tree_managers_.end()) {
    shard_tree_managers_[account_id.Clone()] = OrchardShardTreeManager::Create(
        std::make_unique<OrchardShardTreeDelegateImpl>(account_id.Clone(), storage_));
  }
  return shard_tree_managers_[account_id.Clone()].get();
}

base::expected<ZCashOrchardStorage::AccountMeta, ZCashOrchardStorage::Error>
ZCashOrchardSyncState::RegisterAccount(
    mojom::AccountIdPtr account_id,
    uint64_t account_birthday_block,
    const std::string& account_bithday_block_hash) {
  return storage_->RegisterAccount(std::move(account_id),
                                   account_birthday_block,
                                   account_bithday_block_hash);
}

base::expected<ZCashOrchardStorage::AccountMeta, ZCashOrchardStorage::Error>
ZCashOrchardSyncState::GetAccountMeta(mojom::AccountIdPtr account_id) {
  return storage_->GetAccountMeta(std::move(account_id));
}

std::optional<ZCashOrchardStorage::Error>
ZCashOrchardSyncState::HandleChainReorg(mojom::AccountIdPtr account_id,
                                        uint32_t reorg_block_id,
                                        const std::string& reorg_block_hash) {
  return storage_->HandleChainReorg(std::move(account_id), reorg_block_id,
                                    reorg_block_hash);
}

base::expected<std::vector<OrchardNote>, ZCashOrchardStorage::Error>
ZCashOrchardSyncState::GetSpendableNotes(mojom::AccountIdPtr account_id) {
  return storage_->GetSpendableNotes(std::move(account_id));
}

base::expected<std::vector<OrchardNullifier>, ZCashOrchardStorage::Error>
ZCashOrchardSyncState::GetNullifiers(mojom::AccountIdPtr account_id) {
  return storage_->GetNullifiers(std::move(account_id));
}

base::expected<uint32_t, ZCashOrchardStorage::Error>
ZCashOrchardSyncState::GetLatestShardIndex(mojom::AccountIdPtr account_id) {
  return storage_->GetLatestShardIndex(std::move(account_id));
}

std::optional<ZCashOrchardStorage::Error>
ZCashOrchardSyncState::UpdateSubtreeRoots(
    mojom::AccountIdPtr account_id,
    std::vector<zcash::mojom::SubtreeRootPtr> roots) {
  return std::nullopt;
}

std::optional<ZCashOrchardStorage::Error> ZCashOrchardSyncState::UpdateNotes(
    mojom::AccountIdPtr account_id,
    const std::vector<OrchardNote>& notes_to_add,
    const std::vector<OrchardNullifier>& notes_to_delete,
    const uint32_t latest_scanned_block,
    const std::string& latest_scanned_block_hash) {
  return storage_->UpdateNotes(std::move(account_id), notes_to_add,
                               notes_to_delete, latest_scanned_block,
                               latest_scanned_block_hash);
}

void ZCashOrchardSyncState::ResetDatabase() {
  storage_->ResetDatabase();
}

}  // namespace brave_wallet
