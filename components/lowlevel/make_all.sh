#!/usr/bin/env bash

echo "Warning, this file is an ugly hack that should be part of the build system!!!"

# Run make in all subdirectories.
thisfilename=`basename $0`

for x in soem_master soem_core  soem_ebox soem_beckhoff_drivers
do
	(
	cd soem/$x
	echo "$thisfilename: calling make in subdirectory soem/$x..."
	make
	)
done

#for x in `ls |grep -v soem|grep -v $thisfilename`
#do
	#(
	#cd $x
	#echo "$thisfilename: calling make in subdirectory $x..."
	#make
	#)
#done
