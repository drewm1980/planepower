#!/usr/bin/env python

import sys, os, shutil

from collections import namedtuple

import casadi as C

import rawe
from rawe.models.arianne_conf import makeConf
from offline_mhe_test import MHE
from rawekite.carouselSteadyState import getSteadyState

if __name__=='__main__':
    assert len(sys.argv) == 2, \
        'need to call generateSimulator.py with the properties directory'
    propsDir = sys.argv[1]
    
    options = rawe.RtIntegratorOptions()
    options['INTEGRATOR_TYPE'] = 'INT_IRK_GL2'
    options['NUM_INTEGRATOR_STEPS'] = 2
    options['DYNAMIC_SENSITIVITY'] = 'NO_SENSITIVITY'
    
    Ts = 1e-3
    
    # We make an MHE because there are defined all measurements (output functions)
    # that we would like to simulate
    mhe = MHE.makeMhe(Ts, propertiesDir = propsDir)
    
    sim = rawe.RtIntegrator(mhe.dae, ts = Ts, options = options,
                            measurements = C.veccat([mhe.yx, mhe.yu]),
                            cgOptions = {'CXX': 'clang++', 'CC': 'clang',
                                         'CXXFLAGS': '-fPIC -O3 -march=native -mtune=native -finline-functions -I.',
                                         'CFLAGS': '-fPIC -O3 -march=native -mtune=native -finline-functions -I.',
                                         # For OROCOS compilation, this option is mandatory
                                         'hideSymbols': True
                                         }
                            )
    exportpath = sim.getExportPath()
    
    # Copy the library and the headers to output location
    for filename in ['acado_common.h', 'integrator.o']:
        fullname = os.path.join(exportpath, filename)
        assert os.path.isfile(fullname), fullname + ' is not a file'
        shutil.copy(fullname, filename)
    
    f = open('whereami.txt','w')
    f.write(exportpath+'\n')
    f.close()
    
    # Generate a data file with weights
    fw = open("sim_configuration.h", "w")
    fw.write(
'''
#ifndef SIMULATOR_CONFIGURATION
#define SIMULATOR_CONFIGURATION

'''
            )

    fw.write("// This file was created from the file: " + os.path.realpath(__file__) + "\n")
    
    fw.write("#define sim_sampling_time " + repr( Ts ) + "\n\n\n");
    
    fw.write("// Differential variables\n")
    for k, name in enumerate( mhe.dae.xNames() ):
        fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
    fw.write("\n\n")
    
    fw.write("// Control variables\n")
    for k, name in enumerate( mhe.dae.uNames() ):
        fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
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
    # Output configuration
    #
    
    # Name, sampling time and delay time
    TimeConf = namedtuple('Time', ['name', 'ts', 'td'])
    
    ### LED Tracker
    LedConf = TimeConf("cam", 1 / 12.5, 1 / 12.5)
    
    ### MCU handler
    McuConf = TimeConf("mcu", 1 / 500.0, 1e-3)
    
    ### Encoder
    EncConf = TimeConf("enc", 1 / 1000.0, 0)
    
    ### TODO Line Angle Sensor
    
    ### Winch
    WinchConf = TimeConf("winch", 1 / 50.0, 10e-3)
    
    ### MHE
    # XXX This should be somewhere in the common conf
    MheConf = TimeConf("mhe", 1/ 25.0, 0)
    
    fw.write("// Sensor configuration -- sampling times and delays in [sec]\n")
    for k in [LedConf, McuConf, EncConf, WinchConf, MheConf]:
        name = k.name
        ts = k.ts
        td = k.td
        fw.write("#define " + name + "_" + "ts " + repr( ts ) + "\n")
        fw.write("#define " + name + "_" + "td " + repr( td ) + "\n\n")
    fw.write("\n")
    
    #
    # Generate steady state for NMPC
    #
    
    # Cable length for steady state calculation
    steadyStateCableLength = 1.7
    # Speed for the steady state calculation
    steadyStateSpeed = -4.0

    # Reference parameters
    refP = {'r0': steadyStateCableLength,
            'ddelta0': steadyStateSpeed,
            }

    # Get the steady state
    conf = makeConf()
    steadyState, _ = getSteadyState(mhe.dae, conf, refP['ddelta0'], refP['r0'], verbose = False)

    # Write the steady state to the file
    names = {"x": mhe.dae.xNames(), "u": mhe.dae.uNames(), "z": mhe.dae.zNames()}
    for k, v in names.items():
        fw.write("const double ss_" + k + "[ " + str( len( v ) ) + " ] = {")
        fw.write(", ".join([repr( steadyState[ name ] ) + " /*" + name + "*/" for name in v]))
        fw.write("};\n\n")

    # Write bounds for control surfaces
    fw.write("// Plane's control surface limits [rad] \n")
    for name in ["aileron_bound", "elevator_bound"]:
        fw.write("#define " + name + " " + repr( conf[ name ] ) + "\n")
    
    fw.write("\n\n#endif // SIMULATOR_CONFIGURATION\n")
    fw.close()
