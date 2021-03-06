################################################################################
#
# CMake script for building the NMPC component
# Author: Milan Vukov, milan.vukov@esat.kuleuven.be
#
################################################################################

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
	# This guy has to be exactly here to override ROS default compilation rules
	SET( ROS_BUILD_TYPE Release )

	INCLUDE( $ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake )
	rosbuild_init()
	rosbuild_find_ros_package( rtt )
	rosbuild_find_ros_package( ocl )
	SET( RTT_HINTS HINTS ${rtt_PACKAGE_PATH}/install )
	SET( OCL_HINTS HINTS ${ocl_PACKAGE_PATH}/install )
ENDIF()


FIND_PACKAGE( Orocos-RTT REQUIRED HINTS ${rtt_PACKAGE_PATH}/install )

#
# ACADO package is required of course
#
FIND_PACKAGE( ACADO REQUIRED )

#
# Defines the orocos_* cmake macros. See that file for additional
# documentation.
#
INCLUDE( ${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake )

#
# A relative path to the MATLAB ACADO folder
#
SET( MATLAB_ACADO_DIR
	${PROJECT_SOURCE_DIR}/../../../matlab_acado_codegen_simulation
)

#
# Adding ACADO code generation specific include directories
#
INCLUDE_DIRECTORIES(
	${ACADO_INCLUDE_DIRS}

	${MATLAB_ACADO_DIR}

	${PROJECT_SOURCE_DIR}/src/code_export_nmpc
	${PROJECT_SOURCE_DIR}/src/code_export_nmpc/qpoases
	${PROJECT_SOURCE_DIR}/src/code_export_nmpc/qpoases/INCLUDE
	${PROJECT_SOURCE_DIR}/src/code_export_nmpc/qpoases/SRC

	# qpOASES embedded include folder
# 	${ACADO_QPOASES_EMBEDDED_INC_DIRS}
)

#
# Temporary patch for ACADO generated code
#
ADD_DEFINITIONS( -DACADO_CMAKE_BUILD )

#
# Important define for parameters, see MATLAB_ACADO_DIR/model_constants.hpp
#
ADD_DEFINITIONS( -DCODEGEN_FOR_OROCOS=1 )

#
# Stupid hack to wrap ACADO generated code into a namespace
#
ADD_DEFINITIONS( -D__ACADOIF__ )

#
# C compiler, C99 support
#
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99" )

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=core2" )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=core2" )

###
### Building c++ executables and libraries
###

#
# Executable for building the autogenerated code
#
ADD_EXECUTABLE( nmpc_codegen
	${MATLAB_ACADO_DIR}/NMPC.cpp
)
TARGET_LINK_LIBRARIES(
	nmpc_codegen
	${ACADO_SHARED_LIBRARIES}
)

#
# A list of autogenerated files
#
SET( NMPC_GENERATED_FILES
	${PROJECT_SOURCE_DIR}/src/code_export_nmpc/acado.h
	${PROJECT_SOURCE_DIR}/src/code_export_nmpc/condensing.c
	${PROJECT_SOURCE_DIR}/src/code_export_nmpc/gauss_newton_method.c
	${PROJECT_SOURCE_DIR}/src/code_export_nmpc/integrator.c
	${PROJECT_SOURCE_DIR}/src/code_export_nmpc/qpoases/solver.hpp
	${PROJECT_SOURCE_DIR}/src/code_export_nmpc/qpoases/solver.cpp
)

#
# Call a script run_mpc_codegen.sh to suck the arguments that are passed
# to the executable above, call it, and fix acado.h
#
ADD_CUSTOM_TARGET( nmpc_generate_code
	COMMAND
		sh ${PROJECT_SOURCE_DIR}/run_mpc_codegen.sh
	WORKING_DIRECTORY
		${PROJECT_SOURCE_DIR}/src
	DEPENDS
		nmpc_codegen
)

#
# Our OROCOS component
#
orocos_component(
	${PROJECT_NAME}

	# Component sources
	src/dynamicMPC.cpp	src/dynamicMPC.hpp
)
#
# Our component is dependent on target that generates the code
#
ADD_DEPENDENCIES(
	${PROJECT_NAME}
	
	nmpc_generate_code
)

orocos_generate_package()

#target_link_libraries(${PROJECT_NAME} another_library)
#rosbuild_add_boost_directories()
#rosbuild_link_boost(${PROJECT_NAME} thread)
#rosbuild_add_executable(example examples/example.cpp)
#target_link_libraries(example ${PROJECT_NAME})
