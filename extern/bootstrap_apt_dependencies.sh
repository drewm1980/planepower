#!/usr/bin/env bash
echo "Installing dependencies that are in apt..."
sudo apt-get install libopencv-dev
sudo apt-get install gcc llvm-dev octave3.2-headers gfortran git cmake liblapack-dev swig ipython python-dev python-numpy python-scipy python-matplotlib --install-recommends
sudo apt-get install ruby-nokogiri wget

sudo apt-get install libfuse-dev
sudo apt-get install libdc1394-22-dev
