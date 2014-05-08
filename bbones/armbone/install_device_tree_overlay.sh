#!/usr/bin/env bash
mkdir /mnt/boot
mount /dev/mmcblk0p1 /mnt/boot
echo "Please add something like the following to the end of the single line of /mnt/boot/uEnv.txt:"
echo "capemgr.enable_partno=BB-SPI1-01"
echo " or "
echo "capemgr.enable_partno=BB-Bitbanging"
