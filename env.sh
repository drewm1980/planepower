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

if [ -d /opt/ros/hydro ]; then
	. /opt/ros/hydro/setup.bash
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/ros/hydro/lib/orocos/gnulinux/ocl/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/ros/hydro/lib/orocos/gnulinux/ocl/plugins/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/ros/hydro/lib/orocos/gnulinux/ocl/types/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/ros/hydro/lib

export LUA_PATH="$LUA_PATH;$PLANEPOWER_ROOT/extern/rttlua_completion/?.lua"
export LUA_PATH="$LUA_PATH;$PLANEPOWER_ROOT/extern/rFSM/?.lua"
export LUA_CPATH="$LUA_CPATH;$PLANEPOWER_ROOT/extern/rttlua_completion/?.so"
alias rttlua='rlwrap -a -r -H ~/.rttlua-history rttlua-gnulinux -lreadline'

export RTT_COMPONENT_PATH=$RTT_COMPONENT_PATH:$PLANEPOWER_ROOT/components/main
# Here is another place we need to switch between carousel 1 and carousel 2 manually...
if false; then
	export RTT_COMPONENT_PATH=$RTT_COMPONENT_PATH:$PLANEPOWER_ROOT/components/carousel1
else
	export RTT_COMPONENT_PATH=$RTT_COMPONENT_PATH:$PLANEPOWER_ROOT/components/carousel2
	export RTT_COMPONENT_PATH=$RTT_COMPONENT_PATH:$PLANEPOWER_ROOT/components/carousel2/lineAngleSensor2
	export RTT_COMPONENT_PATH=$RTT_COMPONENT_PATH:$PLANEPOWER_ROOT/components/carousel2/armboneLisaSensors
	export RTT_COMPONENT_PATH=$RTT_COMPONENT_PATH:$PLANEPOWER_ROOT/components/carousel2/siemensSensors
	export RTT_COMPONENT_PATH=$RTT_COMPONENT_PATH:$PLANEPOWER_ROOT/components/carousel2/siemensActuators
	export RTT_COMPONENT_PATH=$RTT_COMPONENT_PATH:$PLANEPOWER_ROOT/components/carousel2/carouselSimulator
fi

export RTT_COMPONENT_PATH=$RTT_COMPONENT_PATH:$PLANEPOWER_ROOT/components/carousel2/gainLoader/resampler

DEFAULT_RAWESOME_ROOT=$PLANEPOWER_ROOT/extern/rawesome
if [ -d $DEFAULT_RAWESOME_ROOT ]; then
	export RAWESOME_ROOT=$DEFAULT_RAWESOME_ROOT
else
	echo WARNING!!! Could not find Gregs rawesome repo in the usual place.
	echo            Please manually set RAWESOME_ROOT environment if you
	echo			installed rawesome repo in a non-standard place
fi

export PYTHONPATH=$PYTHONPATH:$RAWESOME_ROOT
export PYTHONPATH=$PYTHONPATH:$PLANEPOWER_ROOT/rawesome_stuff

export PYTHONPATH=$PYTHONPATH:$PLANEPOWER_ROOT/shared
export PYTHONPATH=$PYTHONPATH:$PLANEPOWER_ROOT/bbones/shared
export PYTHONPATH=$PYTHONPATH:$PLANEPOWER_ROOT/components/carousel2/lineAngleSensor2

# TODO ACADO related (next two lines) stuff should be optional, same as for rawesome. 
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PLANEPOWER_ROOT/extern/acado/build
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PLANEPOWER_ROOT/extern/acado/build/experimental/mvukov/ocg2

# This makes sure the highwind version of deployer is found
# instead of the stock version.
#export PATH=$PLANEPOWER_ROOT/tools:$PATH
