/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <utility>

#include "base/base64.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "bat/ledger/internal/credentials/credentials_util.h"
#include "brave/components/challenge_bypass_ristretto/cxxbridge.h"

namespace ledger {
namespace credential {

using challenge_bypass_ristretto::BlindedToken;
using challenge_bypass_ristretto::decode_base64_batch_dleq_proof;
using challenge_bypass_ristretto::decode_base64_blinded_tokens;
using challenge_bypass_ristretto::decode_base64_public_key;
using challenge_bypass_ristretto::decode_base64_signed_tokens;
using challenge_bypass_ristretto::decode_base64_tokens;
using challenge_bypass_ristretto::decode_base64_unblinded_token;
using challenge_bypass_ristretto::generate_token;
using challenge_bypass_ristretto::Token;
using rust::cxxbridge1::Box;

std::vector<Box<Token>> GenerateCreds(const int count) {
  DCHECK_GT(count, 0);
  std::vector<Box<Token>> creds;

  for (auto i = 0; i < count; i++) {
    auto cred = challenge_bypass_ristretto::generate_token();
    creds.push_back(std::move(cred));
  }

  return creds;
}

std::string GetCredsJSON(const std::vector<Box<Token>>& creds) {
  base::Value creds_list(base::Value::Type::LIST);
  for (auto& cred : creds) {
    auto cred_base64 = std::string(cred->encode_base64());
    auto cred_value = base::Value(cred_base64);
    creds_list.Append(std::move(cred_value));
  }

  std::string json;
  base::JSONWriter::Write(creds_list, &json);
  return json;
}

std::vector<Box<BlindedToken>> GenerateBlindCreds(
    const std::vector<Box<Token>>& creds) {
  DCHECK_NE(creds.size(), 0UL);

  std::vector<Box<BlindedToken>> blinded_creds;
  for (unsigned int i = 0; i < creds.size(); i++) {
    auto& cred = creds.at(i);
    auto blinded_cred = cred->blind();

    blinded_creds.push_back(std::move(blinded_cred));
  }

  return blinded_creds;
}

std::string GetBlindedCredsJSON(
    const std::vector<Box<BlindedToken>>& blinded_creds) {
  base::Value blinded_list(base::Value::Type::LIST);
  for (auto& cred : blinded_creds) {
    auto cred_base64 = cred->encode_base64();
    auto cred_value = base::Value(std::string(cred_base64));
    blinded_list.Append(std::move(cred_value));
  }

  std::string json;
  base::JSONWriter::Write(blinded_list, &json);
  return json;
}

std::vector<std::string> ParseStringToBaseList(
    const std::string& string_list) {
  base::Optional<base::Value> value = base::JSONReader::Read(string_list);

  std::vector<std::string> results;
  if (!value || !value->is_list())
    return results;

  for (const base::Value& item : value->GetList()) {
    results.push_back(item.GetString());
  }
  return results;
}

bool UnBlindCreds(const type::CredsBatch& creds_batch,
                  std::vector<std::string>* unblinded_encoded_creds,
                  std::string* error) {
  DCHECK(error && unblinded_encoded_creds);

  auto batch_proof = decode_base64_batch_dleq_proof(creds_batch.batch_proof);

  if (!batch_proof->is_ok()) {
    *error = std::string(batch_proof->error().msg);
    return false;
  }

  auto creds_base64 = ParseStringToBaseList(creds_batch.creds);
  auto creds = decode_base64_tokens(creds_base64);

  if (!creds->is_ok()) {
    *error = std::string(creds->error().msg);
    return false;
  }

  auto blinded_creds_base64 = ParseStringToBaseList(creds_batch.blinded_creds);
  auto blinded_creds = decode_base64_blinded_tokens(blinded_creds_base64);

  if (!blinded_creds->is_ok()) {
    *error = std::string(blinded_creds->error().msg);
    return false;
  }

  auto signed_creds_base64 = ParseStringToBaseList(creds_batch.signed_creds);
  auto signed_creds = decode_base64_signed_tokens(signed_creds_base64);

  if (!signed_creds->is_ok()) {
    *error = std::string(signed_creds->error().msg);
    return false;
  }

  const auto public_key = decode_base64_public_key(creds_batch.public_key);

  auto unblinded_cred = batch_proof->unwrap().verify_and_unblind(
      creds->unwrap(), blinded_creds->unwrap(), signed_creds->unwrap(),
      public_key->unwrap());

  if (!unblinded_cred->is_ok()) {
    *error = std::string(unblinded_cred->error().msg);
    return false;
  }

  for (const auto& cred : unblinded_cred->unwrap().as_vec()) {
    unblinded_encoded_creds->push_back(std::string(cred.encode_base64()));
  }

  return true;
}

bool UnBlindCredsMock(const type::CredsBatch& creds,
                      std::vector<std::string>* unblinded_encoded_creds) {
  DCHECK(unblinded_encoded_creds);

  auto signed_creds_base64 = ParseStringToBaseList(creds.signed_creds);

  for (const auto& item : signed_creds_base64) {
    unblinded_encoded_creds->push_back(item);
  }

  return true;
}

std::string ConvertRewardTypeToString(const type::RewardsType type) {
  switch (type) {
    case type::RewardsType::AUTO_CONTRIBUTE: {
      return "auto-contribute";
    }
    case type::RewardsType::ONE_TIME_TIP: {
      return "oneoff-tip";
    }
    case type::RewardsType::RECURRING_TIP: {
      return "recurring-tip";
    }
    case type::RewardsType::PAYMENT: {
      return "payment";
    }
    case type::RewardsType::TRANSFER: {
      return "";
    }
  }
}

void GenerateCredentials(const std::vector<type::UnblindedToken>& token_list,
                         const std::string& body,
                         base::Value* credentials) {
  DCHECK(credentials);

  for (auto& item : token_list) {
    base::Value token(base::Value::Type::DICTIONARY);
    bool success;
    if (ledger::is_testing) {
      success = GenerateSuggestionMock(item.token_value, item.public_key, body,
                                       &token);
    } else {
      success =
          GenerateSuggestion(item.token_value, item.public_key, body, &token);
    }

    if (!success) {
      continue;
    }

    credentials->Append(std::move(token));
  }
}

bool GenerateSuggestion(const std::string& token_value,
                        const std::string& public_key,
                        const std::string& body,
                        base::Value* result) {
  DCHECK(result);
  if (token_value.empty() || public_key.empty() || body.empty()) {
    return false;
  }

  auto unblinded = decode_base64_unblinded_token(token_value);
  if (!unblinded->is_ok()) {
    return false;
  }

  auto verification_key = unblinded->unwrap().derive_verification_key();
  auto signature = verification_key->sign(body);
  const std::string pre_image =
      std::string(unblinded->unwrap().preimage().encode_base64());

  result->SetStringKey("t", pre_image);
  result->SetStringKey("publicKey", public_key);
  result->SetStringKey("signature", std::string(signature->encode_base64()));
  return true;
}

bool GenerateSuggestionMock(const std::string& token_value,
                            const std::string& public_key,
                            const std::string& body,
                            base::Value* result) {
  DCHECK(result);
  result->SetStringKey("t", token_value);
  result->SetStringKey("publicKey", public_key);
  result->SetStringKey("signature", token_value);
  return true;
}

}  // namespace credential
}  // namespace ledger
