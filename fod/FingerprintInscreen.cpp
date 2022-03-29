/*
 * Copyright (C) 2019 The LineageOS Project
 * Copyright (C) 2019-2020 The MoKee Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "InscreenService"

#include "FingerprintInscreen.h"
#include "KeyEventWatcher.h"

#include <cutils/properties.h>
#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include <fstream>
#include <cmath>
#include <thread>

#define NOTIFY_FINGER_DOWN 1536
#define NOTIFY_FINGER_UP 1537
#define NOTIFY_UI_READY 1607
#define NOTIFY_UI_DISAPPER 1608
// #define NOTIFY_ENABLE_PAY_ENVIRONMENT 1609
// #define NOTIFY_DISABLE_PAY_ENVIRONMENT 1610

#define HBM_ENABLE_PATH "/sys/class/meizu/lcm/display/hbm"
#define BRIGHTNESS_PATH "/sys/class/backlight/panel0-backlight/brightness"

#define TOUCHPANAL_DEV_PATH "/dev/input/event2"

#define FOD_POS_X 149 * 3
#define FOD_POS_Y 604 * 3
#define FOD_SIZE 62 * 3

#define KEY_FOD 0x0272

namespace vendor {
namespace asop {
namespace biometrics {
namespace fingerprint {
namespace inscreen {
namespace V1_0 {
namespace implementation {

/*
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
}

template <typename T>
static T get(const std::string& path, const T& def) {
    std::ifstream file(path);
    T result;

    file >> result;
    return file.fail() ? def : result;
}

static KeyEventWatcher *keyEventWatcher;

static void sighandler(int) {
    LOG(INFO) << "Exiting";
    keyEventWatcher->exit();
}

FingerprintInscreen::FingerprintInscreen()
    : mIconShown{false}
    , mFingerPressed{false}
    {
    this->mGoodixFpDaemon = IGoodixFingerprintDaemon::getService();

    keyEventWatcher = new KeyEventWatcher(TOUCHPANAL_DEV_PATH, [this](const std::string&, input_event evt) {
        if (evt.code == KEY_FOD) {
            notifyKeyEvent(evt.value);
        }
    });

    signal(SIGTERM, sighandler);
}

Return<int32_t> FingerprintInscreen::getPositionX() {
    return FOD_POS_X;
}

Return<int32_t> FingerprintInscreen::getPositionY() {
    return FOD_POS_Y;
}

Return<int32_t> FingerprintInscreen::getSize() {
    return FOD_SIZE;
}

Return<void> FingerprintInscreen::onStartEnroll() {
    return Void();
}

Return<void> FingerprintInscreen::onFinishEnroll() {
    return Void();
}

Return<void> FingerprintInscreen::onPress() {
    mFingerPressed = true;
    set(HBM_ENABLE_PATH, 1);
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        if (mFingerPressed) {
            notifyHal(NOTIFY_FINGER_DOWN);
        }
    }).detach();
    return Void();
}

Return<void> FingerprintInscreen::onRelease() {
    mFingerPressed = false;
    set(HBM_ENABLE_PATH, 0);
    notifyHal(NOTIFY_FINGER_UP);
    return Void();
}

Return<void> FingerprintInscreen::onShowFODView() {
    mIconShown = true;
    notifyHal(NOTIFY_UI_READY);
    return Void();
}

Return<void> FingerprintInscreen::onHideFODView() {
    mIconShown = false;
    notifyHal(NOTIFY_UI_DISAPPER);
    return Void();
}

Return<bool> FingerprintInscreen::handleAcquired(int32_t, int32_t) {
    return false;
}

Return<bool> FingerprintInscreen::handleError(int32_t, int32_t) {
    return false;
}

Return<void> FingerprintInscreen::setLongPressEnabled(bool) {
    return Void();
}

Return<int32_t> FingerprintInscreen::getDimAmount(int32_t) {
    int brightness = get(BRIGHTNESS_PATH, 0);
    float alpha = 1.0 - pow(brightness / 255.0f, 0.455);
    float min = (float) property_get_int32("fod.dimming.min", 0);
    float max = (float) property_get_int32("fod.dimming.max", 255);
    return min + (max - min) * alpha;
}

Return<bool> FingerprintInscreen::shouldBoostBrightness() {
    return false;
}

Return<void> FingerprintInscreen::setCallback(const sp<IFingerprintInscreenCallback>& callback) {
    std::lock_guard<std::mutex> _lock(mCallbackLock);
    mCallback = callback;
    return Void();
}

void FingerprintInscreen::notifyKeyEvent(int value) {
    if (!mIconShown) {
        return;
    }

    LOG(INFO) << "notifyKeyEvent: " << value;

    std::lock_guard<std::mutex> _lock(mCallbackLock);
    if (mCallback == nullptr) {
        return;
    }

    if (value) {
        Return<void> ret = mCallback->onFingerDown();
        if (!ret.isOk()) {
            LOG(ERROR) << "FingerDown() error: " << ret.description();
        }
    } else {
        Return<void> ret = mCallback->onFingerUp();
        if (!ret.isOk()) {
            LOG(ERROR) << "FingerUp() error: " << ret.description();
        }
    }
}

void FingerprintInscreen::notifyHal(int32_t cmd) {
    hidl_vec<int8_t> data;
    Return<void> ret = this->mGoodixFpDaemon->sendCommand(cmd, data, [&](int32_t, const hidl_vec<int8_t>&) {
    });
    if (!ret.isOk()) {
        LOG(ERROR) << "notifyHal(" << cmd << ") error: " << ret.description();
    }
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace inscreen
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace asop
}  // namespace vendor
