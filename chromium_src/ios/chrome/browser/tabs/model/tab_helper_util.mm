/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#import "ios/chrome/browser/tabs/model/tab_helper_util.h"

#import "components/omnibox/common/omnibox_features.h"
#include "ios/chrome/browser/complex_tasks/model/ios_task_tab_helper.h"
#import "ios/chrome/browser/https_upgrades/model/https_only_mode_upgrade_tab_helper.h"
#import "ios/chrome/browser/https_upgrades/model/https_upgrade_service_factory.h"
#import "ios/chrome/browser/https_upgrades/model/typed_navigation_upgrade_tab_helper.h"
#import "ios/chrome/browser/prerender/model/prerender_service_factory.h"
#include "ios/chrome/browser/sessions/model/ios_chrome_session_tab_helper.h"
#import "ios/chrome/browser/shared/model/browser_state/chrome_browser_state.h"
#include "ios/chrome/browser/tabs/model/ios_chrome_synced_tab_delegate.h"
#import "ios/chrome/browser/web/model/session_state/web_session_state_tab_helper.h"
#import "ios/components/security_interstitials/https_only_mode/feature.h"
#import "ios/components/security_interstitials/https_only_mode/https_only_mode_container.h"
#import "ios/components/security_interstitials/ios_blocking_page_tab_helper.h"
#import "ios/components/security_interstitials/lookalikes/lookalike_url_container.h"
#import "ios/components/security_interstitials/lookalikes/lookalike_url_tab_allow_list.h"
#import "ios/components/security_interstitials/lookalikes/lookalike_url_tab_helper.h"
#import "ios/components/security_interstitials/safe_browsing/safe_browsing_client.h"
#import "ios/components/security_interstitials/safe_browsing/safe_browsing_query_manager.h"
#import "ios/components/security_interstitials/safe_browsing/safe_browsing_tab_helper.h"
#import "ios/components/security_interstitials/safe_browsing/safe_browsing_unsafe_resource_container.h"

void AttachTabHelpers(web::WebState* web_state, TabHelperFilter filter_flags) {
  ChromeBrowserState* const browser_state =
      ChromeBrowserState::FromBrowserState(web_state->GetBrowserState());

  IOSChromeSessionTabHelper::CreateForWebState(web_state);
  IOSChromeSyncedTabDelegate::CreateForWebState(web_state);
  WebSessionStateTabHelper::CreateForWebState(web_state);
  IOSTaskTabHelper::CreateForWebState(web_state);

  security_interstitials::IOSBlockingPageTabHelper::CreateForWebState(
      web_state);

  if (base::FeatureList::IsEnabled(
          security_interstitials::features::kHttpsOnlyMode) ||
      base::FeatureList::IsEnabled(
          security_interstitials::features::kHttpsUpgrades)) {
    HttpsOnlyModeUpgradeTabHelper::CreateForWebState(
        web_state, browser_state->GetPrefs(),
        PrerenderServiceFactory::GetForBrowserState(browser_state),
        HttpsUpgradeServiceFactory::GetForBrowserState(browser_state));
    HttpsOnlyModeContainer::CreateForWebState(web_state);
  }

  if (base::FeatureList::IsEnabled(omnibox::kDefaultTypedNavigationsToHttps)) {
    TypedNavigationUpgradeTabHelper::CreateForWebState(
        web_state, PrerenderServiceFactory::GetForBrowserState(browser_state),
        HttpsUpgradeServiceFactory::GetForBrowserState(browser_state));
  }
}
