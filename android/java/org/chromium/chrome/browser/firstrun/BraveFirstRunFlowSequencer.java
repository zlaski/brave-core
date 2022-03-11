/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.firstrun;

import android.app.Activity;

import org.chromium.base.ApiCompatibilityUtils;
import org.chromium.base.CommandLine;
import org.chromium.base.Log;
import org.chromium.chrome.browser.flags.ChromeSwitches;

public abstract class BraveFirstRunFlowSequencer extends FirstRunFlowSequencer {
    public BraveFirstRunFlowSequencer(Activity activity) {
        super(activity);
    }

    @Override
    public void start() {
        super.start();
    }
}
