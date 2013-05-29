#!/usr/bin/env bash

echo "Bootstrapping CasADi..."
(
scp nonfree@moinette:casadi/tested/casadi_latest_amd64.deb .
sudo dpkg -i casadi_latest_amd64.deb
)
