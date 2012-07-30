#!/bin/sh
wget http://sourceforge.net/projects/libdc1394/files/libdc1394-2/2.0.3/libdc1394-2.0.3.tar.gz/download
tar xvfz libdc1394*
cd libd*
./configure
make
