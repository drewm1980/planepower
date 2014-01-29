#******************************************************************************
#
# Makefile - Rules for building the board finder utility.
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
# This is part of revision 6852 of the Stellaris Firmware Development Package.
#
#******************************************************************************

#
# The name of this application.
#
APP:=finder

#
# The object files that comprise this application.
#
OBJS:=finder.o

#
# Include the generic rules.
#
include ./toolsdefs

#
# Modify some of the generic defines.
#
CFLAGS:=${CFLAGS} -g
LDFLAGS:=${LDFLAGS} -g
LIBS:=pthread

#
# Clean out additional build products.
#
clean::
	@rm -rf

#
# A dependency to ensure that fltk is built before the application.
#
finder.o: finder.c
