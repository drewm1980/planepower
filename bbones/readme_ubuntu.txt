these are Andrew's notes for installing ubuntu on bbb; note, not working yet!


http://www.armhf.com/index.php/download/

http://www.armhf.com/index.php/getting-started-with-ubuntu-img-file/

To copy the image to an sd card do something like:
xz -cd ./ubuntu-13.10-armhf-minfs-*.img.xz > /dev/mmcblk129836

partprobe

To boot from the SD card on a board NOT flashed with ubuntu yet, insert flashed sd card, hold S2 (the one above the SD card) while booting


