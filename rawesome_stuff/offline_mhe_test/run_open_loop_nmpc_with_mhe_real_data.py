#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt
import scipy.io as sio

from sys import exit

# Rawesome deps
from rawe.models.arianne_conf import makeConf
from rawekite.carouselSteadyState import getSteadyState
from rawe.utils.bcolors import bcolors

# Local imports from this folder
import MHE
import NMPC
import carouselModel
        
###############################################################################
#
# Simulation configuration parameters (which we change often)
#
###############################################################################

# Visualization
visualiseStepsMhe = False
visualiseStepsMpc = False

# Number of simulation steps, added to the MHE horizon length later
nSimSteps = 100

# Maximum number of SQP iterations for MHE
nSqpMhe = 1

###############################################################################
#
# Load measurement data from a file
#
###############################################################################

# With high-speed winch data
data = sio.loadmat('dataset_20140211_175800_dmhe_testing.mat')
data = sio.loadmat('dataset_20140211_200325_dmhe_testing.mat')

# #samples, representing camera delay
nDelay = 2

# Sampling time, 50 Hz as in the datasets
Ts = 0.04

# @ 25 Hz, from 2014
measStart = 12400 + 1000  # 25 * 80
measEnd = measStart + 2000

# Read data to temporary data structures
time = data['dataset'][0, 0]['time']
imu_first = data['dataset'][0, 0]['imu_first']
imu_avg = data['dataset'][0, 0]['imu_avg']
enc_data = data['dataset'][0, 0]['enc_data']
cam_led = data['dataset'][0, 0]['cam_led']
cam_pose = data['dataset'][0, 0]['cam_pose']
cam_flag = data['dataset'][0, 0]['cam_flag']
control_surf = data['dataset'][0, 0]['controls']
winch_data = data['dataset'][0, 0]['winch_data']
winch_delay = data['dataset'][0, 0]['winch_delay']
winch_flag = data['dataset'][0, 0]['winch_flag']

#
# Trim the data set
#

while cam_flag[measStart] != 1:
    measStart += 1

measurements = {}
measurements['time'] = time[measStart: measEnd, :]
measurements['marker_positions'] = cam_led[measStart: measEnd, :]

measurements['cam_flag'] = cam_flag[measStart: measEnd, :]

measurements['pfm_position'] = cam_pose[measStart: measEnd, :3]
# TODO convert pfm_dcm data to 3x3 matrix
measurements['pfm_dcm'] = cam_pose[measStart: measEnd, 3:]

# measurements['xyz'] = cam_pose[measStart+nDelay:measEnd+nDelay:nDelay,:]
measurements['cos_delta'] = enc_data[measStart: measEnd, 2]
measurements['sin_delta'] = -enc_data[measStart: measEnd, 1]

# For the IMU data there are two variants:

# 1. First measurements from the interval (k-1, k]
# measurements['IMU_angular_velocity'] = imu_first[measStart:measEnd:nDelay,:3]
# measurements['IMU_acceleration'] = imu_first[measStart:measEnd:nDelay,3:]

# 2. Averaged measurements on the interval (k-1, k]
measurements['IMU_angular_velocity'] = imu_avg[measStart: measEnd, :3]
measurements['IMU_acceleration'] = imu_avg[measStart: measEnd, 3:]

measurements['aileron'] = control_surf[measStart: measEnd, 0]
measurements['elevator'] = control_surf[measStart: measEnd, 2]

measurements['r'] = winch_data[measStart: measEnd, 0]
measurements['r_delay'] = winch_delay[measStart: measEnd, 0]
measurements['r_flag'] = winch_flag[measStart: measEnd, 0]

measurements['dr'] = control_surf[measStart: measEnd, 2] * 0
measurements['ddr'] = control_surf[measStart: measEnd, 2] * 0

###############################################################################
#
# MHE solver configuration
#
###############################################################################

# Construct the object 
mheRT = MHE.makeMheRT(Ts = Ts)

# Define actual weights for the MHE
diagWeightNY = np.array([])
for name in mheRT.ocp.yxNames + mheRT.ocp.yuNames:
    diagWeightNY = np.append(diagWeightNY,
            np.ones(mheRT.ocp.dae[name].shape) * MHE.mheWeights[ name ])
