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

. $PLANEPOWER_ROOT/extern/orocos/env.sh
