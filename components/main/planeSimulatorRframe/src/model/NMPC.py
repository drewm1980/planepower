# -*- coding: utf-8 -*-
import numpy as np
from casadi import*
from numpy import*
import matplotlib.pyplot as plt
from copy import deepcopy
from powerplaneModels import*
from mpl_toolkits.mplot3d import Axes3D
import time
plt.interactive(True)
#from generate_reference4 import *
#from powerkiteFunctions import*

tic = time.time()
#def powerkiteCollDualClosureInit():
if True:
    # Time
    t = ssym("t")
    
    # Define the controls
    # ------------------------------------------------------------------
    dddelta = ssym("dddelta")	# Carousel rotational acceleration
#    ddr     = ssym("ddr")	      # Tether acceleration
    dua     = ssym("dua")	      # Aileron input
    due     = ssym("due")	      # Elevator input
    
    # All controls
    #    U = vertcat([Fjx,Fjy,Fjz,u11,u21,u31,kappa1,u12,u22,u32,kappa2])
    U = vertcat([dddelta,dua,due])
    
    # Define the differential states
    # ------------------------------------------------------------------
    x = ssym('x')
    y = ssym('y')
    z = ssym('z')
    dx = ssym("dx")		# Velocity
    dy = ssym("dy")
    dz = ssym("dz")
    e11 = ssym("e11")
    e12 = ssym("e12")
    e13 = ssym("e13")
    e21 = ssym("e21")
    e22 = ssym("e22")
    e23 = ssym("e23")
    e31 = ssym("e31")
    e32 = ssym("e32")
    e33 = ssym("e33")
    q3 = ssym("q3")
    w1 = ssym("w1")
    w2 = ssym("w2")
    w3 = ssym("w3")
#    r = ssym("r")
#    dr = ssym("dr")
    delta = ssym("delta")
    ddelta = ssym("ddelta")
    ua     = ssym("ua")	      # Aileron input
    ue     = ssym("ue")	      # Elevator input
    
    # Gather all the states in one vector
    XD = SXMatrix([x,y,z,dx,dy,dz,e11,e12,e13,e21,e22,e23,e31,e32,e33,w1,w2,w3,delta,ddelta,ua,ue])
    
   
    # Define the slack variables
    # ------------------------------------------------------------------
    sCL = ssym('sCL')
    saT = ssym('saT')
    sb = ssym('sb')
    
    S = SXMatrix([sCL,saT,sb])
    
    Us = U
    XDs = XD
    
    # (Define the scaling factors for the state)
    # ------------------------------------------------------------------
    Scaling = np.ones(XD.shape[0]) # Don't use the scaling right now, it is not implemented yet!!
    
    # Define the parameters
    # ------------------------------------------------------------------
    w0 = 0.
    r = 1.2
#    dw = 0.
    P = np.array([w0,r])   
    
    dX = RotModelCarouselR(XD,U,P,Scaling,"")
    IMU = RotModelCarouselR(XD,U,P,Scaling,"IMU")

    ffcn = SXFunction([XD,U],[dX])
    ffcn.init()
    
    
    XD0 = np.loadtxt("NMPC_X0.txt")
#    U00 = loadtxt("Uinit.txt")
    
#    X0 = np.array([append(XD0[0,:],U00[0,-2:])])
    X0 = XD0[1:]   
    U0 = np.array([zeros(3)])
    print X0
    
    ffcn.setInput(X0,0)
    ffcn.setInput(U0.T,1)
    ffcn.evaluate()
    print ffcn.output()
        
    h = ssym("h")    
#    X_0 = ssym("X_0", XD.shape[0], 1)
    
    
    
    k1 = h * RotModelCarouselR(XD,U,P,Scaling,"")
    k2 = h * RotModelCarouselR(XD+k1/2.,U,P,Scaling,"")
    k3 = h * RotModelCarouselR(XD+k2/2.,U,P,Scaling,"")
    k4 = h * RotModelCarouselR(XD+k3,U,P,Scaling,"")
    
    X_1 = XD + 1./6. * ( k1 + 2*k2 + 2*k3 + k4 )
    
    Ifcn = SXFunction([XD,U,h],[X_1,IMU])
    Ifcn.init()
    Ifcn.generateCode("rk4.c")
    
    X0 = [1.1365549923016651e+00, -3.8502305057510727e-01, 0.0000000000000000e+00, 0.0000000000000000e+00, 0.0000000000000000e+00, 0.0000000000000000e+00, 2.5366718640228059e-01, -5.6665870432704568e-01, 7.8393295080200864e-01, 9.6482527701369714e-01, 2.0606092542526261e-01, -1.6325158452011110e-01, -6.9030017950115444e-02, 7.9776989653864372e-01, 5.9899753655464105e-01, -4.3372940878416538e-01, 5.0125478139316062e+00, 3.7636213216801204e+00, 0.0000000000000000e+00, 6.2831999999999999e+00, 1.9382406007636174e-02, -2.2180874727969077e-02]
    X = np.array([X0])
    T = [0]
    h = 0.01
    for k in range(200):
        Ifcn.setInput(X[-1,:],0)
        Ifcn.setInput(U0.T,1)
        Ifcn.setInput(h,2)
        Ifcn.evaluate()
        X = append(X,Ifcn.output()[:22,:].T,axis=0)
        T = append(T,T[-1]+h)
    
    print "Error after one turn", X[-1,:]-X[0,:]

    #assert(1==0)
    ## Compare matlab simulation (in files below) with this integrator
    #TU = np.loadtxt('../../../../../matlab_acado_codegen_simulation/TU.txt')
    #TX = np.loadtxt('../../../../../matlab_acado_codegen_simulation/TX.txt')
    
    #X = DMatrix(TX[0,1:]).T
    
    #h = 1./500/5;
    #t = 0. #Time of simulation
    #T = t
    #i = 0 # Index to decide which entry of TU to take.
    #while t < TU[-1,0]:
        #if TU[i,0] < t:
            #i+=1
        #U = TU[i,1:]
        #Ifcn.setInput(X[-1,:].T,0)
        #Ifcn.setInput(U,1)
        #Ifcn.setInput(h,2)
        #Ifcn.evaluate()
        #X = append(X,Ifcn.output()[:22,:].T,axis=0)
        #t+=h;
        #T = append(T,t)
    
    #plt.figure(1)
    #plt.clf()
    #plt.plot(vec(T),vec(X[:,0]),'b--')
    #plt.plot(vec(T),vec(X[:,1]),'b-')
    #plt.plot(vec(T),vec(X[:,2]),'b-.')
    #plt.plot(vec(TX[:,0]),vec(TX[:,1]),'r--')
    #plt.plot(vec(TX[:,0]),vec(TX[:,2]),'r-')
    #plt.plot(vec(TX[:,0]),vec(TX[:,3]),'r-.')
    #plt.title("Position")
    #plt.xlabel('Time')
    #plt.legend(['x','y','z','x_sol','y_sol','z_sol'])
    #plt.grid()
