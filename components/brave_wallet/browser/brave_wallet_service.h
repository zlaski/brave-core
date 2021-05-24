/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_BRAVE_WALLET_SERVICE_H_
#define BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_BRAVE_WALLET_SERVICE_H_

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "brave/components/brave_wallet/browser/eth_json_rpc_controller.h"
#include "brave/components/brave_wallet/browser/keyring_controller.h"
#include "components/keyed_service/core/keyed_service.h"
#include "url/gurl.h"

namespace content {
class BrowserContext;
}  // namespace content

class BraveWalletService : public KeyedService,
                           public base::SupportsWeakPtr<BraveWalletService> {
 public:
  explicit BraveWalletService(content::BrowserContext* context);
  ~BraveWalletService() override;

  brave_wallet::EthJsonRpcController* rpc_controller() const;
  brave_wallet::KeyringController* keyring_controller() const;

 private:
  content::BrowserContext* context_;
  std::unique_ptr<brave_wallet::EthJsonRpcController> rpc_controller_;
  std::unique_ptr<brave_wallet::KeyringController> keyring_controller_;
  DISALLOW_COPY_AND_ASSIGN(BraveWalletService);
};

#endif  // BRAVE_COMPONENTS_BRAVE_WALLET_BROWSER_BRAVE_WALLET_SERVICE_H_
