#! /usr/bin/env bash

# find / -name "*.dts" | grep UART
# ls -1 /lib/firmware/*UART*.dts

# cat /sys/devices/bone_capemgr.8/slots

# Kick out HDMI
#HDMI_SLOTNO=`grep HDMI /sys/devices/bone_capemgr.8/slots | cut -b2`
#echo "-$HDMI_SLOTNO" > /sys/devices/bone_capemgr.8/slots

# The above seems to cause a segfault with current version of the stock Angstrom kernel. adafruit warned this might happen

# Add our chosen UART port
UART_NO=2
FIRMWARE_STRING=`ls -1 /lib/firmware/*UART$UART_NO*.dts | xargs -I{} basename {} .dts`
#echo $FIRMWARE_STRING > /sys/devices/bone_capemgr.*/slots
echo "Found firmware string $FIRMWARE_STRING"

HDMI_FIRMWARE_STRING=Bone-Black-HDMI-00A0

mkdir /mnt/boot
mount /dev/mmcblk0p1 /mnt/boot
STR0="optargs=quiet drm.debug=7"
STR1="capemgr.disanable_partno=\"$HDMI_FIRMWARE_STRING\""
STR2="capemgr.enable_partno=\"$FIRMWARE_STRING\""
#cat /mnt/boot/uEnv.txt
echo "$STR0 $STR1 $STR2" > /mnt/boot/uEnv.txt
echo "You need to reboot for this to take effect"

# cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pingroups | grep group


