#!/usr/bin/env bash

# Run make in all subdirectories
thisfilename=`basename $0`
for x in `ls | grep -v soem | grep -v $thisfilename`
do
	(
	cd $x
	rosrun rtt_rosnode rtt-upgrade-2.5
	rm -rf bin build lib
	echo "Building in subdirectory $x..."
	make
	)
done
