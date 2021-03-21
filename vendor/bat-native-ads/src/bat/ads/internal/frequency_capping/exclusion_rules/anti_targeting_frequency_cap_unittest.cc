/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/frequency_capping/exclusion_rules/anti_targeting_frequency_cap.h"

#include "base/strings/string_number_conversions.h"
#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

namespace {

}  // namespace

class BatAdsAntiTargetingFrequencyCapTest : public UnitTestBase {
 protected:
  BatAdsAntiTargetingFrequencyCapTest() = default;

  ~BatAdsAntiTargetingFrequencyCapTest() override = default;
};

TEST_F(BatAdsAntiTargetingFrequencyCapTest, Foobar) {
  // Arrange
  CreativeAdInfo ad;
  ad.creative_set_id = kCreativeSetId;

  // Act
  AntiTargetingFrequencyCap frequency_cap;
  const bool should_exclude = frequency_cap.ShouldExclude(ad);

  // Assert
  EXPECT_FALSE(should_exclude);
}

}  // namespace ads
