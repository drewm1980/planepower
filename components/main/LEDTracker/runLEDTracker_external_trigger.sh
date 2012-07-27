#!/bin/sh
#$MYOROCOSCOMPONENTS/usecases/common.sh

reinsertCameras

#rosrun ocl deployer-gnulinux -lDebug -s LEDTracker_external_trigger.ops
rosrun ocl deployer-gnulinux -lInfo -s LEDTracker_external_trigger.ops
#rosrun ocl deployer-gnulinux -lWarning -s LEDTracker_external_trigger.ops
#rosrun ocl deployer-gnulinux -lError -s LEDTracker_external_trigger.ops

