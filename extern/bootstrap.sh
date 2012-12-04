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

(./bootstrap_orocos.sh)

(./bootstrap_casadi.sh)

(./bootstrap_acado.sh)
