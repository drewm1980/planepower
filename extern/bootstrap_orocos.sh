#! /usr/bin/env bash
echo -e "Bootstrapping OROCOS...\n\n"
echo -e "If build fails you will need to configure Ubuntu repos as follows: "
echo -e "1) Setup your sources.list:\n\t sudo sh -c 'echo \"""deb http://packages.ros.org/ros/ubuntu precise main\""" > /etc/apt/sources.list.d/ros-latest.list'"
echo -e "2) Set up your keys:\n\t wget http://packages.ros.org/ros.key -O - | sudo apt-key add -"

sudo sh -c "echo 'deb http://packages.ros.org/ros/ubuntu precise main' > /etc/apt/sources.list.d/ros-latest.list"
sudo apt-get update
sudo apt-get install ros-hydro-orocos-toolchain ros-hydro-ocl ros-hydro-rtt ros-hydro-rtt-typelib
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

echo "Building the Jitter service from Intermodalics"
(
mkdir -p jitter/build
cd jitter/build
cmake -DCMAKE_INSTALL_PREFIX:STRING=`pwd`/../install ..
make install
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