mheNY = diagWeightNY.shape[ 0 ]

diagWeightNYN = np.array([])
for name in mheRT.ocp.yxNames:
    diagWeightNYN = np.append(diagWeightNYN,
            np.ones(mheRT.ocp.dae[name].shape) * MHE.mheWeights[ name ])
mheNYN = diagWeightNYN.shape[ 0 ]

mheRT.y = np.zeros(mheRT.y.shape)
mheRT.yN = np.zeros(mheRT.yN.shape)

defaultDiagWeightNY = np.array(diagWeightNY)
defaultDiagWeightNY[ :12 ] *= 0  # Camera measurements
defaultDiagWeightNY[ 22 ] *= 0  # Cable measurement
for n in range(MHE.mheHorizonN):
    mheRT.S[n * mheNY: (n + 1) * mheNY, :] = np.diag(defaultDiagWeightNY)

diagWeightNYN[ :12 ] *= 0  # Camera measurements
diagWeightNYN[ 22 ] *= 0  # Cable measurement
mheRT.SN = np.diag(diagWeightNYN)

###############################################################################
#
# NMPC configuration
#
###############################################################################

mpcRT = NMPC.makeNmpcRT(Ts = Ts)

# Weights
mpcDiagWeightX = []
mpcDiagWeightU = []
for name in mpcRT.ocp.dae.xNames():
    mpcDiagWeightX.append( NMPC.mpcWeights[name] )
for name in mpcRT.ocp.dae.uNames():
    mpcDiagWeightU.append( NMPC.mpcWeights[name] )
mpcRT.S = np.diag(mpcDiagWeightX + mpcDiagWeightU)
mpcRT.SN = np.diag( mpcDiagWeightX ) * 100

# utility function
def getDeltaRange(Ts, delta, omega, kRange):
    return np.array([delta + k * Ts * omega for k in kRange])

# reference
def setMpcReference(sin_delta, cos_delta, omega, steadyState):
    """
    A function for creating the reference for the NMPC
    """
    nx = len( mpcRT.ocp.dae.xNames() )
    
    delta = np.arctan2(sin_delta, cos_delta)
    
    def getDeltaRangeMpc(Ts, delta, omega):
        return getDeltaRange(Ts, delta, omega, range(0, NMPC.mpcHorizonN + 1))
    
    for k, name in enumerate(mpcRT.ocp.dae.xNames()):
        if name == 'sin_delta':
            yall = np.sin( getDeltaRangeMpc(Ts, delta, omega) )
            y = yall[:-1]
            yn = yall[-1]
        elif name == 'cos_delta':
            yall = np.cos( getDeltaRangeMpc(Ts, delta, omega) )
            y = yall[:-1]
            yn = yall[-1]
        else:
            y = steadyState[name]
            yn = y
            
        mpcRT.y[:, k] = y
        mpcRT.yN[k] = yn
    
    for k, name in enumerate(mpcRT.ocp.dae.uNames()):
        mpcRT.y[:, k + nx] = steadyState[name]

###############################################################################
#
# Get the initial guess for the MHE & NMPC by calculating a steady state
#
###############################################################################

# Get the plane configuration parameters
conf = makeConf()
# conf['stabilize_invariants'] = True
dae = carouselModel.makeModel(makeConf(), propertiesDir = '../../properties')

# Reference parameters
# refP = {'r0': 1.275, # [m], cable length used for SS calculations
#         'ddelta0': -4.0, # [rad/s], speed used for SS calculations
#         }

refP = {'r0': 2.0,  # [m], cable length used for SS calculations
        'ddelta0':-5,  # [rad/s], speed used for SS calculations
        }

# Get the steady state
steadyState, dSS = getSteadyState(dae, conf, refP['ddelta0'], refP['r0'])

# Initialize differential state variables
for k, name in enumerate(mheRT.ocp.dae.xNames()):
    mheRT.x[:, k] = steadyState[name]
    mpcRT.x[:, k] = steadyState[name]

# Initialize algebraic variables
for k, name in enumerate(mheRT.ocp.dae.zNames()):
    mheRT.z[:, k] = steadyState[name]
    mpcRT.z[:, k] = steadyState[name]

