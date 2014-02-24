#!/usr/bin/env bash

# Install stuff for doing gpio on bbb using python
# From http://learn.adafruit.com/setting-up-io-python-library-on-beaglebone-black/installation

/usr/bin/ntpdate -b -s -u pool.ntp.org
opkg update && opkg install python-pip python-setuptools python-smbus
pip install Adafruit_BBIO

