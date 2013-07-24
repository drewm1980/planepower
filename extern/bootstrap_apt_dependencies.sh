#!/usr/bin/env bash
echo "Installing dependencies that are in apt..."
sudo apt-get install cmake libopencv-dev gcc llvm-dev octave3.2-headers gfortran git cmake liblapack-dev swig ipython python-dev python-numpy python-scipy python-matplotlib --install-recommends ruby-nokogiri wget libfuse-dev libdc1394-22-dev
