/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/strings/string_split.h"
#include "base/strings/stringprintf.h"
#include "base/memory/weak_ptr.h"
#include "base/test/bind_test_util.h"
#include "bat/ledger/internal/bat_helper.h"
#include "bat/ledger/internal/uphold/uphold_util.h"
#include "bat/ledger/internal/request/request_util.h"
#include "bat/ledger/internal/static_values.h"
#include "bat/ledger/ledger.h"
#include "brave/browser/extensions/api/brave_action_api.h"
#include "brave/common/brave_paths.h"
#include "brave/common/extensions/extension_constants.h"
#include "brave/components/brave_rewards/browser/rewards_service_factory.h"
#include "brave/components/brave_rewards/browser/rewards_service_impl.h"
#include "brave/components/brave_rewards/browser/rewards_service_observer.h"
#include "brave/components/brave_rewards/browser/rewards_notification_service_impl.h"  // NOLINT
#include "brave/components/brave_rewards/browser/rewards_notification_service_observer.h"  // NOLINT
#include "brave/components/brave_rewards/common/pref_names.h"
#include "brave/browser/ui/views/brave_actions/brave_actions_container.h"
#include "brave/browser/ui/views/location_bar/brave_location_bar_view.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/test/browser_test_utils.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/http_request.h"
#include "net/test/embedded_test_server/http_response.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/perf/perf_test.h"

// npm run test -- brave_browser_tests --filter=PublisherServerListBrowserPerfTest.*

using braveledger_request_util::ServerTypes;

using RewardsNotificationType =
    brave_rewards::RewardsNotificationService::RewardsNotificationType;

namespace {

std::unique_ptr<net::test_server::HttpResponse> HandleRequest(
    const net::test_server::HttpRequest& request) {
  std::unique_ptr<net::test_server::BasicHttpResponse> http_response(
      new net::test_server::BasicHttpResponse());
  http_response->set_code(net::HTTP_OK);
  http_response->set_content_type("text/html");

    http_response->set_content(
        "<html>"
        "  <head></head>"
        "  <body>"
        "    <div>Hello, world!</div>"
        "  </body>"
        "</html>");

  return std::move(http_response);
}

bool URLMatches(const std::string& url,
                const std::string& path,
                const std::string& prefix,
                const ServerTypes& server) {
  const std::string target_url =
      braveledger_request_util::BuildUrl(path, prefix, server);
  return (url.find(target_url) == 0);
}

namespace {

// constexpr int kTestIterations = 1;
// static const char kTestResultString[] = "PublishServerList";

std::string LoadFile() {
  auto path = base::FilePath(FILE_PATH_LITERAL("brave"))
                  .Append(FILE_PATH_LITERAL("vendor"))
                  .Append(FILE_PATH_LITERAL("bat-native-ledger"))
                  .Append(FILE_PATH_LITERAL("resources"))
                  .Append(FILE_PATH_LITERAL("full.json"));

  std::string value;
  const bool ok = ReadFileToString(path, &value);
  if (!ok)
    return {};
  return value;
}

// void CalculateMeanAndMax(const std::vector<double>& inputs,
//                          double* mean,
//                          double* std_dev,
//                          double* max) {
//   double sum = 0.0;
//   double sqr_sum = 0.0;
//   double max_so_far = 0.0;
//   size_t count = inputs.size();
//   for (const auto& input : inputs) {
//     sum += input;
//     sqr_sum += input * input;
//     max_so_far = std::max(input, max_so_far);
//   }
//   *max = max_so_far;
//   *mean = sum / count;
//   *std_dev = sqrt(std::max(0.0, count * sqr_sum - sum * sum)) / count;
// }

// void PrintMeanAndMax(const std::string& var_name,
//                      const std::string& name_modifier,
//                      const std::string& unit,
//                      const std::vector<double>& vars) {
//   double mean = 0.0;
//   double std_dev = 0.0;
//   double max = 0.0;
//   CalculateMeanAndMax(vars, &mean, &std_dev, &max);
//   perf_test::PrintResultMeanAndError(
//       kTestResultString, name_modifier, var_name + " Mean",
//       base::StringPrintf("%.0lf,%.0lf", mean, std_dev), unit, true);
//   perf_test::PrintResult(kTestResultString, name_modifier, var_name + " Max",
//                          base::StringPrintf("%.0lf", max), unit, true);
// }

}  //namespace

}  // namespace

