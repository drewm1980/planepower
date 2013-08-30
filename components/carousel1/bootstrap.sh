#!/usr/bin/env bash

(
cd soem
for x in some_*
do
	(
	cd $x
	cmake .
	make
	)
done
)
