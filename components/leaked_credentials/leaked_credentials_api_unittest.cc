/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_api.h"

#include "brave/components/leaked_credentials/rs/cxx/src/lib.rs.h"
#include "third_party/rust/cxx/v1/crate/include/cxx.h"

// chromium networking test

// profile specific url loader
/*#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"*/

// system-wide url loader
//#include "chrome/browser/net/system_network_context_manager.h"

// url loader for testing
#include "base/memory/scoped_refptr.h"
#include "services/network/public/cpp/weak_wrapper_shared_url_loader_factory.h"
#include "services/network/test/test_url_loader_factory.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gmock/include/gmock/gmock.h"

#include <string>
#include <vector>

using ::testing::Return;

namespace leaked_credentials { 

class Server {
  //virtual Server() {}
  virtual ~Server() = delete;// {}
  virtual std::string Response() const = 0;
};

class MockServer: public Server {
    public: 
        MOCK_METHOD(std::string, Response, (), (const, override));
};

TEST(LeakedCredentialsUnittest, TestSha256) {
  // EXPECT_EQ(expected, actual)
  std::vector<uint8_t> hash;
  rust::Vec<std::uint8_t> hashresult = leaked_credentials::sha256("hello world");
  std::copy(hashresult.begin(), hashresult.end(), std::back_inserter(hash));
  std::vector<uint8_t> v2 = {0xb9, 0x4d, 0x27, 0xb9, 0x93, 0x4d, 0x3e, 0x08, \
                             0xa5, 0x2e, 0x52, 0xd7, 0xda, 0x7d, 0xab, 0xfa, \
                             0xc4, 0x84, 0xef, 0xe3, 0x7a, 0x53, 0x80, 0xee, \
                             0x90, 0x88, 0xf7, 0xac, 0xe2, 0xef, 0xcd, 0xe9};
  EXPECT_EQ(v2, hash);
}

TEST(LeakedCredentialsUnittest, TestClientGetBucketId) {
  // EXPECT_EQ(expected, actual)
  std::string username = "username";
  size_t bucket_id = 0;
  EXPECT_EQ(bucket_id, leaked_credentials::client_get_bucket_id(username, 15, 16));
}

/*TEST(LeakedCredentialsUnittest, TestClientSendBlockingGetRequest) {
  // EXPECT_EQ(expected, actual)
  std::string url = "http://www.google.com";
  std::string response = "TODO";

  LeakedCredentialsNetworkCalls network_request = LeakedCredentialsNetworkCalls(
    base::MakeRefCounted<network::WeakWrapperSharedURLLoaderFactory>(
                &url_loader_factory_));
  network_request.send_blocking_get_request(url);

  MockServer server;
  EXPECT_CALL(server, Response()).WillOnce(Return(response));

  std::unique_ptr<std::string> s(&response);
  EXPECT_EQ(response, network_request.OnSimpleLoaderComplete(s));
}*/

TEST(LeakedCredentialsUnittest, TestClientParseJSON) {
  leaked_credentials::LeakedCredentialsNetworkCalls network_request = leaked_credentials::LeakedCredentialsNetworkCalls();

  std::unique_ptr<std::string> response_body (new std::string("{\"life\": 42}")); 
  network_request.OnSimpleLoaderComplete(std::move(response_body));
  // EXPECT_EQ(expected, actual)
  EXPECT_EQ(42, network_request.test);
}


} // namespace leaked_credentials
