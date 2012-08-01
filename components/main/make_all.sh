#!/usr/bin/env bash

# Run make in all subdirectories
thisfilename=`basename $0`
for x in `ls | grep -v soem | grep -v $thisfilename`
do
	(
	cd $x
	echo "Building in subdirectory $x..."
	make
	)
done
