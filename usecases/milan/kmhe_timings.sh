#!/bin/sh

OPS_NAME='kmhe_timings'

deployer-gnulinux -lerror -s ${OPS_NAME}.ops

timeStamp=`date +%Y%m%d_%H%m%S`'_'${OPS_NAME}
mkdir ${timeStamp}
cp imuData.nc ${timeStamp}/
cp cameraData.nc ${timeStamp}/
cp encoderData.nc ${timeStamp}/
cp lineAngleData.nc ${timeStamp}/
cp kinematicMheData.nc ${timeStamp}/


