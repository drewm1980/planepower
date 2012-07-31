#!/bin/bash
for x in `ls *.log`
do
	out=`basename $x .log`.dat
	cat $x | grep Logger >> $out
	sed -i -ne "/nan nan nan nan nan nan nan nan nan nan nan nan/,/Unloading/ p" $out
	sed -i -e "/nan nan nan nan nan nan nan nan nan nan nan nan/d" $out
	sed -i -e "/Unloading/d" $out
	cp $out foo
	cut -d' ' -f7- <foo >$out
	rm foo
done
