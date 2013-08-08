#!/usr/bin/env python

import numpy
#import time
import rawe
import matplotlib.pyplot as plt

import scipy

import MHE
#from bufStuff.protobufBridgeWrapper import ProtobufBridge

from mpc_mhe_utils import Plotter
from rawe.models.arianne_conf import makeConf
#from rawe.models.betty_conf import makeConf
import carouselModel
from rawekite.carouselSteadyState import getSteadyState

#data = scipy.io.loadmat('dataset_20130729_180717_kmhe_timings.mat')
data = scipy.io.loadmat('dataset_20130729_190735_kmhe_timings.mat')
time         = data['dataset'][0,0]['time']
imu_first    = data['dataset'][0,0]['imu_first']
imu_avg      = data['dataset'][0,0]['imu_avg']
enc_data     = data['dataset'][0,0]['enc_data']
cam_led      = data['dataset'][0,0]['cam_led']
cam_pose     = data['dataset'][0,0]['cam_pose']
cam_flag     = data['dataset'][0,0]['cam_flag']
las_data     = data['dataset'][0,0]['las_data']
control_surf = data['dataset'][0,0]['controls']


start = 4612
while cam_flag[start] != 1:
    start += 1

nSamples = 4
Ts = float(time[nSamples,0]-time[0,0])


conf = makeConf()
conf['stabilize_invariants'] = True
daeSim = carouselModel.makeModel(conf,propertiesDir='../../properties')

# Create the MHE class
mheRT = MHE.makeMheRT(Ts=Ts)
measX = ['marker_positions','cos_delta','sin_delta','IMU_angular_velocity','IMU_acceleration']
measX += ['aileron','elevator']

measU = ['dddr']

delta = enc_data[start:,0]
ddelta = numpy.diff(delta)/float(time[1,0]-time[0,0])

for index,speed in enumerate(ddelta):
    if speed < 0: ddelta[index] = ddelta[index-1]

plt.ion()
#plt.figure()
#plt.plot(ddelta)
#plt.figure()
#plt.plot(control_surf)
##plt.plot(delta)
#plt.show()

cut_end = 23600
measurements = {}
measurements['time'] = time[start:cut_end:nSamples,:]
measurements['marker_positions'] = cam_led[start+nSamples:cut_end+nSamples:nSamples,:]
#measurements['xyz'] = cam_pose[start+nSamples:cut_end+nSamples:nSamples,:]
measurements['cos_delta'] = enc_data[start:cut_end:nSamples,2]
measurements['sin_delta'] = -enc_data[start:cut_end:nSamples,1]
#measurements['IMU_angular_velocity'] = imu_first[start:cut_end:nSamples,:3]
#measurements['IMU_acceleration'] = imu_first[start:cut_end:nSamples,3:]
measurements['IMU_angular_velocity'] = imu_avg[start:cut_end:nSamples,:3]
measurements['IMU_acceleration'] = imu_avg[start:cut_end:nSamples,3:]
measurements['aileron'] = control_surf[start:cut_end:nSamples,0]
measurements['elevator'] = control_surf[start:cut_end:nSamples,2]

#measurements['daileron'] = enc_data[start:-cut_end:nSamples,1]
#measurements['delevator'] = enc_data[start:-cut_end:nSamples,1]
#measurements['dmotor_torque'] = enc_data[start:-cut_end:nSamples,1]
measurements['dddr'] = enc_data[start:cut_end:nSamples,0]*0

#numpy.sqrt(numpy.dot(measurements['xyz'][0,:],measurements['xyz'][0,:]))


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


# Reference parameters
refP = {'r0':1.2,
        'ddelta0':-ddelta[0],
        }

ref_dict = {
            'z':(-10,10),
            'aileron':(0,0),
            'elevator':(0,0),
            'rudder':(0,0),
            'flaps':(0,0),
            }
#ref_dict = {
#            'z':(0,0),
#            'aileron':(-1,1),
#            'elevator':(-1,1),
#            'rudder':(-1,1),
#            'flaps':(-1,1),
#            }
steadyState,dSS = getSteadyState(daeSim, conf, refP['ddelta0'], refP['r0'], ref_dict)

# utility function
def getDeltaRange(delta0, kRange):
    return numpy.array([delta0 + k*Ts*refP['ddelta0'] for k in kRange])
def getDeltaRangeMhe(delta0):
    return getDeltaRange(delta0, range(-MHE.mheHorizonN, 1))

