#!/usr/bin/env bash

echo "Bootstrapping rawesome dependencies"
#sudo apt-get install haskell-compiler
# echo "export PATH=$PATH:~/.cabal/bin/" >> ~/.bashrc
cabal update
cabal install happy
cabal install haskell-src-exts
cabal install alex
cabal install hprotoc

(
cd rawesome
./mkprotos.sh
)
