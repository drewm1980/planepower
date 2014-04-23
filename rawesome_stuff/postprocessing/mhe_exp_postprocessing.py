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

    if len(data["ts_trigger"]) <= 1:
        return []
    
    start = 0;
    while True:
        start += 1
        if data["ts_trigger"][ start ] != 0:
            break;
            
    t_mhe = np.array(data["ts_trigger"] - data["ts_trigger"][ start ]) * 1e-9
    
    cam_all_positive = np.array([(row != -1.0).all() for row in data["cam_markers"][start: , :]]).astype(int)
    
    cam_indices = np.flatnonzero(data[ "num_cam_samples" ][start: ].flatten() * cam_all_positive.flatten())
    
    cam_time = t_mhe[start + cam_indices] - data["dbg_cam_delay"][start + cam_indices] * MHE.samplingTime
    # Convert to NED
    cam_pose = np.array([data["cam_pose"][start + cam_indices, 0],
                         -data["cam_pose"][start + cam_indices, 1],
                         -data["cam_pose"][start + cam_indices, 2]]).T
    
    cam_dcm = data["cam_pose"][start + cam_indices, 3: ]
    cam_dcm_ned = np.zeros( cam_dcm.shape )
    conv = np.diag([1, -1, -1])
    for r in xrange( cam_dcm.shape[ 0 ] ):
        _R = np.reshape(np.ravel(cam_dcm[r, :], order='C'), (3, 3), order='C').T
        R = np.dot(conv, np.dot(_R, conv))
        cam_dcm_ned[r, :] = R.flatten()
    
    rpy_cam = getRPY(cam_dcm_ned[:, 1],
                     cam_dcm_ned[:, 0],
                     cam_dcm_ned[:, 2],
                     cam_dcm_ned[:, 5],
                     cam_dcm_ned[:, 8])
    
    rpy_mhe = getRPY(data["x"][start: , gimmeCurrentIndex(mhe, "e12")],
                     data["x"][start: , gimmeCurrentIndex(mhe, "e11")],
                     data["x"][start: , gimmeCurrentIndex(mhe, "e13")],
                     data["x"][start: , gimmeCurrentIndex(mhe, "e23")],
                     data["x"][start: , gimmeCurrentIndex(mhe, "e33")])
    
    position = plt.figure()
    for i, n in enumerate(["x", "y", "z"]):
        plt.subplot(3, 1, i + 1)
        plt.plot(cam_time, cam_pose[:, i], 'g')
        plt.plot(t_mhe[start: ], data["x"][start: , gimmeCurrentIndex(mhe, n)], 'b')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated position; blue - MHE; green - LED Tracker\n" + logName)
    
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
        plt.plot(cam_time, cam_dcm_ned[:, i], 'g')
        plt.plot(t_mhe[start: ], data["x"][start: , gimmeCurrentIndex(mhe, n)], 'b')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated DCM; blue - MHE; green - LED Tracker\n" + logName)
    
    rpy = plt.figure()
    for i, n in enumerate(["roll [deg]", "pitch [deg]", "yaw [deg]"]):
        plt.subplot(3, 1, i + 1)
        plt.plot(cam_time, rpy_cam[:, i], 'g')
        plt.plot(t_mhe[start:], rpy_mhe[:, i], 'b')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated Euler angles; blue - MHE; green - LED Tracker\n" + logName)
    
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
    
    return [position, speed, dcm, rpy, omega_body, ctrl_surf, tether, carousel,
            virtual_torques,
            perf, exec_times]
    
def getMheInfo( mhe ):
    
    txt = "\\begin{table}[!h]\n \\begin{tabular}{ll}\n"
    txt += ""
    txt += "Measurement item & Standard deviation \\\\\n"
    for name in mhe.yxNames + mhe.yuNames:
            txt += "\\verb|" + name + "| & " + repr( MHE.mheSigmas[ name ] ) + "\\\\\n"
    
    txt += "\\end{tabular}\n"
    txt += "\\caption{Measurement standard deviations.}\n"
    txt += "\\end{table}\n\n"
    
    txt += "\\begin{table}[!h]\n \\begin{tabular}{ll}\n"
    txt += ""
    txt += "Measurement item & Scaled MHE weights \\\\\n"
    for name in mhe.yxNames + mhe.yuNames:
            txt += "\\verb|" + name + "| & " + repr( MHE.mheWeights[ name ] ) + "\\\\\n"
    
    txt += "\\end{tabular}\n"
    txt += "\\caption{MHE weights; scaling factor is " + str( MHE.mheWeightScaling ) + ".}\n"
    txt += "\\end{table}\n\n"
        
    txt += "\\clearpage"
    
    txt += "\\begin{verbatim}\n"
    txt += repr( mhe )
    txt += "\\end{verbatim}\n"
    
    txt += "\\clearpage"
    
    return txt

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="A script for generating MHE report")
    parser.add_argument("properties", nargs=1, type=str, help="Folder with model properties")
    parser.add_argument("cwd", nargs="?", const=None, default=None, type=str, help="Working folder")
    parser.add_argument("-q", nargs="?", const=False, default=False, type=bool, help="Quiet mode of operation")
    args = parser.parse_args()
    
    if args.q is False:
        app = QtGui.QApplication(sys.argv)
        folder = FileFolderDialog().getFolder( os.getcwd() )
        ynDlg = YesNoDialog("Do you want to save plots?")
        savePlots = ynDlg.getReply()
    else:
        assert args.cwd is not None
        folder = args.cwd
        savePlots = True
    
    mhe = MHE.makeMhe(MHE.samplingTime, propertiesDir = args.properties[0])
    
    data, logName = gimmeMheData( str(folder) )
    plots = makePlots(logName, mhe, data)
    info = getMheInfo( mhe )
    
    if savePlots is True:
        savePlotsToPdf(plots, str(folder), "mhe_exp", pretext = info)
    
    if args.q is False:
        plt.show()
