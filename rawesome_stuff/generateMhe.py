#!/usr/bin/env python

import sys
import os
import shutil

from rawe.ocp.Ocp import generateProto

from rawe.models.arianne_conf import makeConf

#
# We import the MHE that is tested on real measurements, not the one used in simulations
#
from offline_mhe_test import MHE
from offline_mhe_test import carouselModel

from rawekite.carouselSteadyState import getSteadyState

def generateTestFile( mhe ):
        
    #
    # Generate the test file
    #
    
    import numpy as np
    weights = np.array([])
    for name in mhe.yxNames + mhe.yuNames:
        weights = np.append( weights, np.ones(mhe[ name ].shape) * MHE.mheWeights[ name ] )
    weights_code = "const double weights[] = { " + ", ".join([repr( n ) for n in weights]) + "};\n\n"     
    
    test = """\
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>

#include "acado_common.h"
#include "acado_auxiliary_functions.h"
#include "mhe_configuration.h"

#define NUM_STEPS 100

%(weights_code)s

using namespace std;

extern void condensePrep();
extern void condenseFdb(  );
extern int solve( );
extern void expand(  );

extern void modelSimulation();
extern void evaluateObjective();

extern void lsqExtern(const double* x0, double* r0);
extern void lsqEndTermExtern(const double* x0, double* r0);

int main( void )
{
    memset(&acadoWorkspace, 0, sizeof( acadoWorkspace ));
    memset(&acadoVariables, 0, sizeof( acadoVariables ));
    
    for (unsigned blk = 0; blk < ACADO_N + 1; ++blk)
        for (unsigned el = 0; el < ACADO_NX; ++el)
            acadoVariables.x[blk * ACADO_NX + el] = ss_x[ el ];
            
    double ddelta_ss = ss_x[ idx_ddelta ];
    double angle = atan2(acadoVariables.x[ idx_sin_delta ], acadoVariables.x[ idx_cos_delta ]);

    for (unsigned blk = 0; blk < ACADO_N + 1; ++blk)
    {
        acadoVariables.x[blk * ACADO_NX + idx_cos_delta] = cos( angle );
        acadoVariables.x[blk * ACADO_NX + idx_sin_delta] = sin( angle );
        angle += mhe_sampling_time * ddelta_ss;
    }
        
    for (unsigned blk = 0; blk < ACADO_N; ++blk)
        for (unsigned el = 0; el < ACADO_NU; ++el)
            acadoVariables.u[blk * ACADO_NU + el] = ss_u[ el ];
            
    for (unsigned blk = 0; blk < ACADO_N; ++blk)
        for (unsigned el = 0; el < ACADO_NXA; ++el)
            acadoVariables.z[blk * ACADO_NXA + el] = ss_z[ el ];
        
    double lsqIn[ACADO_NX + ACADO_NU], lsqOut[ACADO_NY * (1 + ACADO_NX + ACADO_NU)];
    
    for (unsigned blk = 0; blk < ACADO_N; ++blk)
    {
        for (unsigned el = 0; el < ACADO_NX; ++el)
            lsqIn[ el ] = acadoVariables.x[blk * ACADO_NX + el];
        for (unsigned el = 0; el < ACADO_NU; ++el)
            lsqIn[ACADO_NX + el] = acadoVariables.u[blk * ACADO_NU + el];
        lsqExtern(lsqIn, lsqOut);
        
        for (unsigned el = 0; el < ACADO_NY; ++el)
            acadoVariables.y[blk * ACADO_NY + el] = lsqOut[ el ];
    }
    
    for (unsigned el = 0; el < ACADO_NX; ++el)
        lsqIn[ el ] = acadoVariables.x[ACADO_N * ACADO_NX + el];
    lsqEndTermExtern(lsqIn, lsqOut);
    
    for (unsigned el = 0; el < ACADO_NYN; ++el)
        acadoVariables.yN[ el ] = lsqOut[ el ];
        
    for (unsigned blk = 0; blk < ACADO_N; ++blk)
        for (unsigned el = 0; el < ACADO_NY; ++el)
            acadoVariables.W[blk * ACADO_NY * ACADO_NY + el * ACADO_NY + el] = weights[ el ];
            
    for (unsigned el = 0; el < ACADO_NYN; ++el)
        acadoVariables.WN[el * ACADO_NYN + el] = weights[ el ];
        
    double sumT1 = 0, sumT2 = 0, sumT3 = 0, sumT4 = 0, sumT5 = 0;
    timer t; 
    
    for (unsigned it = 0; it < NUM_STEPS; ++it)
    {
        tic( &t );
        modelSimulation();
        evaluateObjective();
        sumT1 += toc( &t );

        tic( &t );
        condensePrep(  );
        sumT2 += toc( &t );

        tic( &t );
        condenseFdb(  );
        sumT3 += toc( &t );

        tic( &t );
        int status = solve( );
        sumT4 += toc( &t );

        tic( &t );
        expand(  );
        sumT5 += toc( &t );
        
        if ( status )
        {
            cout << "Iteration:" << it << ", QP problem! QP status: " << status << endl;   
            exit( 1 );
        }
        
        for (unsigned el = 0; el < (ACADO_N + 1) * ACADO_NX; ++el)
            if (acadoVariables.x[ el ] != acadoVariables.x[ el ])
            {
                cout << "Iteration:" << it << ", NaN problems with diff. variables" << endl;
                exit( 1 );
            }
            
        shiftStates(2, 0, 0);
        shiftControls( 0 );
    }
    
    double prepSum = sumT1 + sumT2;
    double fdbSum = sumT3 + sumT4 + sumT5;
    
    cout << "Average feedback time:    " << scientific << fdbSum / NUM_STEPS * 1e6 << "usec" << endl;
    cout << "Average preparation Time: " << scientific << prepSum / NUM_STEPS * 1e6 << "usec" << endl;
    cout << "Total average time:       " << scientific << (fdbSum + prepSum) / NUM_STEPS * 1e6 << "usec" << endl;
    cout << endl << endl;
    
    cout    << "Detailed average runtimes: " << endl << scientific
            << "\t Integration and obj eval: " << sumT1 / NUM_STEPS * 1e6 << "usec" << endl
            << "\t Condensing, preparation:  " << sumT2 / NUM_STEPS * 1e6 << "usec" << endl
            << "\t Condensing, feedback:     " << sumT3 / NUM_STEPS * 1e6 << "usec" << endl
            << "\t QP solver time:           " << sumT4 / NUM_STEPS * 1e6 << "usec" << endl
            << "\t Condensing, expansion:    " << sumT5 / NUM_STEPS * 1e6 << "usec" << endl;

    return 0;
}

""" % {"weights_code": weights_code}

    fw = open("mhe_speed_test.cpp", "w")
    fw.write( test )
    fw.close()
    
    makefile = """\
CXX       = %(CXX)s
CXXFLAGS  = -Wall -Wpedantic -Wextra -O3 -fPIC -finline-functions -I.
LDFLAGS   = -lm -lrt

SRC = mhe_speed_test.cpp
OBJ = $(SRC:%%.cpp=%%.o)

.PHONY: clean all mhe_speed_test
all : $(OBJ) mhe_speed_test

%%.o : %%.cpp
\t@echo CXX $@: $(CXX) $(CXXFLAGS) -c $< -o $@
\t@$(CXX) $(CXXFLAGS) -c $< -o $@

HEADERS = acado_common.h acado_qpoases_interface.hpp acado_auxiliary_functions.h

mhe_speed_test : $(HEADERS) ocp.a mhe_speed_test.o
\t@echo CXX mhe_speed_test.o ocp.a -o $@
\t@$(CXX) mhe_speed_test.o ocp.a -o $@

clean :
\trm -f mhe_speed_test.o mhe_speed_test

""" % {"CXX": "clang++"}
    
    fw = open("Makefile", "w")
    fw.write( makefile )
    fw.close()

