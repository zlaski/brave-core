package org.chromium.chrome.browser.app;

import android.content.Context;

import com.wireguard.config.Config;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class FileConfigStore {
    private Context mContext;
    FileConfigStore(Context context) {
        this.mContext = context;
    }

    public Config create(String name, Config config) throws IOException {
        File file = fileFor(name);
        if (!file.createNewFile()) {
            throw new IOException("Configuration file already exists");
        }
        FileOutputStream fileOutputStream = new FileOutputStream(file, false);
        fileOutputStream.write(config.toWgQuickString().getBytes(StandardCharsets.UTF_8));

        // Check for flushing the stream
        return config;
    }

    public void delete(String name) throws Exception {
        File file = fileFor(name);
        if (!file.delete()) {
            throw new IOException("Cannot delete configuration file");
        }
    }

    public Config load(String name) throws Exception {
        FileInputStream fileInputStream = new FileInputStream(fileFor(name));
        return Config.parse(fileInputStream);
    }

    public Config save(String name, Config config) throws Exception {
        File file = fileFor(name);
        if(!file.isFile()) {
            throw new FileNotFoundException("Configuration file not found");
        }
        FileOutputStream fileOutputStream = new FileOutputStream(file, false);
        fileOutputStream.write(config.toWgQuickString().getBytes(StandardCharsets.UTF_8));
        return config;
    }

    private File fileFor(String fileName) {
        return new File(mContext.getFilesDir(), fileName+".conf");
    }
}