// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/path_service.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/memory/ptr_util.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/test/bind_test_util.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/time.h"
#include "base/trace_event/memory_usage_estimator.h"
#include "base/values.h"
#include "bat/ledger/internal/ledger_client_mock.h"
#include "bat/ledger/internal/ledger_impl_mock.h"
#include "bat/ledger/internal/publisher/publisher.h"
#include "bat/ledger/internal/publisher/publisher_server_list.h"
#include "bat/ledger/mojom_structs.h"
#include "brave/common/brave_paths.h"
#include "brave/components/brave_rewards/browser/rewards_service.h"
#include "brave/components/brave_rewards/browser/rewards_service_impl.h"
#include "brave/components/brave_rewards/browser/rewards_service_factory.h"
#include "brave/components/brave_rewards/common/pref_names.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "components/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/test/browser_test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/perf/perf_test.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/http_request.h"
#include "net/test/embedded_test_server/http_response.h"
#include "components/network_session_configurator/common/network_switches.h"

namespace {

constexpr int kTestIterations = 1;
static const char kTestResultString[] = "PublishServerList";

void OnParsePublisherList(const ledger::Result result) {
  (void)result;
}

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

void CalculateMeanAndMax(const std::vector<double>& inputs,
                         double* mean,
                         double* std_dev,
                         double* max) {
  double sum = 0.0;
  double sqr_sum = 0.0;
  double max_so_far = 0.0;
  size_t count = inputs.size();
  for (const auto& input : inputs) {
    sum += input;
    sqr_sum += input * input;
    max_so_far = std::max(input, max_so_far);
  }
  *max = max_so_far;
  *mean = sum / count;
  *std_dev = sqrt(std::max(0.0, count * sqr_sum - sum * sum)) / count;
}

void PrintMeanAndMax(const std::string& var_name,
                     const std::string& name_modifier,
                     const std::string& unit,
                     const std::vector<double>& vars) {
  double mean = 0.0;
  double std_dev = 0.0;
  double max = 0.0;
  CalculateMeanAndMax(vars, &mean, &std_dev, &max);
  perf_test::PrintResultMeanAndError(
      kTestResultString, name_modifier, var_name + " Mean",
      base::StringPrintf("%.0lf,%.0lf", mean, std_dev), unit, true);
  perf_test::PrintResult(kTestResultString, name_modifier, var_name + " Max",
                         base::StringPrintf("%.0lf", max), unit, true);
}

std::unique_ptr<net::test_server::HttpResponse> HandleRequest(
    const net::test_server::HttpRequest& request) {
  std::unique_ptr<net::test_server::BasicHttpResponse> http_response(
      new net::test_server::BasicHttpResponse());
  http_response->set_code(net::HTTP_OK);
  http_response->set_content_type("application/json");
  if (request.relative_url == "/api/v3/public/channels") {
    auto data = LoadFile();
    http_response->set_content(data);
  }
  return std::move(http_response);
}

}  // namespace

class PublisherServerListPerfTest : public testing::Test {
 public:
  PublisherServerListPerfTest() {
    mock_ledger_client_ = std::make_unique<ledger::MockLedgerClient>();
    mock_ledger_impl_ =
        std::make_unique<bat_ledger::MockLedgerImpl>(mock_ledger_client_.get());
    server_list_ = std::make_unique<braveledger_publisher::PublisherServerList>(
        mock_ledger_impl_.get());
  }

  void TestParse(const std::string& data, const std::string& test) {
    base::ScopedAllowBlockingForTesting allow_blocking;
    std::vector<double> times;
    for (int i = 0; i < kTestIterations; i++) {
      base::TimeTicks start_read = base::TimeTicks::Now();
      server_list_->ParsePublisherList(data, OnParsePublisherList);
      base::TimeTicks end_read = base::TimeTicks::Now();
      times.push_back((end_read - start_read).InMillisecondsF());
    }
    PrintMeanAndMax("Parse Time", test, "ms", times);
  }

 protected:
  std::unique_ptr<ledger::MockLedgerClient> mock_ledger_client_;
  std::unique_ptr<bat_ledger::MockLedgerImpl> mock_ledger_impl_;
  std::unique_ptr<braveledger_publisher::PublisherServerList> server_list_;
};

