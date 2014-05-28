#!/usr/bin/env bash
echo "Installing dependencies that are in apt..."
sudo apt-get install cmake libopencv-dev gcc llvm-dev octave3.2-headers gfortran git cmake liblapack-dev swig ipython python-dev python-numpy python-scipy python-matplotlib --install-recommends ruby-nokogiri wget libfuse-dev libdc1394-22-dev

# on Ubuntu 12.04:
sudo apt-get install python-pip
sudo pip install pycparser

# on newer versions
sudo apt-get install python-pycparser
