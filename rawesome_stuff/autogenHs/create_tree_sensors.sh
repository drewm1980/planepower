#!/usr/bin/env bash
names="LineAngle Servos"
ln -sf SensorsRealFiles/Sensors.hs Sensors.hs
mkdir -p Sensors
for x in $names;
do
	#echo $x
	#mkdir -p Sensors/$x
	(
	cd Sensors
	ln -sf ../SensorsRealFiles/$x.hs $x.hs
	)
done