if __name__=='__main__':
    assert len(sys.argv) == 2 or len(sys.argv) == 3, \
        'need to call generateMhe.py with the properties directory'
    propsDir = sys.argv[1]

    mhe = MHE.makeMhe(MHE.samplingTime, propertiesDir = propsDir)
    
    #
    # Generate protobuf specs for the MHE
    #
    fw = open("DynamicMheTelemetry.proto", "w")
    fw.write( generateProto(mhe, "DynamicMhe") )
    fw.close()
    
    if len(sys.argv) == 3 and sys.argv[ 2 ] == "proto_only":
        print "MHE protobuf is generated and I am out..."
        sys.exit( 0 )

    # Options for code compilation
    cgOptions = {
        'CXX': 'clang++', 'CC': 'clang',
        'CXXFLAGS': '-fPIC -O3 -march=native -mtune=native -g',
        'CFLAGS': '-fPIC -O3 -march=native -mtune=native -g',
        # For OROCOS compilation, this option is mandatory
        'hideSymbols': True
    }
    # Now export the code
    exportpath = mhe.exportCode(MHE.mheOpts, MHE.mheIntOpts, cgOptions, {})

    # Copy the library and the headers to output location
    for filename in ['acado_common.h', 'acado_qpoases_interface.hpp', 'acado_auxiliary_functions.h', 'ocp.o', 'ocp.a']:
        fullname = os.path.join(exportpath, filename)
        assert os.path.isfile(fullname), fullname + ' is not a file'
        shutil.copy(fullname, filename)
    
    # Generate info file 
    f = open('whereami.txt','w')
    f.write(exportpath+'\n')
    f.close()
    
    # Generate MHE configuration file

    Ts = MHE.samplingTime
    nDelay = 2
    numMarkers = 12
    
    fw = open("mhe_configuration.h", "w")
    fw.write(
'''
#ifndef MHE_CONFIGURATION
#define MHE_CONFIGURATION

'''
            )

    fw.write("// This file was created from the file: " + os.path.realpath(__file__) + "\n\n")

    fw.write("#define mhe_sampling_time " + repr( Ts ) + "\n");
    fw.write("#define mhe_num_markers " + repr( numMarkers ) + "\n");
    fw.write("\n\n")

    # Write weights
    for k, v in MHE.mheWeights.items():
        fw.write("#define weight_" + str( k ) + " " + repr( v ) + "\n")
    fw.write("\n\n")
    
    # Output offsets for measurements
    fw.write("// Measurement offsets\n")
    yOffset = 0
    for name in mhe.yxNames:
        fw.write("#define offset_" + str( name ) + " " + str( yOffset ) + "\n")
        yOffset += mhe[ name ].shape[ 0 ]
    for name in mhe.yuNames:
        fw.write("#define offset_" + str( name ) + " " + str( yOffset ) + "\n")
        yOffset += mhe[ name ].shape[ 0 ]
    fw.write("\n\n")

    #
    # Write indices
    #
    fw.write("// Differential variables\n")
    for k, name in enumerate( mhe.dae.xNames() ):
        fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
    
    fw.write("\n\n")
    fw.write("// Control variables\n")
    for k, name in enumerate( mhe.dae.uNames() ):
        fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
    fw.write("\n\n")
    
    #
    # Generate steady state for MHE
    #
    
    # Get the plane configuration parameters
    conf = makeConf()
    conf[ 'stabilize_invariants' ] = False
    conf[ 'useVirtualTorques' ]    = True
    conf[ 'useVirtualForces' ]     = False
    
    # Cable length for steady state calculation
    steadyStateCableLength = 1.7
    # Speed for the steady state calculation
    steadyStateSpeed = -4.0

    # Reference parameters
    refP = {'r0': steadyStateCableLength,
            'ddelta0': steadyStateSpeed,
            }

    # Get the steady state
    steadyState, dSS = getSteadyState(mhe.dae, conf, refP['ddelta0'], refP['r0'], verbose = False)
    
    names = {"x": mhe.dae.xNames(), "u": mhe.dae.uNames(), "z": mhe.dae.zNames()}
    ss_code = ""
    for k, v in names.items():
        ss_code += "const double ss_" + k + "[ " + str( len( v ) ) + " ] = {"
        ss_code += ", ".join([repr( steadyState[ name ] ) + " /*" + name + "*/" for name in v])
        ss_code += "};\n\n"
        
    fw.write( ss_code )
    
    fw.write("#endif // MHE_CONFIGURATION\n")
    fw.close()
    
    #
    # Generate test file for speed tests
    #
    generateTestFile( mhe )
