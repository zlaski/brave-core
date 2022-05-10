/**
 * Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.chromium.chrome.browser.firstrun;

public class BraveFirstRunUtils {
    public static void firstRunFlowComplete() {
        FirstRunStatus.setFirstRunFlowComplete(true);
    }
}
