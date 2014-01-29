To get started with a Beaglebone running Angstrom:

You ~could plug in with an hdmi monitor, but we never seem to have the cables on hand.  

1. Plug in the USB cable to a linux box, and it will appear as a network adapter (and also as a drive?).

2. 
sudo apt-get install sshpass
sshpass -p '' ssh root@192.168.7.2

(Then configure the ethernet connection using normal linux stuff, i.e. ifconfig, etc...)
