#!/bin/sh
$MYOROCOSCOMPONENTS/usecases/common.sh

rosrun ocl deployer-gnulinux -lerror -s Dynamic_EKF.ops