# Create a sim and initalize it
sim = rawe.RtIntegrator(daeSim, ts=Ts, options=MHE.mheIntOpts)
#sim = rawe.sim.Sim(daeSim,Ts)

sim.x = steadyState
sim.z = steadyState
sim.u = steadyState
sim.p = steadyState

####### initialize MHE #########
# x/z/u
delta0 = numpy.arctan2(measurements['sin_delta'][0],measurements['cos_delta'][0])
#delta0 = enc_data[start,0]

delta[:21*4:4]-getDeltaRange(delta0,range(MHE.mheHorizonN+1))

for k,name in enumerate(mheRT.ocp.dae.xNames()):
    mheRT.x[:,k] = steadyState[name]
    if name == 'sin_delta':
        mheRT.x[:,k] = numpy.sin(getDeltaRange(delta0,range(MHE.mheHorizonN+1)))
    if name == 'cos_delta':
        mheRT.x[:,k] = numpy.cos(getDeltaRange(delta0,range(MHE.mheHorizonN+1)))
for k,name in enumerate(mheRT.ocp.dae.zNames()):
    mheRT.z[:,k] = steadyState[name]
for k,name in enumerate(mheRT.ocp.dae.uNames()):
    mheRT.u[:,k] = steadyState[name]

# expected measurements
ny = numpy.concatenate([mheRT.computeYX(mheRT.x[k,:]),
                                      numpy.array([mheRT.computeYU(mheRT.u[k,:])])]).shape[0]
y = numpy.zeros((mheRT.ocp.N,ny))
for k in range(mheRT.ocp.N):
    y[k,:] = numpy.concatenate([mheRT.computeYX(mheRT.x[k,:]),
                                      numpy.array([mheRT.computeYU(mheRT.u[k,:])])])
yN = mheRT.computeYX(mheRT.x[-1,:])

# real measurements
for k in range(mheRT.ocp.N):
    mheRT.y[k,:] = numpy.concatenate([numpy.concatenate([numpy.array([measurements[name][k]]).flatten() for name in measX]),
                                      numpy.concatenate([numpy.array([measurements[name][k]]).flatten() for name in measU])])
mheRT.yN = numpy.concatenate([numpy.array([measurements[name][mheRT.ocp.N]]).flatten() for name in measX])

#print y-mheRT.y
print y[0,:12]-mheRT.y[0,:12]
print y[0,-9:-3]-mheRT.y[0,-9:-3]


def plot(y,y_hat):

    plt.figure()
    plt.subplot(2,2,1)
    plt.plot([0,1200],[0,1200],linewidth=0)
    color = ['b','r','k']
    for k in range(3):
        plt.plot(y[:,2*k],y[:,2*k+1],'x'+color[k])
        plt.plot(y_hat[:,2*k],y_hat[:,2*k+1],'.'+color[k])
    plt.grid('on')
    plt.subplot(2,2,2)
    plt.plot([0,1200],[0,1200],linewidth=0)
    for k in range(3):
        plt.plot(y[:,2*k+6],y[:,2*k+7],'x'+color[k])
        plt.plot(y_hat[:,2*k+6],y_hat[:,2*k+7],'.'+color[k])
    plt.grid('on')
    
    
    plt.figure()
    plt.subplot(2,1,1)
    color = ['b','r','k']
    for k in range(3):
        plt.plot(y[:,k+14],'x'+color[k])
        plt.plot(y_hat[:,k+14],'.'+color[k])
    plt.grid('on')
    plt.subplot(2,1,2)
    for k in range(3):
        plt.plot(y[:,k+17],'x'+color[k])
        plt.plot(y_hat[:,k+17],'.'+color[k])
    plt.grid('on')
    
    plt.figure()
    for k in range(2):
        plt.plot(y[:,k+12],'x'+color[k])
        plt.plot(y_hat[:,k+12],'.'+color[k])
    plt.grid('on')
    
    plt.figure()
    for k in range(2):
        plt.plot(y[:,k+20],'x'+color[k])
        plt.plot(y_hat[:,k+20],'.'+color[k])
    plt.grid('on')

plot(y,mheRT.y)
#assert(1==0)

# weights
Weight_ = numpy.array([])
for name in mheRT.ocp.yxNames+mheRT.ocp.yuNames:
    Weight_ = numpy.append( Weight_, numpy.ones(mheRT.ocp.dae[name].shape)*(1.0/mheSigmas[name]**2) )
mheRT.S  = numpy.diag(Weight_)
Weight_ = numpy.array([])
for name in mheRT.ocp.yxNames:
    Weight_ = numpy.append( Weight_, numpy.ones(mheRT.ocp.dae[name].shape)*(1.0/mheSigmas[name]**2) )
