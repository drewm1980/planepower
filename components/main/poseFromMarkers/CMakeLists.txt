cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#set(ROS_BUILD_TYPE RelWithDebInfo)

rosbuild_init()

#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#uncomment if you have defined messages
#rosbuild_genmsg()
#uncomment if you have defined services
#rosbuild_gensrv()

#common commands for building c++ executables and libraries
#rosbuild_add_library(${PROJECT_NAME} src/example.cpp)
#target_link_libraries(${PROJECT_NAME} another_library)
#rosbuild_add_boost_directories()
#rosbuild_link_boost(${PROJECT_NAME} thread)
#rosbuild_add_executable(example examples/example.cpp)
#target_link_libraries(example ${PROJECT_NAME})
#rosbuild_add_library(poseFromMarkers src/poseFromMarkers.cpp)
#ADD_DEFINITIONS(-DOCL_DLL_EXPORT) 
 
rosbuild_find_ros_package(rtt )
find_package(Orocos-RTT HINTS ${rtt_PACKAGE_PATH}/install )
 
# Defines the orocos_* cmake macros. See that file for additional
# documentation.
include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)

ADD_CUSTOM_COMMAND(
	COMMAND
		python ${CMAKE_CURRENT_SOURCE_DIR}/../cameraCalibration/pose_from_markers.py
	WORKING_DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}/../cameraCalibration/
	OUTPUT
		${CMAKE_CURRENT_SOURCE_DIR}/../cameraCalibration/pose_from_markers.c
	COMMENT
		"Creating a file: pose_from_markers.c"
)

ADD_CUSTOM_COMMAND(
	COMMAND
		sh ${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/rename_casadi_functions
	WORKING_DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}/src
	OUTPUT
		${CMAKE_CURRENT_SOURCE_DIR}/src/casadi_functions.cpp
	DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/../cameraCalibration/pose_from_markers.c
	COMMENT
		"Creating a file: casadi_functions.cpp"
)

#common commands for building c++ executables and libraries
orocos_component(
	# Component name
	${PROJECT_NAME}

	# Source files
	src/poseFromMarkers.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/casadi_functions.cpp
)

# ADD_DEPENDENCIES( gen_cpp_file gen_c_file )

# ADD_DEPENDENCIES( ${PROJECT_NAME} gen_cpp_file )

rosbuild_add_library(
	# Library names	
	poseFromMarkers

	# Library sources 
	src/poseFromMarkers.cpp
)
target_link_libraries(poseFromMarkers ${OpenCV_LIBS})

#target_link_libraries(${PROJECT_NAME} another_library)
#rosbuild_add_boost_directories()
#rosbuild_link_boost(${PROJECT_NAME} thread)
#rosbuild_add_executable(clapack_test src/clapack_test.cpp)
#target_link_libraries(example ${PROJECT_NAME})
