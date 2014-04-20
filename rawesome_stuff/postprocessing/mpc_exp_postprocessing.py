#!/usr/bin/env python

from postprocessing_tools import *

import matplotlib.pyplot as plt
import numpy as np

from offline_mhe_test import NMPC

def gimmeMpcData( folder ):
    try:
        import scipy.io as sio
        data = sio.loadmat(os.path.join(folder, "dynamicMpcData.mat"))
    except:
        from orocos_netcdf_postprocessing import processNetCdfFile, saveDataToMatFile 
        data = processNetCdfFile(os.path.join(folder, "dynamicMpcData.nc"), trimDataSets = False)
        saveDataToMatFile(data, os.path.join(folder, "dynamicMpcData.mat"))
        
    #
    # Get the log name
    #
    logName = folder.split("/")[ -1 ]
        
    return data, logName

def gimmeCurrentIndex(mpc, name):
    try:
        return mpc.dae.xNames().index( name )
    except:
        pass
    
    try:
        return mpc.dae.uNames().index( name )
    except:
        raise KeyError("Cannot find " + name +"neither in x nor u names")
    
def makePlots(logName, mpc, data):
    
    start = 0;
    while True:
        start += 1
        if data["ts_trigger"][ start ] != 0:
            break;
            
    t_mpc = np.array(data["ts_trigger"] - data["ts_trigger"][ start ]) * 1e-9
    
    NX = len(mhe.dae.xNames())
    
    position = plt.figure()
    for i, n in enumerate(["x", "y", "z"]):
        plt.subplot(3, 1, i + 1)
        plt.plot(t_mpc[start: ], data["x"][start: , gimmeCurrentIndex(mpc, n)], 'b')
        plt.plot(t_mpc[start: ], data["y"][start: , gimmeCurrentIndex(mpc, n)], 'r')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated position\n" + logName)
    
    speed = plt.figure()
    for i, n in enumerate(["dx", "dy", "dz"]):
        plt.subplot(3, 1, i + 1)
        plt.plot(t_mpc[start: ], data["x"][start: , gimmeCurrentIndex(mpc, n)], 'b')
        plt.plot(t_mpc[start: ], data["y"][start: , gimmeCurrentIndex(mpc, n)], 'r')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated speed\n" + logName)
    
    dcm = plt.figure()
    for i, n in enumerate(["e11", "e12", "e13", "e21", "e22", "e23", "e31", "e32", "e33",]):
        plt.subplot(9, 1, i + 1)
        plt.plot(t_mpc[start: ], data["x"][start: , gimmeCurrentIndex(mpc, n)], 'b')
        plt.plot(t_mpc[start: ], data["y"][start: , gimmeCurrentIndex(mpc, n)], 'r')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated DCM\n" + logName)
    
    omega_body = plt.figure()
    for i, n in enumerate(["w_bn_b_x", "w_bn_b_y", "w_bn_b_z"]):
        plt.subplot(3, 1, i + 1)
        plt.plot(t_mpc[start: ], data["x"][start: , gimmeCurrentIndex(mpc, n)], 'b')
        plt.plot(t_mpc[start: ], data["y"][start: , gimmeCurrentIndex(mpc, n)], 'r')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated body angular velocity\n" + logName)
    
    ctrl_surf = plt.figure()
    for i, n in enumerate(["aileron", "elevator"]):
        plt.subplot(4, 1, i + 1)
        plt.plot(t_mpc[start: ], data["x"][start: , gimmeCurrentIndex(mpc, n)], 'b')
        plt.plot(t_mpc[start: ], data["y"][start: , gimmeCurrentIndex(mpc, n)], 'r')
        plt.ylabel( n )
    for i, n in enumerate(["daileron", "delevator"]):
        plt.subplot(4, 1, 2 + i + 1)
        plt.plot(t_mpc[start: ], data["u"][start: , gimmeCurrentIndex(mpc, n)], 'b')
        plt.plot(t_mpc[start: ], data["y"][start: , NX + gimmeCurrentIndex(mpc, n)], 'r')
        plt.ylabel( n )
        
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated ctrl surfaces & control actions \n" + logName)
    
    tether = plt.figure()
    for i, n in enumerate(["r", "dr", "ddr"]):
        plt.subplot(4, 1, i + 1)
        plt.plot(t_mpc[start: ], data["x"][start: , gimmeCurrentIndex(mpc, n)], 'b')
        plt.plot(t_mpc[start: ], data["y"][start: , gimmeCurrentIndex(mpc, n)], 'r')
        plt.ylabel( n )
    for i, n in enumerate(["dddr"]):
        plt.subplot(4, 1, 3 + i + 1)
        plt.plot(t_mpc[start: ], data["u"][start: , gimmeCurrentIndex(mpc, n)], 'b')
        plt.plot(t_mpc[start: ], data["y"][start: , NX + gimmeCurrentIndex(mpc, n)], 'r')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated tether info\n" + logName)
    
    carousel = plt.figure()
    for i, n in enumerate(["cos_delta", "sin_delta", "ddelta", "motor_torque"]):
        plt.subplot(5, 1, i + 1)
        plt.plot(t_mpc[start: ], data["x"][start: , gimmeCurrentIndex(mpc, n)], 'b')
        plt.plot(t_mpc[start: ], data["y"][start: , gimmeCurrentIndex(mpc, n)], 'r')
        plt.ylabel( n )
    for i, n in enumerate(["dmotor_torque"]):
        plt.subplot(5, 1, 4 + i + 1)
        plt.plot(t_mpc[start: ], data["u"][start: , gimmeCurrentIndex(mpc, n)], 'b')
        plt.plot(t_mpc[start: ], data["y"][start: , NX + gimmeCurrentIndex(mpc, n)], 'r')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated carousel info\n" + logName)
    
    perf = plt.figure()
    for i, n in enumerate(["kkt_value", "obj_value"]):
        plt.subplot(2, 1, i + 1)
        plt.plot(t_mpc[start: ], data[ n ][start: ])
        plt.semilogy()
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("KKT tolerance and objective\n" + logName)
        
    exec_times = plt.figure()
    for i, n in enumerate(["exec_fdb", "exec_prep"]):
        plt.subplot(2, 1, i + 1)
        plt.plot(data[ n ])
        plt.ylabel( n )
    plt.xlabel("Samples")
    plt.suptitle("Execution times\n" + logName)
    
    return [position, speed, dcm, omega_body, ctrl_surf, tether, carousel,
            perf, exec_times]
    
if __name__ == "__main__":
    assert len(sys.argv) == 2, \
        'need to call with the properties directory'
    propsDir = sys.argv[ 1 ]
    
    mhe = NMPC.makeNmpc(NMPC.samplingTime, propertiesDir = propsDir)
    
    app = QtGui.QApplication(sys.argv)
    
    folder = FileFolderDialog().getFolder( os.getcwd() )
    savePlots = YesNoDialog("Do you want to save plots?")
    
    data, logName = gimmeMpcData( str(folder) )
    
    plots = makePlots(logName, mhe, data)
    
#     if savePlots.getReply() == True:
    savePlotsToPdf(plots, str(folder), "nmpc_exp")
    
    plt.show()