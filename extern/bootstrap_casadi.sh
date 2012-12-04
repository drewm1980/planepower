#!/usr/bin/env bash

sudo apt-get install gcc llvm-dev octave3.2-headers gfortran git cmake liblapack-dev swig ipython python-dev python-numpy python-scipy python-matplotlib --install-recommends

echo "Bootstrapping IPOPT..."

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
git clean -xdf swig
cmake .
sudo make install
sudo make install_python
)
