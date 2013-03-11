cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

set(CMAKE_VERBOSE_MAKEFILE true)

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
#rosbuild_add_library(LEDTracker src/LEDTracker.cpp)
#ADD_DEFINITIONS(-DOCL_DLL_EXPORT)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -ggdb")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O0 -ggdb")
IF( NOT CMAKE_BUILD_TYPE )
	#SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING FORCE)
	SET(CMAKE_BUILD_TYPE Debug CACHE STRING FORCE)
ENDIF( NOT CMAKE_BUILD_TYPE )
 
rosbuild_find_ros_package(rtt )
find_package(Orocos-RTT HINTS ${rtt_PACKAGE_PATH}/install )
 
# Defines the orocos_* cmake macros. See that file for additional
# documentation.
include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)

link_directories(/usr/lib/x86_64-linux-gnu/)

rosbuild_add_library(Tracer src/Tracer.cpp)
target_link_libraries(Tracer ${OpenCV_LIBS})
add_library(StereoPair src/StereoPair.cpp)
target_link_libraries(StereoPair dc1394)
add_library(BlobExtractor src/BlobExtractor.cpp src/MedianFinder.cpp)
add_library(${PROJECT_NAME} src/LEDTracker.cpp)

#include_directories(src/cvblobs)
orocos_component(${PROJECT_NAME} src/LEDTracker.cpp src/BlobExtractor.cpp src/MedianFinder.cpp src/StereoPair.cpp )
target_link_libraries(${PROJECT_NAME} dc1394)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
#target_link_libraries(${PROJECT_NAME} StereoPair)
#target_link_libraries(${PROJECT_NAME} BlobExtractor)

# Example for "cvblob" library from http://code.google.com/p/cvblob/
#rosbuild_add_executable(red_object_tracking src/cvblob_samples/red_object_tracking.cpp)
#target_link_libraries(red_object_tracking ${OpenCV_LIBS} )
#target_link_libraries(red_object_tracking cvblob)

# Example for "cvblobslib" library from http://opencv.willowgarage.com/wiki/cvBlobsLib
#rosbuild_add_library(blob src/cvblobs/BlobContour.cpp
				#src/cvblobs/blob.cpp
				#src/cvblobs/BlobOperators.cpp
				#src/cvblobs/BlobProperties.cpp
				#src/cvblobs/BlobResult.cpp
				#src/cvblobs/ComponentLabeling.cpp)
#rosbuild_add_executable(libblob_example src/cvblobs/example.cpp)
#target_link_libraries(libblob_example blob)
#target_link_libraries(libblob_example ${OpenCV_LIBS} )
#include_directories(src/cvblobs)

# Check bayer pattern generation
#add_executable(test_bayer src/test_bayer.cpp)
#target_link_libraries(test_bayer ${OpenCV_LIBS} )

# Benchmark of the blob detector libraries
#add_executable(test_extractors src/test_extractors.cpp)
#target_link_libraries(test_extractors ${OpenCV_LIBS})
#target_link_libraries(test_extractors blob)

add_executable(test_stereo_pair src/test_stereo_pair.cpp)
#target_link_libraries(test_stereo_pair dc1394)
target_link_libraries(test_stereo_pair StereoPair)
target_link_libraries(test_stereo_pair Tracer)

#add_executable(test_stereo_pair_and_blob_extractor src/test_stereo_pair_and_blob_extractor.cpp)
#target_link_libraries(test_stereo_pair_and_blob_extractor dc1394)
#target_link_libraries(test_stereo_pair_and_blob_extractor StereoPair)
#target_link_libraries(test_stereo_pair_and_blob_extractor Tracer)
#target_link_libraries(test_stereo_pair_and_blob_extractor BlobExtractor)

#add_executable(flattened src/flattened.cpp)
#target_link_libraries(flattened dc1394)
#target_link_libraries(flattened blob)

