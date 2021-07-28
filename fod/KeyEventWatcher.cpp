/*
 * Copyright (C) 2020 The MoKee Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#define LOG_TAG "KeyEventWatcher"

#include "KeyEventWatcher.h"

#include <android-base/logging.h>
#include <stdio.h>
#include <stdint.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/epoll.h>

#define EPOLLEVENTS 20

namespace vendor {
namespace lineage {
namespace biometrics {
namespace fingerprint {
namespace inscreen {
namespace V1_0 {
namespace implementation {

static void *work(void *arg);

KeyEventWatcher::KeyEventWatcher(const std::string& file, const WatcherCallback& callback)
    : mFile(file)
    , mCallback(callback)
    , mExit(false)
    {
    if (pthread_create(&mPoll, NULL, work, this)) {
        LOG(ERROR) << "pthread creation failed: " << errno;
    }
}

void KeyEventWatcher::exit() {
    mExit = true;
    LOG(INFO) << "Exit";
}

static void *work(void *arg) {
    int epoll_fd, input_fd;
    struct epoll_event ev;
    struct input_event key_event;
    int nevents = 0;

    LOG(INFO) << "Creating thread";

    KeyEventWatcher *thiz = (KeyEventWatcher *)arg;

    input_fd = open(thiz->mFile.c_str(), O_RDONLY);
    if (input_fd < 0) {
        LOG(ERROR) << "Failed opening input dev " << thiz->mFile << ": " << errno;
        return NULL;
    }

    ev.events = EPOLLIN;
    ev.data.fd = input_fd;

    epoll_fd = epoll_create(EPOLLEVENTS);
    if (epoll_fd == -1) {
        LOG(ERROR) << "Failed epoll_create: " << errno;
        goto error;
    }

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, input_fd, &ev) == -1) {
        LOG(ERROR) << "Failed epoll_ctl: " << errno;
        goto error;
    }

    while (!thiz->mExit) {
        struct epoll_event events[EPOLLEVENTS];

        nevents = epoll_wait(epoll_fd, events, EPOLLEVENTS, -1);
        if (nevents == -1) {
            if (errno == EINTR) {
                continue;
            }
            LOG(ERROR) << "Failed epoll_wait: " << errno;
            break;
        }

        for (int i = 0; i < nevents; i++) {
            int ret;
            int fd = events[i].data.fd;
            lseek(fd, 0, SEEK_SET);
            ret = read(fd, &key_event, sizeof(key_event));
            if (ret && key_event.type == EV_KEY) {
                thiz->mCallback(thiz->mFile, key_event);
            }
        }
    }

    LOG(INFO) << "Exiting worker thread";

error:
    close(input_fd);

    if (epoll_fd >= 0)
        close(epoll_fd);

    return NULL;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace inscreen
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace lineage
}  // namespace vendor
