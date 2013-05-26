#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Mon Apr 22 15:48:33 2013

@author: mzanon
"""

import numpy
import rawe

import NMPC
import MHE
from mpc_mhe_utils import *
from bufStuff.protobufBridgeWrapper import ProtobufBridge

from highwind_carousel_conf import conf
import carouselModel
from rawekite.carouselSteadyState import getSteadyState

from common_conf import Ts

conf['stabilize_invariants'] = False
dae = carouselModel.makeModel(conf)
conf['stabilize_invariants'] = True
daeSim = carouselModel.makeModel(conf)

Covariance = {'marker_positions':1e3,
              'IMU_angular_velocity':1.,
              'IMU_acceleration':10.,
              'r':1.,
              'cos_delta':1., 'sin_delta':1.,
              'aileron':1e-2, 'elevator':1e-2,
              'daileron':1e-4, 'delevator':1e-4,
              'motor_torque':20.,
              'ddr':1e-4,
              'dmotor_torque':1.,
              'dddr':1e-4}
for name in dae.xNames() + dae.uNames():
    Covariance[name] = 1.

# Define the weights
Wp  = 1e-1
Wdp = 1e-1
We  = 1.
Ww  = 1.
Wr  = 1.
Wdr  = Wr
Wdelta = 1e2
Wddelta = Wdelta
Wae = 1e2
Wddr = 1.
Wmt = 1e-1

Wdmt = 1.
Wdddr = 1.
Wdae = 1.

MPCweights = {}
for name in ['x','y','z']: MPCweights[name] = Wp
for name in ['dx','dy','dz']: MPCweights[name] = Wdp
for name in ['e11', 'e12', 'e13', 'e21', 'e22', 'e23', 'e31', 'e32', 'e33']: MPCweights[name] = We
for name in ['w1','w2','w3']: MPCweights[name] = Ww
MPCweights['r'] = Wr
MPCweights['dr'] = Wdr
MPCweights['ddr'] = Wddr
MPCweights['cos_delta'] = MPCweights['sin_delta'] = Wdelta
MPCweights['ddelta'] = Wddelta
MPCweights['motor_torque'] = Wmt
MPCweights['aileron'] = MPCweights['elevator'] = Wae

MPCweights['dmotor_torque'] = Wdmt
MPCweights['dddr'] = Wdddr
MPCweights['daileron'] = MPCweights['delevator'] = Wdae

## Simulation parameters
Tf = 50.0   # Simulation duration

# Create the MPC class
mheRT = MHE.makeMheRT()
mpcRT = NMPC.makeNmpcRT(daeSim)

# Reference parameters
refP = {'r0':1.2,
        'ddelta0':2*np.pi,
        'z0':-0.1}

# utility function
def getDeltaRange(delta0, kRange):
    return np.array([delta0 + k*Ts*refP['ddelta0'] for k in kRange])
def getDeltaRangeMhe(delta0):
    return getDeltaRange(delta0, range(-MHE.mheHorizonN, 1))
def getDeltaRangeMpc(delta0):
    return getDeltaRange(delta0, range(0, NMPC.mpcHorizonN + 1))

# Create a sim and initalize it
sim = rawe.RtIntegrator(daeSim,ts=Ts,options=NMPC.mpcIntOpts)
steadyState,_ = getSteadyState(daeSim, conf, refP['ddelta0'], refP['r0'], refP['z0'])
sim.x = steadyState
sim.z = steadyState
sim.u = steadyState
sim.p = steadyState

####### initialize MHE #########
# x/z/u
for k,name in enumerate(dae.xNames()):
    mheRT.x[:,k] = steadyState[name]
    if name == 'sin_delta':
        mheRT.x[:,k] = np.sin(getDeltaRangeMhe(0))
    if name == 'cos_delta':
        mheRT.x[:,k] = np.cos(getDeltaRangeMhe(0))
for k,name in enumerate(dae.zNames()):
    mheRT.z[:,k] = steadyState[name]
for k,name in enumerate(dae.uNames()):
    mheRT.u[:,k] = steadyState[name]

# expected measurements
for k in range(mheRT.ocp.N):
    mheRT.y[k,:] = mheRT.computeY(mheRT.x[k,:], mheRT.u[k,:])
mheRT.yN = mheRT.computeYN(mheRT.x[-1,:])

# weights
Cov = np.array([])
for name in mheRT.ocp.yNames:
    Cov = numpy.append( Cov, numpy.ones(dae[name].shape)*Covariance[name] )
mheRT.S  = numpy.diag(1.0/Cov)
Cov = np.array([])
for name in mheRT.ocp.yNNames:
    Cov = numpy.append( Cov, numpy.ones(dae[name].shape)*Covariance[name] )
mheRT.SN  = numpy.diag(1.0/Cov)


####### initialize MPC #########
# x/z/u
for k,name in enumerate(dae.xNames()):
    mpcRT.x[:,k] = steadyState[name]
    if name == 'sin_delta':
        mpcRT.x[:,k] = np.sin(getDeltaRangeMpc(0))
    if name == 'cos_delta':
        mpcRT.x[:,k] = np.cos(getDeltaRangeMpc(0))
for k,name in enumerate(dae.zNames()):
    mpcRT.z[:,k] = steadyState[name]
for k,name in enumerate(dae.uNames()):
    mpcRT.u[:,k] = steadyState[name]

# x0
mpcRT.x0 = mpcRT.x[0,:]

# reference
mpcRT.y = numpy.hstack((mpcRT.x[:-1,:], mpcRT.u))
mpcRT.yN = mpcRT.x[1,:]

# weights
Q = []
R = []
for name in dae.xNames():
    Q.append(MPCweights[name])
for name in dae.uNames():
    R.append(MPCweights[name])
mpcRT.S = np.diag( Q + R )
mpcRT.Q = np.diag( Q )
mpcRT.R = np.diag( R )


# Simulation loop
time = 0

pbb = ProtobufBridge()
log = []
while time < Tf:
    # run MHE

    mheIt = 0
    while True:
        mheIt += 1
        
        mheRT.preparationStep()
        mheRT.feedbackStep()

        print "MHE exec time: ", str(mheRT.preparationTime + mheRT.feedbackTime)
        
        if mheRT.getKKT() < 1e-2:
            print "Time: ", time, "MHE its needed: ", mheIt
            break
            
    # set mhe xN as mpc estimate
    mpcRT.x0 = mheRT.x[-1,:]
    
    # run MPC
    #mpcRT.preparationStep()
    #mpcRT.feedbackStep()

    # set the next control
    sim.u = mpcRT.u[-1,:]

    # first compute the new final full measurement
    y_Nm1 = mheRT.computeY(sim.x, sim.u)
    yN = mheRT.computeYN(sim.x)
    y_Nm1 += numpy.random.random(y_Nm1.size)*0.01
    yN += numpy.random.random(yN.size)*0.01

    # send the protobuf and log the message
    pbb.setMhe(mheRT)
    pbb.setMpc(mpcRT)
    pbb.setSimState(sim.x, sim.z, sim.u, y_Nm1, yN, mheRT.computeOutputs(sim.x,sim.u))
    log.append( pbb.sendMessage() )

    # step the simulation
    print "sim time:",time,"mhe kkt:",mheRT.getKKT()
    sim.step()

    # first compute the final partial measurement
    mheRT.simpleShiftXZU()
    mheRT.simpleShiftReference(y_Nm1, mheRT.computeYN(sim.x))

#    SimulateAndShift(mpcRT,mheRT,sim,Rint,dae,conf,refP)
    time += Ts

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
