#!/usr/bin/env bash

echo "Bootstrapping IPOPT..."

mkdir -p ipopt
if [ ! -d ipopt/.svn ]; then
	svn co https://projects.coin-or.org/svn/Ipopt/stable/3.10 ipopt
fi

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
mkdir -p build
(cd build
../configure --prefix=/usr/local --disable-shared ADD_FFLAGS=-fPIC ADD_CFLAGS=-fPIC ADD_CXXFLAGS=-fPIC
make -j3
make test
sudo make install
)
)
