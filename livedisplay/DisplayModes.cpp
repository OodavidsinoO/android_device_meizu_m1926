/*
 * Copyright (C) 2019 The LineageOS Project
 * Copyright (C) 2020 The MoKee Open Source Project
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

#include "DisplayModes.h"
#include "PictureAdjustment.h"

#include <android-base/file.h>
#include <android-base/strings.h>

#include <fstream>

using android::base::ReadFileToString;
using android::base::Trim;
using android::base::WriteStringToFile;

using ::vendor::lineage::livedisplay::V2_0::sdm::PictureAdjustment;

#define INVALID -1
#define DEFAULT 1

namespace vendor {
namespace lineage {
namespace livedisplay {
namespace V2_0 {
namespace sysfs {

DisplayModes::DisplayModes() {
    mDisplayModes.push_back(DisplayMode{0, "dynamic"});
    mDisplayModes.push_back(DisplayMode{1, "standard"});
    mDisplayModes.push_back(DisplayMode{2, "photography"});
    mDisplayModes.push_back(DisplayMode{3, "adaptive"});

    if (isSupported()) {
        DisplayMode mode = getDefaultDisplayModeInternal();
        setDisplayMode(mode.id, true);
    }
}

bool DisplayModes::isSupported() {
    std::fstream hbm(FILE_LUT, hbm.in | hbm.out);
    return hbm.good();
}

DisplayMode DisplayModes::getDisplayModeInternal(const char *file) {
    std::string tmp;
    int32_t id = INVALID;

    if (ReadFileToString(file, &tmp)) {
        id = std::stoi(Trim(tmp));
    }

    if (id < 0 || id > mDisplayModes.size() - 1) {
        return DisplayMode{INVALID, ""};
    }

    return mDisplayModes[id];
}

DisplayMode DisplayModes::getDefaultDisplayModeInternal() {
    DisplayMode defaultMode = getDisplayModeInternal(FILE_LUT_DEFAULT);
    return defaultMode.id != INVALID ? defaultMode : mDisplayModes[DEFAULT];
}

// Methods from ::vendor::lineage::livedisplay::V2_0::IDisplayModes follow.
Return<void> DisplayModes::getDisplayModes(getDisplayModes_cb _hidl_cb) {
    _hidl_cb(mDisplayModes);
    return Void();
}

Return<void> DisplayModes::getCurrentDisplayMode(getCurrentDisplayMode_cb _hidl_cb) {
    _hidl_cb(getDefaultDisplayModeInternal());
    return Void();
}

Return<void> DisplayModes::getDefaultDisplayMode(getDefaultDisplayMode_cb _hidl_cb) {
    _hidl_cb(getDefaultDisplayModeInternal());
    return Void();
}

Return<bool> DisplayModes::setDisplayMode(int32_t modeID, bool makeDefault) {
    DisplayMode currentMode = getDefaultDisplayModeInternal();

    if (modeID < 0 || modeID > mDisplayModes.size() - 1) {
        return false;
    }

    std::string lut = std::to_string(modeID);
    if (!WriteStringToFile(lut, FILE_LUT, true)) {
        return false;
    }

    PictureAdjustment::updateDefaultPictureAdjustment();

    if (makeDefault && !WriteStringToFile(lut, FILE_LUT_DEFAULT, true)) {
        return false;
    }

    return true;
}

}  // namespace sysfs
}  // namespace V2_0
}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
