#
# Copyright (C) 2020 The MoKee Open Source Project
#
# SPDX-License-Identifier: Apache-2.0
#

$(call inherit-product, vendor/meizu/m1926/m1926-vendor.mk)

# Overlays
DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay \
    $(LOCAL_PATH)/overlay-lineage

# Boot animation
TARGET_SCREEN_HEIGHT := 2232
TARGET_SCREEN_WIDTH := 1080

# Permissions
PRODUCT_COPY_FILES += \
    vendor/lineage/config/permissions/vendor.lineage.biometrics.fingerprint.inscreen.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/vendor.lineage.biometrics.fingerprint.inscreen.xml

# Audio
PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/a2dp_audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/audio_policy_configuration.xml

# Charger
PRODUCT_COPY_FILES += \
    $(call find-copy-subdir-files,*,$(LOCAL_PATH)/charger,root/res/images/charger)

# Fingerprint
PRODUCT_PACKAGES += \
    lineage.biometrics.fingerprint.inscreen@1.0-service.m1926

# Init
PRODUCT_PACKAGES += \
    init.target.rc

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/vendor_fstab.qcom:$(TARGET_COPY_OUT_SYSTEM)/etc/vendor_fstab.qcom

# Input
PRODUCT_COPY_FILES += \
    $(call find-copy-subdir-files,*,$(LOCAL_PATH)/keylayout,$(TARGET_COPY_OUT_SYSTEM)/usr/keylayout)

# LiveDisplay
PRODUCT_PACKAGES += \
    lineage.livedisplay@2.0-init.sh \
    lineage.livedisplay@2.0-service.m1926

# Power
PRODUCT_PACKAGES += \
    android.hardware.power@1.0-impl.m1926

# Touch
PRODUCT_PACKAGES += \
    lineage.touch@1.0-service.m1926

# Inherit from sm6150-common
$(call inherit-product, device/meizu/sm6150-common/common.mk)
