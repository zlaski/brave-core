/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_api.h"

#include "brave/components/leaked_credentials/rs/cxx/src/lib.rs.h"
#include "third_party/rust/cxx/v1/crate/include/cxx.h"

#include "testing/gtest/include/gtest/gtest.h"

#include <string>
#include <vector>

namespace leaked_credentials {

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

//struct BlobMetadata;
/*
TEST(LeakedCredentialsUnittest, TestHelloMetadata) {
  // EXPECT_EQ(expected, actual)
  BlobMetadata metadata{};
  metadata.size = 5;
  //EXPECT_EQ(metadata.size, leaked_credentials::HelloMetadata().size);
  EXPECT_EQ(5, leaked_credentials::HelloMetadata());
}*/

} // namespace leaked_credentials
