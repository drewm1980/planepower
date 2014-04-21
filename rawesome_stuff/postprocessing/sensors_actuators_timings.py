#!/usr/bin/env python

import sys, os

import matplotlib.pyplot as plt
import numpy as np

from postprocessing_tools import *
from orocos_netcdf_postprocessing import processNetCdfFile

#from matplotlib import rc
#rc('font', **{'family':'sans-serif', 'sans-serif':['Helvetica']})
## for Palatino and other serif fonts use:
#rc('font',**{'family':'serif','serif':['Palatino']})
#rc('text', usetex = True)

def processFiles(folder, savePlots):
    assert( os.path.isdir( folder ) )
    
    #
    # Get the log name
    #
    logName = folder.split("/")[ -1 ]
    
    #
    # Load data sets
    #
    
    imu = processNetCdfFile(os.path.join(folder, "imuData.nc"), removeDuplicates = False, trimDataSets = True)
    enc = processNetCdfFile(os.path.join(folder, "encoderData.nc"), removeDuplicates = False, trimDataSets = True)    
    cam = processNetCdfFile(os.path.join(folder, "cameraData.nc"), removeDuplicates = False, trimDataSets = True)
    winch = processNetCdfFile(os.path.join(folder, "winchData.nc"), removeDuplicates = False, trimDataSets = True)  
    las = processNetCdfFile(os.path.join(folder, "angleData.nc"), removeDuplicates = False, trimDataSets = True)
    
    #
    # Plot stuff
    #
    imuPlots = processImuData(logName,  imu)
    encPlots = processEncData(logName,  enc)
    camPlots = processCamData(logName,  cam)
    winchPlots = processWinchData(logName,  winch)
    lasPlots = processLasData(logName,  las)
    
    gyroSCPlots = gyroSanityCheck(logName, imu, enc)
    
    if savePlots is True:
        savePlotsToPdf(imuPlots + encPlots + camPlots + winchPlots + lasPlots + gyroSCPlots,
                       folder,
                       prefix = "sensors_actuators_timing")

def  processImuData(logName, data):
    
    t_log = data["TimeStamp"] - data["TimeStamp"][ 0 ];
    t_imu = (data["ts_trigger"] - data["ts_trigger"][ 0 ]) * 1e-9;
    t_exe = (data["ts_elapsed"]) * 1e3 # [msec]
    
    f1 = plt.figure()
    
    plt.subplot(3, 1, 1)
    plt.step(t_log[: -1], np.diff(t_log) * 1e3, '-*b')
    plt.legend(['diff(Logger time-stamp) [ms]']);
    plt.xlabel('t_log [s]')
    
    plt.subplot(3, 1, 2)
    plt.step(t_log[: -1], np.diff(t_imu) * 1e3, '-*r')
    plt.legend(['diff(MCU handler time-stamp) [ms]']);
    plt.xlabel('t_log [s]')
    
    plt.subplot(3, 1, 3)
    plt.step(t_imu, t_exe)
    plt.ylabel('Exec time [ms]')
    plt.xlabel('t_imu [s]')
    
    plt.suptitle("MCU handler: time-stamps and execution time\n" + logName)
    
    f2 = plt.figure()
    for it, name in enumerate(["gyro_x", "gyro_y", "gyro_z", "accl_x", "accl_y", "accl_z"]):
        plt.subplot(6, 1, it + 1)
        plt.step(t_imu, data[ name ])
        plt.ylabel( name )
    plt.xlabel('t_imu [s]')
    
    plt.suptitle( 'MCU handler: IMU data\n' + logName )

    f3 = plt.figure()
    for it, name in enumerate(["ua1", "ua2", "ue"]):
        plt.subplot(2, 3, it + 1)
        plt.step(t_imu, data[ name ])
        plt.ylabel( name )
    for it, name in enumerate(["d_ua1", "d_ua2", "d_ue"]):
        plt.subplot(2, 3, it + 4)
        plt.step(t_imu, data[ name ])
        plt.ylabel( name )
        plt.xlabel('t_imu [s]')

    plt.suptitle('MCU handler: control surfaces and derivatives\n' + logName)
    
    # Return figure handles os that we can save them later
    return [f1, f2, f3]

