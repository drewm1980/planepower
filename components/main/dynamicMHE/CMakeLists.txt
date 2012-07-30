CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

#
# CMake module(s) path
#
SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR} )

#
# Do setup in case of ros package, If ROS_ROOT is set, it is
# recommended to use RTT/OCL through the ros packages.
#
#
# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#
SET( ROS_ROOT $ENV{ROS_ROOT} )
IF( ROS_ROOT )
	SET( ROS_BUILD_TYPE Release )

	INCLUDE( $ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake )
	rosbuild_init()
	rosbuild_find_ros_package( rtt )
	rosbuild_find_ros_package( ocl )
	SET( RTT_HINTS HINTS ${rtt_PACKAGE_PATH}/install )
	SET( OCL_HINTS HINTS ${ocl_PACKAGE_PATH}/install )
ENDIF()

FIND_PACKAGE( Orocos-RTT REQUIRED HINTS ${rtt_PACKAGE_PATH}/install )
FIND_PACKAGE( ACADO REQUIRED )

#
# Defines the orocos_* cmake macros. See that file for additional
# documentation.
#
INCLUDE( ${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake )

#
# Adding ACADO code generation specific include directories
#
INCLUDE_DIRECTORIES(
	${PROJECT_SOURCE_DIR}/src/MHE_export/kite_export
	${PROJECT_SOURCE_DIR}/src/MHE_export/kite_export/qpoases
	${ACADO_QPOASES_EMBEDDED_INC_DIRS}
)

#
# Temporary patch for ACADO generated code
#
ADD_DEFINITIONS( -DACADO_CMAKE_BUILD )

#
# C compiler, C99 support
#
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -march=core2" )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=core2" )

#
# Common commands for building c++ executables and libraries
#

# orocos_use_package( ocl-logging )

orocos_component(
	${PROJECT_NAME}

	# Component sources
	src/dynamicMHE.cpp	src/dynamicMHE.hpp
	
	# Generated sources
	./src/MHE_export/kite_export/acado.h
	./src/MHE_export/kite_export/condensing.c
	./src/MHE_export/kite_export/gauss_newton_method.c
	./src/MHE_export/kite_export/integrator.c
	./src/MHE_export/kite_export/qpoases/solver.hpp
	./src/MHE_export/kite_export/qpoases/solver.cpp

	# qpOASES embedded sources
	${ACADO_QPOASES_EMBEDDED_SOURCES}
)

orocos_generate_package()

#target_link_libraries(${PROJECT_NAME} another_library)
#rosbuild_add_boost_directories()
#rosbuild_link_boost(${PROJECT_NAME} thread)
#rosbuild_add_executable(example examples/example.cpp)
#target_link_libraries(example ${PROJECT_NAME})
