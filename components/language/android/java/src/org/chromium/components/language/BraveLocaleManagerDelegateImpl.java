// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.language;

import android.app.LocaleManager;
import android.os.Build;
import android.os.LocaleList;

import androidx.annotation.RequiresApi;

import org.chromium.base.ContextUtils;
import org.chromium.base.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class BraveLocaleManagerDelegateImpl extends LocaleManagerDelegateImpl {
    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @Override
    public Locale getApplicationLocale() {
        Log.e("tapan", "BraveLocaleManagerDelegateImpl getApplicationLocale");
        // TODO(https://crbug.com/1293523): Replace with calls to {@link LocaleManager} once the T
        // SDK is available.
        LocaleList currentAppLocales = ContextUtils.getApplicationContext()
                                               .getSystemService(LocaleManager.class)
                                               .getApplicationLocales();
        if (!currentAppLocales.isEmpty()) {
            return currentAppLocales.get(0);
        }
        return null;
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @Override
    public void setApplicationLocale(String languageName) {
        // TODO(https://crbug.com/1293523): Replace with calls to {@link LocaleManager} once the T
        // SDK is available.
        Log.e("tapan", "BraveLocaleManagerDelegateImpl setApplicationLocale:" + languageName);
        ContextUtils.getApplicationContext()
                .getSystemService(LocaleManager.class)
                .setApplicationLocales(new LocaleList(Locale.forLanguageTag(languageName)));
    }
}
