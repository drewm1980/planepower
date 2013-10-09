 #!/bin/bash
bash mkcard.sh /dev/mmcblk0

mkdir /mnt/p1
mkdir /mnt/p2

mount /dev/mmcblk0p1 /mnt/p1
mount /dev/mmcblk0p2 /mnt/p2

cp uEnv.txt /mnt/p1
cp sdcard_present /mnt/p2


umount /mnt/p1
umount /mnt/p2

echo "FORMATTING SDCARD FOR BEAGLEBONE SUCCEEDED\n"