namespace brave_test_resp {
  std::string registrarVK_;
  std::string verification_;
  std::string promotions_;
  std::string promotion_claim_;
  std::string promotion_tokens_;
  std::string captcha_;
  std::string wallet_properties_;
  std::string wallet_properties_defaults_;
  std::string uphold_auth_resp_;
  std::string uphold_transactions_resp_;
  std::string uphold_commit_resp_;
}  // namespace brave_test_resp

class PublisherServerListBrowserPerfTest
    : public InProcessBrowserTest,
      public brave_rewards::RewardsServiceObserver,
      public base::SupportsWeakPtr<PublisherServerListBrowserPerfTest> {
 public:
  PublisherServerListBrowserPerfTest() {
    // You can do set-up work for each test here
  }

  ~PublisherServerListBrowserPerfTest() override {
    // You can do clean-up work that doesn't throw exceptions here
  }

  void SetUpOnMainThread() override {
    // Code here will be called immediately after the constructor (right before
    // each test)

    InProcessBrowserTest::SetUpOnMainThread();

    host_resolver()->AddRule("*", "127.0.0.1");

    // Setup up embedded test server for HTTPS requests
    https_server_.reset(new net::EmbeddedTestServer(
        net::test_server::EmbeddedTestServer::TYPE_HTTPS));
    https_server_->SetSSLConfig(net::EmbeddedTestServer::CERT_OK);
    https_server_->RegisterRequestHandler(base::BindRepeating(&HandleRequest));
    ASSERT_TRUE(https_server_->Start());

    brave::RegisterPathProvider();
    ReadTestData();

    auto* browser_profile = browser()->profile();

    rewards_service_ = static_cast<brave_rewards::RewardsServiceImpl*>(
        brave_rewards::RewardsServiceFactory::GetForProfile(browser_profile));
    rewards_service_->ForTestingSetTestResponseCallback(
        base::BindRepeating(&PublisherServerListBrowserPerfTest::GetTestResponse,
                            base::Unretained(this)));
    rewards_service_->AddObserver(this);
    if (!rewards_service_->IsWalletInitialized()) {
      WaitForWalletInitialization();
    }
    rewards_service_->SetLedgerEnvForTesting();
  }

  void TearDown() override {
    // Code here will be called immediately after each test (right before the
    // destructor)

    InProcessBrowserTest::TearDown();
  }

  void SetUpCommandLine(base::CommandLine* command_line) override {
    // HTTPS server only serves a valid cert for localhost, so this is needed
    // to load pages from other hosts without an error
    command_line->AppendSwitch(switches::kIgnoreCertificateErrors);
  }

  PrefService* GetPrefs() const {
    return browser()->profile()->GetPrefs();
  }

  bool IsRewardsEnabled() const {
    return GetPrefs()->GetBoolean(brave_rewards::prefs::kBraveRewardsEnabled);
  }

  void GetTestResponse(const std::string& url,
                       int32_t method,
                       int* response_status_code,
                       std::string* response,
                       std::map<std::string, std::string>* headers) {
    request_made_ = true;
    std::vector<std::string> tmp = base::SplitString(url,
                                                     "/",
                                                     base::TRIM_WHITESPACE,
                                                     base::SPLIT_WANT_ALL);
    const std::string persona_url =
        braveledger_request_util::BuildUrl(REGISTER_PERSONA, PREFIX_V2);
    if (url.find(persona_url) == 0 && tmp.size() == 6) {
      *response = brave_test_resp::registrarVK_;
    } else if (URLMatches(url, REGISTER_PERSONA, PREFIX_V2,
                          ServerTypes::LEDGER) &&
               tmp.size() == 7) {
      *response = brave_test_resp::verification_;
    } else if (URLMatches(url, WALLET_PROPERTIES, PREFIX_V2,
                          ServerTypes::BALANCE)) {
      if (show_defaults_in_properties_) {
        *response = brave_test_resp::wallet_properties_defaults_;
      } else {
        *response = brave_test_resp::wallet_properties_;
      }
    } else if (URLMatches(url, "/promotions?", PREFIX_V1,
                          ServerTypes::kPromotion)) {
      *response = brave_test_resp::promotions_;
    } else if (URLMatches(url, "/promotions/", PREFIX_V1,
                          ServerTypes::kPromotion)) {
      if (url.find("claims") != std::string::npos) {
        *response = brave_test_resp::promotion_tokens_;
      } else {
        *response = brave_test_resp::promotion_claim_;
      }
    } else if (URLMatches(url, "/captchas", PREFIX_V1,
                          ServerTypes::kPromotion)) {
      *response = brave_test_resp::captcha_;
    } else if (URLMatches(url, GET_PUBLISHERS_LIST, "",
                          ServerTypes::PUBLISHER_DISTRO)) {
        *response = LoadFile();
    } else if (base::StartsWith(
        url,
        braveledger_uphold::GetAPIUrl("/oauth2/token"),
        base::CompareCase::INSENSITIVE_ASCII)) {
      *response = brave_test_resp::uphold_auth_resp_;
    } else if (base::StartsWith(
        url,
        braveledger_uphold::GetAPIUrl("/v0/me/cards"),
        base::CompareCase::INSENSITIVE_ASCII)) {
      if (base::EndsWith(
          url,
          "transactions",
          base::CompareCase::INSENSITIVE_ASCII)) {
        *response = brave_test_resp::uphold_transactions_resp_;
        *response_status_code = net::HTTP_ACCEPTED;
      } else if (base::EndsWith(
          url,
          "commit",
          base::CompareCase::INSENSITIVE_ASCII)) {
        *response = brave_test_resp::uphold_commit_resp_;
      }
    }
  }

  void WaitForWalletInitialization() {
    if (wallet_initialized_)
      return;
    wait_for_wallet_initialization_loop_.reset(new base::RunLoop);
    wait_for_wallet_initialization_loop_->Run();
  }

  void WaitForPublisherListNormalized() {
    if (publisher_list_normalized_)
      return;
    wait_for_publisher_list_normalized_loop_.reset(new base::RunLoop);
    wait_for_publisher_list_normalized_loop_->Run();
  }

  void GetEnvironment() {
    rewards_service()->GetEnvironment(
        base::Bind(&PublisherServerListBrowserPerfTest::OnGetEnvironment,
          base::Unretained(this)));
  }

  void GetDebug() {
    rewards_service()->GetDebug(
        base::Bind(&PublisherServerListBrowserPerfTest::OnGetDebug,
          base::Unretained(this)));
  }

  void GetTestDataDir(base::FilePath* test_data_dir) {
    base::ScopedAllowBlockingForTesting allow_blocking;
    ASSERT_TRUE(base::PathService::Get(brave::DIR_TEST_DATA, test_data_dir));
    *test_data_dir = test_data_dir->AppendASCII("rewards-data");
    ASSERT_TRUE(base::PathExists(*test_data_dir));
  }

  void ReadTestData() {
    base::ScopedAllowBlockingForTesting allow_blocking;
    base::FilePath path;
    GetTestDataDir(&path);
    ASSERT_TRUE(
        base::ReadFileToString(path.AppendASCII("register_persona_resp.json"),
                               &brave_test_resp::registrarVK_));
    ASSERT_TRUE(
        base::ReadFileToString(path.AppendASCII("verify_persona_resp.json"),
                               &brave_test_resp::verification_));

    ASSERT_TRUE(base::ReadFileToString(path.AppendASCII("promotions_resp.json"),
                                       &brave_test_resp::promotions_));

    ASSERT_TRUE(base::ReadFileToString(path.AppendASCII("captcha_resp.json"),
                                       &brave_test_resp::captcha_));
    ASSERT_TRUE(
        base::ReadFileToString(path.AppendASCII("promotion_claim_resp.json"),
                               &brave_test_resp::promotion_claim_));
    ASSERT_TRUE(
        base::ReadFileToString(path.AppendASCII("promotion_tokens_resp.json"),
                               &brave_test_resp::promotion_tokens_));
    ASSERT_TRUE(
        base::ReadFileToString(path.AppendASCII("wallet_properties_resp.json"),
                               &brave_test_resp::wallet_properties_));
    ASSERT_TRUE(base::ReadFileToString(
        path.AppendASCII("wallet_properties_resp_defaults.json"),
        &brave_test_resp::wallet_properties_defaults_));
    ASSERT_TRUE(base::ReadFileToString(
        path.AppendASCII("uphold_auth_resp.json"),
        &brave_test_resp::uphold_auth_resp_));
    ASSERT_TRUE(base::ReadFileToString(
        path.AppendASCII("uphold_transactions_resp.json"),
        &brave_test_resp::uphold_transactions_resp_));
    ASSERT_TRUE(base::ReadFileToString(
        path.AppendASCII("uphold_commit_resp.json"),
        &brave_test_resp::uphold_commit_resp_));
  }

  void EnableRewardsViaCode() {
    base::RunLoop run_loop;
    bool wallet_created = false;
    rewards_service_->CreateWallet(
        base::BindLambdaForTesting([&](int32_t result) {
          wallet_created =
              (result == static_cast<int32_t>(ledger::Result::WALLET_CREATED));
          run_loop.Quit();
        }));

    run_loop.Run();

    ASSERT_TRUE(wallet_created);
    ASSERT_TRUE(IsRewardsEnabled());
  }

  brave_rewards::RewardsServiceImpl* rewards_service() {
    return rewards_service_;
  }

  void OnWalletInitialized(brave_rewards::RewardsService* rewards_service,
                           int32_t result) {
    const auto converted_result = static_cast<ledger::Result>(result);
    ASSERT_TRUE(converted_result == ledger::Result::WALLET_CREATED ||
                converted_result == ledger::Result::NO_LEDGER_STATE ||
                converted_result == ledger::Result::LEDGER_OK);
    wallet_initialized_ = true;
    if (wait_for_wallet_initialization_loop_)
      wait_for_wallet_initialization_loop_->Quit();
  }

  void OnPublisherListNormalized(brave_rewards::RewardsService* rewards_service,
                                 const brave_rewards::ContentSiteList& list) {
    if (list.size() == 0)
      return;
    publisher_list_normalized_ = true;
    if (wait_for_publisher_list_normalized_loop_)
      wait_for_publisher_list_normalized_loop_->Quit();
  }

  void OnRefreshPublisher(uint32_t status, const std::string& publisher_key) {
    LOG(ERROR) << "Publisher refreshed";
    EXPECT_EQ(status, 0UL);
    publisher_list_parsed_callback_was_called_ = true;
    if (wait_for_publisher_list_parsed_loop_)
      wait_for_publisher_list_parsed_loop_->Quit();
  }

  void WaitForPublisherListCallback() {
    if (publisher_list_parsed_callback_was_called_)
      return;
    wait_for_publisher_list_parsed_loop_.reset(new base::RunLoop);
    wait_for_publisher_list_parsed_loop_->Run();
  }

  MOCK_METHOD1(OnGetEnvironment, void(ledger::Environment));
  MOCK_METHOD1(OnGetDebug, void(bool));
  MOCK_METHOD1(OnGetReconcileTime, void(int32_t));
  MOCK_METHOD1(OnGetShortRetries, void(bool));

  std::unique_ptr<net::EmbeddedTestServer> https_server_;

  brave_rewards::RewardsServiceImpl* rewards_service_;

  std::unique_ptr<base::RunLoop> wait_for_wallet_initialization_loop_;
  bool wallet_initialized_ = false;

  std::unique_ptr<base::RunLoop> wait_for_publisher_list_normalized_loop_;
  bool publisher_list_normalized_ = false;

  std::unique_ptr<base::RunLoop> wait_for_publisher_list_parsed_loop_;
  bool publisher_list_parsed_callback_was_called_ = false;

  bool alter_publisher_list_ = false;
  bool show_defaults_in_properties_ = false;
  bool request_made_ = false;
};


IN_PROC_BROWSER_TEST_F(PublisherServerListBrowserPerfTest, RefreshPublisher) {
  base::ScopedAllowBlockingForTesting allow_blocking;
  std::string publisher_key = "";
  EnableRewardsViaCode();
  // WaitForPublisherListNormalized();
  rewards_service_->RefreshPublisher(
      publisher_key,
      base::BindOnce(&PublisherServerListBrowserPerfTest::OnRefreshPublisher,
                     AsWeakPtr()));
  WaitForPublisherListCallback();
}

IN_PROC_BROWSER_TEST_F(PublisherServerListBrowserPerfTest,
                       PanelShowsCorrectPublisherData) {
  // Enable Rewards
  EnableRewardsViaCode();
}
