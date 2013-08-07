#!/usr/bin/env python

import numpy
import time
import rawe

import NMPC
import MHE
from bufStuff.protobufBridgeWrapper import ProtobufBridge

from rawe.models.arianne_conf import makeConf
import carouselModel
from rawekite.carouselSteadyState import getSteadyState

from common_conf import Ts

conf = makeConf()
conf['stabilize_invariants'] = True
daeSim = carouselModel.makeModel(conf)

# Create the MPC class
mheRT = MHE.makeMheRT()
mpcRT = NMPC.makeNmpcRT(daeSim)

mheSigmas = {'cos_delta':1e-1, 'sin_delta':1e-1,
             'IMU_angular_velocity':1.0,
             'IMU_acceleration':10.0,
             'marker_positions':1e2,
             'r':1e-1,
             'aileron':1e-2,
             'elevator':1e-2,
             'daileron':1e-4,
             'dmotor_torque':1e-4,
             'delevator':1e-4,
             'dddr':1e-4}

# Define the weights
mpcSigmas = {}
for name in ['x','y','z']: mpcSigmas[name] = 1.0
for name in ['dx','dy','dz']: mpcSigmas[name] = 1.0
for name in ['e11', 'e12', 'e13', 'e21', 'e22', 'e23', 'e31', 'e32', 'e33']: mpcSigmas[name] = 1.0
for name in ['w_bn_b_x','w_bn_b_y','w_bn_b_z']: mpcSigmas[name] = 1.0
mpcSigmas['r'] = 1.0
mpcSigmas['dr'] = 1.0
mpcSigmas['ddr'] = 1.0
mpcSigmas['cos_delta'] = mpcSigmas['sin_delta'] = 1.0
mpcSigmas['ddelta'] = 1.0
mpcSigmas['motor_torque'] = 1e1
mpcSigmas['aileron'] = mpcSigmas['elevator'] = 1e-2

mpcSigmas['dmotor_torque'] = 1.0
mpcSigmas['dddr'] = 1.0
mpcSigmas['daileron'] = mpcSigmas['delevator'] = 1.0

mpcWeights = {}
for name in mpcSigmas:
    mpcWeights[name] = 1.0/mpcSigmas[name]**2
#for name in [ 'x', 'y', 'z']: mpcWeights[name] *= 1e-1
for name in ['dx','dy','dz']: mpcWeights[name] *= 5e-1

## Simulation parameters
Tf = 500.0   # Simulation duration

# Reference parameters
refP = {'r0':2,
        'ddelta0':2*numpy.pi}

# utility function
def getDeltaRange(delta0, kRange):
    return numpy.array([delta0 + k*Ts*refP['ddelta0'] for k in kRange])
def getDeltaRangeMhe(delta0):
    return getDeltaRange(delta0, range(-MHE.mheHorizonN, 1))
def getDeltaRangeMpc(delta0):
    return getDeltaRange(delta0, range(0, NMPC.mpcHorizonN + 1))

# Create a sim and initalize it
sim = rawe.RtIntegrator(daeSim, ts=Ts, options=MHE.mheIntOpts)
steadyState,_ = getSteadyState(daeSim, conf, refP['ddelta0'], refP['r0'])

sim.x = steadyState
sim.z = steadyState
sim.u = steadyState
sim.p = steadyState

####### initialize MHE #########
# x/z/u
for k,name in enumerate(mheRT.ocp.dae.xNames()):
    mheRT.x[:,k] = steadyState[name]
    if name == 'sin_delta':
        mheRT.x[:,k] = numpy.sin(getDeltaRangeMhe(0))
    if name == 'cos_delta':
        mheRT.x[:,k] = numpy.cos(getDeltaRangeMhe(0))
for k,name in enumerate(mheRT.ocp.dae.zNames()):
    mheRT.z[:,k] = steadyState[name]
for k,name in enumerate(mheRT.ocp.dae.uNames()):
    mheRT.u[:,k] = steadyState[name]

# expected measurements
for k in range(mheRT.ocp.N):
    mheRT.y[k,:] = numpy.concatenate((mheRT.computeYX(mheRT.x[k,:]),
                                      mheRT.computeYU(mheRT.u[k,:])))
mheRT.yN = mheRT.computeYX(mheRT.x[-1,:])

# weights
Weight_ = numpy.array([])
for name in mheRT.ocp.yxNames+mheRT.ocp.yuNames:
    Weight_ = numpy.append( Weight_, numpy.ones(mheRT.ocp.dae[name].shape)*(1.0/mheSigmas[name]**2) )
mheRT.S  = numpy.diag(Weight_)
Weight_ = numpy.array([])
for name in mheRT.ocp.yxNames:
    Weight_ = numpy.append( Weight_, numpy.ones(mheRT.ocp.dae[name].shape)*(1.0/mheSigmas[name]**2) )
mheRT.SN  = numpy.diag(Weight_)


####### initialize MPC #########
# x/z/u
for k,name in enumerate(mpcRT.ocp.dae.xNames()):
    mpcRT.x[:,k] = steadyState[name]
    if name == 'sin_delta':
        mpcRT.x[:,k] = numpy.sin(getDeltaRangeMpc(0))
    if name == 'cos_delta':
        mpcRT.x[:,k] = numpy.cos(getDeltaRangeMpc(0))
for k,name in enumerate(mpcRT.ocp.dae.zNames()):
    mpcRT.z[:,k] = steadyState[name]
for k,name in enumerate(mpcRT.ocp.dae.uNames()):
    mpcRT.u[:,k] = steadyState[name]
# x0
mpcRT.x0 = mpcRT.x[0,:]