#    
#    XDDOT = ssym("XDDOT", XD.shape[0], 1)
#    # Inputs of the DAE right hand side function
#    ffcn_in = DAE_NUM_IN*[[]]
#    ffcn_in[DAE_T] = t			# to transform a scalar in a vector
#    ffcn_in[DAE_Y] = XD
#    ffcn_in[DAE_YDOT] = XDDOT
#    ffcn_in[DAE_P] = U			# The controls are passed as parameters
#    
#    # Outputs of the DAE right hand side function
#    ffcn_out = [dX-XDDOT]
#
#    # Create a DAE right hand side 
#    ffcn = SXFunction(ffcn_in,ffcn_out)
#    ffcn.init()
#    
#    ffcn.setInput(X[0,:].T,DAE_Y)
#    ffcn.setInput(U0[0,:].T,DAE_P)
#    ffcn.setInput(zeros(XDDOT.shape),DAE_YDOT)
#    ffcn.evaluate()
#    
#    # Save the initial state derivative
#    XDOT0 = DMatrix(ffcn.output())
#    XDOT0[-1]=0.
#    
#    # Allocate an integrator object
##    integrator = CVodesIntegrator(ffcn)
#    integrator = IdasIntegrator(ffcn)
#    integrator.setOption("abstol",1e-8) # tolerance
#    integrator.setOption("reltol",1e-8) # tolerance
#    integrator.setOption("linear_solver","user_defined") 
#    integrator.setOption("linear_solver_creator",CSparse) 
#    integrator.setOption("exact_jacobian",True) 
#    integrator.setOption("steps_per_checkpoint",1000) 
#    integrator.setOption("tf",1.) 
#
#    integrator.init()
#    integrator.evaluate()
#    integrator.output()
#    
#    print X[-1,:]-integrator.output()
#    
    
    #assert(1==0)    
    
    
    
    #Const, dConst, ConstR1, ConstR2, ConstR3, ConstR4, ConstR5, ConstR6, ConstRpos = RotModelCarouselR(XD,XDDOT,U,P,Scaling,"constraints")
    #CL = RotModelCarouselR(XD,XDDOT,U,P,Scaling,"CL")
    #F, T = RotModelCarouselR(XD,XDDOT,U,P,Scaling,"FT")
    #alpha, beta, alphaT, wE = RotModelCarouselR(XD,XDDOT,U,P,Scaling,"alphabeta")
    #lambda_ = RotModelCarouselR(XD,XDDOT,U,P,Scaling,"lambda")
    
    #FTfcn = SXFunction([XD,XDDOT,U],[F,T,alpha,beta,alphaT,wE])
    #FTfcn.init()
    
    #CLfcn =SXFunction([XD,U],[CL])
    #CLfcn.init()
    
    #Lfcn =SXFunction([XD,U],[lambda_])
    #Lfcn.init()
    
    ## ------------------------------------------------------------------
##    filename = 'powerplaneSingle'
    ## Read state trajectory from file
##    XD0 = loadtxt(filename + "_diffState.txt")
    #XD0 = np.loadtxt("XDinit.txt")
##    idx = append(arange(XD0.shape[1]-2),XD0.shape[1]-1)
##    XD0 = XD0[:,idx]
    
    ## Get the time vector
##    T0 = loadtxt(filename + "_time.txt")
    #T0 = loadtxt("Tinit.txt")
    
    ## We only need the states (at the initial time)
##    XD0 = XD0/Scaling
##    XA0 = loadtxt(filename + "_algState.txt")
    
##    X0t = [append(XD0[0,:],XA0[0] )]
##    for k in range(1,len(XD0)):
##        X0t = append(X0t, [append(XD0[k,:],XA0[k] )], axis=0)
##    X0 = X0t
    #X0 = XD0
    
    ## Read control trajectory from file
##    U0 = loadtxt(filename + "_controls.txt")
    #U0 = loadtxt("Uinit.txt")
    
    ## The DAE formulation:
    ## ------------------------------------------------------------------
    ## Sundials require the DAE to be formulated in full implicit form, add a state derivative vector
    ##XDDOT = ssym("XDDOT",XD.size())
##    XADOT = ssym("XADOT",XA.size())
    
##    X = vertcat((XD,XA))
##    XDOT = vertcat((XDDOT,XADOT))
    #X = XD
    #XDOT = XDDOT
    
    ## Inputs of the DAE right hand side function
    #ffcn_in = DAE_NUM_IN*[[]]
    #ffcn_in[DAE_T] = t			# to transform a scalar in a vector
    #ffcn_in[DAE_Y] = X
    #ffcn_in[DAE_YDOT] = XDOT
    #ffcn_in[DAE_P] = U			# The controls are passed as parameters
    
    ## Outputs of the DAE right hand side function
    #ffcn_out = [res]
    ##ffcn_out = [ vertcat((XDDOT,XA)) - solve(M,rhs) ]
