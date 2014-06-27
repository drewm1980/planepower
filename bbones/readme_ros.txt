-Andrew

These are my notes for tinkering with using ROS on the beaglebone instead of the code written by our our undergrads for serialization, deserialization, etc...

Follow instructions for installing roscore on angstrom beaglebone
http://wiki.ros.org/BeagleBone

ssh bbusb
opkg install git
git clone git://github.com/vmayoral/beagle-ros.git
cd beagle-ros
sh minimal-ros-install-angstrom.sh
roscore
