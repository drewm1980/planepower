#!/usr/bin/env bash

# This script is intended to perform a complete build of our entire flight software
# and all dependencies for the first time, from scratch.
#
# Once you've made it through here once on a machine, you should rebuild with
# tup upd

(
echo "Bootstrapping dependencies in extern..."
cd extern
./bootstrap.sh
echo "Done bootstrapping dependencies in extern..."
)

# Do this in this (planepower) directory!!
tup init

./setup_precommit_hook
