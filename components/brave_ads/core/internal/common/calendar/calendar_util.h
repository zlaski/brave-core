/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_COMMON_CALENDAR_CALENDAR_UTIL_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_COMMON_CALENDAR_CALENDAR_UTIL_H_

#include "base/check.h"

namespace base {
class Time;
}  // namespace base

namespace brave_ads {

constexpr bool IsLeapYear(const int year) noexcept {
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int DayOfWeek(base::Time time, bool is_local);

// 1-based month (values 1 = January, etc.).
constexpr int DaysInMonth(const int year, const int month) noexcept {
  CHECK((month >= 1 && month <= 12));

  constexpr int kDaysInMonth[] = {
      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31  // non leap year.
  };

  int days_in_month = kDaysInMonth[month - 1];
  if (month == /*february*/ 2 && IsLeapYear(year)) {
    // February has 29 days in leap years.
    days_in_month++;
  }

  return days_in_month;
}

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_COMMON_CALENDAR_CALENDAR_UTIL_H_
