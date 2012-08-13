#!/bin/sh
$MYOROCOSCOMPONENTS/usecases/common.sh

rosrun ocl deployer-gnulinux -lerror -s imu_controlSequence.ops
