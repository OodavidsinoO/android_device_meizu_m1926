/*
 * Copyright (C) 2020 The MoKee Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef VENDOR_MOKEE_TOUCH_V1_0_TOUCHSCREENGESTURE_H
#define VENDOR_MOKEE_TOUCH_V1_0_TOUCHSCREENGESTURE_H

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/mokee/touch/1.0/ITouchscreenGesture.h>
#include <map>

namespace vendor {
namespace mokee {
namespace touch {
namespace V1_0 {
namespace implementation {

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

class TouchscreenGesture : public ITouchscreenGesture {
  public:
    TouchscreenGesture();

    // Methods from ::vendor::mokee::touch::V1_0::ITouchscreenGesture follow.
    Return<void> getSupportedGestures(getSupportedGestures_cb resultCb) override;
    Return<bool> setGestureEnabled(const ::vendor::mokee::touch::V1_0::Gesture& gesture,
                                   bool enabled) override;

  private:
    typedef struct {
        int32_t keycode;
        const char* name;
        uint32_t value;
    } GestureInfo;
    static const std::map<int32_t, GestureInfo> kGestureInfoMap;  // id -> info

    uint32_t mValue;
    bool setValue(uint32_t value, bool enabled);
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace mokee
}  // namespace vendor

#endif  // VENDOR_MOKEE_TOUCH_V1_0_TOUCHSCREENGESTURE_H
