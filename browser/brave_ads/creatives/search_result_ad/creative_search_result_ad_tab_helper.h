/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_BRAVE_ADS_CREATIVES_SEARCH_RESULT_AD_CREATIVE_SEARCH_RESULT_AD_TAB_HELPER_H_
#define BRAVE_BROWSER_BRAVE_ADS_CREATIVES_SEARCH_RESULT_AD_CREATIVE_SEARCH_RESULT_AD_TAB_HELPER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

class GURL;

namespace brave_ads {

class AdsService;
class CreativeSearchResultAdHandler;

class CreativeSearchResultAdTabHelper
    : public content::WebContentsObserver,
      public content::WebContentsUserData<CreativeSearchResultAdTabHelper> {
 public:
  explicit CreativeSearchResultAdTabHelper(content::WebContents* web_contents);
  ~CreativeSearchResultAdTabHelper() override;

  CreativeSearchResultAdTabHelper(const CreativeSearchResultAdTabHelper&) =
      delete;
  CreativeSearchResultAdTabHelper& operator=(
      const CreativeSearchResultAdTabHelper&) = delete;

  static void SetAdsServiceForTesting(AdsService* ads_service);

  static void MaybeCreateForWebContents(content::WebContents* web_contents);

  bool ShouldHandleCreativeAdEvents() const;

  void MaybeTriggerCreativeAdClickedEvent(
      const GURL& url,
      base::OnceCallback<void(bool success)> callback);

 private:
  friend class content::WebContentsUserData<CreativeSearchResultAdTabHelper>;

  AdsService* GetAdsService() const;

  void MaybeTriggerCreativeAdClickedEventCallback(bool success);

  void MaybeCreateCreativeSearchResultAdHandler(
      content::NavigationHandle* navigation_handle);

  void MaybeExtractCreativeAdPlacementIdsFromWebPageAndHandleViewedEvents();

  void MaybeHandleCreativeAdViewedEvents(
      std::vector<std::string> placement_ids);
  void MaybeHandleCreativeAdViewedEvent(const std::string& placement_id);
  void MaybeHandleCreativeAdViewedEventCallback(const std::string& placement_id,
                                                const base::Value value);

  void MaybeHandleCreativeAdClickedEvent(
      content::NavigationHandle* navigation_handle);

  // content::WebContentsObserver:
  void DidStartNavigation(
      content::NavigationHandle* navigation_handle) override;
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;
  void DocumentOnLoadCompletedInPrimaryMainFrame() override;
  void WebContentsDestroyed() override;

  std::unique_ptr<CreativeSearchResultAdHandler>
      creative_search_result_ad_handler_;

  base::WeakPtrFactory<CreativeSearchResultAdTabHelper> weak_factory_{this};

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace brave_ads

#endif  // BRAVE_BROWSER_BRAVE_ADS_CREATIVES_SEARCH_RESULT_AD_CREATIVE_SEARCH_RESULT_AD_TAB_HELPER_H_
