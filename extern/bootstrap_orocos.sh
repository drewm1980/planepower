#! /usr/bin/env bash
echo "Bootstrapping OROCOS..."

sudo echo "deb http://packages.ros.org/ros/ubuntu precise main" > /etc/apt/sources.list.d/ro-latest.list
sudo apt-get update
sudo apt-get install ros-hydro-orocos-toolchain
source /opt/ros/hydro/setup.bash

echo "Giving special rights to OROCOS deployer..."
sh setcap.sh

echo "Bootstrapping rttlua tab completion..."
if [ ! -d rttlua_completion ]; then
	git clone git://gitorious.org/orocos-toolchain/rttlua_completion.git
else
	(
	cd rttlua_completion
	git pull origin master
	)
fi
(
cd rttlua_completion
make
)

echo "Bootstrapping rFSM..."
if [ ! -d rFSM ]; then
	git clone git@github.com:kmarkus/rFSM
else
	(
	cd rFSM
	git pull origin master
	)
fi
(
cd rFSM
make
)

#echo "Bootstrapping dot service..."
#if [ ! -d rtt_dot_service ]; then
	#git clone git@gitorious.org:rtt_dot_service/rtt_dot_service.git
#else
	#(
	#cd rtt_dot_service
	#git pull origin master
	#)
#fi
#(
#cd rtt_dot_service
#make
#)
