#! /usr/bin/env bash

# find / -name "*.dts" | grep UART
# ls -1 /lib/firmware/*UART*.dts

# cat /sys/devices/bone_capemgr.8/slots

# Add our chosen UART port
# You need to use the strings from "part-number" from instide the .dts files in /lib/firmware

DISABLE_HDMI_STRING="capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN"
#ENABLE_UART_STRING="capemgr.enable_partno=BB-UART2,BB-UART4"
ENABLE_UART_STRING=""

mkdir -p /mnt/boot
mount /dev/mmcblk0p1 /mnt/boot
STR0="optargs=quiet drm.debug=7"
echo "$STR0 $DISABLE_HDMI_STRING $ENABLE_UART_STRING" > /mnt/boot/uEnv.txt
cat /mnt/boot/uEnv.txt
echo "You need to reboot for this to take effect"

# cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pingroups | grep group


