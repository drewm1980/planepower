#!/usr/bin/env bash

echo "Bootstrapping ACADO..."

echo "Downloading and unpacking a stable acado revision from tar file..."
ACADO_PRIVATE=acado_private_r3094_trimmed
if [ ! -f $ACADO_PRIVATE.tar.gz ]; then
	scp gitmirrorbot@moinette.esat.kuleuven.be:acado_private_r3094_trimmed.tar.gz .
fi
[ ! -d $ACADO_PRIVATE.tar.gz ] && tar -xvf $ACADO_PRIVATE.tar

echo "Make main acado..."
(
mkdir -p $ACADO_PRIVATE/build
cd $ACADO_PRIVATE/build
cmake ..
make
)

source $ACADO_PRIVATE/build/acado_env.sh

echo "Make mhe codegen stuff..."
(
cd $ACADO_PRIVATE/testing/mvukov/mhe_export
cmake .
make
)

