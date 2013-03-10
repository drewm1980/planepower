Instructions for building the flight software

1. Make sure you're using our recommended operating system:
$ cat /etc/issue
Ubuntu 12.04.1 LTS

2. Install ROS (paraphrased from http://www.ros.org/wiki/fuerte/Installation/Ubuntu)

echo "deb http://packages.ros.org/ros/ubuntu precise main" > /etc/apt/sources.list.d/ros-latest.list
sudo apt-get update
sudo apt-get install ros-fuerte-desktop-full 

echo "source /opt/ros/fuerte/setup.bash" >> ~/.bashrc
export ROS_PACKAGE_PATH=$ROS_PACKAGE_PATH:$HOME/planepower

3. Install OROCOS from source (paraphrased from http://www.ros.org/wiki/orocos_toolchain_ros)

mkdir $HOME/src/rosstacks
echo "export ROS_PACKAGE_PATH=\$ROS_PACKAGE_PATH:\$HOME/src/rosstacks" >> ~/.bashrc
export ROS_PACKAGE_PATH=$ROS_PACKAGE_PATH:$HOME/src/rosstacks
cd $HOME/src/rosstacks

sudo apt-get install libreadline-dev omniorb omniidl omniorb-nameserver libomniorb4-1 libomniorb4-dev libomnithread3-dev libomnithread3c2 gccxml antlr libantlr-dev libxslt1-dev liblua5.1-0-dev ruby1.8-dev libruby1.8 rubygems1.8 

if you have DLL hell with yaml-cpp and libassimp-dev:

http://answers.ros.org/question/33374/upgrade-to-fuerte-problem-with-yaml-cpp/?answer=47985#post-id-47985

git clone --recursive git://gitorious.org/orocos-toolchain/orocos_toolchain.git
git clone http://git.mech.kuleuven.be/robotics/rtt_ros_integration.git
git clone http://git.mech.kuleuven.be/robotics/rtt_ros_comm.git
git clone http://git.mech.kuleuven.be/robotics/rtt_common_msgs.git
git clone http://git.mech.kuleuven.be/robotics/rtt_geometry.git
roscd orocos_toolchain

Yet another thing to add to your environment:

source $HOME/src/rosstacks/orocos_toolchain/env.sh

rosmake orocos_toolchain rtt_ros_integration rtt_ros_comm rtt_common_msgs rtt_geometry

4. Build the flight software...

cd planepower
git submodule init
git submodule update

components/lowlevel/make_all.sh