##    ffcn_out = [f-XDOT]
    
    ## Create a DAE right hand side 
    #ffcn = SXFunction(ffcn_in,ffcn_out)
    #ffcn.init()
    
    #ffcn.setInput(X0[0,:],DAE_Y)
    #ffcn.setInput(U0[0,:],DAE_P)
    #ffcn.setInput(zeros(XDOT.shape),DAE_YDOT)
    #ffcn.evaluate()
    
    ## Save the initial state derivative
    #XDOT0 = DMatrix(ffcn.output())
    #XDOT0[-1]=0.
    
    ## Allocate an integrator object
    #integrator = IdasIntegrator(ffcn)
    ## Set some options
    #ndiff = XD.shape[0]
    #nalg  = 0#XA.shape[0]
    #integrator.setOption("is_differential",ndiff*[1]+nalg*[0]) 
    #integrator.setOption("abstol",1e-8) # tolerance
    #integrator.setOption("reltol",1e-8) # tolerance
    #integrator.setOption("linear_solver","user_defined") 
    #integrator.setOption("linear_solver_creator",CSparse) 
    #integrator.setOption("exact_jacobian",True) 
    #integrator.setOption("steps_per_checkpoint",1000) 
    
    ## Multiply the number of intervals
    #nfac = 1
    ## Number of (intermediate) collocation points per control interval
    #nicp = 1
    
    #npointsu = nfac*(len(X0))
    #nk = npointsu-nfac  # Control discretization
    #tf = T0[-1]            # End time
    
    ## Dimensions
    #nx = X.size()
    #nu = U.size()
    
    ## Legendre collocation points
    #legendre_points1 = [0,0.500000]
    #legendre_points2 = [0,0.211325,0.788675]
    #legendre_points3 = [0,0.112702,0.500000,0.887298]
    #legendre_points4 = [0,0.069432,0.330009,0.669991,0.930568]
    #legendre_points5 = [0,0.046910,0.230765,0.500000,0.769235,0.953090]
    #legendre_points = [0,legendre_points1,legendre_points2,legendre_points3,legendre_points4,legendre_points5]
    
    ## Radau collocation points
    #radau_points1 = [0,1.000000]
    #radau_points2 = [0,0.333333,1.000000]
    #radau_points3 = [0,0.155051,0.644949,1.000000]
    #radau_points4 = [0,0.088588,0.409467,0.787659,1.000000]
    #radau_points5 = [0,0.057104,0.276843,0.583590,0.860240,1.000000]
    #radau_points = [0,radau_points1,radau_points2,radau_points3,radau_points4,radau_points5]
    
    ## Type of collocation points
    #LEGENDRE = 0
    #RADAU = 1
    #collocation_points = [legendre_points,radau_points]
    
    ## Degree of interpolating polynomial
    #deg = 4
    
    ## Radau collocation points
    #cp = RADAU
    
    ## Size of the finite elements
    #h = tf/nk/nicp
    
    ## Coefficients of the collocation equation
    #C = zeros((deg+1,deg+1))
    
    ## Coefficients of the continuity equation
    #D = zeros(deg+1)
    ##Da = zeros(deg+1)
    
    ## Collocation point
    #tau = ssym("tau")
      
    ## All collocation time points
    #tau_root = collocation_points[cp][deg]
    #T = zeros((nk,deg+1))
    #for i in range(nk):
      #for j in range(deg+1):
    	#T[i][j] = h*(i + tau_root[j])
    
    ## For all collocation points: eq 10.4 or 10.17 in Biegler's book
    #for j in range(deg+1):
        ## Construct Lagrange polynomials to get the polynomial basis at the collocation point
        #L = 1
        #for j2 in range(deg+1):
            #if j2 != j:
                #L *= (tau-tau_root[j2])/(tau_root[j]-tau_root[j2])
        #lfcn = SXFunction([tau],[L])
        #lfcn.init()
        ## Evaluate the polynomial at the final time to get the coefficients of the continuity equation
        #lfcn.setInput(1.0)
        #lfcn.evaluate()
        #D[j] = lfcn.output()
        ## Evaluate the time derivative of the polynomial at all collocation points to get the coefficients of the continuity equation
        #for j2 in range(deg+1):
            #lfcn.setInput(tau_root[j2])
            #lfcn.setFwdSeed(1.0)
            #lfcn.evaluate(1,0)
            #C[j][j2] = lfcn.fwdSens()
    
    
    #tgrid = np.array(tau_root)*h
    #sim = Simulator(integrator,tgrid)
    #sim.init()

    #npoints = nicp*nfac*(deg+1)*(len(X0)-1)
    #Uint = nfac*[U0[0]]
    #for k in range(1,len(U0)-1):
        #Uint = append(Uint,nfac*[U0[k]],axis=0)

    #for k in range(nicp*nfac*(len(X0)-1)):
##    for k in range(13):
##        if k == 0:
##            sim.setInput(X0[0],0)
##        else:
##            sim.setInput(Xsol[-1],0)
        #sim.setInput(X0[k],0)
        #sim.setInput(U0[k],1)
        #sim.evaluate()
        
        #X = array(sim.output())
        #if k == 0:
            #Xsol = X
            #Tint = tgrid
        #else:
            #Xsol = append(Xsol,X,axis=0)
            #Tint = append(Tint,Tint[-1]+tgrid)
        
##        if k%nicp == 0:
##            Xsol[-1,:-3] = X0[k/nicp+1,:-3]
           
    
##    assert(1==0)
    
    ## Set up the optimization routine (collocation method):
    ## ------------------------------------------------------------------
    
    ## Objective function
##    R = np.eye(U.size()) * 1e2
    ## ------------------------------------------------------------------
    ##  Controls
    ## ------------------------------------------------------------------
    ##             Sddr  Sdddelta Sua       Sue
    #R = np.diag([  1,    1,      1/0.2**2, 1/0.2**2 ]) * 1e2
    ## ------------------------------------------------------------------
    ##  States
    ## ------------------------------------------------------------------
    ##         Scale x, y, z
    #Wx = 1e4 * np.array( [ 1/2.**2, 1/2.**2, 1/2.**2 ] )
    ##            Scale dx, dy, dz
    #Wdx = 1e2 * np.array( [ 1/10.**2, 1/10.**2, 1/10.**2 ] )
    ##            Scale e11 ... e33
    #WR = 1e2 * np.ones( 9 )
    ##            Scale w1, w2, w3
    #Ww = 1e2 * np.ones( 3 )/10.**2
    ##            Scale dr, ddr             weights
    #Wr = 1e0 * np.array( [ 1., 1/3.**2 ] )*np.array([1e4, 1.] )
    ##            Scale delta, ddelta
    #Wdelta = 1e2 * np.array( [ 1., 1. ] )
    #Q = np.diag(np.concatenate([ Wx*np.ones(3), Wdx*np.ones(3), WR*np.ones(9), Ww*np.ones(3), Wr*np.ones(2), Wdelta*np.ones(2) ] ))
    #Ur = ssym("Ur",U.size())
    #XDr = ssym("XDr",XD.size())
    #Ut = U-Ur
    #XDt = XD-XDr
    ## ------------------------------------------------------------------
    ##  Slack variables
    ## ------------------------------------------------------------------
    ##         Scale sCL   saT             sb
    #WS = np.array([[ 1., 1./(15.*pi/180), 1./(20.*pi/180) ]])**2 * ( 25 * 1e2 )
    ## ------------------------------------------------------------------
    ##  Side slip
    ## ------------------------------------------------------------------
    ##          Scaling
    #WAASS = 1 * (1./(10./180*pi))**2
    #regS = mul(WS,S)
    #regU = mul(Ut.T,mul(R,Ut))
    #regX = mul(XDt.T,mul(Q,XDt))
    #regAASS = WAASS * ( beta**2 + alpha**2 ) #1e2 * ( XD[6]**2 + XD[9]**2 + XD[12]**2 ) #
    #reg = regU + regX + regS + regAASS
    ##                               controls  state  slack  sideslip
    #mfcn = SXFunction([XD,U,S,XDr,Ur],[reg*1e-6])
    
    
    ##                sCL, saT,       sb
    #s_min = array([  0.,  0.,        0.        ])
    #s_max = array([  0.5, 5.*pi/180, 5.*pi/180 ])
    #s_init = [0., 0., 0.]
    
    #AccRate = 30*pi/180
    ##              dddelta    ddr   ua    ue
    #u_min = array([-AccRate, -0.5, -0.2, -0.2 ])
    #u_max = array([ AccRate,  0.5,  0.2,  0.2 ])
    ##u_init = U0
    #u_init = Uint
    
    ## Remove degrees of freedom
    
    #SpdRate = 60*pi/180
    
    ## State bounds and initial guess
    ##                      x,    y,   z,   dx,   dy,   dz,  e11, e12, e13  e21, e22, e23  e31, e32, e33,  w1,   w2,   w3,    r,  dr,  delta,ddelta
    #xD_min  = np.array([-inf, -inf, -1., -inf, -inf, -inf, -1., -1., -1., -1., -1., -1., -1., -1., -1., -inf, -inf, -inf, 0.5, -3.0,  0.,  0. ])
    #xD_max  = np.array([ inf,  inf, inf,  inf,  inf,  inf,  1.,  1.,  1.,  1.,  1.,  1.,  1.,  1.,  1.,  inf,  inf,  inf, inf,  3.0, inf, inf ])
    ##                
    #xA_min = np.array([])
    #xA_max = np.array([])
