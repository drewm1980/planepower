#!/bin/bash
NAME=$1
NAME=`basename $NAME /`
UNAME=`perl -e "print ucfirst('$NAME')"`
UUNAME=`perl -e "print uc('$NAME')"`

if [ ! -d "$NAME" ]; then
	echo "Calling roscreate-pkg"
	roscreate-pkg $NAME
fi

echo "Creating OCL::$UNAME out of $NAME..."
echo "  First, we create a Package..."
yes | ./createNewPkg.sh $NAME > /dev/null
echo "  Next we add to the CMakeLists.txt..."
cd $NAME
if [[ -z `grep src/$NAME.cpp CMakeLists.txt` ]]; then
  echo "#rosbuild_add_library($NAME src/$NAME.cpp)" >> CMakeLists.txt
  echo "#ADD_DEFINITIONS(-DOCL_DLL_EXPORT)" >> CMakeLists.txt
  echo " " >> CMakeLists.txt
  echo " " >> CMakeLists.txt
  echo "rosbuild_find_ros_package(rtt )" >> CMakeLists.txt
  echo "find_package(Orocos-RTT HINTS \${rtt_PACKAGE_PATH}/install )" >> CMakeLists.txt
  echo " " >> CMakeLists.txt
  echo "# Defines the orocos_* cmake macros. See that file for additional" >> CMakeLists.txt
  echo "# documentation." >> CMakeLists.txt
  echo "include(\${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)" >> CMakeLists.txt
  echo " " >> CMakeLists.txt
  echo "#common commands for building c++ executables and libraries" >> CMakeLists.txt
  echo "orocos_component(\${PROJECT_NAME} src/$NAME.cpp)" >> CMakeLists.txt
  echo "rosbuild_add_library($NAME src/$NAME.cpp)" >> CMakeLists.txt
  echo "#target_link_libraries(\${PROJECT_NAME} another_library)" >> CMakeLists.txt
  echo "#rosbuild_add_boost_directories()" >> CMakeLists.txt
  echo "#rosbuild_link_boost(\${PROJECT_NAME} thread)" >> CMakeLists.txt
  echo "#rosbuild_add_executable(example examples/example.cpp)" >> CMakeLists.txt
  echo "#target_link_libraries(example \${PROJECT_NAME})" >> CMakeLists.txt
fi

echo "  Made $NAME.cpp and $NAME.hpp and put in src directory..."
mkdir -p src
cp ../template/src/template.cpp src/$NAME.cpp
cp ../template/src/template.hpp src/$NAME.hpp

sed -i "s/SimulateBall/$UNAME/g" src/$NAME.cpp
sed -i "s/SimulateBall/$UNAME/g" src/$NAME.hpp
sed -i "s/simulateBall/$NAME/g" src/$NAME.cpp
sed -i "s/simulateBall/$NAME/g" src/$NAME.hpp
sed -i "s/SIMULATEBALL/$UUNAME/g" src/$NAME.cpp
sed -i "s/SIMULATEBALL/$UUNAME/g" src/$NAME.hpp

rosrun rtt_rosnode rtt-upgrade-2.5

echo "Done"
