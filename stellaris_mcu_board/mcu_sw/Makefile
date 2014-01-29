#******************************************************************************
#
# Makefile - Rules for building the Ethernet with lwIP.
#
# Copyright (c) 2009-2011 Texas Instruments Incorporated.  All rights reserved.
# Software License Agreement
# 
# Texas Instruments (TI) is supplying this software for use solely and
# exclusively on TI's microcontroller products. The software is owned by
# TI and/or its suppliers, and is protected under applicable copyright
# laws. You may not combine this software with "viral" open-source
# software in order to form a larger program.
# 
# THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
# NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
# NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
# CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
# DAMAGES, FOR ANY REASON WHATSOEVER.
# 
# This is part of revision 6852 of the EK-LM3S9B92 Firmware Package.
#
#******************************************************************************

#
# Defines the (binary & executable) image name
#
IMAGE=planepower

#
# Defines the part type that this project uses.
#
PART=LM3S9B92

#
# The base directory for StellarisWare.
#
ROOT=.

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH+=${ROOT}/utils

#
# Where to find header files that do not live in the source directory.
#
IPATH=.
IPATH+=${ROOT}
IPATH+=${ROOT}/third_party/lwip-1.3.2/ports/stellaris/include
IPATH+=${ROOT}/third_party/lwip-1.3.2/src/include
IPATH+=${ROOT}/third_party/lwip-1.3.2/src/include/ipv4
IPATH+=${ROOT}/third_party

#
# The default rule, which causes the Ethernet with lwIP to be built.
#
all: ${COMPILER}
all: ${COMPILER}/planepower.axf
all: ${COMPILER}/planepower.lss ${COMPILER}/planepower.sym

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir ${COMPILER}

#
# Rules for building the Ethernet with lwIP.
#
${COMPILER}/planepower.axf: ${COMPILER}/planepower.o
${COMPILER}/planepower.axf: ${COMPILER}/bsp.o

${COMPILER}/planepower.axf: ${COMPILER}/locator.o
${COMPILER}/planepower.axf: ${COMPILER}/lwiplib.o

${COMPILER}/planepower.axf: ${COMPILER}/startup_${COMPILER}.o

${COMPILER}/planepower.axf: ${COMPILER}/uartstdio.o
${COMPILER}/planepower.axf: ${COMPILER}/ustdlib.o

${COMPILER}/planepower.axf: ${COMPILER}/imu.o

${COMPILER}/planepower.axf: ${COMPILER}/rcservo.o

${COMPILER}/planepower.axf: ${COMPILER}/ui_ethernet.o
${COMPILER}/planepower.axf: ${COMPILER}/ui_common.o

${COMPILER}/planepower.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/planepower.axf: planepower.ld
SCATTERgcc_planepower=planepower.ld
ENTRY_planepower=ResetISR
CFLAGSgcc=-DTARGET_IS_TEMPEST_RB1

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
