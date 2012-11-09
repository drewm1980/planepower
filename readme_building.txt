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

3. Build the flight software...