class PublisherServerListBrowserPerfTest
    : public InProcessBrowserTest,
      public base::SupportsWeakPtr<PublisherServerListBrowserPerfTest> {
 public:
  void SetUpOnMainThread() override {
    InProcessBrowserTest::SetUpOnMainThread();

    // host_resolver()->AddRule("*", "127.0.0.1");

    // Setup up embedded test server for HTTPS requests
    // https_server_.reset(new net::EmbeddedTestServer(
    //     net::test_server::EmbeddedTestServer::TYPE_HTTPS));
    // https_server_->SetSSLConfig(net::EmbeddedTestServer::CERT_OK);
    // https_server_->RegisterRequestHandler(base::BindRepeating(&HandleRequest));
    // ASSERT_TRUE(https_server_->Start());

    brave::RegisterPathProvider();
    ReadTestData();

    rewards_service_ = std::unique_ptr<brave_rewards::RewardsServiceImpl>(
        static_cast<brave_rewards::RewardsServiceImpl*>(
        brave_rewards::RewardsServiceFactory::GetForProfile(
            browser()->profile())));

    rewards_service_->ForTestingSetTestResponseCallback(
        base::BindRepeating(&BraveRewardsBrowserTest::GetTestResponse,
                            base::Unretained(this)));

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

  PrefService* GetPrefs() const { return browser()->profile()->GetPrefs(); }

  bool IsRewardsEnabled() const {
    return GetPrefs()->GetBoolean(brave_rewards::prefs::kBraveRewardsEnabled);
  }

  void SetUpCommandLine(base::CommandLine* command_line) override {
    // HTTPS server only serves a valid cert for localhost, so this is needed
    // to load pages from other hosts without an error
    command_line->AppendSwitch(switches::kIgnoreCertificateErrors);
  }

  void ReadTestData() {
    base::ScopedAllowBlockingForTesting allow_blocking;
    base::FilePath path;
    GetTestDataDir(&path);
  }

  void GetTestDataDir(base::FilePath* test_data_dir) {
    base::ScopedAllowBlockingForTesting allow_blocking;
    ASSERT_TRUE(base::PathService::Get(brave::DIR_TEST_DATA, test_data_dir));
    *test_data_dir = test_data_dir->AppendASCII("rewards-data");
    ASSERT_TRUE(base::PathExists(*test_data_dir));
  }

  void TearDown() override { InProcessBrowserTest::TearDown(); }

  void WaitForPublisherListCallback() {
    if (publisher_list_parsed_callback_was_called_)
      return;
    base::RunLoop run_loop;
    run_loop.Run();
  }

  void OnRefreshPublisher(uint32_t status, const std::string& publisher_key) {
    EXPECT_EQ(status, 0UL);
    publisher_list_parsed_callback_was_called_ = true;
  }

  void GetTestResponse(const std::string& url,
                       int32_t method,
                       int* response_status_code,
                       std::string* response,
                       std::map<std::string, std::string>* headers) {
    std::vector<std::string> tmp = base::SplitString(url,
                                                     "/",
                                                     base::TRIM_WHITESPACE,
                                                     base::SPLIT_WANT_ALL);
    const std::string persona_url =
        braveledger_request_util::BuildUrl(REGISTER_PERSONA, PREFIX_V2);
    if (URLMatches(url, GET_PUBLISHERS_LIST, "",
                          ServerTypes::PUBLISHER_DISTRO)) {
      *response = LoadFile();
    }
  }

 protected:
  std::unique_ptr<brave_rewards::RewardsService> rewards_service_;
  bool publisher_list_parsed_callback_was_called_ = false;
  std::unique_ptr<net::EmbeddedTestServer> https_server_;
};

TEST_F(PublisherServerListPerfTest, ParseTestEmpty) {
  EXPECT_CALL(*mock_ledger_impl_, RunDBTransaction(testing::_, testing::_))
      .Times(0);
  TestParse("", "Empty");
}

TEST_F(PublisherServerListPerfTest, ParseTestFull) {
  EXPECT_CALL(*mock_ledger_impl_, RunDBTransaction(testing::_, testing::_))
      .Times(testing::AnyNumber());
  auto data = LoadFile();
  TestParse(data, "Full");
}

TEST_F(PublisherServerListPerfTest, ParseJSONOnly) {
  auto data = LoadFile();
  base::TimeTicks start_read = base::TimeTicks::Now();
  base::Optional<base::Value> parsed = base::JSONReader::Read(data);
  perf_test::PrintResult(kTestResultString, "JSONSize", "Memory",
                         parsed->EstimateMemoryUsage(), "B", true);
  base::TimeTicks end_read = base::TimeTicks::Now();
  perf_test::PrintResult(kTestResultString, "JSONParse", "Time",
                         (end_read - start_read).InMillisecondsF(), "ms", true);
}

IN_PROC_BROWSER_TEST_F(PublisherServerListBrowserPerfTest, RefreshPublisher) {
  std::string publisher_key = "";
  EnableRewardsViaCode();
  rewards_service_->RefreshPublisher(
      publisher_key,
      base::BindOnce(&PublisherServerListBrowserPerfTest::OnRefreshPublisher,
                     AsWeakPtr()));
  WaitForPublisherListCallback();
}


// #3 - Panel shows correct publisher data
IN_PROC_BROWSER_TEST_F(PublisherServerListBrowserPerfTest,
                       PanelShowsCorrectPublisherData) {
  // Enable Rewards
  EnableRewardsViaCode();
}