# Initialize controls
for k, name in enumerate(mheRT.ocp.dae.uNames()):
    mheRT.u[:, k] = steadyState[name]
    mpcRT.u[:, k] = steadyState[name]
    
mheRT.initializeNodesByForwardSimulation()

# For the NMPC we cannot do that since that depends on the actual
# angle returned by the MHE
# mpcRT.initializeNodesByForwardSimulation()

###############################################################################
#
# Simulation visualisation stuff
#
###############################################################################

if visualiseStepsMhe == True:
    fig1 = plt.figure()
    fig1.suptitle("diamonds: measurement, circles: projection")
    
    sinCosPlt = fig1.add_subplot(3, 1, 1)
    # sinCosPlt.set_title("Sine and cos of delta")
    
    gyroPlt = fig1.add_subplot(3, 1, 2)
    # gyroPlt.set_title("Gyroscope measurements")
    
    acclPlt = fig1.add_subplot(3, 1, 3)
    # acclPlt.set_title("Accelerometers measurements")
    
    box = sinCosPlt.get_position()
    sinCosPlt.set_position([box.x0, box.y0, box.width * 0.8, box.height])
    
    box = gyroPlt.get_position()
    gyroPlt.set_position([box.x0, box.y0, box.width * 0.8, box.height])
    
    box = acclPlt.get_position()
    acclPlt.set_position([box.x0, box.y0, box.width * 0.8, box.height])
    
    ledPlt = []
    fig4 = plt.figure()
    fig4.suptitle("Camera measurements. diamonds: measurement, circles: projection")
    for row in range(2):
        for col in range(6):
            ledPlt.append(fig4.add_subplot(2, 6, row * 6 + col + 1))
            
    # For visualisation
    

def visualizeMhe(what, num):
    
#     visualizeMhe(scY, scYN, mheRT.y, mheRT.yN, itSim)

    def joinStuff(a, b):
        return np.concatenate((a.flatten(), np.array([b])))
    
    projY = np.zeros(mheRT.y.shape)
    for k in range(mheRT.ocp.N):
        projY[k, :] = np.concatenate([what.computeYX(mheRT.x[k, :]),
                                      what.computeYU(mheRT.u[k, :])
                                      ])
    projYN = what.computeYX(what.x[-1, :])
    
    measY = what.y
    measYN = what.yN

    # Plot sin and cos of the carousel angle
    sinCosPlt.clear()
    p1, = sinCosPlt.plot(joinStuff(projY[:, 12], projYN[ 12 ]), '-ob')
    sinCosPlt.plot(joinStuff(measY[:, 12], measYN[ 12 ]), '-db')

    p2, = sinCosPlt.plot(joinStuff(projY[:, 13], projYN[ 13 ]), '-og')
    sinCosPlt.plot(joinStuff(measY[:, 13], measYN[ 13 ]), '-dg')

    sinCosPlt.legend([p1, p2], ['$\cos\ \delta$', '$\sin\ \delta$'], bbox_to_anchor = (1.05, 1), loc = 2)

    sinCosPlt.axes.get_xaxis().set_ticklabels([])
    sinCosPlt.set_ylim(-1.5, 1.5)

    gyroPlt.clear()

    p1, = gyroPlt.plot(joinStuff(projY[:, 14], projYN[ 14 ]), '-ob')
    gyroPlt.plot(joinStuff(measY[:, 14], measYN[ 14 ]), '-db')

    p2, = gyroPlt.plot(joinStuff(projY[:, 15], projYN[ 15 ]), '-og')
    gyroPlt.plot(joinStuff(measY[:, 15], measYN[ 15 ]), '-dg')

    p3, = gyroPlt.plot(joinStuff(projY[:, 16], projYN[ 16 ]), '-or')
    gyroPlt.plot(joinStuff(measY[:, 16], measYN[ 16 ]), '-dr')

    gyroPlt.legend([p1, p2, p3], ['$w_x [rad/s]$', '$w_y [rad/s]$', '$w_z [rad/s]$'], bbox_to_anchor = (1.05, 1), loc = 2)

    gyroPlt.axes.get_xaxis().set_ticklabels([])
    gyroPlt.set_ylim(-3, 6)