##    #                lambda_
##    xA_min = np.array([  0.])
##    xA_max = np.array([ inf])
##    X0neq = np.loadtxt("X0neq.txt")
    #xDi_min = deepcopy(X0[0])
    #xDi_max = deepcopy(X0[0])
##    xDi_min = deepcopy(X0neq)
##    xDi_max = deepcopy(X0neq)
##    xDi_min[-4] = 0.  # delta = 0
##    xDi_max[-4] = 0.  
    
    #xDf_min = deepcopy(xD_min)
    #xDf_max = deepcopy(xD_max)


    ##x_init = X0
    #xD_init = Xsol
    #xA_init = np.zeros((Xsol.shape[0],0))
    
    ## Path constraint (specify the variables in one array and the bounds in 2 other arrays)
    #p = SXMatrix()
    #pmin = np.array([])
    #pmax = np.array([])
    #p.append(CL-sCL);        pmin = append(pmin, -inf);         pmax = append(pmax, 1.)
    #p.append(CL+sCL);        pmin = append(pmin, -0.);          pmax = append(pmax, inf)
    #p.append(alphaT-saT);    pmin = append(pmin, -inf);         pmax = append(pmax, 10.*pi/180)
    #p.append(alphaT+saT);    pmin = append(pmin, -10.*pi/180);  pmax = append(pmax, inf)
    #p.append(beta-sb);       pmin = append(pmin, -inf);         pmax = append(pmax, 15.*pi/180)
    #p.append(beta+sb);       pmin = append(pmin, -15.*pi/180);  pmax = append(pmax, inf)
    #p.append(lambda_);       pmin = append(pmin,  0.);          pmax = append(pmax, inf)

    ## Path constraint function
    #pfcn = SXFunction([XD,S,U], [p])
    
    ## Mech constraint (specify the variables in one array and the bounds in 2 other arrays)
    #c = SXMatrix()
    #cmin = np.array([])
    #cmax = np.array([])
##    c.append(ConstC);       cmin = append(cmin, 0.);    cmax = append(cmax, 0.)
##    c.append(dConstC);      cmin = append(cmin, 0.);    cmax = append(cmax, 0.)
##    c.append(ConstQ);       cmin = append(cmin, 0.);    cmax = append(cmax, 0.)
    
    ## Mech constraint function
    #cfcn = SXFunction([XD,U], [c])
    
    
    ##ffcn = SXFunction(ffcn_in,[f])
    #ffcn = SXFunction([XD,XDDOT,U],[res])
##    ffcn = SXFunction([XD,XA,U],[rhs-mul(M,vertcat((XDDOT,XA)))])
    
    ## Initialize functions
    #ffcn.init()		# System dynamics
    #mfcn.init()		# Objective functions
    #pfcn.init()		# Path constraint function
    #cfcn.init()		# Mech constraint function
    
    
    
    ## Total number of variables
    #NXD = nicp*nk*(deg+1)*ndiff # Collocated differential states
    #NXA = nicp*nk*deg*nalg      # Collocated algebraic states
    #NU  = nk*nu                 # Parametrized controls
    #NXF = ndiff                 # Final state
    #NS  = S.size()              # Slack variables
    #NV = NXD+NXA+NU+NXF+NS
    
    ## NLP variable vector
    #V = msym("V",NV)
      
    ## All variables with bounds and initial guess
    #vars_lb = zeros(NV)
    #vars_ub = zeros(NV)
    #vars_init = zeros(NV)
    #offset = 0
    
    
    ## Get the parameters
    #S = V[offset:offset+NS]
    
    #vars_init[offset:offset+NS] = s_init
    #vars_lb[offset:offset+NS] = s_min
    #vars_ub[offset:offset+NS] = s_max
    #offset += NS

    ## Get collocated states and parametrized control
    #XD = np.resize(np.array([],dtype=MX),(nk+1,nicp,deg+1)) # NB: same name as above
    #XA = np.resize(np.array([],dtype=MX),(nk,nicp,deg)) # NB: same name as above
    #U  = np.resize(np.array([],dtype=MX),nk)
    #for k in range(nk):  
        ## Collocated states
        #for i in range(nicp):
            ##
            #for j in range(deg+1):
                          
                ## Get the expression for the state vector
                #XD[k][i][j] = V[offset:offset+ndiff]
                #if j !=0:
                    #XA[k][i][j-1] = V[offset+ndiff:offset+ndiff+nalg]
                ## Add the initial condition
                #index = (deg+1)*(nicp*k+i) + j
                #if k==0 and j==0 and i==0:
##                    vars_init[offset:offset+ndiff] = xD_init[nicp*k+i,:]
                    #vars_init[offset:offset+ndiff] = xD_init[index,:]
                    
                    #vars_lb[offset:offset+ndiff] = xDi_min
                    #vars_ub[offset:offset+ndiff] = xDi_max                    
                    #offset += ndiff
                #else:
                    #if j!=0:
##                        vars_init[offset:offset+nx] = append(xD_init[nicp*k+i,:],xA_init[nicp*k+i,:])
                        #vars_init[offset:offset+nx] = append(xD_init[index,:],xA_init[index,:])
                        
                        #vars_lb[offset:offset+nx] = append(xD_min,xA_min)
                        #vars_ub[offset:offset+nx] = append(xD_max,xA_max)
                        #offset += nx
                    #else:
