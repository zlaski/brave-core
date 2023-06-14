/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/history/filters/saved_history_filter.h"

namespace brave_ads {

HistoryItemList SavedHistoryFilter::Apply(
    const HistoryItemList& history) const {

  HistoryItemList filtered_history;
  for (const auto& history_item : history) {
    if (history_item.ad_content.is_saved) {
      filtered_history.push_back(history_item);
    }
  }

  return filtered_history;
}

}  // namespace brave_ads
