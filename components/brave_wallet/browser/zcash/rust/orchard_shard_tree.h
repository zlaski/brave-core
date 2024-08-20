// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include <vector>

#include "brave/components/brave_wallet/common/zcash_utils.h"

namespace brave_wallet::orchard {

class OrchardShardTree {
 public:
  virtual ~OrchardShardTree() = 0;

  virtual bool InsertSubtreeRoots(std::vector<OrchardShard> subtrees) = 0;
  virtual bool InsertCommitments(
      std::vector<OrchardCommitment> commitments) = 0;

  static std::unique_ptr<OrchardShardTree> Create(
      std::unique_ptr<::brave_wallet::OrchardShardTreeDelegate> delegate);
};

}  // namespace brave_wallet::orchard
