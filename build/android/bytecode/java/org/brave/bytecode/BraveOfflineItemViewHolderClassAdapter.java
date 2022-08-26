/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.brave.bytecode;

import org.objectweb.asm.ClassVisitor;

public class BraveOfflineItemViewHolderClassAdapter extends BraveClassVisitor {
    static String sOfflineItemViewHolderClassName = "org/chromium/chrome/browser/download/home/list/holder/OfflineItemViewHolder";

    static String sBraveOfflineItemViewHolderClassName = "org/chromium/chrome/browser/download/home/list/holder/BraveOfflineItemViewHolder";

    public BraveOfflineItemViewHolderClassAdapter(ClassVisitor visitor) {
        super(visitor);

        redirectConstructor(sOfflineItemViewHolderClassName, sBraveOfflineItemViewHolderClassName);

        makePublicMethod(sOfflineItemViewHolderClassName, "bindMenuButtonCallbacks");
        addMethodAnnotation(
                sBraveOfflineItemViewHolderClassName, "bindMenuButtonCallbacks", "Ljava/lang/Override;");
    }
}
