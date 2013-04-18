import matplotlib.pyplot as plt
import casadi as C
import numpy as np

from rawekite.carouselSteadyState import getSteadyState
import rawe
import NMPC

if __name__=='__main__':
    print "creating model"
    from highwind_carousel_conf import conf
    conf['rArm'] = 1.085
    dae = rawe.models.carousel(conf)
    N = 10
    dt_NMPC = 0.1
    mpcRT = NMPC.makeNmpc(dae,N,dt_NMPC)

    #Read the steady state and cost matrices
    steadyState = getSteadyState(dae,conf,2*C.pi,conf['rArm'])
    steadyState_matlab = C.DMatrix(np.loadtxt('data/Xref_example.dat'))
    steadyState['x'] = np.double(steadyState_matlab[0])
    steadyState['y'] = np.double(steadyState_matlab[1])
    steadyState['z'] = np.double(steadyState_matlab[2])
    steadyState['dx'] = np.double(steadyState_matlab[3])
    steadyState['dy'] = np.double(steadyState_matlab[4])
    steadyState['dz'] = np.double(steadyState_matlab[5])
    steadyState['e11'] = np.double(steadyState_matlab[6])
    steadyState['e21'] = np.double(steadyState_matlab[7])
    steadyState['e31'] = np.double(steadyState_matlab[8])
    steadyState['e12'] = np.double(steadyState_matlab[9])
    steadyState['e22'] = np.double(steadyState_matlab[10])
    steadyState['e32'] = np.double(steadyState_matlab[11])
    steadyState['e13'] = np.double(steadyState_matlab[12])
    steadyState['e23'] = np.double(steadyState_matlab[13])
    steadyState['e33'] = np.double(steadyState_matlab[14])
    steadyState['w1'] = np.double(steadyState_matlab[15])
    steadyState['w2'] = np.double(steadyState_matlab[16])
    steadyState['w3'] = np.double(steadyState_matlab[17])
    steadyState['sin_delta'] = np.sin(np.double(steadyState_matlab[18]))
    steadyState['cos_delta'] = np.cos(np.double(steadyState_matlab[18]))
    steadyState['ddelta'] = np.double(steadyState_matlab[19])
    steadyState['r'] = 1.2
    steadyState['motor_torque'] = np.double(steadyState['motor_torque'])
    steadyState['aileron'] = np.double(steadyState_matlab[20])
    steadyState['elevator'] = np.double(steadyState_matlab[21])
    steadyState['dddelta'] = np.double(steadyState_matlab[22])
    steadyState['daileron'] = np.double(steadyState_matlab[23])
    steadyState['delevator'] = np.double(steadyState_matlab[24])

    for k in range(N):
        for l in range(18):
            mpcRT.y[k,l] = steadyState_matlab[l]
        mpcRT.y[k,18:19] = 0.0
    for k in range(18):
        mpcRT.yN[k] = steadyState_matlab[k]

    arrivalCost_matlab = C.reshape(C.DMatrix(np.loadtxt('data/S_example.dat')),22,22)
    arrivalCost_matlab = arrivalCost_matlab[0:18,0:18]

    MPC_Q = C.DMatrix(np.loadtxt('data/MPC_Q.dat'))
    MPC_Q = MPC_Q[0:18,0:18]
    MPC_R = C.DMatrix(np.loadtxt('data/MPC_R.dat'))
    MPC_R = MPC_R[1:,1:]
    S = C.DMatrix(20,20)
    S[:18,:18] = MPC_Q
    S[18:,18:] = MPC_R
    SN = MPC_Q

    mpcRT.S = np.array(S,dtype=np.double)
    mpcRT.SN = np.array(SN,dtype=np.double)

    for k in range(N):
        mpcRT.u[k,0] = 0.0
        mpcRT.u[k,1] = 0.0
        mpcRT.u[k,2] = steadyState['motor_torque']
        mpcRT.u[k,3] = 0.0
    for k in range(len(dae.xNames())):
        mpcRT.x0[k] = steadyState[dae.xNames()[k]]
        # We need to set mpcRT.x[0,:] in order to be able to use initializeNodesByForwardSimulation()
        mpcRT.x[0,k] = mpcRT.x0[k]
    mpcRT.initializeNodesByForwardSimulation()
    
    # Create a simulator
    dt = 0.02
    sim = rawe.sim.Sim(dae,dt)
    # run a sim
    x = dict([(name,mpcRT.x0[k]) for k,name in enumerate(dae.xNames())])
    Nsim = 800
    xh = C.DMatrix(len(dae.xNames()),Nsim)
    for k in range(Nsim):
        for i in range(len(dae.xNames())):
            mpcRT.x0[i] = x[dae.xNames()[i]]
        xh[:,k] = C.DMatrix(mpcRT.x0)
        mpcRT.preparationStep()
        fbret = mpcRT.feedbackStep()
        u = dict([(name,mpcRT.u[0,k]) for k,name in enumerate(dae.uNames())])
        for i in range(np.int(dt_NMPC/dt)):
            x = sim.step(x,u,{})
    t = np.linspace(0,(Nsim-1)*dt_NMPC,Nsim)
    plt.figure(1)
    plt.subplot(311)
    plt.plot(t,C.vec(xh[0,:]))
    plt.subplot(312)
    plt.plot(t,C.vec(xh[1,:]))
    plt.subplot(313)
    plt.plot(t,C.vec(xh[2,:]))
    plt.show()
