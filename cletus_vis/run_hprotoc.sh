#!/usr/bin/env bash

# remove old local protos
rm proto_defs/*.proto

# copy to proto_defs
proto_defs=`find ../components/carousel2 -iname '*.proto'`
for file in $proto_defs; do
    cp $file proto_defs/
done

# call hprotoc
proto_defs=`find proto_defs -iname '*.proto'`
for file in $proto_defs; do
    echo Running protoc on $file ...
    com="hprotoc --haskell_out=src -p Protos $file"
    echo $com
    $com
done
