/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVELEDGER_CREDENTIALS_CREDENTIALS_UTIL_H_
#define BRAVELEDGER_CREDENTIALS_CREDENTIALS_UTIL_H_

#include <memory>
#include <string>
#include <vector>

#include "base/values.h"
#include "bat/ledger/internal/credentials/credentials_redeem.h"
#include "bat/ledger/mojom_structs.h"
#include "brave/components/challenge_bypass_ristretto/cxxbridge.h"

using challenge_bypass_ristretto::Token;
using challenge_bypass_ristretto::BlindedToken;
using rust::cxxbridge1::Box;

namespace ledger {
namespace credential {

std::vector<Box<Token>> GenerateCreds(const int count);

std::string GetCredsJSON(const std::vector<Box<Token>>& creds);

std::vector<Box<BlindedToken>> GenerateBlindCreds(
    const std::vector<Box<Token>>& tokens);

std::string GetBlindedCredsJSON(const std::vector<Box<BlindedToken>>& blinded);

std::vector<std::string> ParseStringToBaseList(
    const std::string& string_list);

bool UnBlindCreds(
    const type::CredsBatch& creds,
    std::vector<std::string>* unblinded_encoded_creds,
    std::string* error);

bool UnBlindCredsMock(
    const type::CredsBatch& creds,
    std::vector<std::string>* unblinded_encoded_creds);

std::string ConvertRewardTypeToString(const type::RewardsType type);

void GenerateCredentials(
    const std::vector<type::UnblindedToken>& token_list,
    const std::string& body,
    base::Value* credentials);

bool GenerateSuggestion(
    const std::string& token_value,
    const std::string& public_key,
    const std::string& suggestion_encoded,
    base::Value* result);

bool GenerateSuggestionMock(
    const std::string& token_value,
    const std::string& public_key,
    const std::string& suggestion_encoded,
    base::Value* result);

}  // namespace credential
}  // namespace ledger

#endif  // BRAVELEDGER_CREDENTIALS_CREDENTIALS_UTIL_H_
