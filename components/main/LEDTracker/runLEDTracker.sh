#!/bin/sh
#$MYOROCOSCOMPONENTS/usecases/common.sh

reinsertCameras

#rosrun ocl deployer-gnulinux -lDebug -s LEDTracker.ops
rosrun ocl deployer-gnulinux -lInfo -s LEDTracker.ops
#rosrun ocl deployer-gnulinux -lWarning -s LEDTracker.ops
#rosrun ocl deployer-gnulinux -lError -s LEDTracker.ops

