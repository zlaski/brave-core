/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/browser_command/brave_browser_command_handler.h"

#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/browser/brave_wallet/brave_wallet_service_factory.h"
#include "brave/browser/ui/brave_rewards/rewards_panel_coordinator.h"
#include "brave/browser/ui/browser_commands.h"
#include "brave/components/brave_vpn/common/buildflags/buildflags.h"
#include "brave/components/constants/webui_url_constants.h"
#include "chrome/browser/command_updater.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_navigator.h"
#include "chrome/browser/ui/chrome_pages.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/web_contents_observer.h"

#if BUILDFLAG(ENABLE_BRAVE_VPN)
#include "brave/browser/brave_vpn/vpn_utils.h"
#endif

namespace {

using browser_command::mojom::Command;
using browser_command::mojom::CommandHandler;

bool CanShowWalletOnboarding(Profile* profile) {
  CHECK(profile);
  return brave_wallet::BraveWalletServiceFactory::GetServiceForContext(profile);
}

bool CanShowRewardsOnboarding(Profile* profile) {
  CHECK(profile);
  return brave_rewards::RewardsServiceFactory::GetForProfile(profile);
}

bool CanShowVPNBubble(Profile* profile) {
  CHECK(profile);
#if BUILDFLAG(ENABLE_BRAVE_VPN)
  return brave_vpn::IsAllowedForContext(profile);
#else
  return false;
#endif
}

class Delegate : public BraveBrowserCommandHandler::BrowserDelegate {
 public:
  explicit Delegate(Profile* profile) : profile_(profile) {}
  ~Delegate() override = default;

  void OpenURL(const GURL& url, WindowOpenDisposition disposition) override {
    NavigateParams params(profile_, url, ui::PAGE_TRANSITION_LINK);
    params.disposition = disposition;
    Navigate(&params);
  }

  void OpenRewardsPanel() override {
    auto* panel_coordinator =
        brave_rewards::RewardsPanelCoordinator::FromBrowser(GetBrowser());
    if (panel_coordinator) {
      panel_coordinator->OpenRewardsPanel();
    }
  }

  void OpenVPNPanel() override {
#if BUILDFLAG(ENABLE_BRAVE_VPN)
    brave::ShowBraveVPNBubble(GetBrowser());
#endif
  }

 private:
  Browser* GetBrowser() { return chrome::FindBrowserWithProfile(profile_); }

  raw_ptr<Profile> profile_;
};

}  // namespace

BraveBrowserCommandHandler::BraveBrowserCommandHandler(
    mojo::PendingReceiver<CommandHandler> pending_command_handler,
    Profile* profile,
    std::vector<Command> supported_commands)
    : BrowserCommandHandler(std::move(pending_command_handler),
                            profile,
                            std::move(supported_commands)),
      delegate_(std::make_unique<Delegate>(profile)),
      profile_(profile) {
  CHECK(profile_);
}

BraveBrowserCommandHandler::~BraveBrowserCommandHandler() = default;

void BraveBrowserCommandHandler::CanExecuteCommand(
    Command command,
    CanExecuteCommandCallback callback) {
  if (CanExecute(command)) {
    std::move(callback).Run(true);
  } else {
    BrowserCommandHandler::CanExecuteCommand(command, std::move(callback));
  }
}

void BraveBrowserCommandHandler::ExecuteCommandWithDisposition(
    int id,
    WindowOpenDisposition disposition) {
  Command command = static_cast<Command>(id);
  if (!CanExecute(command)) {
    return;
  }
  switch (static_cast<Command>(id)) {
    case Command::kOpenWalletOnboarding:
      delegate_->OpenURL(GURL(kBraveUIWalletURL), disposition);
      break;
    case Command::kOpenWeb3Settings:
      delegate_->OpenURL(GURL(chrome::GetSettingsUrl("web3")), disposition);
      break;
    case Command::kOpenRewardsOnboarding:
      delegate_->OpenRewardsPanel();
      break;
    case Command::kOpenContentFilterSettings:
      delegate_->OpenURL(GURL(chrome::GetSettingsUrl("shields/filters")),
                         disposition);
      break;
    case Command::kOpenShieldsSettings:
      delegate_->OpenURL(GURL(chrome::GetSettingsUrl("shields")), disposition);
      break;
    case Command::kOpenPrivacySettings:
      delegate_->OpenURL(GURL(chrome::GetSettingsUrl("privacy")), disposition);
      break;
    case Command::kOpenVPNOnboarding:
      delegate_->OpenVPNPanel();
      break;
    default:
      break;
  }
}

bool BraveBrowserCommandHandler::CanExecute(Command command) {
  if (!GetCommandUpdater()->SupportsCommand(static_cast<int>(command))) {
    return false;
  }
  switch (command) {
    case Command::kOpenWalletOnboarding:
      return CanShowWalletOnboarding(profile_);
    case Command::kOpenWeb3Settings:
      return true;
    case Command::kOpenRewardsOnboarding:
      return CanShowRewardsOnboarding(profile_);
    case Command::kOpenContentFilterSettings:
      return true;
    case Command::kOpenShieldsSettings:
      return true;
    case Command::kOpenPrivacySettings:
      return true;
    case Command::kOpenVPNOnboarding:
      return CanShowVPNBubble(profile_);
    default:
      return false;
  }
}
