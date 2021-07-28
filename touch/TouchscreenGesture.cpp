/*
 * Copyright (C) 2020 The MoKee Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#define LOG_TAG "TouchscreenGesture"

#include "TouchscreenGesture.h"
#include "FifoWatcher.h"

#include <android-base/logging.h>
#include <stdio.h>
#include <stdint.h>

#define GESTURE_CONTROL_PATH "/sys/class/meizu/tp/gesture_control"

#define DT2W_FIFO_PATH "/dev/lineage.touch@1.0/dt2w"

#define SLIDE_LEFT_ENABLE   (1 << 0)
#define SLIDE_RIGHT_ENABLE  (1 << 1)
#define SLIDE_UP_ENABLE     (1 << 2)
#define SLIDE_DOWN_ENABLE   (1 << 3)
#define DOUBLE_TAP_ENABLE   (1 << 4)
#define ONECE_TAP_ENABLE    (1 << 5)
#define LONG_TAP_ENABLE     (1 << 6)
#define DRAW_E_ENABLE       (1 << 7)
#define DRAW_C_ENABLE       (1 << 8)
#define DRAW_W_ENABLE       (1 << 9)
#define DRAW_M_ENABLE       (1 << 10)
#define DRAW_O_ENABLE       (1 << 11)
#define DRAW_S_ENABLE       (1 << 12)
#define DRAW_V_ENABLE       (1 << 13)
#define DRAW_Z_ENABLE       (1 << 14)
#define FOD_ENABLE          (1 << 24)
#define ALL_GESTURE_ENABLE  (1 << 31)

namespace {
static std::string hex(uint32_t value) {
    char buf[9];
    snprintf(buf, sizeof(buf), "%08x", value);
    return buf;
}
}  // anonymous namespace

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

const std::map<int32_t, TouchscreenGesture::GestureInfo> TouchscreenGesture::kGestureInfoMap = {
    {0, {0x0280, "one_finger_left_swipe", SLIDE_LEFT_ENABLE}},
    {1, {0x0281, "one_finger_right_swipe", SLIDE_RIGHT_ENABLE}},
    {2, {0x0282, "one_finger_up_swipe", SLIDE_UP_ENABLE}},
    {3, {0x0283, "one_finger_down_swipe", SLIDE_DOWN_ENABLE}},
    {4, {0x0291, "letter_c", DRAW_C_ENABLE}},
    {5, {0x0296, "letter_v", DRAW_V_ENABLE}},
};

static FifoWatcher *dt2wWatcher;

static void sighandler(int) {
    LOG(INFO) << "Exiting";
    dt2wWatcher->exit();
}

TouchscreenGesture::TouchscreenGesture() {
    dt2wWatcher = new FifoWatcher(DT2W_FIFO_PATH, [this](const std::string& file, int value) {
        LOG(INFO) << "WatcherCallback: " << file << ": " << value;
        setValue(DOUBLE_TAP_ENABLE, value);
    });

    signal(SIGTERM, sighandler);

    setValue(FOD_ENABLE, true);
}

Return<void> TouchscreenGesture::getSupportedGestures(getSupportedGestures_cb resultCb) {
    std::vector<Gesture> gestures;

    for (const auto& entry : kGestureInfoMap) {
        gestures.push_back({entry.first, entry.second.name, entry.second.keycode});
    }

    resultCb(gestures);
    return Void();
}

Return<bool> TouchscreenGesture::setGestureEnabled(
    const ::vendor::lineage::touch::V1_0::Gesture& gesture, bool enabled) {
    const auto entry = kGestureInfoMap.find(gesture.id);
    if (entry == kGestureInfoMap.end()) {
        return false;
    }

    const uint32_t value = entry->second.value;
    LOG(INFO) << "setGestureEnabled: " << hex(value) << " " << enabled;

    return setValue(value, enabled);
}

bool TouchscreenGesture::setValue(uint32_t value, bool enabled) {
    FILE *file = fopen(GESTURE_CONTROL_PATH, "wb");
    if (!file) {
        LOG(ERROR) << "setValue: Failed opening " << GESTURE_CONTROL_PATH;
        return false;
    }

    mValue &= ~ALL_GESTURE_ENABLE;

    if (enabled) {
      mValue |= value;
    } else {
      mValue &= ~value;
    }

    if (mValue != 0) {
        mValue |= ALL_GESTURE_ENABLE;
    }

    uint8_t buf[4];
    buf[0] = mValue & 0xFF;
    buf[1] = (mValue >> 8) & 0xFF;
    buf[2] = (mValue >> 16) & 0xFF;
    buf[3] = (mValue >> 24) & 0xFF;

    size_t ret = fwrite(buf, sizeof(buf), 1, file);
    fclose(file);

    LOG(INFO) << "setValue: " << hex(mValue) << " " << ret;

    return true;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
