/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/profiles/profile_util.h"

#include "base/metrics/histogram_macros.h"
#include "brave/components/brave_shields/content/browser/brave_shields_p3a.h"
#include "brave/components/brave_shields/core/common/brave_shield_utils.h"
#include "brave/components/ntp_background_images/common/pref_names.h"
#include "brave/components/search_engines/brave_prepopulated_engines.h"
#include "brave/components/tor/buildflags/buildflags.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/pref_names.h"
#include "components/content_settings/core/browser/cookie_settings.h"
#include "components/content_settings/core/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "components/search_engines/search_engines_pref_names.h"

using brave_shields::ControlType;
using ntp_background_images::prefs::kNewTabPageShowBackgroundImage;
using ntp_background_images::prefs::
    kNewTabPageShowSponsoredImagesBackgroundImage;  // NOLINT

namespace brave {

void RecordSponsoredImagesEnabledP3A(Profile* profile) {
  bool is_sponsored_image_enabled =
      profile->GetPrefs()->GetBoolean(kNewTabPageShowBackgroundImage) &&
      profile->GetPrefs()->GetBoolean(
          kNewTabPageShowSponsoredImagesBackgroundImage);
  UMA_HISTOGRAM_BOOLEAN("Brave.NTP.SponsoredImagesEnabled",
                        is_sponsored_image_enabled);
}

void RecordInitialP3AValues(Profile* profile) {
  // Preference is unregistered for some reason in profile_manager_unittest
  // TODO(bsclifton): create a proper testing profile
  if (!profile->GetPrefs()->FindPreference(kNewTabPageShowBackgroundImage) ||
      !profile->GetPrefs()->FindPreference(
          kNewTabPageShowSponsoredImagesBackgroundImage)) {
    return;
  }
  RecordSponsoredImagesEnabledP3A(profile);
  if (profile->IsRegularProfile()) {
    brave_shields::MaybeRecordInitialShieldsSettings(
        profile->GetPrefs(),
        HostContentSettingsMapFactory::GetForProfile(profile));
  }
}

void SetDefaultSearchVersion(Profile* profile, bool is_new_profile) {
  const PrefService::Preference* pref_default_search_version =
      profile->GetPrefs()->FindPreference(prefs::kBraveDefaultSearchVersion);
  if (!pref_default_search_version->HasUserSetting()) {
    profile->GetPrefs()->SetInteger(
        prefs::kBraveDefaultSearchVersion,
        is_new_profile
            ? TemplateURLPrepopulateData::kBraveCurrentDataVersion
            : TemplateURLPrepopulateData::kBraveFirstTrackedDataVersion);
  }
}

void SetDefaultThirdPartyCookieBlockValue(Profile* profile) {
  profile->GetPrefs()->SetDefaultPrefValue(
      prefs::kCookieControlsMode,
      base::Value(static_cast<int>(
          content_settings::CookieControlsMode::kBlockThirdParty)));
}

void MigrateHttpsUpgradeSettings(Profile* profile) {
  // If user flips the HTTPS by Default feature flag
  auto* prefs = profile->GetPrefs();
  // The HostContentSettingsMap might be null for some irregular profiles, e.g.
  // the System Profile.
  auto* map = HostContentSettingsMapFactory::GetForProfile(profile);
  if (!map) {
    return;
  }
  if (brave_shields::IsHttpsByDefaultFeatureEnabled()) {
    // Migrate forwards from HTTPS-Only Mode to HTTPS Upgrade Strict setting.
    if (prefs->GetBoolean(prefs::kHttpsOnlyModeEnabled)) {
      brave_shields::SetHttpsUpgradeControlType(map, ControlType::BLOCK,
                                                GURL());
      prefs->SetBoolean(prefs::kHttpsOnlyModeEnabled, false);
    }
  } else {
    // Migrate backwards from HTTPS Upgrade Strict setting to HTTPS-Only Mode.
    if (brave_shields::GetHttpsUpgradeControlType(map, GURL()) ==
        ControlType::BLOCK) {
      prefs->SetBoolean(prefs::kHttpsOnlyModeEnabled, true);
      brave_shields::SetHttpsUpgradeControlType(
          map, ControlType::BLOCK_THIRD_PARTY, GURL());
    }
  }
}

}  // namespace brave