mheRT.SN  = numpy.diag(Weight_)


# Simulation loop
current_time = 0

#pbb = ProtobufBridge()
log = []
#steadyState2,dSS = getSteadyState(daeSim, conf, refP['ddelta0'], refP['r0']+3, refP['z0']-0.5)


#assert(1==0)
for k in range(12):
#    mheRT.S[k,k] = 0
    if mheRT.y[0,k] < 0:
        mheRT.S[k,k] = 0
for k in range(12):
    mheRT.SN[k,k] = 0
        
        

while current_time < time[-1]:
    # run MHE
#    for j in range(mheRT.ocp.N):
    mheIt = 0
    while True:
        mheRT.preparationStep()
        mheRT.feedbackStep()
        mheIt += 1
        print mheRT.getKKT()
#        break
        if mheRT.getKKT() < 1e-9:
            break
        assert mheIt < 100, "mhe took too may iterations"
        
    y = numpy.zeros((mheRT.ocp.N,ny))
    for k in range(mheRT.ocp.N):
        y[k,:] = numpy.concatenate([mheRT.computeYX(mheRT.x[k,:]),
                                          numpy.array([mheRT.computeYU(mheRT.u[k,:])])])

    plot(y,mheRT.y)
    assert(1==0)
    mheRT.log()    

    # first compute the new final full measurement
#    yxNsim = #mheRT.computeYX(sim.x)
#    yuNsim = #mheRT.computeYU(sim.u)
    
    # send the protobuf and log the message
#    pbb.setMhe(mheRT)
#    pbb.setMpc(mpcRT)
#    pbb.setSimState(sim.x, sim.z, sim.u, yxNsim, yuNsim, mheRT.computeOutputs(sim.x,sim.u))
#    log.append( pbb.sendMessage() )

    # step the simulation
    print "sim time: %6.2f | mhe {kkt: %.4e, iter: %2d, time: %.2e} " \
        % (current_time,
           mheRT.getKKT(), mheIt, mheRT.preparationTime + mheRT.feedbackTime)
    
    sim.u = numpy.zeros((4,1))
    sim.x = mheRT.x[-1,:]
    sim.step()
#    time.sleep(Ts)

    # first compute the final partial measurement
    mheRT.shiftXZU(strategy='copy', xEnd=sim.x, uEnd=sim.u)

    # sensor measurements
#    yN = #mheRT.computeYX(sim.x)
#    y_Nm1 = #numpy.concatenate((yxNsim, yuNsim))

    # shift reference
    mheRT.simpleShiftReference(y_Nm1, yN)

    current_time += Ts

#import sys; sys.exit()
plt.ion()

plotter = Plotter(sim,mheRT,mpcRT)

plotter.subplot([['x','y','z'],['dx','dy','dz']],what=['sim','mhe'])
plotter.subplot([['r'],['dr']],what=['sim','mhe'])
plotter.subplot([['e11','e12','e13'],['e21','e22','e23'],['e31','e32','e33']],what=['sim','mhe'])
plotter.subplot(['w_bn_b_x','w_bn_b_y','w_bn_b_z'],what=['sim','mhe'])
plotter.subplot([['aileron','elevator'],['daileron','delevator']],what=['sim','mhe'])
plotter.subplot([['cos_delta','sin_delta'],['ddelta'],['motor_torque']],what=['sim','mhe'])
plotter.subplot([['r'],['dr'],['ddr']],what=['sim','mhe'])
plotter.subplot([['c'],['cdot']],what=['sim','mhe'])
plotter.subplot([['_kkt'],['_objective'],['_prep_time','_fb_time']],what=['mpc','mhe'])
plotter.plot(['_kkt'],what=['mpc','mhe'])
plotter.plot(['tether_tension'],what=['sim','mhe'])
plotter.plot(['ConstR1','ConstR2','ConstR3','ConstR4','ConstR5','ConstR6'],what=['sim','mhe'])
plotter.plot(['ConstR1','ConstR2','ConstR3','ConstR4','ConstR5','ConstR6'],what=['sim'])
plotter.subplot([['c'],['cdot'],['ConstDelta']],what=['sim','mhe'])
plotter.subplot([['c'],['cdot'],['ConstDelta']],what=['sim'])
plotter.plot(['fL'],what=['sim','mhe'])
plotter.plot(['aero_mx'],what=['sim','mhe'])


#mpcrt.plot(['x','v'],when='all')

plt.show()
