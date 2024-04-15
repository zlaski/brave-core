/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <memory>
#include <string>
#include <vector>

#include "base/test/bind.h"
#include "base/test/test_future.h"
#include "brave/browser/ui/webui/browser_command/brave_browser_command_handler.h"
#include "brave/components/brave_vpn/common/buildflags/buildflags.h"
#include "chrome/test/base/scoped_testing_local_state.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/testing_profile.h"
#include "content/public/test/browser_task_environment.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "testing/gtest/include/gtest/gtest.h"

using browser_command::mojom::ClickInfo;
using browser_command::mojom::Command;
using browser_command::mojom::CommandHandler;

class TestBrowserDelegate : public BraveBrowserCommandHandler::BrowserDelegate {
 public:
  using AddActionCallback = base::RepeatingCallback<void(std::string)>;

  explicit TestBrowserDelegate(AddActionCallback add_action)
      : add_action_(std::move(add_action)) {}

  ~TestBrowserDelegate() override = default;

  void OpenURL(const GURL& url, WindowOpenDisposition disposition) override {
    add_action_.Run("open-url: " + url.spec());
  }

  void OpenRewardsPanel() override { add_action_.Run("open-rewards-panel"); }
  void OpenVPNPanel() override { add_action_.Run("open-vpn-panel"); }

 private:
  AddActionCallback add_action_;
};

class BraveBrowserCommandHandlerTest : public testing::Test {
 protected:
  mojo::Remote<CommandHandler>& CreateHandler(
      Profile* profile,
      std::vector<Command> supported_commands) {
    command_handler_ = std::make_unique<BraveBrowserCommandHandler>(
        remote_.BindNewPipeAndPassReceiver(), profile,
        std::move(supported_commands));

    command_handler_->SetBrowserDelegateForTesting(
        std::make_unique<TestBrowserDelegate>(
            base::BindLambdaForTesting([this](std::string action) {
              actions_.push_back(std::move(action));
            })));

    return remote_;
  }

  const std::vector<std::string>& actions() { return actions_; }

 private:
  content::BrowserTaskEnvironment task_environment_;
  ScopedTestingLocalState local_state{TestingBrowserProcess::GetGlobal()};
  mojo::Remote<CommandHandler> remote_;
  std::unique_ptr<BraveBrowserCommandHandler> command_handler_;
  std::vector<std::string> actions_;
};

TEST_F(BraveBrowserCommandHandlerTest, OnlySupportedCommandsAreExecuted) {
  TestingProfile::Builder builder;
  auto profile = builder.Build();
  auto& handler = CreateHandler(profile.get(), {});

  base::test::TestFuture<bool> future;
  handler->ExecuteCommand(Command::kOpenWeb3Settings, ClickInfo::New(),
                          future.GetCallback());

  ASSERT_FALSE(future.Get<0>());
  ASSERT_TRUE(actions().empty());
}

TEST_F(BraveBrowserCommandHandlerTest, CommandsExecuted) {
  TestingProfile::Builder builder;
  auto profile = builder.Build();
  auto& handler = CreateHandler(
      profile.get(),
      {Command::kOpenWalletOnboarding, Command::kOpenWeb3Settings,
       Command::kOpenRewardsOnboarding, Command::kOpenContentFilterSettings,
       Command::kOpenShieldsSettings, Command::kOpenPrivacySettings,
       Command::kOpenVPNOnboarding});

  base::test::TestFuture<bool> future;

  handler->ExecuteCommand(Command::kOpenWalletOnboarding, ClickInfo::New(),
                          base::DoNothing());
  handler->ExecuteCommand(Command::kOpenWeb3Settings, ClickInfo::New(),
                          base::DoNothing());
  handler->ExecuteCommand(Command::kOpenRewardsOnboarding, ClickInfo::New(),
                          base::DoNothing());
  handler->ExecuteCommand(Command::kOpenContentFilterSettings, ClickInfo::New(),
                          base::DoNothing());
  handler->ExecuteCommand(Command::kOpenShieldsSettings, ClickInfo::New(),
                          base::DoNothing());
  handler->ExecuteCommand(Command::kOpenPrivacySettings, ClickInfo::New(),
                          base::DoNothing());
  handler->ExecuteCommand(Command::kOpenVPNOnboarding, ClickInfo::New(),
                          future.GetCallback());

  ASSERT_TRUE(future.Get<0>());
  EXPECT_EQ(actions()[0], "open-url: chrome://wallet/");
  EXPECT_EQ(actions()[1], "open-url: chrome://settings/web3");
  EXPECT_EQ(actions()[2], "open-rewards-panel");
  EXPECT_EQ(actions()[3], "open-url: chrome://settings/shields/filters");
  EXPECT_EQ(actions()[4], "open-url: chrome://settings/shields");
  EXPECT_EQ(actions()[5], "open-url: chrome://settings/privacy");
#if BUILDFLAG(ENABLE_BRAVE_VPN)
  EXPECT_EQ(actions()[6], "open-vpn-panel");
#endif
}

TEST_F(BraveBrowserCommandHandlerTest, OffTheRecordProfile) {
  TestingProfile::Builder builder;
  auto profile = builder.Build();
  auto* otr_profile = profile->GetOffTheRecordProfile(
      Profile::OTRProfileID::CreateUniqueForTesting(),
      /*create_if_needed=*/true);

  auto& handler = CreateHandler(
      otr_profile,
      {Command::kOpenWalletOnboarding, Command::kOpenRewardsOnboarding,
       Command::kOpenVPNOnboarding, Command::kOpenPrivacySettings});

  base::test::TestFuture<bool> future;

  handler->ExecuteCommand(Command::kOpenWalletOnboarding, ClickInfo::New(),
                          base::DoNothing());
  handler->ExecuteCommand(Command::kOpenRewardsOnboarding, ClickInfo::New(),
                          base::DoNothing());
  handler->ExecuteCommand(Command::kOpenVPNOnboarding, ClickInfo::New(),
                          base::DoNothing());
  handler->ExecuteCommand(Command::kOpenPrivacySettings, ClickInfo::New(),
                          future.GetCallback());

  ASSERT_TRUE(future.Get<0>());
  EXPECT_EQ(actions()[0], "open-url: chrome://settings/privacy");
}
