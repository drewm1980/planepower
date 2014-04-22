#!/bin/sh

OPS_NAME='dmhe_testing'

deployer-gnulinux -lerror -s ${OPS_NAME}.ops

echo "Wait 2 more seconds to copy all log files..."
sleep 2

timeStamp=`date +%Y%m%d_%H%M%S`'_'${OPS_NAME}
mkdir ${timeStamp}
cp imuData.nc ${timeStamp}/
cp cameraData.nc ${timeStamp}/
cp encoderData.nc ${timeStamp}/
cp angleData.nc ${timeStamp}/
cp winchData.nc ${timeStamp}/

cp dynamicMheData.nc ${timeStamp}/
echo "All log files are copied to folder: ${timeStamp}"

# Generate a PDF out of the netCDF log file for the MHE
cd ${timeStamp}
$PLANEPOWER_ROOT/rawesome_stuff/postprocessing/mhe_exp_postprocessing.py $PLANEPOWER_ROOT/properties `pwd` -q=True
cd ..

