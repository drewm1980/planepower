#!/usr/bin/env bash

sudo apt-get install haskell-compiler
sudo apt-get install ghc cabal-install

echo "Bootstrapping rawesome dependencies"
echo "Warning, due to the lameness of this script, you may need to add ~/.cabal/bin to your PATH before the script can succeed"
# echo "export PATH=\$PATH:~/.cabal/bin/" >> ~/.bashrc
cabal update
cabal install happy
cabal install haskell-src-exts
cabal install alex
cabal install hprotoc

