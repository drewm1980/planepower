import casadi as C
import numpy as np

from rawe.carouselSteadyState import getSteadyState
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
    steadyState = getSteadyState(dae,conf,2*C.pi,conf['rArm'])
    steadyState_matlab = C.DMatrix(np.loadtxt('Xref_example.dat'))
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

    arrivalCost_matlab = C.reshape(C.DMatrix(np.loadtxt('S_example.dat')),22,22)
    arrivalCost_matlab = arrivalCost_matlab[0:18,0:18]

    MPC_Q = C.DMatrix(np.loadtxt('MPC_Q.dat'))
    MPC_Q = MPC_Q[0:18,0:18]
    MPC_R = C.DMatrix(np.loadtxt('MPC_R.dat'))
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
    mpcRT.initializeNodesByForwardSimulation()
    for k in range(N):
        for l in range(18):
            mpcRT.y[k,l] = steadyState_matlab[l]
        mpcRT.y[k,18:19] = 0.0
    for k in range(18):
        mpcRT.yN[k] = steadyState_matlab[k]

    from rawe.dae.rienIntegrator import RienIntegrator
    integrator = RienIntegrator(dae,ts=0.03)
    #integrator = rawe.sim.Sim(dae,0.03)
    p = {'endTime':0.03}
    x = steadyState
    log = {'x':[x['x']], 'y':[x['y']], 'z':[x['z']]}
    for k in range(400):
        x = integrator.run(x,steadyState,p)
        #x = integrator.step(x,steadyState,{})
        log['x'].append(x['x'])
        log['y'].append(x['y'])
        log['z'].append(x['z'])

    import matplotlib.pylab as plt
    plt.figure()
    plt.subplot(311)
    plt.plot(log['x'])
    plt.title('x')
    plt.subplot(312)
    plt.plot(log['y'])
    plt.title('y')
    plt.subplot(313)
    plt.plot(log['z'])
    plt.title('z')
    plt.show()

    assert(1==0)
    dt = 0.02
    sim = rawe.sim.Sim(dae,dt)
    communicator = rawe.simutils.Communicator()
#    js = joy.Joy()
    x = {}
    for name in dae.xNames():
        x[name] = steadyState[name]
    u = {}
    for name in dae.uNames():
        u[name] = steadyState[name]
    p = {}
    for name in dae.pNames():
        p[name] = steadyState[name]

    print "simulating..."
    timer = rawe.simutils.Timer(dt)
    timer.start()
    try:
        while True:
            timer.sleep()
            outs = sim.getOutputs(x,u,p)
            communicator.sendKite(x,u,p,outs,conf)
            try:
                x = sim.step(x,u,p)
            except RuntimeError:
                communicator.close()
                raise Exception('OH NOES, IDAS CHOKED')
    except KeyboardInterrupt:
        print "closing..."
        communicator.close()
        pass
