#!/usr/bin/env bash

(
echo "Bootstrapping tup..."
cd tup
if [ -x `which tup` ]
then
	echo "Tup installed!"
	tup upd .
else
	echo "Tup not installed!"
	./bootstrap.sh
fi
sudo cp tup /usr/local/bin/
sudo cp tup.1 /usr/local/share/man/man1/
echo "Done bootstrapping tup"
)

echo "Bootstrapping OROCOS..."
(
cd rtt
cmake . -DENABLE_CORBA=ON -DCORBA_IMPLEMENTATION=OMNIORB
make -j4
sudo make install
)
(
cd log4cpp
cmake .
make -j4
sudo make install
)
(
cd ocl
cmake .
make -j4
sudo make install
)

echo "Bootstrapping IPOPT..."
svn co https://projects.coin-or.org/svn/Ipopt/stable/3.10 ipopt
(cd ipopt
(cd ThirdParty/HSL
scp -r nonfree@moinette.esat.kuleuven.be:hsl/ .
mv hsl/*.f .
rmdir hsl
)
for x in Blas Metis Mumps
do
	(cd ThirdParty/$x && ./get.$x)
done
./configure --prefix=/usr/local
make
sudo make install
)
echo "Bootstrapping CasADi..."
sudo apt-get install gcc llvm-dev octave3.2-headers gfortran git cmake liblapack-dev swig ipython python-dev python-numpy python-scipy python-matplotlib --install-recommends
(
cd casadi
cmake .
make_python
sudo make install_python
)

(./bootstrap_acado.sh)
