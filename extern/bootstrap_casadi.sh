#!/usr/bin/env bash

echo "Bootstrapping CasADi..."

sudo apt-get install libgfortran3

(
scp nonfree@moinette.esat.kuleuven.be:casadi/1.7.1/casadi_1.7.1-2_amd64.deb ./casadi.deb

sudo dpkg -i casadi.deb

)
