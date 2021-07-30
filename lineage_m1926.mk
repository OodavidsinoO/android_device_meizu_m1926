#
# Copyright (C) 2020 The MoKee Open Source Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_p.mk)

# Inherit from m1926 device
$(call inherit-product, device/meizu/m1926/device.mk)

# Inherit some common MoKee stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

PRODUCT_PROPERTY_OVERRIDES += \
    ro.mk.maintainer=DavidSin

PRODUCT_NAME := lineage_m1926
PRODUCT_BRAND := meizu
PRODUCT_DEVICE := m1926
PRODUCT_MANUFACTURER := meizu
PRODUCT_MODEL := meizu 16Xs

PRODUCT_BUILD_PROP_OVERRIDES += \
    TARGET_DEVICE="meizu16Xs" \
    PRODUCT_NAME="meizu_16Xs_CN" \
    PRIVATE_BUILD_DESC="meizu_16Xs_CN-user 9 PKQ1.190302.001 1573695900 release-keys"

BUILD_FINGERPRINT := meizu/meizu_16Xs_CN/meizu16Xs:9/PKQ1.190302.001/1573695900:user/release-keys

PRODUCT_PRODUCT_PROPERTIES += \
    ro.sf.lcd_density=400

PRODUCT_PRODUCT_PROPERTIES += \
    fod.dimming.min=5 \
    fod.dimming.max=250
