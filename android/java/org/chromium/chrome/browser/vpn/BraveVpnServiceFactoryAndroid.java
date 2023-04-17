/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.vpn;

import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;
import org.chromium.brave_vpn.mojom.ServiceHandler;
import org.chromium.chrome.browser.crypto_wallet.util.Utils;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.mojo.bindings.ConnectionErrorHandler;
import org.chromium.mojo.bindings.Interface;
import org.chromium.mojo.bindings.Interface.Proxy.Handler;
import org.chromium.mojo.system.MessagePipeHandle;
import org.chromium.mojo.system.impl.CoreImpl;

@JNINamespace("chrome::android")
public class BraveVpnServiceFactoryAndroid {
    private static final Object lock = new Object();
    private static BraveVpnServiceFactoryAndroid instance;

    public static BraveVpnServiceFactoryAndroid getInstance() {
        synchronized (lock) {
            if (instance == null) {
                instance = new BraveVpnServiceFactoryAndroid();
            }
        }
        return instance;
    }

    private BraveVpnServiceFactoryAndroid() {}

    public ServiceHandler getServiceHandler(ConnectionErrorHandler connectionErrorHandler) {
        Profile profile = Utils.getProfile(false); // Always use regular profile
        if (profile == null) {
            return null;
        }
        long nativeHandle =
                BraveVpnServiceFactoryAndroidJni.get().getInterfaceToServiceHandler(profile);
        if (nativeHandle == -1) {
            return null;
        }
        MessagePipeHandle handle = wrapNativeHandle(nativeHandle);
        ServiceHandler serviceHandler = ServiceHandler.MANAGER.attachProxy(handle, 0);
        Handler handler = ((Interface.Proxy) serviceHandler).getProxyHandler();
        handler.setErrorHandler(connectionErrorHandler);

        return serviceHandler;
    }

    private MessagePipeHandle wrapNativeHandle(long nativeHandle) {
        return CoreImpl.getInstance().acquireNativeHandle(nativeHandle).toMessagePipeHandle();
    }

    @NativeMethods
    interface Natives {
        long getInterfaceToServiceHandler(Profile profile);
    }
}
