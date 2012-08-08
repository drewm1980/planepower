#!/usr/bin/env bash
# This script automates what used to be a frustrating
# 10 step process we used to have to nag Milan about
mkdir -p build
(
cd build
cmake ..
make
)
. mpc_settings.m
./NMPC $W0 $Ncvp $Tc 1 $r
