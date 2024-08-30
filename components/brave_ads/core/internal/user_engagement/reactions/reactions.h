/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_USER_ENGAGEMENT_REACTIONS_REACTIONS_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_USER_ENGAGEMENT_REACTIONS_REACTIONS_H_

#include <map>
#include <set>
#include <string>

#include "base/observer_list.h"
#include "brave/components/brave_ads/browser/ads_service_callback.h"
#include "brave/components/brave_ads/core/internal/user_engagement/reactions/reactions_observer.h"
#include "brave/components/brave_ads/core/mojom/brave_ads.mojom-forward.h"
#include "brave/components/brave_ads/core/public/account/confirmations/confirmation_type.h"

namespace brave_ads {

// `id` can be either `advertiser_id` for liking/disliking an ad or `segment`
// for liking/disliking a segment.
using ReactionMap = std::map</*id*/ std::string, mojom::ReactionType>;

// `id` can be either `creative_instance_id` for saving an ad or
// `creative_set_id` for marking an ad as inappropriate.
using ReactionSet = std::set</*id*/ std::string>;

struct AdHistoryItemInfo;

class Reactions final {
 public:
  Reactions();

  Reactions(const Reactions&) = delete;
  Reactions& operator=(const Reactions&) = delete;

  Reactions(Reactions&&) noexcept = delete;
  Reactions& operator=(Reactions&&) noexcept = delete;

  ~Reactions();

  void ToggleLikeAd(mojom::ReactionInfoPtr reaction,
                    ToggleReactionCallback callback);
  void ToggleDislikeAd(mojom::ReactionInfoPtr reaction,
                       ToggleReactionCallback callback);
  mojom::ReactionType AdReactionTypeForId(
      const std::string& advertiser_id) const;
  const ReactionMap& Ads() const { return ad_reactions_; }

  void ToggleLikeSegment(mojom::ReactionInfoPtr reaction,
                         ToggleReactionCallback callback);
  void ToggleDislikeSegment(mojom::ReactionInfoPtr reaction,
                            ToggleReactionCallback callback);
  mojom::ReactionType SegmentReactionTypeForId(
      const std::string& segment) const;
  const ReactionMap& Segments() const { return segment_reactions_; }

  void ToggleSaveAd(mojom::ReactionInfoPtr reaction,
                    ToggleReactionCallback callback);
  bool IsAdSaved(const std::string& creative_instance_id) const;

  void ToggleMarkAdAsInappropriate(mojom::ReactionInfoPtr reaction,
                                   ToggleReactionCallback callback);
  bool IsAdMarkedAsInappropriate(const std::string& creative_set_id) const;

 private:
  void LoadAdReactions();
  void LoadSegmentReactions();
  void LoadSavedAds();
  void LoadMarkedAsInappropriate();
  void Load();

  static void Deposit(const mojom::ReactionInfo* reaction,
                      ConfirmationType confirmation_type);

  void NotifyDidLikeAd(const std::string& advertiser_id) const;
  void NotifyDidDislikeAd(const std::string& advertiser_id) const;
  void NotifyDidLikeSegment(const std::string& segment) const;
  void NotifyDidDislikeSegment(const std::string& segment) const;
  void NotifyDidToggleSaveAd(const std::string& creative_instance_id) const;
  void NotifyDidToggleMarkAdAsInappropriate(
      const std::string& creative_set) const;

  ReactionMap ad_reactions_;
  ReactionMap segment_reactions_;
  ReactionSet saved_ads_;
  ReactionSet marked_as_inappropriate_;

  base::ObserverList<ReactionsObserver> observers_;
};

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_USER_ENGAGEMENT_REACTIONS_REACTIONS_H_
