/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.settings;


import org.chromium.chrome.browser.privacy.settings.PrivacySettings;
import android.os.Bundle;
import org.chromium.components.browser_ui.settings.SettingsUtils;

import org.chromium.chrome.R;

public class BraveSearchEnginesPreferences extends BravePreferenceFragment {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActivity().setTitle(R.string.brave_search_engines);
        SettingsUtils.addPreferencesFromResource(this, R.xml.brave_search_engines_preferences);
    }
}
