/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_client.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace leaked_credentials {

TEST(LeakedCredentialsUnittest, TestBucketInfo) {
  BucketInfo info = BucketInfo(0);
  EXPECT_EQ("http://ec2-54-184-23-71.us-west-2.compute.amazonaws.com:8080/params", info.params_url_);
  EXPECT_EQ("http://ec2-54-184-23-71.us-west-2.compute.amazonaws.com:8080/query", info.query_url_);
}

} // namespace leaked_credentials
