// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/components/brave_wallet/browser/zcash/rust/orchard_shard_tree_impl.h"

#include "base/memory/ptr_util.h"
#include "base/ranges/algorithm.h"
#include "brave/components/brave_wallet/browser/zcash/rust/cxx/src/shard_store.h"
#include "brave/components/brave_wallet/common/zcash_utils.h"

namespace brave_wallet::orchard {

::brave_wallet::OrchardShardAddress From(const FfiShardAddress& addr) {
  return ::brave_wallet::OrchardShardAddress{addr.level, addr.index};
}

FfiShardAddress From(const ::brave_wallet::OrchardShardAddress& addr) {
  return FfiShardAddress{addr.level, addr.index};
}

FfiCap From(::brave_wallet::OrchardCap& cap) {
  ::rust::Vec<uint8_t> data;
  base::ranges::copy(cap.data.begin(), cap.data.end(), data.begin());
  return FfiCap{std::move(data)};
}

::brave_wallet::OrchardCap From(const FfiCap& cap) {
  std::vector<uint8_t> data;
  base::ranges::copy(cap.data.begin(), cap.data.end(), data.begin());
  return ::brave_wallet::OrchardCap{std::move(data)};
}

::brave_wallet::OrchardShard From(const FfiShardTree& tree) {
  std::vector<uint8_t> data;
  base::ranges::copy(tree.data.begin(), tree.data.end(), data.begin());
  return ::brave_wallet::OrchardShard(From(tree.address), tree.hash,
                                      std::move(data), tree.contains_marked);
}

FfiShardTree From(const ::brave_wallet::OrchardShard& tree) {
  ::rust::Vec<uint8_t> data;
  base::ranges::copy(tree.shard_data.begin(), tree.shard_data.end(),
                     data.begin());

  return FfiShardTree{From(tree.address), tree.root_hash, std::move(data),
                      tree.contains_marked};
}

FfiCheckpoint From(const ::brave_wallet::OrchardCheckpoint& checkpoint) {
  return FfiCheckpoint{};
}

FfiCheckpointBundle From(const ::brave_wallet::OrchardCheckpointBundle& checkpoint_bundle) {
  return FfiCheckpointBundle(checkpoint_bundle.checkpoint_id, From(checkpoint_bundle.checkpoint));
}

::brave_wallet::OrchardCheckpoint From(const FfiCheckpoint& checkpoint) {
  return ::brave_wallet::OrchardCheckpoint{ checkpoint.empty, checkpoint.position, std::vector<uint32_t>(checkpoint.mark_removed.begin(), checkpoint.mark_removed.end()) };
}

ShardStoreStatusCode orchard_get_shard(const ShardStoreContext& ctx,
                                       const FfiShardAddress& addr,
                                       FfiShardTree& input) {
  auto shard = ctx.GetShard(From(addr));
  if (!shard.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!shard.value()) {
    return ShardStoreStatusCode::None;
  }
  input = From(**shard);
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_last_shard(const ShardStoreContext& ctx,
                                        FfiShardTree& input) {
  auto shard = ctx.LastShard(0);
  if (!shard.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!shard.value()) {
    return ShardStoreStatusCode::None;
  }
  input = From(**shard);
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_put_shard(ShardStoreContext& ctx,
                                       const FfiShardTree& tree) {
  auto result = ctx.PutShard(From(tree));
  if (!result.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!result.value()) {
    return ShardStoreStatusCode::None;
  }
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_get_shard_roots(
    const ShardStoreContext& ctx,
    ::rust::Vec<FfiShardAddress>& input) {
  auto shard = ctx.GetShardRoots(0);
  if (!shard.has_value()) {
    return ShardStoreStatusCode::Error;
  }
  for (const auto& root : *shard) {
    input.push_back(From(root));
  }
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_truncate(ShardStoreContext& ctx,
                                      const FfiShardAddress& address) {
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_get_cap(const ShardStoreContext& ctx,
                                     FfiCap& input) {
  auto result = ctx.GetCap();
  if (!result.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!result.value()) {
    return ShardStoreStatusCode::None;
  }
  input = From(**result);
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_put_cap(ShardStoreContext& ctx,
                                     const FfiCap& tree) {
  auto result = ctx.PutCap(From(tree));
  if (!result.has_value()) {
    return ShardStoreStatusCode::Error;
  }
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_min_checkpoint_id(const ShardStoreContext& ctx,
                                               uint32_t& input) {
  auto result = ctx.MinCheckpointId();
  if (!result.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!result.value()) {
    return ShardStoreStatusCode::None;
  }
  input = **result;
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_max_checkpoint_id(const ShardStoreContext& ctx,
                                               uint32_t& input) {
  auto result = ctx.MaxCheckpointId();
  if (!result.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!result.value()) {
    return ShardStoreStatusCode::None;
  }
  input = **result;
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_add_checkpoint(ShardStoreContext& ctx,
                                            uint32_t checkpoint_id,
                                            const FfiCheckpoint& checkpoint) {
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_checkpoint_count(const ShardStoreContext& ctx,
                                              size_t& into) {
  auto result = ctx.CheckpointCount();
  if (!result.has_value()) {
    return ShardStoreStatusCode::Error;
  }
  into = *result;
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_get_checkpoint_at_depth(
    const ShardStoreContext& ctx,
    size_t depth,
    uint32_t& into_checkpoint_id,
    FfiCheckpoint& into_checkpoint) {
  auto checkpoint_id = ctx.GetCheckpointAtDepth(depth);
  if (!checkpoint_id.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!checkpoint_id.value()) {
    return ShardStoreStatusCode::None;
  }
  into_checkpoint_id = **checkpoint_id;

  auto checkpoint = ctx.GetCheckpoint(into_checkpoint_id);
  if (!checkpoint.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!checkpoint.value()) {
    return ShardStoreStatusCode::None;
  }
  into_checkpoint = From((**checkpoint).checkpoint);
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_get_checkpoint(const ShardStoreContext& ctx,
                                            uint32_t checkpoint_id,
                                            FfiCheckpoint& input) {
  auto checkpoint = ctx.GetCheckpoint(checkpoint_id);
  if (!checkpoint.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!checkpoint.value()) {
    return ShardStoreStatusCode::None;
  }
  input = From((**checkpoint).checkpoint);
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_update_checkpoint(
    ShardStoreContext& ctx,
    uint32_t checkpoint_id,
    const FfiCheckpoint& checkpoint) {
  auto result = ctx.UpdateCheckpoint(checkpoint_id, From(checkpoint));
  if (!result.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!result.value()) {
    return ShardStoreStatusCode::None;
  }
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_remove_checkpoint(ShardStoreContext& ctx,
                                               uint32_t checkpoint_id) {
  auto result = ctx.RemoveCheckpoint(checkpoint_id);
  if (!result.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!result.value()) {
    return ShardStoreStatusCode::None;
  }
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_truncate_checkpoint(ShardStoreContext& ctx,
                                                 uint32_t checkpoint_id) {
  auto result = ctx.TruncateCheckpoints(checkpoint_id);
  if (!result.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (!result.value()) {
    return ShardStoreStatusCode::None;
  }
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_with_checkpoints(const ShardStoreContext& ctx,
                                              size_t limit,
                                              rust::cxxbridge1::Fn<ShardStoreStatusCode(uint32_t,
                                                                        const FfiCheckpoint& checkpoint)> fn) {
  auto checkpoints = ctx.GetCheckpoints(limit);
  if (!checkpoints.has_value()) {
    return ShardStoreStatusCode::Error;
  } else if (checkpoints->empty()) {
    return ShardStoreStatusCode::None;
  }

  // TODO(cypt4): Make via a call to the DB
  for (const auto& checkpoint : checkpoints.value()) {
    auto r = fn(checkpoint.checkpoint_id, From(checkpoint.checkpoint));
    if (r != ShardStoreStatusCode::Ok) {
      return r;
    }
  }
  return ShardStoreStatusCode::Ok;
}

ShardStoreStatusCode orchard_get_checkpoints(const ShardStoreContext& ctx,
                                             size_t limit,
                                             ::rust::Vec<FfiCheckpoint>& into) {
  auto checkpoints = ctx.GetCheckpoints(limit);
  if (!checkpoints.has_value()) {
    return ShardStoreStatusCode::Error;
  }
  if (checkpoints->empty()) {
    return ShardStoreStatusCode::None;
  }
  for (const auto& checkpoint : checkpoints.value()) {
    into.push_back(From(checkpoint.checkpoint));
  }
  return ShardStoreStatusCode::Ok;
}

bool OrchardShardTreeImpl::InsertCommitments(
    std::vector<::brave_wallet::OrchardCommitment> commitments) {
  // ::rust::Vec<FfiOrchardCommitment> rust_commitments;
  // for (const auto& item : commitments) {
  //   OrchardCommitment commitment;
  //   rust_commitments.emplace_back(std::move(orchard_compact_action));
  // }
  // return orcard_shard_tree_->insert_commitments(std::move(rust_commitments));
  return false;
}

bool OrchardShardTreeImpl::InsertSubtreeRoots(
    std::vector<::brave_wallet::OrchardShard> subtrees) {
  // ::rust::Vec<FfiShardTree> rust_subtrees;
  // for (const auto& item : subtrees) {
  //   FfiShardTree rust_subtree;
  //   rust_subtrees.emplace_back(std::move(rust_subtree));
  // }
  // return orcard_shard_tree_->insert_subtree_roots(std::move(rust_subtrees));
  return false;
}

OrchardShardTreeImpl::~OrchardShardTreeImpl() {}

// static
std::unique_ptr<OrchardShardTree> OrchardShardTree::Create(
    std::unique_ptr<::brave_wallet::OrchardShardTreeDelegate> delegate) {
  // auto shard_tree_context = base::WrapUnique<ShardStoreContext>(
  //     new ShardStoreContextImpl(std::move(delegate)));
  auto shard_tree_result =
      ::brave_wallet::orchard::create_shard_tree(std::move(delegate));
  if (!shard_tree_result->is_ok()) {
    return nullptr;
  }
  return base::WrapUnique<OrchardShardTree>(
      new OrchardShardTreeImpl(shard_tree_result->unwrap()));
}

}  // namespace brave_wallet::orchard
