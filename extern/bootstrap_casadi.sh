#!/usr/bin/env bash

echo "Bootstrapping CasADi..."
(
cd casadi
mkdir -p build
(
cd build
rm CMakeCache.txt
cmake ..
make -j4
make python
sudo make install_python
)
)
