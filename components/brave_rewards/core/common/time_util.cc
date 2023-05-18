/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_rewards/core/common/time_util.h"

#include <algorithm>

#include "base/time/time.h"
#include "brave_base/random.h"
#include "third_party/icu/source/i18n/unicode/timezone.h"

namespace brave_rewards::internal {
namespace util {

std::pair<base::Time::Exploded, base::Time::Exploded>
GetUTCDateTimeRangeForCurrentLocalMonth() {
  base::Time now = base::Time::Now();
  // Local time
  base::Time::Exploded start;
  now.LocalExplode(&start);

  // Set the start value to the beginning of the current month
  start.day_of_month = 1;
  start.hour = 0;
  start.minute = 0;
  start.second = 0;
  start.millisecond = 0;

  // Set the end value to the beginning of the next month
  base::Time::Exploded end = start;
  end.month = end.month == 12 ? 1 : end.month + 1;

  // Convert to UTC
  base::Time utcStart, utcEnd;
  if (base::Time::FromLocalExploded(start, &utcStart) &&
      base::Time::FromLocalExploded(end, &utcEnd)) {
    // Adjust the end time by 1 millisecond to set it to the last millisecond
    // of the current month
    utcEnd -= base::Milliseconds(1);

    // Explode in UTC
    utcStart.UTCExplode(&start);
    utcEnd.UTCExplode(&end);
  }

  return {start, end};
}

mojom::ActivityMonth GetCurrentMonth() {
  base::Time now = base::Time::Now();
  return GetMonth(now);
}

mojom::ActivityMonth GetMonth(const base::Time& time) {
  base::Time::Exploded exploded;
  time.LocalExplode(&exploded);
  return (mojom::ActivityMonth)exploded.month;
}

uint32_t GetCurrentYear() {
  base::Time now = base::Time::Now();
  return GetYear(now);
}

uint32_t GetYear(const base::Time& time) {
  base::Time::Exploded exploded;
  time.LocalExplode(&exploded);
  return exploded.year;
}

uint64_t GetCurrentTimeStamp() {
  return static_cast<uint64_t>(base::Time::Now().ToDoubleT());
}

base::TimeDelta GetRandomizedDelay(base::TimeDelta delay) {
  uint64_t seconds = brave_base::random::Geometric(delay.InSecondsF());
  return base::Seconds(static_cast<int64_t>(seconds));
}

base::TimeDelta GetRandomizedDelayWithBackoff(base::TimeDelta delay,
                                              base::TimeDelta max_delay,
                                              int backoff_count) {
  delay *= 1 << std::min(backoff_count, 24);
  return GetRandomizedDelay(std::min(delay, max_delay));
}

}  // namespace util
}  // namespace brave_rewards::internal
