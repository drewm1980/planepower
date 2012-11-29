#!/usr/bin/env bash

# List of orocos components
c=`ls|grep -v *.cpp|grep -v *.h|grep -v .sh | grep -v Tupfile`

c = `find lowlevel -name manifest.xml | xargs -n1 dirname | xargs -n1 basename | grep -v soem

for x in $c
do
	./upgrade_component_to_tup.sh $x
done
