#! /usr/bin/env bash
echo "Bootstrapping OROCOS..."

sudo apt-get install ruby-nokogiri wget

mkdir -p orocos
(
cd orocos
wget http://rock-robotics.org/stable/autoproj_bootstrap
ruby autoproj_bootstrap $@ git git://gitorious.org/orocos-toolchain/build.git branch=toolchain-2.6 push_to=git@gitorious.org:orocos-toolchain/build.git
. $PWD/env.sh
autoproj update
autoproj fast-build
)

echo "Bootstrapping rttlua tab completion..."
git clone git://gitorious.org/orocos-toolchain/rttlua_completion.git
(
cd rttlua_completion
make
)

