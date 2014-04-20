#!/usr/bin/env python

from postprocessing_tools import *

import matplotlib.pyplot as plt
import numpy as np

from offline_mhe_test import MHE

def gimmeMheData( folder ):
    try:
        import scipy.io as sio
        data = sio.loadmat(os.path.join(folder, "dynamicMheData.mat"))
    except:
        from orocos_netcdf_postprocessing import processNetCdfFile, saveDataToMatFile 
        data = processNetCdfFile(os.path.join(folder, "dynamicMheData.nc"), trimDataSets = True)
        saveDataToMatFile(data, os.path.join(folder, "dynamicMheData.mat"))
        
    #
    # Get the log name
    #
    logName = folder.split("/")[ -1 ]
        
    return data, logName

def gimmeCurrentIndex(mhe, name):
    
    try:
        lenX = len(mhe.dae.xNames())
        offset = MHE.mheHorizonN * lenX
        
        return offset + mhe.dae.xNames().index( name )
    
    except:
        pass
    
    try:
        lenU = len(mhe.dae.uNames())
        offset = (MHE.mheHorizonN - 1) * lenU
        
        return offset + mhe.dae.uNames().index( name )
        
    except:
        raise KeyError("Cannot find " + name +"neither in x nor u names")

def makePlots(logName, mhe, data):
    
    start = 0;
    while True:
        start += 1
        if data["ts_trigger"][ start ] != 0:
            break;
            
    t_mhe = np.array(data["ts_trigger"] - data["ts_trigger"][ start ]) * 1e-9
    
    position = plt.figure()
    for i, n in enumerate(["x", "y", "z"]):
        plt.subplot(3, 1, i + 1)
        plt.plot(t_mhe[start: ], data["x"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated position\n" + logName)
    
    speed = plt.figure()
    for i, n in enumerate(["dx", "dy", "dz"]):
        plt.subplot(3, 1, i + 1)
        plt.plot(t_mhe[start: ], data["x"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated speed\n" + logName)
    
    dcm = plt.figure()
    for i, n in enumerate(["e11", "e12", "e13", "e21", "e22", "e23", "e31", "e32", "e33",]):
        plt.subplot(9, 1, i + 1)
        plt.plot(t_mhe[start: ], data["x"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated DCM\n" + logName)
    
    omega_body = plt.figure()
    for i, n in enumerate(["w_bn_b_x", "w_bn_b_y", "w_bn_b_z"]):
        plt.subplot(3, 1, i + 1)
        plt.plot(t_mhe[start: ], data["x"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated body angular velocity\n" + logName)
    
    ctrl_surf = plt.figure()
    for i, n in enumerate(["aileron", "elevator"]):
        plt.subplot(4, 1, i + 1)
        plt.plot(t_mhe[start: ], data["x"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    for i, n in enumerate(["daileron", "delevator"]):
        plt.subplot(4, 1, 2 + i + 1)
        plt.plot(t_mhe[start: ], data["u"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
        
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated ctrl surfaces \n" + logName)
    
    tether = plt.figure()
    for i, n in enumerate(["r", "dr", "ddr"]):
        plt.subplot(4, 1, i + 1)
        plt.plot(t_mhe[start: ], data["x"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    for i, n in enumerate(["dddr"]):
        plt.subplot(4, 1, 3 + i + 1)
        plt.plot(t_mhe[start: ], data["u"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated tether info\n" + logName)
    
    carousel = plt.figure()
    for i, n in enumerate(["cos_delta", "sin_delta", "ddelta", "motor_torque"]):
        plt.subplot(5, 1, i + 1)
        plt.plot(t_mhe[start: ], data["x"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    for i, n in enumerate(["dmotor_torque"]):
        plt.subplot(5, 1, 4 + i + 1)
        plt.plot(t_mhe[start: ], data["u"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated carousel info\n" + logName)
    
    virtual_torques = plt.figure()
    for i, n in enumerate(["t1_disturbance", "t2_disturbance", "t3_disturbance"]):
        plt.subplot(6, 1, i + 1)
        plt.plot(t_mhe[start: ], data["x"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    for i, n in enumerate(["dt1_disturbance", "dt2_disturbance", "dt3_disturbance"]):
        plt.subplot(6, 1, 3 + i + 1)
        plt.plot(t_mhe[start: ], data["u"][start: , gimmeCurrentIndex(mhe, n)])
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Virtual torques\n" + logName)
    
    perf = plt.figure()
    for i, n in enumerate(["kkt_value", "obj_value"]):
        plt.subplot(2, 1, i + 1)
        plt.plot(t_mhe[start: ], data[ n ][start: ])
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
            virtual_torques,
            perf, exec_times]

if __name__ == "__main__":
    assert len(sys.argv) == 2, \
        'need to call generateMhe.py with the properties directory'
    propsDir = sys.argv[ 1 ]
    
    mhe = MHE.makeMhe(MHE.samplingTime, propertiesDir = propsDir)
    
    app = QtGui.QApplication(sys.argv)
    
    folder = FileFolderDialog().getFolder( os.getcwd() )
    savePlots = YesNoDialog("Do you want to save plots?")
    
    data, logName = gimmeMheData( str(folder) )
    
    plots = makePlots(logName, mhe, data)
    
#     if savePlots.getReply() == True:
    savePlotsToPdf(plots, str(folder), "mhe_exp")
    
    plt.show()
    