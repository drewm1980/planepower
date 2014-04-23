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
        data = processNetCdfFile(os.path.join(folder, "dynamicMpcData.nc"), trimDataSets = True)
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

    if len(data["ts_trigger"]) <= 1:
        return []
    
    start = 0;
    while True:
        start += 1
        if data["ts_trigger"][ start ] != 0:
            break;
            
    t_mpc = np.array(data["ts_trigger"] - data["ts_trigger"][ start ]) * 1e-9
    
    NX = len(mpc.dae.xNames())
    
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


    rpy_est = getRPY( data["x"][start: , gimmeCurrentIndex(mpc, "e12")]
                    , data["x"][start: , gimmeCurrentIndex(mpc, "e11")]
                    , data["x"][start: , gimmeCurrentIndex(mpc, "e13")]
                    , data["x"][start: , gimmeCurrentIndex(mpc, "e23")]
                    , data["x"][start: , gimmeCurrentIndex(mpc, "e33")])

    rpy_ref = getRPY( data["y"][start: , gimmeCurrentIndex(mpc, "e12")]
                    , data["y"][start: , gimmeCurrentIndex(mpc, "e11")]
                    , data["y"][start: , gimmeCurrentIndex(mpc, "e13")]
                    , data["y"][start: , gimmeCurrentIndex(mpc, "e23")]
                    , data["y"][start: , gimmeCurrentIndex(mpc, "e33")])

    rpy = plt.figure()
    for i, n in enumerate(["roll [deg]", "pitch [deg]", "yaw [deg]"]):
        plt.subplot(3, 1, i + 1)
        plt.plot(t_mpc[start: ], rpy_est[:, i], 'b')
        plt.plot(t_mpc[start: ], rpy_ref[:, i], 'r')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Estimated Euler angles\n" + logName)

    dcmEst = data["x"][start: , gimmeCurrentIndex(mpc, "e11"): gimmeCurrentIndex(mpc, "e33") + 1]
    dcmRef = data["y"][start: , gimmeCurrentIndex(mpc, "e11"): gimmeCurrentIndex(mpc, "e33") + 1]

    dcmErr = np.zeros( dcmEst.shape )
    for i in xrange( dcmEst.shape[ 0 ] ):
        Rest = np.reshape(dcmEst[i, :], (3, 3), order='C')
        Rref = np.reshape(dcmRef[i, :], (3, 3), order='C')
        Rerr = np.dot(Rest, Rref.T)
#        Rerr = np.dot(Rref.T, Rest)
        dcmErr[i, :] = Rerr.flatten()
    rpy_err = getRPY(dcmErr[:, 1],
                     dcmErr[:, 0],
                     dcmErr[:, 2],
                     dcmErr[:, 5],
                     dcmErr[:, 8])

    rpy_error = plt.figure()
    for i, n in enumerate(["roll [deg]", "pitch [deg]", "yaw [deg]"]):
        plt.subplot(3, 1, i + 1)
        plt.plot(t_mpc[start: ], dcmErr[:, i], 'k')
        plt.ylabel( n )
    plt.xlabel("Time [s]")
    plt.suptitle("Error Euler angles -- from Rest * Rref'\n" + logName)
        
    
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
    for i, n in enumerate(["exec_fdb", "exec_prep", "n_asc", "solver_status"]):
        plt.subplot(4, 1, i + 1)
        plt.plot(data[ n ])
        plt.ylabel( n )
    plt.xlabel("Samples")
    plt.suptitle("Execution times and QP perf\n" + logName)
    
    return [position, speed, dcm, rpy, rpy_error, omega_body, ctrl_surf, tether, carousel,
            perf, exec_times]

def getNmpcInfo( mpc ):
    
    txt = "\\begin{table}[h]\n \\begin{tabular}{ll}\n"
    txt += ""
    txt += "Reference item & Scaled NMPC weights \\\\\n"
    for name in mpc.dae.xNames():
        if name in NMPC.mpcWeights:
            txt += "\\verb|" + name + "| & " + repr( NMPC.mpcWeights[ name ] ) + "\\\\\n"
    
    txt += "\\end{tabular}\n"
    txt += "\\caption{NMPC weights; scaling factor is " + str( NMPC.mpcWeightScaling ) + ".}\n"
    txt += "\\end{table}\n\n"
        
    txt += "\\clearpage"
    
    txt += "\\begin{verbatim}\n"
    txt += repr( mpc )
    txt += "\\end{verbatim}\n"
    
    txt += "\\clearpage"
    
    return txt
    
if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="A script for generating NMPC report")
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
    
    mpc = NMPC.makeNmpc(NMPC.samplingTime, propertiesDir = args.properties[0])
    
    data, logName = gimmeMpcData( str(folder) )
    plots = makePlots(logName, mpc, data)
    info = getNmpcInfo( mpc )
    
    if savePlots == True:
        savePlotsToPdf(plots, str(folder), "nmpc_exp", pretext = info)
    
    if args.q is False:
        plt.show()
