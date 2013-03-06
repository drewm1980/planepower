#! /usr/bin/env bash

# Figure out where PLANEPOWER_ROOT in a way
# That is robust to this file being sourced
# from "anywhere"
PRG="$BASH_SOURCE"
progname=`basename "$BASH_SOURCE"`
while [ -h "$PRG" ] ; do
    ls=`ls -ld "$PRG"`
    link=`expr "$ls" : '.*-> \(.*\)$'`
    if expr "$link" : '/.*' > /dev/null; then
        PRG="$link"
    else
        PRG=`dirname "$PRG"`"/$link"
    fi
done
tmp=`dirname $PRG`
export PLANEPOWER_ROOT=`readlink -f $tmp`

source $PLANEPOWER_ROOT/extern/orocos/env.sh
source $PLANEPOWER_ROOT/extern/acado_public/build/acado_env.sh
source $PLANEPOWER_ROOT/extern/acado_private/testing/mvukov/mhe_export/build/mhe_export_env.sh

DEFAULT_RAWESOME_ROOT=`dirname $PLANEPOWER_ROOT`/rawesome
if [ -d $DEFAULT_RAWESOME_ROOT ]; then
	export RAWESOME_ROOT=$DEFAULT_RAWESOME_ROOT
else
	echo WARNING!!! Could not find Gregs rawesome repo in the usual place.
	echo            Please manually set RAWESOME_ROOT environment if you
	echo			installed rawesome repo in a non-standard place
fi

# This makes sure the highwind version of deployer is found
# instead of the stock version.
export PATH=$PLANEPOWER_ROOT/tools:$PATH