##                        vars_init[offset:offset+ndiff] = xD_init[nicp*k+i,:]
                        #vars_init[offset:offset+ndiff] = xD_init[index,:]
                        
                        #vars_lb[offset:offset+ndiff] = xD_min
                        #vars_ub[offset:offset+ndiff] = xD_max
                        #offset += ndiff
        
        ## Parametrized controls
        #U[k] = V[offset:offset+nu]
        #vars_lb[offset:offset+nu] = u_min
        #vars_ub[offset:offset+nu] = u_max
        #vars_init[offset:offset+nu] = u_init[k,:]
        ##  vars_init[offset:offset+nu] = u_init
        #offset += nu
    
    ## State at end time
    #XD[nk][0][0] = V[offset:offset+ndiff]
    #vars_lb[offset:offset+ndiff] = xDf_min
    #vars_ub[offset:offset+ndiff] = xDf_max
    #vars_init[offset:offset+ndiff] = xD_init[-1,:]
    #offset += ndiff
    #assert(offset==NV)
    
    ## Constraint function for the NLP
    #g = []
    #lbg = []
    #ubg = []
    
    ## Initial constraints (Const, dConst, ConstQ)
    #[ck] = cfcn.call([XD[0][0][0], U[0]])
    #g += [ck]
    #lbg.append(cmin)
    #ubg.append(cmax)
    
    ## For all finite elements
    #for k in range(nk):
        #for i in range(nicp):
            ## For all collocation points
            #for j in range(1,deg+1):   		
                ## Get an expression for the state derivative at the collocation point
                #xp_jk = 0
                #for j2 in range (deg+1):
                    #xp_jk += C[j2][j]*XD[k][i][j2]       # get the time derivative of the differential states (eq 10.19b)
                
                ## Add collocation equations to the NLP
                #[fk] = ffcn.call([XD[k][i][j], xp_jk/h, U[k]])
                ##g += [h*fk[:ndiff] - xp_jk]                     # impose system dynamics (for the differential states (eq 10.19b))
                #g += [fk[:ndiff]]                     # impose system dynamics (for the differential states (eq 10.19b))
                #lbg.append(zeros(ndiff)) # equality constraints
                #ubg.append(zeros(ndiff)) # equality constraints
                #g += [fk[ndiff:]]                               # impose system dynamics (for the algebraic states (eq 10.19b))
                #lbg.append(zeros(nalg)) # equality constraints
                #ubg.append(zeros(nalg)) # equality constraints
                
                ## Evaluate the path constraint function
                #[pk] = pfcn.call([XD[k][i][j], S, U[k]])
                #g += [pk]
                #lbg.append(pmin)
                #ubg.append(pmax)
            
            ## Get an expression for the state at the end of the finite element
            #xf_k = 0
            #for j in range(deg+1):
                #xf_k += D[j]*XD[k][i][j]
                
            ## Add continuity equation to NLP
            #if i==nicp-1:
    ##            print "a ", k, i
                #g += [XD[k+1][0][0] - xf_k]
            #else:
    ##            print "b ", k, i
                #g += [XD[k][i+1][0] - xf_k]
            
            #lbg.append(zeros(ndiff))
            #ubg.append(zeros(ndiff))
        
    ## Nonlinear constraint function
    #gfcn = MXFunction([V],[vertcat(g)])
##    gfcn.init()
##    Jg = gfcn.jacobian()
##    Jg.init()
##    Jg.setInput(np.ones(V.shape))
##    Jg.evaluate()
##    JG = np.array(Jg.output())
##    plt.spy(JG)
##    plt.show()
    
    
##    assert(1==0)
    ## Objective function of the NLP
    #Obj = 0
##    for k in range(nk):
##        for i in range(nicp):
##            # For all collocation points
##            for j in range(1,deg+1):
##                [obj] = mfcn.call([XD[k][i][j],U[k]])
##                Obj += obj
###    [obj] = mfcn.call([XD[nk][0][0],0*U[nk-1]])
##    [obj] = mfcn.call([XD[nk][0][0],zeros(4)])

    #XDr = np.loadtxt("refXD.txt")
    #Ur = np.loadtxt("refU.txt")
    #z = 0
    #r = 1.180
    #delta = 0
    #ddelta = XDr[0,-1]
    #XDr, Ur = Reference(z,r,delta,ddelta)
    
##    print X0[0]
##    print XDr[0]
##    
##    assert(1==0)
    
    #for k in range(nk):
        #[obj] = mfcn.call([XD[k][0][0],U[k],S,XDr[k],Ur[k]])
        #Obj += obj
    #[obj] = mfcn.call([XD[nk][0][0],np.zeros(4),S,XDr[nk],np.zeros(4)])
    #Obj += obj*1e2
    ##obj = inner_prod(V,V)/1e6
    ##obj = 0
    
    #ofcn = MXFunction([V], [Obj])
    
    ### ----
    ### SOLVE THE NLP
    ### ----
      
    ##assert(1==0)
    ## Allocate an NLP solver
    #solver = IpoptSolver(ofcn,gfcn)
    
    ## Set options
    #solver.setOption("expand_f",True)
    #solver.setOption("expand_g",True)
    #solver.setOption("generate_hessian",True)
    #solver.setOption("max_iter",1000)
    #solver.setOption("tol",1e-2)
##    solver.setOption("mu_init",1e-5)
##    solver.setOption("acceptable_tol",1e-2)
    ##solver.setOption("constr_viol_tol",1e-4)
    ##solver.setOption("bound_relax_factor",0)
    ##solver.setOption("derivative_test","first-order")
    ##solver.setOption("derivative_test_perturbation",1e-4)
    ##solver.setOption("derivative_test_tol",1e-2)
    
##    solver.setOption("linear_solver","ma57")

##    solver.setOption("linear_solver","mumps")
    
    ## initialize the solver
    #solver.init()
      
    ## Initial condition
    #solver.setInput(vars_init,NLP_X_INIT)
    
    ## Bounds on x
    #solver.setInput(vars_lb,NLP_LBX)
    #solver.setInput(vars_ub,NLP_UBX)
    
    ## Bounds on g
    #solver.setInput(concatenate(lbg),NLP_LBG)
    #solver.setInput(concatenate(ubg),NLP_UBG)
    
    ## Solve the problem
    #solver.solve()
    
    ## Print the optimal cost
    #print "optimal cost: ", float(solver.output(NLP_COST))
    
    ## Retrieve the solution
    #v_opt = array(solver.output(NLP_X_OPT))
    #l_opt = array(solver.output(NLP_LAMBDA_G))
    
##    mult_opt = array(solver.output(NLP_LAMBDA_OPT))
##    mult_lbx = array(solver.output(NLP_LAMBDA_LBX))
##    mult_ubx = array(solver.output(NLP_LAMBDA_UBX))
    
    #xD_opt = resize(array([],dtype=MX),(ndiff,(deg+1)*nicp*(nk)+1))
    #xA_opt = resize(array([],dtype=MX),(nalg,(deg)*nicp*(nk)))
    #u_opt = resize(array([],dtype=MX),(nu,(deg+1)*nicp*(nk)+1))
    #offset = 0
    #offset2 = 0
    #offset3 = 0
    #offset4 = 0
    
    #s_opt = v_opt[:NS]
    #offset += NS
    
    #for k in range(nk):  
        #for i in range(nicp):
            #for j in range(deg+1):
                #xD_opt[:,offset2] = v_opt[offset:offset+ndiff][:,0]
                #offset2 += 1
                #offset += ndiff
                #if j!=0:
                    #xA_opt[:,offset4] = v_opt[offset:offset+nalg][:,0]
                    #offset4 += 1
                    #offset += nalg
        #utemp = v_opt[offset:offset+nu][:,0]
        #for i in range(nicp):
            #for j in range(deg+1):
                #u_opt[:,offset3] = utemp
                #offset3 += 1
        ##    u_opt += v_opt[offset:offset+nu]
        #offset += nu
        
    #xD_opt[:,-1] = v_opt[offset:offset+ndiff][:,0]
    
    #tgrid = append(Tint,Tint[-1])
    #filename = 'powerplaneSingleClose'
    #savetxt(filename + '_time.txt', tgrid)
    #savetxt(filename + '_diffState.txt',xD_opt)
    #savetxt(filename + '_algState.txt',xA_opt)
    #savetxt(filename + '_controls.txt', u_opt)
    #savetxt(filename + '_parameters.txt', P)
    
    #tgrid = loadtxt(filename + '_time.txt')
    #x_opt = loadtxt(filename + '_diffState.txt')
