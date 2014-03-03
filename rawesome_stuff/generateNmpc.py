#!/usr/bin/env python

import sys
import os
import shutil

from rawe.models.arianne_conf import makeConf
from offline_mhe_test import NMPC

from rawekite.carouselSteadyState import getSteadyState

if __name__=='__main__':
	assert len(sys.argv) == 2, \
		'need to call generateNmpc.py with the properties directory'
	propsDir = sys.argv[1]

	nmpc = NMPC.makeNmpc(propertiesDir=propsDir)
	cgOptions= {'CXX':'clang++', 'CC':'clang',
				'CXXFLAGS':'-O3 -fPIC -finline-functions -march=native',
				'CFLAGS':'-O3 -fPIC -finline-functions -march=native',
				'hideSymbols':True}
	exportpath =  nmpc.exportCode(NMPC.mpcOpts, NMPC.mpcIntOpts, cgOptions, {})

	# Copy the library and the headers to output location
	for filename in ['acado_common.h', 'acado_qpoases_interface.hpp', 'ocp.o']:
		fullname = os.path.join(exportpath, filename)
		assert os.path.isfile(fullname), fullname+' is not a file'
		shutil.copy(fullname, filename)
	
	f = open('whereami.txt','w')
	f.write(exportpath+'\n')
	f.close()
	
	Ts = 0.04
	
	# Generate a data file with weights
	fw = open("mpc_configuration.h", "w")
	fw.write(
'''
#ifndef NMPC_CONFIGURATION
#define NMPC_CONFIGURATION

'''
			)

	fw.write("// This file was created from the file: " + os.path.realpath(__file__) + "\n")
	
	fw.write("#define mpc_sampling_time " + repr( Ts ) + "\n\n\n");
	
	xuNames = nmpc.yxNames + nmpc.yuNames
	fw.write("const double mpc_weights[ " + str( len( xuNames ) ) + " ] =\n{\n")
	for name in xuNames:
		fw.write("\t" + repr( NMPC.mpcWeights[ name ] ) + ", // " + name + "\n")
	fw.write("};\n\n")
	
	fw.write("// Differential variables\n")
	for k, name in enumerate( nmpc.dae.xNames() ):
		fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
	fw.write("\n\n")
	
	fw.write("// Control variables\n")
	for k, name in enumerate( nmpc.dae.uNames() ):
		fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
	fw.write("\n\n")
	
	#
	# Generate steady state for NMPC
	#
	
	# Cable length which we are going to supply to the MHE as a fake measurement
	measCableLength = 1.275
	
	# Speed for the steady state calculation
	steadyStateSpeed = -4.0

	# Reference parameters
	refP = {'r0': measCableLength,
			'ddelta0': steadyStateSpeed,
			}

	# Get the steady state
	steadyState, dSS = getSteadyState(nmpc.dae, makeConf(), refP['ddelta0'], refP['r0'])

	# Write the steady state to the file
	names = {"x": nmpc.dae.xNames(), "u": nmpc.dae.uNames(), "z": nmpc.dae.zNames()}
	for k, v in names.items():
		fw.write("const double ss_" + k + "[ " + str( len( v ) ) + " ] = {")
		fw.write(", ".join([repr( steadyState[ name ] ) + " /*" + name + "*/" for name in v]))
		fw.write("};\n\n")
	
	fw.write("#endif // NMPC_CONFIGURATION\n")
	fw.close()
	
	#
	# Generate protobuf specs for the NMPC
	#
	
	xNames = ""
	for k, name in enumerate( nmpc.dae.xNames() ):
		xNames = xNames + "idx_" + str( name ) + " = " + str( k ) + "; "
		
	zNames = ""
	for k, name in enumerate( nmpc.dae.zNames() ):
		zNames = zNames + "idx_" + str( name ) + " = " + str( k ) + "; "
		
	uNames = ""
	for k, name in enumerate( nmpc.dae.uNames() ):
		uNames = uNames + "idx_" + str( name ) + " = " + str( k ) + "; "
	
	proto = """
package DynamicMpcProto;

message DynamicMpcMsg
{
	enum Configuration
	{
		N = %(N)d;
	}

	enum xNames
	{
		%(xNames)s
	}
	
	enum zNames
	{
		%(zNames)s
	}
	
	enum uNames
	{
		%(uNames)s
	}

	message Horizon
	{
		repeated float h = 1;
	}

	repeated Horizon x = 1;
	repeated Horizon z = 2;
	repeated Horizon u = 3;
	
	repeated Horizon y	= 4;
	repeated float	 yN = 5;
	
	required int32 solver_status = 6;
	required float kkt_value = 7;
	required float obj_value = 8;
	
	required float exec_fdb	 = 9;
	required float exec_prep = 10;
	
	required double ts_trigger = 11;
	required double ts_elapsed = 12;
}
""" % {"N": NMPC.mpcHorizonN, "xNames": xNames, "zNames": zNames, "uNames": uNames}
	
	fw = open("DynamicMpcTelemetry.proto", "w")
	fw.write( proto )
	fw.close()
