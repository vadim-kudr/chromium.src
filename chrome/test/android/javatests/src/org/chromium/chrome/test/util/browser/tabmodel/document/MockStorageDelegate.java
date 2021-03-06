// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.test.util.browser.tabmodel.document;

import android.util.Base64;
import android.util.Log;

import junit.framework.Assert;

import org.chromium.chrome.browser.TabState;
import org.chromium.chrome.browser.tabmodel.document.StorageDelegate;
import org.chromium.chrome.browser.util.StreamUtil;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Mocks out a directory on the file system for TabState storage. Because of the way that
 * {@link TabState} relies on real files and because it's not possible to mock out a
 * {@link FileInputStream}, we manage real files in a temporary directory that get written when
 * {@link #addEncodedTabState} is called.
 */
public class MockStorageDelegate extends StorageDelegate {
    private static final String TAG = "MockStorageDelegate";

    private byte[] mTaskFileBytes;
    private final File mStateDirectory;

    public MockStorageDelegate(File cacheDirectory, boolean isIncognito) {
        super(isIncognito);
        mStateDirectory = new File(cacheDirectory, "DocumentTabModelTest");
        ensureDirectoryDestroyed();
    }

    @Override
    public byte[] readTaskFileBytes() {
        if (mIsIncognito) return null;
        return mTaskFileBytes == null ? null : mTaskFileBytes.clone();
    }

    @Override
    public void writeTaskFileBytes(byte[] bytes) {
        if (mIsIncognito) return;
        mTaskFileBytes = bytes.clone();
    }

    @Override
    public File getStateDirectory() {
        if (!mStateDirectory.exists() && !mStateDirectory.mkdir()) {
            Assert.fail("Failed to create state directory.  Tests should fail.");
        }
        return mStateDirectory;
    }

    /**
     * Sets the task file byte buffer to be the decoded format of the given string.
     * @param encoded Base64 encoded task file.
     */
    public void setTaskFileBytesFromEncodedString(String encoded) {
        mTaskFileBytes = Base64.decode(encoded, Base64.DEFAULT);
    }

    /**
     * Adds a TabState to the file system.
     * @param tabId ID of the Tab.
     * @param encodedState Base64 encoded TabState.
     * @return Whether or not the TabState was successfully read.
     */
    public boolean addEncodedTabState(int tabId, String encodedState) {
        String filename = TabState.getTabStateFilename(tabId, mIsIncognito);
        File tabStateFile = new File(getStateDirectory(), filename);
        FileOutputStream outputStream = null;
        try {
            outputStream = new FileOutputStream(tabStateFile);
            outputStream.write(Base64.decode(encodedState, 0));
        } catch (FileNotFoundException e) {
            assert false : "Failed to create " + filename;
            return false;
        } catch (IOException e) {
            assert false : "IO exception " + filename;
            return false;
        } finally {
            StreamUtil.closeQuietly(outputStream);
        }

        return true;
    }

    /**
     * Ensures that the state directory and its contents are all wiped from storage.
     */
    public void ensureDirectoryDestroyed() {
        File states = getStateDirectory();
        if (!states.exists()) return;

        File[] files = states.listFiles();
        if (files != null) {
            for (File file : files) {
                if (!file.delete()) Log.e(TAG, "Failed to delete: " + file.getName());
            }
        }
        if (!states.delete()) Log.e(TAG, "Failed to delete: " + states.getName());
    }
}