##    xA_opt = loadtxt(filename + '_algState.txt')
    #u_opt = loadtxt(filename + '_controls.txt')

    

    #X = x_opt.T
    #U = u_opt.T
    #s0 = s_opt
    
    #Xref = [XDr[0]]
    #Uref = [Ur[0]]

##    assert(1==0)
    
##    X = Xsim
##    U = Usim
    #Tsol = tgrid
    
##    print 'Initial quaternion', X[0][6:10]
    #print 'Slack variables', s_opt
##    assert(1==0)
##    
##    
    ## NMPC
    #x0 = X[0,:]
    #u0 = U[0,:]
    
    #TF = 4*tf
##    TF = 4*h+h
    #simtime = np.arange(0,TF,h)
##    simtime = np.arange(0,h,h)
    
    #Xsim = np.array([x0])
    #Usim = np.array([u0])
    #Ssim = np.array([s0])
    
    #rA = 1.085;
    #z = 0
    #r = 1.18
    ##P[1] = 0.5
    #count=1
    #for t in simtime:
        #print count
        #count +=1
##        P[1] = np.random.rand(1)*1e-1
##        if t > 2*tf :
##            P[0] = 2.
        #res = RotModelCarouselR(XDs,XDDOTs,Us,P,Scaling,"")
        ## Outputs of the DAE right hand side function
        #ffcn_out = [res]
        ## Create a DAE right hand side 
        #ffcn = SXFunction(ffcn_in,ffcn_out)
        #ffcn.init()
        
        #integrator = IdasIntegrator(ffcn)    
        #integrator.setOption("abstol",1e-8) # tolerance
        #integrator.setOption("reltol",1e-8) # tolerance
    ##    integrator.setOption("linear_solver","user_defined") 
    ##    integrator.setOption("linear_solver_creator",CSparse) 
        #integrator.setOption("exact_jacobian",True) 
        #integrator.setOption("steps_per_checkpoint",1000) 
        #integrator.setOption("tf",h)
        #integrator.init()
        #integrator.setInput(list(x0),0)
        #integrator.setInput(list(u0),1)
        #integrator.evaluate()
        #[x0] = np.array(integrator.output()).T
        
        #xr = x0[0:3] - np.array([-sin(x0[-2]),cos(x0[-2]),0])*rA
        #print np.dot(x0[6:9],x0[6:9]), np.dot(x0[9:12],x0[9:12]), np.dot(x0[12:15],x0[12:15])
        #print np.dot(xr,xr)-x0[-4]**2
        
        #Xsim = np.append(Xsim,np.array([x0]),axis=0)
##        x0=X[0]
        
        ## Compute the new reference
        #delta,ddelta = x0[-2], x0[-1]
        #if t > tf/4 and z < 2:
            #z += 0.04
            #r += 0.06
        #delta = XDr[1,-2]
        #ddelta = XDr[1,-1]
        #print z, r
##        assert(1==0)
##        XDr1, Ur1 = Reference(x0)
        #XDr, Ur = Reference(z,r,delta,ddelta)
        ## Shift the reference
##        XDr = XDr[1:]
##        Ur  = Ur[1:]
        
        #Obj = 0
        #for k in range(nk):
            #[obj] = mfcn.call([XD[k][0][0],U[k],S,XDr[k],Ur[k]])
            #Obj += obj
        #[obj] = mfcn.call([XD[nk][0][0],np.zeros(4),S,XDr[nk],np.zeros(4)])
        #Obj += obj*1e2
        
        #ofcn = MXFunction([V], [Obj])
        
        #solver = IpoptSolver(ofcn,gfcn)
    
        ## Set options
        #solver.setOption("expand_f",True)
        #solver.setOption("expand_g",True)
        #solver.setOption("generate_hessian",True)
        #solver.setOption("max_iter",1000)
        #solver.setOption("tol",1e-2)
        #solver.setOption("acceptable_tol",1e-2)
##        solver.setOption("linear_solver","ma57")
    
        ## initialize the solver
        #solver.init()
        
        ## shift the solution
        #shift = nx*(deg+1)+nu+NS
        
        #xN = v_opt[-nx:]
##        uN_1 = v_opt[-nx-nu:-nx]        # the last control input
##        uN_1 = v_opt[shift-4:shift]     # the first control input
        #uN_1 = Ur[nk-1]     # the next control input in the reference
##        assert(1==0)
        
        #sim.setInput(xN,0)
        #sim.setInput(uN_1,1)
        #sim.evaluate()
        #x_new = np.concatenate(np.array(sim.output()))
        #v_new = np.concatenate([np.array([x_new]).T,np.array([uN_1]).T,np.array([x_new[-nx:]]).T])
        
        #v_init = np.append(v_opt[:NS],np.append(v_opt[shift:-nx],v_new))#################################################################
        ##assert(1==0)
        ## New initial point
        #vars_lb[3:25] = list(x0)
        #vars_ub[3:25] = list(x0)
        #solver.setInput(vars_lb,NLP_LBX)
        #solver.setInput(vars_ub,NLP_UBX)
        
        #solver.setInput(v_init,NLP_X_INIT)
##        solver.setInput(v_opt,NLP_X_INIT)

        ## Bounds on g
        #solver.setInput(concatenate(lbg),NLP_LBG)
        #solver.setInput(concatenate(ubg),NLP_UBG)
        
        ## Solve the problem
        #solver.solve()
        
        ## Retrieve the solution
        #v_opt = np.array(solver.output(NLP_X_OPT))
        #l_opt = np.array(solver.output(NLP_LAMBDA_G))
        
