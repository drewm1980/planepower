import rawe
import casadi as C
import camModel
import numpy as np

import codegen_utils
def cross(a,b):
    c = C.vertcat([a[1]*b[2]-a[2]*b[1],
                   a[2]*b[0]-a[0]*b[2],
                   a[0]*b[1]-a[1]*b[0]])
    return c

def makeModel(conf):
    print "creating model..."
    dae = rawe.models.carousel(conf)
    dae['accel_magnitude'] = dae.ddt('dx')*dae.ddt('dx') + dae.ddt('dy')*dae.ddt('dy')
    #ddp = C.vertcat([dae.ddt('dx'),dae.ddt('dy'),dae.ddt('dz')])
    #dw  = C.vertcat([dae.ddt('w1'),dae.ddt('w2'),dae.ddt('w3')])
    (xDotSol, zSol) = dae.solveForXDotAndZ()
    ddp = C.vertcat([xDotSol['dx'],xDotSol['dy'],xDotSol['dz']])
    dw  = C.vertcat([xDotSol['w1'],xDotSol['w2'],xDotSol['w3']])
    x =   dae['x']
    y =   dae['y']
    z =   dae['z']

    dx  =  dae['dx']
    dy  =  dae['dy']
    dz  =  dae['dz']

    w1  =  dae['w1']
    w2  =  dae['w2']
    w3  =  dae['w3']
    w = C.vertcat([w1,w2,w3])
    ddelta = dae['ddelta']
    #dddelta = dae.ddt('ddelta')
    dddelta = xDotSol['ddelta']
    r = dae['r']
    dr = dae['dr']
    e11 = dae['e11']
    e12 = dae['e12']
    e13 = dae['e13']

    e21 = dae['e21']
    e22 = dae['e22']
    e23 = dae['e23']

    e31 = dae['e31']
    e32 = dae['e32']
    e33 = dae['e33']
    R = C.veccat( [dae[n] for n in ['e11', 'e12', 'e13',
                                    'e21', 'e22', 'e23',
                                    'e31', 'e32', 'e33']]
                      ).reshape((3,3))

    rA = conf['rArm']
    g = conf['g']
    pIMU = C.DMatrix(np.loadtxt('../properties/IMU/pIMU.dat'))
    RIMU = C.DMatrix(np.loadtxt('../properties/IMU/RIMU.dat'))
    ddpIMU = C.mul(R.T,ddp) - ddelta**2*C.mul(R.T,C.vertcat([x+rA,y,0])) + 2*ddelta*C.mul(R.T,C.vertcat([-dy,dx,0])) + dddelta*C.mul(R.T,C.vertcat([-y,x+rA,0])) + C.mul(R.T,C.vertcat([0,0,g]))
    aShift = cross(dw,pIMU)
    dae['IMU_acceleration'] = C.mul(RIMU,ddpIMU+aShift)
    dae['IMU_angular_velocity'] = C.mul(RIMU,C.vertcat([w1,w2,w3]))

    camConf = {'PdatC1':C.DMatrix(np.loadtxt('../properties/cameras/PC1.dat')),
               'PdatC2':C.DMatrix(np.loadtxt('../properties/cameras/PC2.dat')),
               'RPC1':C.DMatrix(np.loadtxt('../properties/cameras/RPC1.dat')),
               'RPC2':C.DMatrix(np.loadtxt('../properties/cameras/RPC2.dat')),
               'pos_marker_body1':C.DMatrix(np.loadtxt('../properties/markers/pos_marker_body1.dat')),
               'pos_marker_body2':C.DMatrix(np.loadtxt('../properties/markers/pos_marker_body2.dat')),
               'pos_marker_body3':C.DMatrix(np.loadtxt('../properties/markers/pos_marker_body3.dat'))}
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
