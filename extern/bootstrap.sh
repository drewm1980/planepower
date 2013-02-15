#!/usr/bin/env bash

sudo apt-get install libopencv-dev

(./bootstrap_tup.sh)

(./bootstrap_orocos.sh)

(./bootstrap_casadi.sh)

(./bootstrap_acado.sh)
