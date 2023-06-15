/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/leaked_credentials/leaked_credentials_api.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace leaked_credentials {

TEST(LeakedCredentialsUnittest, TestHelloWorld) {
  // EXPECT_EQ(expected, actual)
  EXPECT_EQ(leaked_credentials::HelloWorld(), 0);
}

} // namespace leaked_credentials
