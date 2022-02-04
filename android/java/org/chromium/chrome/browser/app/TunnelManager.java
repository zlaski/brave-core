package org.chromium.chrome.browser.app;

import com.wireguard.android.backend.Tunnel;
import com.wireguard.config.Config;

public class TunnelManager {
    private FileConfigStore fileConfigStore;
    TunnelManager(FileConfigStore fileConfigStore) {
        this.fileConfigStore = fileConfigStore;
    }

    public TunnelModel create(String name, Config config) throws Exception {
        if (Tunnel.isNameInvalid(name)) {
            throw new IllegalAccessException("Invalid name");
        }
        return new TunnelModel(this, name, fileConfigStore.create(name, config), Tunnel.State.DOWN);
    }

    public Config getTunnelConfig(String tunnelName) throws Exception {
        return fileConfigStore.load(tunnelName);
    }
}