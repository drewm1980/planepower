import numpy
import rawe

import NMPC
import MHE
from bufStuff.protobufBridgeWrapper import ProtobufBridge

from highwind_carousel_conf import getConf
import carouselModel
from rawekite.carouselSteadyState import getSteadyState

from common_conf import Ts

conf = getConf()
conf['stabilize_invariants'] = True
daeSim = carouselModel.makeModel(conf)

# Create the MPC class
mheRT = MHE.makeMheRT()
mpcRT = NMPC.makeNmpcRT(daeSim)

mheSigmas = {'cos_delta':1.0, 'sin_delta':1.0,
             'IMU_angular_velocity':16.0,
             'IMU_acceleration':100.0,
             'marker_positions':1e3,
             'r':1.0,
             'aileron':0.05,
             'elevator':0.05,
             'daileron':10.0,
             'dmotor_torque':10.0,
             'delevator':10.0,
             'dddr':10.0}

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
Tf = 500.0   # Simulation duration

# Reference parameters
refP = {'r0':1.2,
        'ddelta0':2*numpy.pi,
        'z0':-0.1}

# utility function
def getDeltaRange(delta0, kRange):
    return numpy.array([delta0 + k*Ts*refP['ddelta0'] for k in kRange])
def getDeltaRangeMhe(delta0):
    return getDeltaRange(delta0, range(-MHE.mheHorizonN, 1))
def getDeltaRangeMpc(delta0):
    return getDeltaRange(delta0, range(0, NMPC.mpcHorizonN + 1))

# Create a sim and initalize it
sim = rawe.RtIntegrator(daeSim, ts=Ts, options=MHE.mheIntOpts)
steadyState,_ = getSteadyState(daeSim, conf, refP['ddelta0'], refP['r0'], refP['z0'])
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
    mheRT.y[k,:] = mheRT.computeY(mheRT.x[k,:], mheRT.u[k,:])
mheRT.yN = mheRT.computeYN(mheRT.x[-1,:])

# weights
Weight_ = numpy.array([])
for name in mheRT.ocp.yNames:
    Weight_ = numpy.append( Weight_, numpy.ones(mheRT.ocp.dae[name].shape)*(1.0/mheSigmas[name]**2) )
mheRT.S  = numpy.diag(Weight_)
Weight_ = numpy.array([])
for name in mheRT.ocp.yNNames:
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
mpcRT.y = numpy.hstack((mpcRT.x[:-1,:], mpcRT.u))
mpcRT.yN = mpcRT.x[1,:]

# weights
Q = []
R = []
for name in mpcRT.ocp.dae.xNames():
    Q.append(MPCweights[name])
for name in mpcRT.ocp.dae.uNames():
    R.append(MPCweights[name])
mpcRT.S = numpy.diag( Q + R )
mpcRT.Q = numpy.diag( Q )
mpcRT.R = numpy.diag( R )


# Simulation loop
time = 0

pbb = ProtobufBridge()
log = []
while time < Tf:
    # run MHE
    mheIt = 0
    while True:
        mheRT.preparationStep()
        mheRT.feedbackStep()
        mheIt += 1
        if mheRT.getKKT() < 1e-2:
            break

    # set mhe xN as mpc estimate
    mpcRT.x0 = mheRT.x[-1,:]
    
    # run MPC
    #mpcRT.preparationStep()
    #mpcRT.feedbackStep()

    # set the next control
    sim.u = mpcRT.u[0,:]

    # first compute the new final full measurement
    y_Nm1 = mheRT.computeY(sim.x, sim.u)
    yN = mheRT.computeYN(sim.x)

    # send the protobuf and log the message
    pbb.setMhe(mheRT)
    pbb.setMpc(mpcRT)
    pbb.setSimState(sim.x, sim.z, sim.u, y_Nm1, yN, mheRT.computeOutputs(sim.x,sim.u))
    log.append( pbb.sendMessage() )

    # step the simulation
    print "sim time: %5.1f,  mhe kkt: %.4e,  mhe iters: %2d,  mhe time: %.2e" % \
        (time, mheRT.getKKT(), mheIt, mheRT.preparationTime+mheRT.feedbackTime)
    sim.step()

    # first compute the final partial measurement
    mheRT.shiftXZU()

    # sensor measurements
    yN = mheRT.computeYN(sim.x)

#    # add noise to y_N
#    yN += numpy.concatenate(\
#         [numpy.random.randn(mheRT.ocp.dae[name].size())*mheSigmas[name]*0.01
#          for name in mheRT.ocp.yNNames])
#
#    # add noise to y_Nm1
#    y_Nm1 += numpy.concatenate(\
#         [numpy.random.randn(mheRT.ocp.dae[name].size())*mheSigmas[name]*0.01
#          for name in mheRT.ocp.yNames])

    # shift reference
    mheRT.simpleShiftReference(y_Nm1, yN)

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
