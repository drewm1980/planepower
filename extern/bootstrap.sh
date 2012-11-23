#!/usr/bin/env bash

(
echo "Bootstrapping tup..."
cd tup
if [ -x `which tup`]
	tup upd .
else
	./bootstrap.sh
fi
sudo cp tup /usr/local/bin/
sudo cp tup.1 /usr/local/share/man/man1/
git clean -xdf .
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

