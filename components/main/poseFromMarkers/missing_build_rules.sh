#!/usr/bin/env bash

GITHOME=`git rev-parse --show-toplevel`

# Partially automate missing build rules with a script.

(
cd ../cameraCalibration
missing_build_rules.sh
)
(
cd src
$GITHOME/tools/rename_casadi_functions # Generates casadi_functions.cpp
)

