/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/brave_education/education_command_handler.h"

#include <utility>

#include "brave/browser/brave_rewards/rewards_util.h"
#include "brave/browser/ui/brave_rewards/rewards_panel_coordinator.h"
#include "brave/components/brave_rewards/common/pref_names.h"
#include "chrome/browser/command_updater.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_finder.h"
#include "components/prefs/pref_service.h"

namespace {

Browser* GetBrowser(Profile* profile) {
  return chrome::FindBrowserWithProfile(profile);
}

bool CanShowRewardsOnboarding(Profile* profile) {
  CHECK(profile);
  return brave_rewards::IsSupportedForProfile(profile) &&
         !profile->GetPrefs()->GetBoolean(brave_rewards::prefs::kEnabled);
}

bool MaybeOpenRewardsOnboarding(Profile* profile) {
  CHECK(profile);
  if (!CanShowRewardsOnboarding(profile)) {
    return false;
  }
  auto* panel_coordinator =
      brave_rewards::RewardsPanelCoordinator::FromBrowser(GetBrowser(profile));
  return panel_coordinator && panel_coordinator->OpenRewardsPanel();
}

}  // namespace

namespace brave_education {

EducationCommandHandler::EducationCommandHandler(
    mojo::PendingReceiver<browser_command::mojom::CommandHandler>
        pending_command_handler,
    Profile* profile,
    std::vector<browser_command::mojom::Command> supported_commands)
    : BrowserCommandHandler(std::move(pending_command_handler),
                            profile,
                            std::move(supported_commands)),
      profile_(profile) {
  CHECK(profile_);
}

EducationCommandHandler::~EducationCommandHandler() = default;

void EducationCommandHandler::CanExecuteCommand(
    browser_command::mojom::Command command,
    CanExecuteCommandCallback callback) {
  auto can_execute = [&]() {
    if (!GetCommandUpdater()->SupportsCommand(static_cast<int>(command))) {
      return false;
    }
    switch (command) {
      case browser_command::mojom::Command::kOpenRewardsOnboarding:
        return CanShowRewardsOnboarding(profile_);
      default:
        return false;
    }
  };

  std::move(callback).Run(can_execute());
}

void EducationCommandHandler::ExecuteCommandWithDisposition(
    int id,
    WindowOpenDisposition disposition) {
  auto command = static_cast<browser_command::mojom::Command>(id);
  switch (command) {
    case browser_command::mojom::Command::kOpenRewardsOnboarding:
      MaybeOpenRewardsOnboarding(profile_);
      break;
    default:
      break;
  }
}

}  // namespace brave_education
