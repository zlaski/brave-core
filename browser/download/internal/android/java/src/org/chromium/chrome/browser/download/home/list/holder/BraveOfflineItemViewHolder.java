/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.download.home.list.holder;

import static org.chromium.components.browser_ui.widget.listmenu.BasicListMenu.buildMenuListItem;

import android.graphics.Matrix;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.view.View;
import android.widget.ImageView;

import androidx.annotation.CallSuper;

import org.chromium.chrome.browser.download.home.filter.Filters;
import org.chromium.chrome.browser.download.home.list.ListItem;
import org.chromium.chrome.browser.download.home.list.BraveListProperties;
import org.chromium.chrome.browser.download.home.list.UiUtils;
import org.chromium.chrome.browser.download.home.metrics.UmaUtils;
import org.chromium.chrome.browser.download.home.view.SelectionView;
import org.chromium.chrome.browser.download.internal.R;
import org.chromium.components.browser_ui.widget.async_image.AsyncImageView;
import org.chromium.components.browser_ui.widget.listmenu.BasicListMenu;
import org.chromium.components.browser_ui.widget.listmenu.ListMenu;
import org.chromium.components.browser_ui.widget.listmenu.ListMenuButton;
import org.chromium.components.browser_ui.widget.listmenu.ListMenuButtonDelegate;
import org.chromium.components.browser_ui.widget.listmenu.ListMenuItemProperties;
import org.chromium.components.offline_items_collection.OfflineItem;
import org.chromium.components.offline_items_collection.OfflineItemVisuals;
import org.chromium.ui.modelutil.MVCListAdapter.ModelList;
import org.chromium.ui.modelutil.PropertyModel;

class BraveOfflineItemViewHolder extends OfflineItemViewHolder {
    
    private Runnable mClearCallback;
    
    public BraveOfflineItemViewHolder(View view) {
        super(view);
    }

    public void bindMenuButtonCallbacks(PropertyModel properties, OfflineItem offlineItem) {
        
    /*    mClearCallback =
                () -> properties.get(BraveListProperties.CALLBACK_REMOVE_FROM_LIST).onResult(offlineItem);*/

        super.bindMenuButtonCallbacks();

        BraveReflectionUtil.InvokeMethod(
                            OfflineItemViewHolder.class, this,
                            "bindMenuButtonCallbacks", PropertyModel.class, properties,
                            OfflineItem.class, offlineItem);
    }

    @Override
    public ListMenu getListMenu() {
        BasicListMenu basicListMenu = super.getListMenu();
        basicListMenu.mAdapter.mModelList.add(buildMenuListItem(R.string.delete, 0, 0))
        return basicListMenu;
    }
}
