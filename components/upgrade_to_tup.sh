#!/usr/bin/env bash

# Upgrade the components in lowlevel
#c=`find lowlevel -name manifest.xml | xargs -n1 dirname | xargs -n1 basename | grep -v soem`

# Building: 
c="safeStopper encoderLatch mcuHandler safeStopper voltageController"

# Not building yet: encoder soem

for x in $c
do
	./upgrade_component_to_tup.sh lowlevel/$x
done

#c=`ls|grep -v *.cpp|grep -v *.h|grep -v .sh | grep -v Tupfile`