def processEncData(logName, data):
    
    t_log = data["TimeStamp"]
    t_enc = (data["ts_trigger"] - data["ts_trigger"][0]) * 1e-9
    t_exe = data["ts_elapsed"] * 1e3

    f1 = plt.figure()
    plt.subplot(3, 1, 1)
    plt.step(t_log[: -1], np.diff(t_log) * 1e3, '-*b')
    plt.legend(['diff(Logger time-stamp) [ms]'])
    plt.xlabel('t_log [s]')
    
    plt.subplot(3, 1, 2)
    plt.step(t_log[: -1], np.diff(t_enc) * 1e3, '-*r')
    plt.legend(['diff(Encoder time-stamp) [ms]'])
    plt.xlabel('t_log [s]')
    
    plt.subplot(3, 1, 3)
    plt.step(t_enc, t_exe)
    plt.ylabel('Exec time [ms]')
    plt.xlabel('t_enc [s]')
        
    plt.suptitle("Encoder: time-stamps and execution time\n" + logName);
  
    f2 = plt.figure()
    plt.subplot(4, 1, 1);
    plt.step(t_enc, data["theta"]);
    plt.legend(['theta [rad]'])
    plt.ylim([-3.2, 3.2])
    
    plt.subplot(4, 1, 2)
    plt.step(t_enc, data["sin_theta"], 'b')
    plt.step(t_enc, data["cos_theta"], 'r')
    plt.legend(['sin_theta', 'cos_theta']);
    
    plt.subplot(4, 1, 3)
    plt.step(t_enc, data["omega"], 'b');
    plt.legend(['omega [rad/s]']);
    plt.xlabel('t_enc [s]')
    plt.ylim([-1, 10])
    
    plt.subplot(4, 1, 4)
    plt.step(t_enc, data["omega_filt_rpm"]);
    plt.legend(['Filtered omega [rpm]'])
    plt.xlabel('t_enc [s]')
    plt.ylim([-5, 65])
        
    plt.suptitle('Encoder: data\n' + logName)
    
    return [f1, f2]

def processCamData(logName, data):
    
    t_log = data["TimeStamp"] - data["TimeStamp"][ 0 ]
    t_cam = (data["ts_trigger"] - data["ts_trigger"][ 0 ]) * 1e-9
    t_frame = (data["ts_frame"] - data["ts_frame"][ 0 ]) * 1e-9
    t_exe = data["ts_elapsed"] * 1e3

    positions = data["positions"]
#     weights = data["weights"]
#     pose = data["pose"]
    
    f1 = plt.figure()
    
    plt.subplot(4, 1, 1)
    plt.step(t_log[: -1], np.diff(t_log) * 1e3, "-*b")
    plt.legend(["diff(Logger time-stamp) [ms]"])
    plt.xlabel('t_log [s]')
    
    plt.subplot(4, 1, 2)
    plt.step(t_log[: -1], np.diff(t_cam) * 1e3, "-*b")
    plt.legend(["diff(LED Tracker time-stamp) [ms]"])
    plt.xlabel('t_log [s]')
    
    plt.subplot(4, 1, 3)
    plt.step(t_log[: -1], np.diff(t_frame) * 1e3, "-*b")
    plt.legend(["diff(ts_frame) [ms]"])
    plt.xlabel('t_log [s]')
    
    plt.subplot(4, 1, 4)
    plt.step(t_cam, t_exe)
    plt.xlabel('t_cam [s]')
    plt.ylabel('Execution time [ms]');
            
    plt.suptitle("LED Tracker: time-stamps and execution time\n" + logName)
    
    clr = ["r", "g", "b"]
    coord = ["u", "v"]
    
    f2 = plt.figure()
    for f in range( 6 ):
        plt.subplot(3, 2, f + 1)
        plt.step(t_cam, positions[:, f], clr[f / 2])
        plt.legend([coord[f % 2]])
    plt.suptitle("Frame 1\n" + logName)
    
    f3 = plt.figure()
    for f in range( 6 ):
        plt.subplot(3, 2, f + 1)
        plt.step(t_cam, positions[:, 6 + f], clr[f / 2])
        plt.legend([coord[f % 2]])
    plt.suptitle("Frame 2\n" + logName)
    
    return [f1, f2, f3]

