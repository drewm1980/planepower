#!/usr/bin/env bash

# Upgrade the components in lowlevel
# Building: 
echo "Upgrading lowlevel components to tup"
c="encoder encoderLatch mcuHandler safeStopper segFaulter voltageController"
# Not building yet: soem

for x in $c; do ./upgrade_component_to_tup.sh lowlevel/$x; done

#c=`ls|grep -v *.cpp|grep -v *.h|grep -v .sh | grep -v Tupfile`

(
echo "Upgrading main components to tup"
cd main
./upgrade_to_tup.sh
)

