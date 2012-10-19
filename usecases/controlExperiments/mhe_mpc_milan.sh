#!/bin/sh
$MYOROCOSCOMPONENTS/usecases/common.sh

rosrun ocl deployer-gnulinux -lerror -s mhe_mpc_milan.ops
