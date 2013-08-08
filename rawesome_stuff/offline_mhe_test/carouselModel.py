import rawe
import casadi as C
import camModel
import numpy as np
import os

def cross(a,b):
    c = C.vertcat([a[1]*b[2]-a[2]*b[1],
                   a[2]*b[0]-a[0]*b[2],
                   a[0]*b[1]-a[1]*b[0]])
    return c

def makeModel(conf,propertiesDir='../properties'):
    dae = rawe.models.carousel(conf)
    (xDotSol, zSol) = dae.solveForXDotAndZ()
    ddp = C.vertcat([xDotSol['dx'],xDotSol['dy'],xDotSol['dz']])
    ddt_w_bn_b  = C.vertcat([xDotSol['w_bn_b_x'],xDotSol['w_bn_b_y'],xDotSol['w_bn_b_z']])
    x =   dae['x']
    y =   dae['y']

    dx  =  dae['dx']
    dy  =  dae['dy']

    ddelta = dae['ddelta']
    dddelta = xDotSol['ddelta']

#    R = C.veccat( [dae[n] for n in ['e11', 'e12', 'e13',
#                                    'e21', 'e22', 'e23',
#                                    'e31', 'e32', 'e33']]
#                      ).reshape((3,3))
    R = dae['R_c2b']
    rA = conf['rArm']
    g = conf['g']
    pIMU = C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'IMU/pIMU.dat')))
    RIMU = C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'IMU/RIMU.dat')))
#    print pIMU
#    print RIMU
    RIMU = np.eye(3)
    RIMU[1,1] = RIMU[2,2] = -1
    ddpIMU_c = ddp - ddelta**2*C.vertcat([x+rA,y,0]) + 2*ddelta*C.vertcat([-dy,dx,0]) + dddelta*C.vertcat([-y,x+rA,0]) + C.vertcat([0,0,g])
    ddpIMU = C.mul(R,ddpIMU_c)
#    ddpIMU = C.mul(R.T,ddp) - ddelta**2*C.mul(R.T,C.vertcat([x+rA,y,0])) + 2*ddelta*C.mul(R.T,C.vertcat([-dy,dx,0])) + dddelta*C.mul(R.T,C.vertcat([-y,x+rA,0])) + C.mul(R.T,C.vertcat([0,0,g]))
    aBridle = cross(ddt_w_bn_b,pIMU)
    dae['IMU_acceleration'] = C.mul(RIMU,ddpIMU+aBridle)
    dae['IMU_angular_velocity'] = C.mul(RIMU,dae['w_bn_b'])

    camConf = {'PdatC1':C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'cameras/PC1.dat'))),
               'PdatC2':C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'cameras/PC2.dat'))),
               'RPC1':C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'cameras/RPC1.dat'))),
               'RPC2':C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'cameras/RPC2.dat'))),
               'pos_marker_body1':C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'markers/pos_marker_body1.dat'))),
               'pos_marker_body2':C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'markers/pos_marker_body2.dat'))),
               'pos_marker_body3':C.DMatrix(np.loadtxt(os.path.join(propertiesDir,'markers/pos_marker_body3.dat')))}
    
#    for name in ['pos_marker_body1','pos_marker_body2','pos_marker_body3']: camConf[name][1:3] *= -1
    dae['marker_positions'] = camModel.fullCamModel(dae,camConf)

    dae['ConstR1'] = dae['e11']*dae['e11'] + dae['e12']*dae['e12'] + dae['e13']*dae['e13'] - 1
    dae['ConstR2'] = dae['e11']*dae['e21'] + dae['e12']*dae['e22'] + dae['e13']*dae['e23']
    dae['ConstR3'] = dae['e11']*dae['e31'] + dae['e12']*dae['e32'] + dae['e13']*dae['e33']
    dae['ConstR4'] = dae['e21']*dae['e21'] + dae['e22']*dae['e22'] + dae['e23']*dae['e23'] - 1
    dae['ConstR5'] = dae['e21']*dae['e31'] + dae['e22']*dae['e32'] + dae['e23']*dae['e33']
    dae['ConstR6'] = dae['e31']*dae['e31'] + dae['e32']*dae['e32'] + dae['e33']*dae['e33'] - 1
    #dae['ConstDelta'] = dae['cos_delta']*dae['cos_delta'] + dae['sin_delta']*dae['sin_delta'] - 1
    dae['ConstDelta'] = ( dae['cos_delta']**2 + dae['sin_delta']**2 - 1 )

    return dae
