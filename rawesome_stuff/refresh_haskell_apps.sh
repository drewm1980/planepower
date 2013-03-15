#!/usr/bin/env bash

cd rawesome && ./mkprotos.sh; cd ..
cd rawesome/plot-ho-matic && cabal build; cd ../..
cd rawesome/wtfviz && cabal build; cd ../..

