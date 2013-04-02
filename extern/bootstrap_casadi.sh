#!/usr/bin/env bash

echo "Bootstrapping CasADi..."
(
cd casadi
mkdir -p build
(
cd build
#git clean -xdf swig
rm CMakeCache.txt
cmake ..
make -j8
make python
sudo make install_python
)
)
