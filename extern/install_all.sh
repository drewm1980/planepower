#!/usr/bin/env bash

(
cd rtt
cmake . -DENABLE_CORBA=ON -DCORBA_IMPLEMENTATION=OMNIORB
make -j4
sudo make install
)
(
cd log4cpp
cmake .
make -j4
sudo make install
)
(
cd ocl
cmake .
make -j4
sudo make install
)

## Set up the run-time linker to find these libs in place.
#(
#sudo su
#SUBDIRS=`find . -name *.so | xargs -L1 dirname | sort | uniq | xargs -L1 readlink -e`
#for x in $SUBIDRS
#do
	#ldconfig $x
#done
#)

