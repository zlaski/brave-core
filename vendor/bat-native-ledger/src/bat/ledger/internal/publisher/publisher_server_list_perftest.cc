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
#include "bat/ledger/internal/request/request_util.h"
#include "bat/ledger/internal/static_values.h"
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

using braveledger_request_util::ServerTypes;

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
