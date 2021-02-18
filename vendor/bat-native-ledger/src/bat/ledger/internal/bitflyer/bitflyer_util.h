/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVELEDGER_BITFLYER_BITFLYER_UTIL_H_
#define BRAVELEDGER_BITFLYER_BITFLYER_UTIL_H_

#include <map>
#include <string>
#include <vector>

#include "bat/ledger/ledger.h"

namespace ledger {
class LedgerImpl;

namespace bitflyer {

const char kUrlStaging[] = BITFLYER_STAGING_URL;
const char kUrlProduction[] = "https://bitflyer.jp";
const char kClientIdStaging[] = "078bf3871f3741432e813376d996a6a0";
const char kClientIdProduction[] = "078bf3871f3741432e813376d996a6a0";
const char kFeeAddressStaging[] = "039292b0-ee61-4c53-bbe7-eca4e75473b9";
const char kFeeAddressProduction[] = "";  // FIXME: Need official deposit_id
const char kACAddressStaging[] = "039292b0-ee61-4c53-bbe7-eca4e75473b9";
const char kACAddressProduction[] = "";   // FIXME: Need official deposit_id

std::string GetClientId();

std::string GetUrl();

std::string GetFeeAddress();

std::string GetACAddress();

std::string GetAuthorizeUrl(const std::string& state);

std::string GetAddUrl(const std::string& address);

std::string GetWithdrawUrl(const std::string& address);

type::ExternalWalletPtr GetWallet(LedgerImpl* ledger);

bool SetWallet(LedgerImpl* ledger, type::ExternalWalletPtr wallet);

std::string GenerateRandomString(bool testing);

std::string GetAccountUrl();

type::ExternalWalletPtr GenerateLinks(type::ExternalWalletPtr wallet);

type::ExternalWalletPtr ResetWallet(type::ExternalWalletPtr wallet);

}  // namespace bitflyer
}  // namespace ledger

#endif  // BRAVELEDGER_BITFLYER_BITFLYER_UTIL_H_
