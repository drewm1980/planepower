#!/usr/bin/env bash

cd rawesome && ./mkprotos.sh; cd ..
cd rawesome/plot-ho-matic && cabal clean && cabal configure && cabal build; cd ../..
cd rawesome/wtfviz && cabal clean && cabal configure && cabal build; cd ../..

