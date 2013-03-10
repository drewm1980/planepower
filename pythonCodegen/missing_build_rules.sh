#!/usr/bin/env bash

GITHOME=`git rev-parse --show-toplevel`
#$GITHOME/tools/rename_casadi_functions # Generates casadi_functions.cpp

# Partially automate missing build rules with a script.
python markers_from_pose.py
python pose_from_markers.py