# reference
def setMpcReference(delta,state):
    nx = len(mpcRT.ocp.dae.xNames())
    for k,name in enumerate(mpcRT.ocp.dae.xNames()):
        if name == 'sin_delta':
            yall = numpy.sin(getDeltaRangeMpc(delta))
            y = yall[:-1]
            yn = yall[-1]
        elif name == 'cos_delta':
            yall = numpy.cos(getDeltaRangeMpc(delta))
            y = yall[:-1]
            yn = yall[-1]
        else:
            y = state[name]
            yn = y
        mpcRT.y[:,k] = y
        mpcRT.yN[k] = yn
    for k,name in enumerate(mpcRT.ocp.dae.uNames()):
        mpcRT.y[:,k+nx] = state[name]
setMpcReference(0,steadyState)

# weights
Q = []
R = []
for name in mpcRT.ocp.dae.xNames():
    Q.append(mpcWeights[name])
for name in mpcRT.ocp.dae.uNames():
    R.append(mpcWeights[name])
mpcRT.S = numpy.diag( Q + R )
mpcRT.SN = numpy.diag( Q )*10
########mpcRT.Q = numpy.diag( Q )
########mpcRT.R = numpy.diag( R )


# Simulation loop
current_time = 0

pbb = ProtobufBridge()
log = []
steadyState2,_ = getSteadyState(daeSim, conf, refP['ddelta0'], refP['r0']+3)
while current_time < Tf:
    # run MHE
    mheIt = 0
    while True:
        mheRT.preparationStep()
        mheRT.feedbackStep()
        mheIt += 1
        break
        if mheRT.getKKT() < 1e-9:
            break
        assert mheIt < 100, "mhe took too may iterations"

    # set mhe xN as mpc estimate
    mpcRT.x0 = mheRT.x[-1,:]
#    mpcRT.x0 = sim.x # perfect estimation

    # set mpc reference
    if round(current_time / 5.0) % 2 == 0:
        setMpcReference(current_time*refP['ddelta0'], steadyState)
    else:
        setMpcReference(current_time*refP['ddelta0'], steadyState2)

    # run MPC
    mpcIt = 0
    while True:
        mpcRT.preparationStep()
        mpcRT.feedbackStep()
        mpcIt += 1
        break
        if mpcRT.getKKT() < 1e0:
            break
        assert mpcIt < 100, "mpc took too may iterations"

    # set the next control
    sim.u = mpcRT.u[0,:]

    # first compute the new final full measurement
    yxNsim = mheRT.computeYX(sim.x)
    yuNsim = mheRT.computeYU(sim.u)

    # send the protobuf and log the message
    pbb.setMhe(mheRT)
    pbb.setMpc(mpcRT)
    pbb.setSimState(sim.x, sim.z, sim.u, yxNsim, yuNsim, mheRT.computeOutputs(sim.x,sim.u))
    log.append( pbb.sendMessage() )

    # step the simulation
    print "sim time: %6.2f | mhe {kkt: %.4e, iter: %2d, time: %.2e} | mpc {kkt: %.4e, iter: %2d, time: %.2e }" \
        % (current_time,
           mheRT.getKKT(), mheIt, mheRT.preparationTime + mheRT.feedbackTime,
           mpcRT.getKKT(), mpcIt, mpcRT.preparationTime + mpcRT.feedbackTime)
    sim.step()
    time.sleep(Ts)

    # first compute the final partial measurement
    mheRT.shiftXZU(strategy='copy', xEnd=mpcRT.x[1,:], uEnd=mpcRT.u[0,:])
    mpcRT.shiftXZU()

    # sensor measurements
    yN = mheRT.computeYX(sim.x)
    y_Nm1 = numpy.concatenate((yxNsim, yuNsim))

    # add noise to y_N
    yN += numpy.concatenate(\
         [numpy.random.randn(mheRT.ocp.dae[name].size())*mheSigmas[name]*0.03
          for name in mheRT.ocp.yxNames])

    # add noise to y_Nm1
    y_Nm1 += numpy.concatenate(\
         [numpy.random.randn(mheRT.ocp.dae[name].size())*mheSigmas[name]*0.03
          for name in mheRT.ocp.yxNames+mheRT.ocp.yuNames])

    # shift reference
    mheRT.simpleShiftReference(y_Nm1, yN)

    current_time += Ts

import sys; sys.exit()
plt.ion()

plotter = Plotter(sim,mheRT,mpcRT)

plotter.subplot([['x','y','z'],['dx','dy','dz']],what=['sim','mhe'])
plotter.subplot([['e11','e12','e13'],['e21','e22','e23'],['e31','e32','e33']],what=['sim','mhe'])
plotter.subplot(['w1','w2','w3'],what=['sim','mhe'])
plotter.subplot([['aileron','elevator'],['daileron','delevator']],what=['sim','mhe'])
plotter.subplot([['cos_delta','sin_delta'],['ddelta'],['motor_torque']],what=['sim','mhe'])
plotter.subplot([['r'],['dr'],['ddr']],what=['sim','mhe'])
plotter.subplot([['c'],['cdot']],what=['sim','mhe'])
plotter.subplot([['_kkt'],['_objective'],['_prep_time','_fb_time']],what=['mpc','mhe'])
plotter.plot(['_kkt'],what=['mpc','mhe'])
plotter.plot(['ConstR1','ConstR2','ConstR3','ConstR4','ConstR5','ConstR6'],what=['sim','mhe'])
plotter.plot(['ConstR1','ConstR2','ConstR3','ConstR4','ConstR5','ConstR6'],what=['sim'])
plotter.subplot([['c'],['cdot'],['ConstDelta']],what=['sim','mhe'])
plotter.subplot([['c'],['cdot'],['ConstDelta']],what=['sim'])


#mpcrt.plot(['x','v'],when='all')

plt.show()
