#!/usr/bin/env bash

#echo "Edit /boot/uboot/uEnv.txt:"
#echo "capemgr.enable_partno=BB-SPI1-01"
#echo " or "
#echo "capemgr.enable_partno=BB-Bitbanging"

echo BB-Bitbanging > /sys/devices/bone_capemgr.9/slots 
cat /sys/devices/bone_capemgr.9/slots 
