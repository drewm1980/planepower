This code communicates between the line angle sensor and (?) sends UDP packets with their values.

Historical note, two implementations were tried:

1. At the time, using the SPI hardware on the beaglebone did not work
due to some driver issue.  We could get see some response from the line angle sensor, but the values
were scrambled somehow.  

2. In desperation, we implemented some bitbanged spi i/o as a hack just to get something working.
It indeed worked almost on the first try, but the readout is not really at a constant rate, and the
sleeping may have realtime issues later.

For more info on hardware SPI issues, see this thread:
https://groups.google.com/forum/#!topic/beagleboard/NADsTC5Ro-I


------

Device Tree Overlay to enable the serial ports on BeagleBoneBlack:
BB-SPI1-01-00A0.dts

Install using install_device_tree_overlay.sh

http://learn.adafruit.com/introduction-to-the-beaglebone-black-device-tree/exporting-and-unexporting-an-overlay
