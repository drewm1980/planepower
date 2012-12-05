#!/usr/bin/env bash

echo "Bootstrapping IPOPT..."

echo "Installing dependencies that are in apt..."
sudo apt-get install gcc llvm-dev octave3.2-headers gfortran git cmake liblapack-dev swig ipython python-dev python-numpy python-scipy python-matplotlib --install-recommends

[ -d "ipopt" ] && svn co https://projects.coin-or.org/svn/Ipopt/stable/3.10 ipopt

(cd ipopt
(cd ThirdParty/HSL
if [ ! -d hsl ]; then
	scp -r nonfree@moinette.esat.kuleuven.be:hsl/ .
	mv hsl/*.f .
fi
)
for x in Blas Lapack Metis Mumps
do
	(cd ThirdParty/$x && ./get.$x)
done
./configure --prefix=/usr/local
make
sudo make install
)

echo "Bootstrapping CasADi..."
(
cd casadi
mkdir -p build
(cd build
#git clean -xdf swig
cmake ..
make all python
sudo make install
sudo make install_python
)
)
