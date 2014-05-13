#!/usr/bin/env python

from postprocessing_tools import *
from orocos_netcdf_postprocessing import processNetCdfFile, saveDataToMatFile

import numpy as np
import os

def processFiles( folder ):
    assert( os.path.isdir( folder ) )
    
    #
    # Get the log name
    #
    logName = folder.split("/")[ -1 ]
    
    #
    # Load data sets
    #
    
    imu = processNetCdfFile(os.path.join(folder, "imuData.nc"), removeDuplicates = True, trimDataSets = True)
    enc = processNetCdfFile(os.path.join(folder, "encoderData.nc"), removeDuplicates = True, trimDataSets = True)    
    cam = processNetCdfFile(os.path.join(folder, "cameraData.nc"), removeDuplicates = True, trimDataSets = True)
    winch = processNetCdfFile(os.path.join(folder, "winchData.nc"), removeDuplicates = True, trimDataSets = True)  
    las = processNetCdfFile(os.path.join(folder, "angleData.nc"), removeDuplicates = True, trimDataSets = True)

    t_min = np.min([imu["ts_trigger"][ 0 ],
                    enc["ts_trigger"][ 0 ],
                    cam["ts_trigger"][ 0 ],
                    winch["ts_trigger"][ 0 ],
                    las["ts_trigger"][ 0 ]])

    for d in [imu, enc, cam, winch, las]:
        d["ts_trigger"] = (d["ts_trigger"] - t_min) * 1e-9
    
    #
    # IMU data
    #
    sImu = {}
    sImu["time"] = imu["ts_trigger"]
    for name in ["gyro_x", "gyro_y", "gyro_z", "accl_x", "accl_y", "accl_z"]:
        sImu[name] = imu[name]

    sCtrl = {}
    sCtrl["time"] = imu["ts_trigger"]
    sCtrl["aileron"] = imu["ua1"]
    sCtrl["elevator"] = imu["ue"]

    #
    # Encoder data
    #
    sEnc = {}
    sEnc["time"] = enc["ts_trigger"]
    sEnc["delta"] = enc["theta"]
    sEnc["sin_delta"] = enc["sin_theta"]
    sEnc["cos_delta"] = enc["cos_theta"]
    sEnc["speed_rpm"] = enc["omega_filt_rpm"]

    #
    # Camera data
    #
    ts_cam = 1 / 12.5 # [sec]

    sCam = {}
    sCam["time"] = cam["ts_trigger"] - ts_cam # Samples are one sample delayed...
    sCam["markers"] = cam["positions"]

    #
    # Winch data
    #
    sWinch = {}
    sWinch["time"] = winch["ts_trigger"]
    sWinch["cable_length"] = winch["length"]

    #
    # Line angle sensor data
    #
    sLas = {}
    sLas["time"] = las["ts_trigger"]
    sLas["angle_hor"] = las["angle_hor"]
    sLas["angle_ver"] = las["angle_ver"]

    #
    # Save data to mat files
    #
    if os.path.isdir( "smc_" + logName ) is False:
        os.mkdir("smc_" + logName)
    
    names = ["imu", "control_surfaces", "encoder", "led_tracker", "cable_length", "line_angle_sensor"]
    for k, data in enumerate([sImu, sCtrl, sEnc, sCam, sWinch, sLas]):
        p = "./smc_" + logName + "/" + names[ k ]
        saveDataToMatFile(data, p)

if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser(description="A script for generating  data sets")
    parser.add_argument("cwd", nargs="?", const=None, default=None, type=str, help="Working folder")
    parser.add_argument("-q", nargs="?", const=False, default=False, type=bool, help="Quiet mode of operation")
    args = parser.parse_args()
    
    if args.q is False:
        app = QtGui.QApplication(sys.argv)
        folder = FileFolderDialog().getFolder( os.getcwd() )
    else:
        assert args.cwd is not None
        folder = args.cwd
    
    processFiles( str( folder ) )
    