def processWinchData(logName, data):
    
    t_log = data["TimeStamp"] - data["TimeStamp"][ 0 ]
    t_winch = (data["ts_trigger"] - data["ts_trigger"][ 0 ]) * 1e-9
    t_exe = data["ts_elapsed"] * 1e3;

    f1 = plt.figure()
    
    plt.subplot(3, 1, 1)
    plt.step(t_log[: -1], np.diff(t_log) * 1e3, "-*b")
    plt.legend(["diff(Logger time-stamp) [ms]"])
    plt.xlabel("t_log [s]")
    
    plt.subplot(3, 1, 2)
    plt.step(t_log[: -1], np.diff(t_winch) * 1e3, "-*b")
    plt.legend(["diff(Winch time-stamp) [ms]"])
    plt.xlabel("t_log [s]")
    
    plt.subplot(3, 1, 3)
    plt.step(t_winch, t_exe)
    plt.ylabel("Exec time [ms]")
    plt.xlabel("t_winch [s]")
    
    plt.suptitle( "Winch: time-stamps and execution time\n" + logName)
    
    f2 = plt.figure()
    
    plt.subplot(3, 1, 1)
    plt.step(t_winch, data["length"])
    plt.legend(["Tether length [m]"])
    
    plt.subplot(3, 1, 2)
    plt.step(t_winch, data["speed"])
    plt.legend(["Tether speed [m/s]"])
    
    plt.subplot(3, 1, 3)
    plt.step(t_winch, data["dbg_current"])
    plt.legend(["Motor current [A]"])
    
    plt.xlabel("t_winch [s]")
    
    plt.suptitle("Winch: data\n" + logName)
        
    return [f1, f2]

def processLasData(logName, data):
    
    t_log = data["TimeStamp"] - data["TimeStamp"][ 0 ]
    t_las = (data["ts_trigger"] - data["ts_trigger"][ 0 ]) * 1e-9
    t_exe = data["ts_elapsed"] * 1e3
    
    f1 = plt.figure()
    plt.subplot(3, 1, 1)
    plt.step(t_log[: -1], np.diff(t_log) * 1e3,'b')
    plt.legend(['Diff logger time [ms]'])
    plt.xlabel('t_log [s]')
    
    plt.subplot(3, 1, 2)
    plt.step(t_log[: -1], np.diff(t_las) * 1e3,'b')
    plt.legend(['Diff trigger time [ms]'])
    plt.xlabel(['t_log [s]'])
    
    plt.subplot(3, 1, 3)
    plt.step(t_log, t_exe,'b')
    plt.ylabel('Exec time [ms]')
    plt.xlabel('t_log [s]')
    
    plt.suptitle('Line angle timings\n' + logName)

    f2 = plt.figure()
    plt.subplot(2, 1, 1)
    plt.step(t_las, data["angle_hor"], 'b')
    plt.legend(['angle_hor [rad]'])
    plt.xlabel('t_las [s]')
    
    plt.subplot(2, 1, 2)
    plt.step(t_las, data["angle_ver"],'b')
    plt.legend(['angle_ver [rad]'])
    plt.xlabel('t_las [s]')
    
    plt.suptitle("Line angle data\n" + logName)
    
    return [f1, f2]

def gyroSanityCheck(logName, imu, enc):
    
    t_min = np.min([imu["ts_trigger"][ 0 ], enc["ts_trigger"][ 0 ]])
    
    t_imu = (imu["ts_trigger"] - t_min) * 1e-9
    t_enc = (enc["ts_trigger"] - t_min) * 1e-9
        
    gyroNorm = np.sum(np.array([imu["gyro_x"], imu["gyro_y"], imu["gyro_z"]]) ** 2, axis = 0) ** (1. / 2)
        
    f1 = plt.figure()
    plt.step(t_imu, gyroNorm, "r")
    plt.step(t_enc, enc["omega"], "b")
    plt.legend(["norm(gyro)", "enc omega"])
    plt.xlabel("Time [s]")
    plt.ylabel("angular speed [rad/s]")
    
    plt.suptitle("Sanity check for gyros\n" + logName)
    
    return [f1]

if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser(description="A script for generating S&A info")
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
    
    processFiles(str( folder ), savePlots)
    
    if args.q is False:
        plt.show()
