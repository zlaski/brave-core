package org.chromium.chrome.browser.app;

import androidx.annotation.NonNull;

import com.wireguard.android.backend.Tunnel;
import com.wireguard.config.Config;

public class TunnelModel implements Tunnel{
    private TunnelManager tunnelManager;
    private String name;
    private Config config;
    private Tunnel.State state;

    public TunnelModel(TunnelManager tunnelManager, String name, Config config, Tunnel.State state) {
        this.tunnelManager = tunnelManager;
        this.name = name;
        this.config = config;
        this.state = state;
    }

    @NonNull
    @Override
    public String getName() {
        return name;
    }

    @Override
    public void onStateChange(@NonNull State newState) {

    }
}