#!/usr/bin/env bash

cd rawesome && ./mkprotos.sh; cd ..
cd rawesome/plotter && cabal build; cd ../..
cd rawesome/wtfviz && cabal build; cd ../..

