#!/bin/sh

OPS_NAME='test_simulator'

deployer-gnulinux -lerror -s ${OPS_NAME}.ops

sleep 1

timeStamp=`date +%Y%m%d_%H%M%S`'_'${OPS_NAME}
mkdir ${timeStamp}
cp imuData.nc ${timeStamp}/
cp cameraData.nc ${timeStamp}/
cp encoderData.nc ${timeStamp}/
cp angleData.nc ${timeStamp}/
cp winchData.nc ${timeStamp}/

cp dynamicMheData.nc ${timeStamp}/

echo "All log files are copied to folder: ${timeStamp}"