#    fig2.canvas.draw()
#    fig2.savefig("mhe_gyro_" + str( num) + ".png", transparent=True)

    acclPlt.clear()

    p1, = acclPlt.plot(joinStuff(projY[:, 17], projYN[ 17 ]), '-ob')
    acclPlt.plot(joinStuff(measY[:, 17], measYN[ 17 ]), '-db')

    p2, = acclPlt.plot(joinStuff(projY[:, 18], projYN[ 18 ]), '-og')
    acclPlt.plot(joinStuff(measY[:, 18], measYN[ 18 ]), '-dg')

    p3, = acclPlt.plot(joinStuff(projY[:, 19], projYN[ 19 ]), '-or')
    acclPlt.plot(joinStuff(measY[:, 19], measYN[ 19 ]), '-dr')

    acclPlt.legend([p1, p2, p3], ['$a_x [m/s^2]$', '$a_y  [m/s^2]$', '$a_z    [m/s^2]$'], bbox_to_anchor = (1.05, 1), loc = 2)
    acclPlt.set_xlabel("Horizon")
#    fig3.canvas.draw()
#    fig3.savefig("mhe_accl_" + str( num ) + ".png", transparent=True)

    acclPlt.set_ylim(-60, 10)

    fig1.canvas.draw()
#    fig1.savefig("mhe_sin_cos_gyro_accl" + str( num) + ".png", transparent=True)

    colors = ['r', 'g', 'b']
    for f in range(2):
        for m in range(3):
            for xy in range(2):
                ind = f * 6 + m * 2 + xy
                ledPlt[ ind ].clear()
                ledPlt[ ind ].plot(joinStuff(projY[:, ind], projYN[ ind ]), 'o' + colors[ m ])
                ledPlt[ ind ].plot(joinStuff(measY[:, ind], measYN[ ind ]), 'd' + colors[ m ])

                if ind in range(6):
                    ledPlt[ ind ].axes.get_xaxis().set_ticklabels([])

                if ind not in [0, 6]:
                    ledPlt[ ind ].axes.get_yaxis().set_ticklabels([])

                ledPlt[ ind ].set_ylim(-1, 1200)

    fig4.canvas.draw()
#    fig4.savefig("mhe_cam_" + str( num) + ".png", transparent=True)

    raw_input("Press enter to continue")
    
#
# Some stuff for NMPC visualisation
#

mpcPos = None
mpcCtrl = None

###############################################################################
#
# After several light years we finally reached the code that runs the simulation
#
###############################################################################

print "\n\n*** Alright, now we start the simulation ***\n\n"

# Number of simulations steps
nSim = MHE.mheHorizonN + nSimSteps

plt.ion()
plt.show()

mpcInitialized = False

