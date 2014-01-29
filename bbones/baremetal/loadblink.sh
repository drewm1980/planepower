echo "mounting..."
sudo mount /media/cardreader

echo "copying..."
sudo cp /home/awagner/AM335X_StarterWare_02_00_01_01/binary/armv7a/cgt_ccs/am335x/beaglebone/gpio/Release/gpioLEDBlink_ti.bin /media/cardreader/app

echo "unmounting..."
sync
sudo umount /media/cardreader

echo "done!"
