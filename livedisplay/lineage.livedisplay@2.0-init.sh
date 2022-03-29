#!/vendor/bin/sh

if [[ -f /data/vendor/display/aosp_color_profile ]]; then
    cat /data/vendor/display/aosp_color_profile > /sys/class/meizu/lcm/display/lut
fi