for itSim in xrange( nSim ):
#     print "Simulation step: " + str(itSim + 1)

    #
    # Alright, fill the estimator with measurements
    #

    led_data = np.array(measurements['marker_positions'][ itSim ])
    led_data_valid = (led_data > 0) * measurements['cam_flag'][ itSim ]
    led_data *= led_data_valid

    ledS = np.eye(12) * led_data_valid * MHE.mheWeights['marker_positions']

    cableMeas = measurements['r'][ itSim ]
    cableMeasFlag = measurements['r_flag'][ itSim ]
    cableMeasDelay = measurements['r_delay'][ itSim ]

    cableMeas *= cableMeasFlag
    cableMeasWeight = cableMeasFlag * MHE.mheWeights[ "r" ]

    y = np.concatenate([
            np.zeros(led_data.shape),
            np.array(measurements['cos_delta'][ itSim ]).flatten(),
            np.array(measurements['sin_delta'][ itSim ]).flatten(),
            np.array(measurements['IMU_angular_velocity'][ itSim ]).flatten(),
            np.array(measurements['IMU_acceleration'][ itSim ]).flatten(),
            np.array(measurements['aileron'][ itSim ]).flatten(),
            np.array(measurements['elevator'][ itSim ]).flatten(),
            ])
    # Append r, dr, ddr
    y = np.append(y, np.array([0, 0, 0]).flatten())
    yN = y
    # Append controls which we actually don't know
    y = np.append(y, np.zeros([1, len(MHE.measU)]).flatten())

    if itSim < MHE.mheHorizonN:
        # Just fill in the measurements

        mheRT.y[itSim, :] = y

        ledInd = itSim - nDelay
        if ledInd >= 0:
            mheRT.y[ledInd, :12] = led_data
            mheRT.S[ledInd * mheNY: ledInd * mheNY + 12, :12] = ledS

        cableInd = itSim - cableMeasDelay
        if cableInd >= 0:
            mheRT.y[cableInd, 22] = cableMeas
            mheRT.S[cableInd * mheNY + 22, 22] = cableMeasWeight

        runMhe = 0

    elif itSim == MHE.mheHorizonN:
        # Initialize

        mheRT.yN = yN

        ledInd = itSim - nDelay
        mheRT.y[ledInd, :12] = led_data
        mheRT.S[ledInd * mheNY: ledInd * mheNY + 12, :12] = ledS

        cableInd = itSim - cableMeasDelay
        if cableInd < MHE.mheHorizonN:
            mheRT.y[cableInd, 22] = cableMeas
            mheRT.S[cableInd * mheNY + 22, 22] = cableMeasWeight
            mheRT.SN[22, 22] = 0.0
        else:
            mheRT.yN[ 22 ] = cableMeas
            mheRT.SN[22, 22] = cableMeasWeight

        runMhe = 1

    else:
        # Shift weighting matrices S_{0..N - 1}
        for n in xrange(MHE.mheHorizonN - 1):
            mheRT.S[n * mheNY: (n + 1) * mheNY, :] = mheRT.S[(n + 1) * mheNY: (n + 2) * mheNY, :]
        # SN -> upper left block of S_{N - 1}
        mheRT.S[(MHE.mheHorizonN - 1) * mheNY: (MHE.mheHorizonN - 1) * mheNY + mheNYN, : mheNYN] = mheRT.SN

        # Shift measurements
        for n in range(MHE.mheHorizonN - 1):
            mheRT.y[n, :] = mheRT.y[n + 1, :]
        mheRT.y[-1, :mheNYN] = mheRT.yN

        #
        # Put new measurements
        #

        mheRT.yN = yN

        ledInd = MHE.mheHorizonN - nDelay
        mheRT.y[ledInd, :12] = led_data
        mheRT.S[ledInd * mheNY: ledInd * mheNY + 12, :12] = ledS

        cableInd = MHE.mheHorizonN - cableMeasDelay
        if cableInd < MHE.mheHorizonN:
            mheRT.y[cableInd, 22] = cableMeas
            mheRT.S[cableInd * mheNY + 22, 22] = cableMeasWeight
            mheRT.SN[22, 22] = 0.0
        else:
            mheRT.yN[ 22 ] = cableMeas
            mheRT.SN[22, 22] = cableMeasWeight

        #
        # Shift states and controls
        #
        mheRT.shiftXZU(strategy = 'simulate')

        runMhe = 1

    mheFailed = False
    mpcFailed = False
    
    if runMhe:
        for itSqp in range(nSqpMhe):
            try:
                mheRT.preparationStep()
                mheRT.feedbackStep()
            except Exception as e:
                print "MHE: ", e

                print "x"
                print mheRT.x
                print "u"
                print mheRT.u

                mheFailed = True

            #
            # Visualise current step
            #
            if visualiseStepsMhe:
                visualizeMhe(mheRT, itSim)

            if mheFailed == True:
                break

        mheRT.log()
        
        #
        # NMPC execution
        #
        if mheFailed is False and itSim > MHE.mheHorizonN + 5:
            # Current state estimate
            x0 = mheRT.x[-1, :]
            
            sinIndex = mpcRT.xNames().index( "sin_delta" )
            cosIndex = mpcRT.xNames().index( "cos_delta" )
            ddeltaIndex = mpcRT.xNames().index( "ddelta" )
            
            if mpcInitialized is False:
                
                for k in xrange(NMPC.mpcHorizonN + 1):
                    mpcRT.x[k, sinIndex] = x0[ sinIndex ]
                    mpcRT.x[k, cosIndex] = x0[ cosIndex ]
                
                mpcRT.initializeNodesByForwardSimulation()
                
                mpcInitialized = True
            
            else:
                # OK this is normal run, we do shifting and stuff
                mheRT.shiftXZU(strategy = 'simulate')
            
            mpcRT.x0 = x0    
