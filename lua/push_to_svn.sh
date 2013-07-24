#!/usr/bin/env bash
dest=~/HIGHWINDPROMO/data
timestamp=`date +%Y%m%d_%H%m%S`

mkdir $dest/$timestamp
cp *.nc *.log *.ops $dest/$timestamp

(
cd $dest
svn add $timestamp
svn commit -m "Sending new data to milan"
)