##        assert(1==0)
##        v_opt = v_init
        
        #xD_opt = resize(array([],dtype=MX),(ndiff,(deg+1)*nicp*(nk)+1))
        #xA_opt = resize(array([],dtype=MX),(nalg,(deg)*nicp*(nk)))
        #u_opt = resize(array([],dtype=MX),(nu,(deg+1)*nicp*(nk)+1))
        #offset = 0
        #offset2 = 0
        #offset3 = 0
        #offset4 = 0
        
        #s_opt = v_opt[:NS]
        #offset += NS
        
        #for k in range(nk):  
            #for i in range(nicp):
                #for j in range(deg+1):
                    #xD_opt[:,offset2] = v_opt[offset:offset+ndiff][:,0]
                    #offset2 += 1
                    #offset += ndiff
                    #if j!=0:
                        #xA_opt[:,offset4] = v_opt[offset:offset+nalg][:,0]
                        #offset4 += 1
                        #offset += nalg
            #utemp = v_opt[offset:offset+nu][:,0]
            #for i in range(nicp):
                #for j in range(deg+1):
                    #u_opt[:,offset3] = utemp
                    #offset3 += 1
            ##    u_opt += v_opt[offset:offset+nu]
            #offset += nu
            
        #xD_opt[:,-1] = v_opt[offset:offset+ndiff][:,0]
        
        #u0 = u_opt[:,0]
        #s0 = s_opt
        #print 'Slack variables', s_opt
        #Usim = np.append(Usim,[u0],axis=0)
        #Ssim = np.append(Ssim,[s0],axis=0)
        #Xref = np.append(Xref,[XDr[0]],axis=0)
        #Uref = np.append(Uref,[Ur[0]],axis=0)
    ##    assert(1==0)
        
    #Xsol = Xsim.T
    #Usim = Usim
    #Tint = np.append(simtime,simtime[-1]+h)
##    Xref = np.append(Xref,[XDr[1]],axis=0)
##    Uref = np.append(Uref,[Ur[1]],axis=0)

##    X = xD_opt.T
##    U = u_opt.T


##    Tr = np.linspace(0,tf,nk+1)
##    Tr = np.loadtxt("refT.txt")
##    Tr = Tr[:-1]
    #Tr = Tint
##    XDr = np.loadtxt("refXD.txt")
##    Ur = np.loadtxt("refU.txt")
    #XDr = Xref
    #Ur = Uref
    #X = Xsim
    #U = Usim
    #Tsol = Tint
    
    #filename = 'NMPC_0ms_1.5dist_'
    #filename = 'NMPC_test_'
    #np.savetxt(filename + 'XDr.txt',XDr)
    #np.savetxt(filename + 'Ur.txt',Ur)
    #np.savetxt(filename + 'Tr.txt',Tr)
    #np.savetxt(filename + 'X.txt',X)
    #np.savetxt(filename + 'U.txt',U)
    #np.savetxt(filename + 'Tsol.txt',Tsol)
    #if True:
        ## Plot    
        #plt.ion()
        ## Plot the results
        #fig = plt.figure()
        #fig.clf()
        #ax = Axes3D(fig, aspect='equal')
        ##        for k in range(len(Tsol)):
        ##            ax.plot(X[k,0] + [0., X[k,6] ],X[k,1] + [0., X[k,9] ],(X[k,2]) + [0., X[k,12]], 'k')
        ##            ax.plot(X[k,0] + [0., X[k,7] ],X[k,1] + [0., X[k,10]],(X[k,2]) + [0., X[k,13]], 'b')
        ##            ax.plot(X[k,0] + [0., X[k,8] ],X[k,1] + [0., X[k,11]],(X[k,2]) + [0., X[k,14]], 'r')
        ##ax.plot(x,y,z*0+z[0])
        ##plt.plot(X[:,0], X[:,1], 'k',linewidth=2.0)
        #ax.plot(X[:,0], X[:,1], X[:,2], 'k',linewidth=2.0)
        #ax.plot(XDr[:,0], XDr[:,1], XDr[:,2], '.k',linewidth=2.0)
        #ax.plot([0, 0], [0,0], [-2,2], 'k',linewidth=0.0) # just to adjust scaling
        #w0 = w0/10.
        #ax.plot([0,w0], [0,0], [-0,0], 'b',linewidth=2.0)
        #ax.plot([w0,w0-w0/4.], [0,-w0/8.], [-0,0], 'b',linewidth=2.0)
        #ax.plot([w0,w0-w0/4.], [0, w0/8.], [-0,0], 'b',linewidth=2.0)
        ##        ax.quiver([0,0], [0,0], [-0,0],[0,w0], [0,0], [-0,0], 'b',linewidth=2.0)
        #ax.set_xlabel('x')
        #ax.set_ylabel('y')
        #ax.set_zlabel('z')
        
