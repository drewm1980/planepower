#!/usr/bin/env bash

GITHOME=`git rev-parse --show-toplevel`

# Partially automate missing build rules with a script.
(
cd src/model
python NMPC.py # Generates rk4.c
$GITHOME/tools/rename_casadi_functions # Generates casadi_functions.cpp
)

