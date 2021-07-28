/*
 * Copyright (C) 2020 The MoKee Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#define LOG_TAG "FifoWatcher"

#include "FifoWatcher.h"

#include <android-base/logging.h>
#include <stdio.h>
#include <stdint.h>

#include <fcntl.h>
#include <sys/stat.h>

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

static void *work(void *arg);

FifoWatcher::FifoWatcher(const std::string& file, const WatcherCallback& callback)
    : mFile(file)
    , mCallback(callback)
    , mExit(false)
    {
    if (pthread_create(&mPoll, NULL, work, this)) {
        LOG(ERROR) << "pthread creation failed: " << errno;
    }
}

void FifoWatcher::exit() {
    mExit = true;
    LOG(INFO) << "Exit";
}

static void *work(void *arg) {
    int fd, len, value;
    char buf[10];

    LOG(INFO) << "Creating thread";

    FifoWatcher *thiz = (FifoWatcher *)arg;
    const char *file = thiz->mFile.c_str();

    unlink(file);

    if (mkfifo(file, 0660) < 0) {
        LOG(ERROR) << "Failed creating " << thiz->mFile << ": " << errno;
        return NULL;
    }

    fd = open(file, O_RDONLY);
    if (fd < 0) {
        LOG(ERROR) << "Failed opening " << thiz->mFile << ": " << errno;
        return NULL;
    }

    while (!thiz->mExit) {
        len = read(fd, buf, sizeof(buf));
        if (len < 0) {
            LOG(ERROR) << "Failed reading " << thiz->mFile << ": " << errno;
            goto error;
        } else if (len == 0) {
            usleep(10 * 1000);
            continue;
        }

        value = atoi(buf);

        thiz->mCallback(thiz->mFile, value);
    }

    LOG(INFO) << "Exiting worker thread";

error:
    close(fd);

    return NULL;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
