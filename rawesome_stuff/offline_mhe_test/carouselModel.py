import rawe
import casadi as C
import camModel
import numpy as np
import os

def makeModel(conf,propertiesDir='../properties'):
    
    # Make model
    dae = rawe.models.carousel(conf)
    (xDotSol, zSol) = dae.solveForXDotAndZ()
    
    # Get variables and outputs from the model
    ddp = C.vertcat([xDotSol['dx'],xDotSol['dy'],xDotSol['dz']])
    ddt_w_bn_b  = C.vertcat([xDotSol['w_bn_b_x'],xDotSol['w_bn_b_y'],xDotSol['w_bn_b_z']])
    x =   dae['x']
    y =   dae['y']

    dx  =  dae['dx']
    dy  =  dae['dy']

    ddelta = dae['ddelta']
    dddelta = xDotSol['ddelta']
    
    R = dae['R_c2b']
    rA = conf['rArm']
    g = conf['g']
    
    # Rotation matrix to convert from NWU to NED frame type
    R_nwu2ned = np.eye( 3 )
    R_nwu2ned[1, 1] = R_nwu2ned[2, 2] = -1.0
    
    ############################################################################
    #
    # IMU model
    #
    ############################################################################
    
    # Load IMU position and orientation w.r.t. body frame
    pIMU = C.mul(R_nwu2ned, C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'IMU/pIMU.dat'))))
    RIMU = C.mul(R_nwu2ned, C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'IMU/RIMU.dat'))))

	# Define IMU measurement functions
	# TODO here is omitted the term: w x w pIMU 
    # The sign of gravity is negative because of the NED convention (z points down!)
    ddpIMU_c = ddp - ddelta ** 2 * C.vertcat([x + rA, y, 0]) + 2 * ddelta * C.vertcat([-dy, dx, 0]) + \
                dddelta * C.vertcat([-y, x + rA, 0]) - C.vertcat([0, 0, g])
    ddpIMU = C.mul(R, ddpIMU_c)
    aBridle = C.cross(ddt_w_bn_b, pIMU)
    
    # For the accelerometers
    dae['IMU_acceleration'] = C.mul(RIMU, ddpIMU + aBridle)
    # ... and for the gyroscopes
    dae['IMU_angular_velocity'] = C.mul(RIMU, dae['w_bn_b'])

	############################################################################
	#
	# Stereo vision subsystem modeling
	#
	############################################################################

	# Load calibration data from configuration files
    camConf = {'PdatC1':C.DMatrix(np.loadtxt(os.path.join(propertiesDir, 'cameras/PC1.dat'))),
               'PdatC2':C.DMatrix(np.loadtxt(os.path.join(propertiesDir, 'cameras/PC2.dat'))),
               'RPC1':C.DMatrix(np.loadtxt(os.path.join(propertiesDir, 'cameras/RPC1.dat'))),
               'RPC2':C.DMatrix(np.loadtxt(os.path.join(propertiesDir, 'cameras/RPC2.dat'))),
               'pos_marker_body1':C.DMatrix(np.loadtxt(os.path.join(propertiesDir, 'markers/pos_marker_body1.dat'))),
               'pos_marker_body2':C.DMatrix(np.loadtxt(os.path.join(propertiesDir, 'markers/pos_marker_body2.dat'))),
               'pos_marker_body3':C.DMatrix(np.loadtxt(os.path.join(propertiesDir, 'markers/pos_marker_body3.dat')))}

	# Construction of the measurement functions
    dae['marker_positions'] = camModel.fullCamModel(dae, camConf)

	############################################################################
	#
	# Constraints in the MHE
	#
	############################################################################
	
    dae['ConstR1'] = dae['e11']*dae['e11'] + dae['e12']*dae['e12'] + dae['e13']*dae['e13'] - 1
    dae['ConstR2'] = dae['e11']*dae['e21'] + dae['e12']*dae['e22'] + dae['e13']*dae['e23']
    dae['ConstR3'] = dae['e11']*dae['e31'] + dae['e12']*dae['e32'] + dae['e13']*dae['e33']
    dae['ConstR4'] = dae['e21']*dae['e21'] + dae['e22']*dae['e22'] + dae['e23']*dae['e23'] - 1
    dae['ConstR5'] = dae['e21']*dae['e31'] + dae['e22']*dae['e32'] + dae['e23']*dae['e33']
    dae['ConstR6'] = dae['e31']*dae['e31'] + dae['e32']*dae['e32'] + dae['e33']*dae['e33'] - 1
    dae['ConstDelta'] = (dae['cos_delta'] ** 2 + dae['sin_delta'] ** 2 - 1)

    return dae
