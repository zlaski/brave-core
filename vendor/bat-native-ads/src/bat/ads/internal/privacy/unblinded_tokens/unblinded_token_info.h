/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_PRIVACY_UNBLINDED_TOKENS_UNBLINDED_TOKEN_INFO_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_PRIVACY_UNBLINDED_TOKENS_UNBLINDED_TOKEN_INFO_H_

#include <vector>

#include "brave/components/challenge_bypass_ristretto/cxxbridge.h"

using challenge_bypass_ristretto::PublicKey;
using challenge_bypass_ristretto::UnblindedToken;
using rust::cxxbridge1::Box;

namespace ads {
namespace privacy {

struct UnblindedTokenInfo {
  UnblindedTokenInfo();
  UnblindedTokenInfo(const UnblindedTokenInfo&) = delete;
  UnblindedTokenInfo& operator=(const UnblindedTokenInfo&) = delete;
  ~UnblindedTokenInfo();

  bool operator==(const UnblindedTokenInfo& rhs) const;
  bool operator!=(const UnblindedTokenInfo& rhs) const;

  Box<UnblindedToken> value;
  Box<PublicKey> public_key;
};

using UnblindedTokenList = std::vector<UnblindedTokenInfo>;

}  // namespace privacy
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_PRIVACY_UNBLINDED_TOKENS_UNBLINDED_TOKEN_INFO_H_
