// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.language;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import org.chromium.base.ContextUtils;
import android.app.LocaleManager;
import android.os.LocaleList;

public class BraveLocaleManagerDelegateImpl extends LocaleManagerDelegateImpl {

    @Override
    public Locale getApplicationLocale() {
        // TODO(https://crbug.com/1293523): Replace with calls to {@link LocaleManager} once the T
        // SDK is available.
        LocaleList currentAppLocales = ContextUtils.getApplicationContext().getSystemService(LocaleManager.class).getApplicationLocales();
        if(!currentAppLocales.isEmpty()) {
            return currentAppLocales.get(0);
        }
        return null;
    }   

    @Override
    public void setApplicationLocale(String languageName) {
        // TODO(https://crbug.com/1293523): Replace with calls to {@link LocaleManager} once the T
        // SDK is available.
        ContextUtils.getApplicationContext().getSystemService(LocaleManager.class
    ).setApplicationLocales(new LocaleList(Locale.forLanguageTag(languageName)));
    }
}
