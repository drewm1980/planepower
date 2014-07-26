#!/usr/bin/env bash
proto_defs=`find ../components/carousel2 -iname '*.proto'`

for file in proto_defs; do
    hprotoc --haskell_out=src -p Protos proto_defs/$(file)
done