#             setMpcReference(x0[ sinIndex ], x0[ cosIndex ], x0[ ddeltaIndex ], steadyState)
            setMpcReference(x0[ sinIndex ], x0[ cosIndex ], steadyState['ddelta'], steadyState)
                
            try:
                mpcRT.preparationStep()
                mpcRT.feedbackStep()
            except Exception as e:
                print "MPC: ", e
                mpcFailed = True
                
            mpcRT.log()
                
            if visualiseStepsMpc is True:
                mpcPos = mpcRT.plot(['x', 'y', 'z'], when = 'horizon', fig = mpcPos)
                mpcPos.canvas.draw()
                
                mpcCtrl = mpcRT.subplot([['aileron', 'elevator'], ['daileron', 'delevator']], when = 'horizon', fig = mpcCtrl)
                mpcCtrl.canvas.draw()
                
                plt.show()
                
                import time
                time.sleep( 0.25 )
#                 raw_input("Press enter to continue")
            
    if itSim % 10 == 0:
        print bcolors.HEADER + "sim it".rjust( 6 ),\
              "mhe kkt".rjust( 13 ), "mhe obj".rjust( 13 ), "mpc kkt".rjust( 13 ), "mpc obj".rjust( 13 ), \
              "mpc nwsr".rjust( 9 ) + \
              bcolors.ENDC
              
    print "%6d  %s%4e  %4e  %s%4e  %4e  %8d" \
        % (itSim + 1, bcolors.OKGREEN if runMhe == 1 else bcolors.WARNING, \
           mheRT.getKKT(), mheRT.getObjective(),\
           bcolors.OKBLUE if mpcInitialized is True else bcolors.WARNING, mpcRT.getKKT(), mpcRT.getObjective(),\
           mpcRT.getNWSR()) \
        + bcolors.ENDC
    
    if mheFailed == True or mpcFailed == True:
        break

###############################################################################
#
# Post processing: plotting of the simulation results
#
###############################################################################

plt.ioff()

mheRT.subplot([['x', 'y', 'z'], ['dx', 'dy', 'dz']])
mheRT.subplot([['e11', 'e12', 'e13'], ['e21', 'e22', 'e23'], ['e31', 'e32', 'e33']])
mheRT.plot(['w_bn_b_x', 'w_bn_b_y', 'w_bn_b_z'])

mheRT.subplot([['aileron', 'elevator'], ['daileron', 'delevator']])
mheRT.subplot([['cos_delta', 'sin_delta'], ['ddelta'], ['motor_torque']])

# Measurements for cable length are usually delayed by one sample
mheRT.subplot([['r'], ['dr'], ['ddr'], ['dddr']], when = MHE.mheHorizonN - 1)

mheRT.subplot([['c'], ['cdot'], ['ConstDelta']])
mheRT.plot(['ConstR1', 'ConstR2', 'ConstR3', 'ConstR4', 'ConstR5', 'ConstR6'])
mheRT.subplot([['_kkt'], ['_objective'], ['_prep_time', '_fb_time']])

mheRT.plot(['IMU_acceleration'])
mheRT.plot(['IMU_angular_velocity'])
# Marker positions are usually delayed by 2 sample (@25 Hz samling rate)
mheRT.plot(['marker_positions'], when = MHE.mheHorizonN - 2)

mpcRT.subplot([['aileron'], ['elevator']], when = 1)
mpcRT.subplot([['daileron'], ['delevator']], when = 0)
mpcRT.subplot([['cos_delta', 'sin_delta'], ['ddelta'], ['motor_torque']], when = 1)

mpcRT.subplot([['c'], ['cdot'], ['ConstDelta']], when = 1)
mpcRT.plot(['ConstR1', 'ConstR2', 'ConstR3', 'ConstR4', 'ConstR5', 'ConstR6'], when = 1)
mpcRT.subplot([['_kkt'], ['_objective'], ['_prep_time', '_fb_time']])

plt.show()
