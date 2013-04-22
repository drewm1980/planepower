import matplotlib.pyplot as plt
import casadi as C
import numpy as np

from rawekite.carouselSteadyState import getSteadyState
import rawe
import NMPC
import MHE
import carouselModel

if __name__=='__main__':
    plt.interactive(True)
    print "creating model"
    from highwind_carousel_conf import conf
    conf['rArm'] = 1.085
    dae = rawe.models.carousel(conf)
    dae = carouselModel.makeModel(dae,conf)
    N = 10
    dt_NMPC = 0.1
    dt_MHE = 0.1
    #mpcRT = NMPC.makeNmpc(dae,N,dt_NMPC)
    mheRT = MHE.makeMhe(dae,N,dt_MHE)
    assert(1==0)

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
        mpcRT.y[k,18] = steadyState['ddelta']
        mpcRT.y[k,-3:-1] = 0.0
        mpcRT.y[k,-1] = steadyState['motor_torque']
    for k in range(18):
        mpcRT.yN[k] = steadyState_matlab[k]
    mpcRT.yN[18] = steadyState['ddelta']

    terminalCost_matlab_big = C.reshape(C.DMatrix(np.loadtxt('data/S_example.dat')),22,22)
    terminalCost_matlab = C.DMatrix(len(mpcRT.yN),len(mpcRT.yN))
    terminalCost_matlab[0:18,0:18] = terminalCost_matlab_big[0:18,0:18]
    terminalCost_matlab[0:18,18] = terminalCost_matlab_big[0:18,19]
    terminalCost_matlab[18,0:18] = terminalCost_matlab_big[19,0:18]
    terminalCost_matlab[18,18] = terminalCost_matlab_big[19,19]
    

    MPC_Q_big = C.DMatrix(np.loadtxt('data/MPC_Q.dat'))
    MPC_Q = C.DMatrix(len(mpcRT.yN),len(mpcRT.yN))
    MPC_Q[0:18,0:18] = MPC_Q_big[0:18,0:18]
    MPC_Q[0:18,18] = MPC_Q_big[0:18,19]
    MPC_Q[18,0:18] = MPC_Q_big[19,0:18]
    MPC_Q[18,18] = MPC_Q_big[19,19]
    MPC_Q[18,18] = 1e3
    MPC_R = C.DMatrix(np.loadtxt('data/MPC_R.dat'))
    MPC_R = MPC_R[1:,1:]
    S = C.DMatrix(mpcRT.S.shape[0],mpcRT.S.shape[1])
    S[:19,:19] = MPC_Q
    S[19:21,19:21] = MPC_R
    S[21,21] = 1e-2
    SN = terminalCost_matlab

    mpcRT.S = np.array(S,dtype=np.double)
    mpcRT.SN = np.array(SN,dtype=np.double)

    for k in range(N):
        mpcRT.u[k,0] = 0.0
        mpcRT.u[k,1] = 0.0
        mpcRT.u[k,2] = steadyState['motor_torque']*1.5
        mpcRT.u[k,3] = 0.0
    for k in range(len(dae.xNames())):
        mpcRT.x0[k] = steadyState[dae.xNames()[k]]
        # We need to set mpcRT.x[0,:] in order to be able to use initializeNodesByForwardSimulation()
        mpcRT.x[0,k] = mpcRT.x0[k]
    mpcRT.initializeNodesByForwardSimulation()
    
    # Create a simulator
    N_simsteps = 3
    dt = dt_NMPC/N_simsteps
    sim = rawe.sim.Sim(dae,dt)
    # run a sim
    x = dict([(name,mpcRT.x0[k]) for k,name in enumerate(dae.xNames())])
    Nsim = 50
    xh = C.DMatrix(len(dae.xNames()),Nsim)
    uh = C.DMatrix(len(dae.uNames()),Nsim)
    for k in range(Nsim):
        for i in range(len(dae.xNames())):
            mpcRT.x0[i] = x[dae.xNames()[i]]
        xh[:,k] = C.DMatrix(mpcRT.x0)
        uh[:,k] = C.DMatrix(mpcRT.u[0,:])
        mpcRT.preparationStep()
        fbret = mpcRT.feedbackStep()
        u = dict([(name,mpcRT.u[0,i]) for i,name in enumerate(dae.uNames())])
        for i in range(N_simsteps):
            x = sim.step(x,u,{})
        uh[:,k] = C.DMatrix(mpcRT.u[0,:])

    t = np.linspace(0,(Nsim-1)*dt_NMPC,Nsim)
    plt.figure(1)
    plt.subplot(511)
    plt.plot(t,C.vec(xh[0,:]))
    plt.subplot(512)
    plt.plot(t,C.vec(xh[1,:]))
    plt.subplot(513)
    plt.plot(t,C.vec(xh[2,:]))
    plt.subplot(514)
    plt.plot(t,C.vec(xh[18,:]))
    plt.subplot(515)
    plt.plot(t,C.vec(uh[2,:]))
    plt.show()
