#!/bin/sh

OPS_NAME='closed_loop_tests'

deployer-gnulinux -lerror -s ${OPS_NAME}.ops

echo "Wait 1 more second(s) to copy all log files..."
sleep 1

timeStamp=`date +%Y%m%d_%H%M%S`'_'${OPS_NAME}
mkdir ${timeStamp}
cp imuData.nc ${timeStamp}/
cp cameraData.nc ${timeStamp}/
cp encoderData.nc ${timeStamp}/
cp angleData.nc ${timeStamp}/
cp winchData.nc ${timeStamp}/

cp dynamicMheData.nc ${timeStamp}/
cp dynamicMpcData.nc ${timeStamp}/

echo "All log files are copied to folder: ${timeStamp}"