##        plt.figure()
##        plt.plot(Tsol,X[:,2], 'r')
##        plt.ylabel('z')
        
        #plt.figure()
        #plt.subplot(2,1,1)
        #plt.grid(True)
        #plt.plot(Tsol,X[:,0], 'k')
        #plt.plot(Tsol,X[:,1], 'b')
        #plt.plot(Tsol,X[:,2], 'r')
        #plt.plot(Tsol,-rA*sin(X[:,-2]), '-.k')
        #plt.plot(Tsol, rA*cos(X[:,-2]), '-.b')
        #plt.plot(Tr,XDr[:,0], '.k')
        #plt.plot(Tr,XDr[:,1], '.b')
        #plt.plot(Tr,XDr[:,2], '.r')
        #plt.legend(['x','y','z','xA','yA'])
        #plt.subplot(2,1,2)
        #plt.grid(True)
        #plt.plot(Tsol,X[:,3], 'k')
        #plt.plot(Tsol,X[:,4], 'b')
        #plt.plot(Tsol,X[:,5], 'r')
        #plt.plot(Tr,XDr[:,3], '.k')
        #plt.plot(Tr,XDr[:,4], '.b')
        #plt.plot(Tr,XDr[:,5], '.r')
        #plt.legend(['dx','dy','dz'])
        
        #plt.figure()
        #plt.grid(True)
        #plt.plot(Tsol,X[:,15], 'k')
        #plt.plot(Tsol,X[:,16], 'b')
        #plt.plot(Tsol,X[:,17], 'r')
        #plt.plot(Tr,XDr[:,15], '.k')
        #plt.plot(Tr,XDr[:,16], '.b')
        #plt.plot(Tr,XDr[:,17], '.r')
        #plt.legend(['wx','wy','wz'])
        
        #plt.figure()
        #plt.subplot(3,1,1)
        #plt.grid(True)
        #plt.plot(Tsol,X[:,6], 'b')
        #plt.plot(Tsol,X[:,7], 'k')
        #plt.plot(Tsol,X[:,8], 'r')
        #plt.plot(Tr,XDr[:,6], '.b')
        #plt.plot(Tr,XDr[:,7], '.k')
        #plt.plot(Tr,XDr[:,8], '.r')
        #plt.legend(['e11','e12','e13'])
        #plt.subplot(3,1,2)
        #plt.grid(True)
        #plt.plot(Tsol,X[:,9], 'b')
        #plt.plot(Tsol,X[:,10], 'k')
        #plt.plot(Tsol,X[:,11], 'r')
        #plt.plot(Tr,XDr[:,9], '.b')
        #plt.plot(Tr,XDr[:,10], '.k')
        #plt.plot(Tr,XDr[:,11], '.r')
        #plt.legend(['e21','e22','e23'])
        #plt.subplot(3,1,3)
        #plt.grid(True)
        #plt.plot(Tsol,X[:,12], 'b')
        #plt.plot(Tsol,X[:,13], 'k')
        #plt.plot(Tsol,X[:,14], 'r')
        #plt.plot(Tr,XDr[:,12], '.b')
        #plt.plot(Tr,XDr[:,13], '.k')
        #plt.plot(Tr,XDr[:,14], '.r')
        #plt.legend(['e31','e32','e33'])
        
        #plt.figure()
        #plt.plot(Tsol,X[:,18], 'g')
        #plt.plot(Tsol,X[:,19], 'k')
        #plt.plot(Tsol,X[:,20], 'b')
        #plt.plot(Tsol,X[:,21], 'r')
        #plt.plot(Tr,XDr[:,18], '.g')
        #plt.plot(Tr,XDr[:,19], '.k')
        #plt.plot(Tr,XDr[:,20], '.b')
        #plt.plot(Tr,XDr[:,21], '.r')
        #plt.legend(['r','dr','delta','ddelta'])
        
        #plt.figure()
        #plt.subplot(3,1,1)
        #plt.grid(True)
        #plt.step(Tsol,U[:,2], 'g')
        #plt.step(Tsol,U[:,3], 'k')
        #plt.legend(('ua','ue'))
        #plt.subplot(3,1,2)
        #plt.grid(True)
        #plt.step(Tsol,U[:,0], 'k')
        #plt.ylabel('ddr')
        #plt.subplot(3,1,3)
        #plt.grid(True)
        #plt.step(Tsol,U[:,1], 'k')
        #plt.ylabel('dddelta')
        ##    plt.legend(['r','dr','delta','ddelta'])
        
        #F = zeros((len(Tsol),3))
        #T = zeros((len(Tsol),3))
        #wE = zeros((len(Tsol),3))
        #alpha = zeros(len(Tsol))
        #beta = zeros(len(Tsol))
        #alphaT = zeros(len(Tsol))
        #[CL,CLS,CLs] = zeros((3,len(Tsol)))
        #[betaS,betas,alphaTS,alphaTs] = zeros((4,len(Tsol)))
        #lambda_ = zeros(len(Tsol))
        #for k in range(len(Tsol)):
            #FTfcn.setInput(list(X[k]),0)
            #FTfcn.setInput(list(X[k]*0),1)
            #FTfcn.setInput(list(U[k]),2)
            #FTfcn.evaluate()
            #Fk = FTfcn.output(0)
            #Tk = FTfcn.output(1)
            #alpha[k] = FTfcn.output(2)
            #beta[k] = FTfcn.output(3)
            #alphaT[k] = FTfcn.output(4)
            #wEk = FTfcn.output(5)
            #F[k] = [Fk[0,0], Fk[1,0], Fk[2,0]]
            #T[k] = [Tk[0,0], Tk[1,0], Tk[2,0]]
            #wE[k] = [wEk[0,0], wEk[1,0], wEk[2,0]]
            #CLfcn.setInput(list(X[k]),0)
            #CLfcn.setInput(list(U[k]),1)
            #CLfcn.evaluate()
            #CLk = CLfcn.output()
            #CL[k] = CLk
            #Lfcn.setInput(list(X[k]),0)
            #Lfcn.setInput(list(U[k]),1)
            #Lfcn.evaluate()
            #lambdak = Lfcn.output()
            #lambda_[k] = lambdak
            
            #CLS[k] = 1+Ssim[k,0]
            #CLs[k] =  -Ssim[k,0]
            #alphaTS[k] =  10+Ssim[k,1]*180/pi
            #alphaTs[k] = -10-Ssim[k,1]*180/pi
            #betaS[k] =  15+Ssim[k,2]*180/pi
            #betas[k] = -15-Ssim[k,2]*180/pi
            
            
        
        #plt.figure()
        #plt.subplot(3,1,1)
        #plt.plot(Tsol,F[:,0],'b')
        #plt.plot(Tsol,F[:,1],'r')
        #plt.plot(Tsol,F[:,2],'k')
        #plt.grid(True)
        #plt.legend(('Fx','Fy','Fz'))
        #plt.subplot(3,1,2)
        #plt.plot(Tsol,T[:,0],'b')
        #plt.plot(Tsol,T[:,1],'r')
        #plt.plot(Tsol,T[:,2],'k')
        #plt.grid(True)
        #plt.legend(('Tx','Ty','Tz'))
        #plt.subplot(3,1,3)
        #plt.plot(Tsol,wE[:,0],'b')
        #plt.plot(Tsol,wE[:,1],'r')
        #plt.plot(Tsol,wE[:,2],'k')
        #plt.grid(True)
        ##    plt.legend(('wEx','wEy','wEz'))
        
        
        #plt.figure()
        #plt.subplot(2,1,1)
        #plt.plot(Tsol,alpha*180/pi,'b')
        #plt.plot(Tsol,beta*180/pi,'r')
        #plt.plot(Tsol,alphaT*180/pi,'k')
        ##        plt.plot(Tsol,np.ones(len(Tsol))*15.+s_opt[2]*180/pi,'--r')
        ##        plt.plot(Tsol,np.ones(len(Tsol))*(-15.)-s_opt[2]*180/pi,'--r')
        #plt.step(Tsol,betaS,':r')
        #plt.step(Tsol,betas,':r')
        #plt.plot(Tsol,np.ones(len(Tsol))*15.,'-.r')
        #plt.plot(Tsol,np.ones(len(Tsol))*(-15.),'-.r')
        #plt.step(Tsol,alphaTS,':k')
        #plt.step(Tsol,alphaTs,':k')
        #plt.plot(Tsol,np.ones(len(Tsol))*10.,'-.k')
        #plt.plot(Tsol,np.ones(len(Tsol))*(-10.),'-.k')
        #plt.grid(True)
        #plt.legend(('alpha','beta','alphaT'))
        #plt.subplot(2,1,2)
        #plt.plot(Tsol,CL,'b')
        #plt.step(Tsol,CLS,'--r')
        #plt.step(Tsol,CLs,'--r')
        #plt.plot(Tsol,np.ones(len(Tsol)),'-k')
        #plt.plot(Tsol,np.zeros(len(Tsol)),'-k')
        #plt.grid(True)
        #plt.ylabel('CL')
        
        
        #plt.figure()
        #plt.plot(Tsol,lambda_,'b')
        #plt.grid(True)
        #plt.ylabel('lambda')
        
        #plt.draw()
    
    
    
#toc = time.time()
#print toc-tic

    
