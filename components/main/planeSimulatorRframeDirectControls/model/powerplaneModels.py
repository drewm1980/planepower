# -*- coding: utf-8 -*-
"""
Created on Sat Nov 12 14:41:22 2011

@author: mzanon
"""

import numpy as np
from casadi import*
#from numpy import*



def RotModelCarouselR(XD,U,P,Scaling,flag):
    """ Simplified rotational model:
            - Assumption: direct control of the rotational accelerations
            - Linear relation CL vs angle of attack
        """
    

    varsxd = ["x","y","z","dx","dy","dz","e11","e12","e13","e21","e22","e23","e31","e32","e33","w1","w2","w3","delta","ddelta"] 
#    varsxddot = ["xdot","ydot","zdot","dxdot","dydot","dzdot","e11dot","e12dot","e13dot","e21dot","e22dot","e23dot","e31dot","e32dot","e33dot","w1dot","w2dot","w3dot","rdot","drdot","deltadot","ddeltadot"] 
    #    varsxa = ["lambda_"] 
#    varsxa = [] 
    varsu  = ["dddelta","ua","ue"] 
    varsp  = ["w0","r"] 
     
    if XD.shape[0] != len(varsxd):
        print "Wrong dimension for vector XD: ", XD.shape[0], " (expected ", len(varsxd), ")"
#    if XDDOT.shape[0] != len(varsxddot):
#        print "Wrong dimension for vector XDDOT: ", XDDOT.shape[0], " (expected ", len(varsxddot), ")"
#    if XA.shape[0] != len(varsxa):
#        print "Wrong dimension for vector XA: ", XA.shape[0], " (expected ", len(varsxa), ")"
    if U.shape[0] != len(varsu):
        print "Wrong dimension for vector U: ", U.shape[0], " (expected ", len(varsu), ")"
    if P.shape[0] != len(varsp):
        print "Wrong dimension for vector P: ", P.shape[0], " (expected ", len(varsp), ")"
#    if len(Scaling) != len(varsS):
#        print "Wrong dimension for vector Scaling: ", len(Scaling), " (expected ", len(varsS), ")"
        
        
    Tfac = 1.
    
    for k in range(len(varsp)):
        exec(varsp[k]+"= P[k]")
    for k in range(len(varsxd)):
        exec(varsxd[k]+"= XD[k]*Scaling[k]")
#    for k in range(len(varsxddot)):
#        exec(varsxddot[k]+"= XDDOT[k]/Tfac")
#    for k in range(len(varsxa)):
#        exec(varsxa[k]+"= XA[k]")
    for k in range(len(varsu)):
        exec(varsu[k]+"= U[k]")
#    for k in range(len(varsS)):
#        exec(varsS[k]+"= Scaling[k]")
    
    #  PARAMETERS OF THE KITE :
    #  -----------------------------
    mk =  0.626;      #  mass of the kite               #  [ kg    ]
    #  A =  0.2;      #  effective area                 #  [ m^2   ]
    
    
    #   PHYSICAL CONSTANTS :
    #  -----------------------------
    g =    9.81;      #  gravitational constant         #  [ m /s^2]
    rho =    1.23;      #  density of the air             #  [ kg/m^3]
    
    #  PARAMETERS OF THE CABLE :
    #  -----------------------------
    rhoc = 1450.00;      #  density of the cable           #  [ kg/m^3]
    #cc =   1.00;      #  frictional constant            #  [       ]
    dc = 1e-3;      #  diameter                       #  [ m     ]
    
    
    
    AQ      =  pi*dc*dc/4.0;
    
    
    #CAROUSEL ARM LENGTH
    rA = 1.085; #(dixit Kurt)
    
    zT = -0.02;
    # 		 XT = [0;0;-0.01];
    
    # 		 ZT = 0;
    #             YT = 0.005;
    
    #INERTIA MATRIX (Kurt's direct measurements)
    I1 = 0.0163;
    I31 = 0.0006;
    I2 = 0.0078;
    I3 = 0.0229;
    
    #IMU POSITION & ANGLE
    #XIMU1 = 0.0246;
    #XIMU2 = -0.0116;
    #XIMU3 = -0.0315;
    # 		 alphaIMU = 0*pi/180;#4
    # 		 betaIMU = 0*pi/180;
    # 		 deltaIMU = 0*pi/180;
    
    # 		 alpha0 = -0*PI/180; 
    
    #TAIL LENGTH
    LT = 0.4;
    
    
    #ROLL DAMPING
    RD = 1e-2; 
    PD = 0*1e-3;
    YD = 0*1e-3;
    #WIND-TUNNEL PARAMETERS
    
    #Lift (report p. 67)
    CLA = 5.064;
    
    CLe = -1.924;#e-5;#0.318;#
    
    CL0 = 0.239;
    
    #Drag (report p. 70)
    CDA = -0.195;
    CDA2 = 4.268;
    CDB2 = 5;#0;#
    # 		 CDe = 0.044;
    # 		 CDr = 0.111;
    CD0 = 0.026;
    
    #Roll (report p. 72)
    CRB = -0.062;
    CRAB = -0.271; 
    CRr = -5.637e-1;#e-6;#-0.244;#
    
    #Pitch (report p. 74)
    CPA = 0.293;
    CPe = -4.9766e-1;#e-6;#-0.821;#
    
    CP0 = 0.03;
    
    #Yaw (report p. 76)
    CYB = 0.05;
    CYAB = 0.229;
    
    SPAN = 0.96;
    CHORD = 0.1;
    
        
    e1 = horzcat([e11,e12,e13])
    e2 = horzcat([e21,e22,e23])
    e3 = horzcat([e31,e32,e33])
    R = vertcat([e1,e2,e3])
    
    #                        MODEL EQUATIONS :
    # ===============================================================

    # CROSS AREA OF THE CABLE :
    # ---------------------------------------------------------------
    AQ      =  pi*dc*dc/4.0                                       ;
    
    # THE EFECTIVE MASS' :
    # ---------------------------------------------------------------
    
    mc      =  0*rhoc*AQ*r  ;   # mass of the cable
    m       =  mk + mc/3.0;   # effective inertial mass
    mgrav   =  mk + mc/2.0;   # effective inertial mass
    
    # -----------------------------   # ----------------------------
    #   dm      =  (rhoc*AQ/ 3.0)*dr;   # time derivative of the mass
    
    # WIND SHEAR MODEL :
    # ---------------------------------------------------------------
    dw = 0
    w = w0 + dw
    
    # EFFECTIVE WIND IN THE KITE`S SYSTEM :
    # ---------------------------------------------------------------	    
    we = SXMatrix(3,1,0)
    we[0]   = -w + dx - ddelta*y;
    we[1]   =	   dy + ddelta*rA + ddelta*x;
    we[2]   =	   dz;
    
    VKite2 = (we[0]*we[0] + we[1]*we[1] + we[2]*we[2]); 
    VKite = sqrt(VKite2); 
    
#    # CALCULATION OF THE KITE`S AXIS :
#    # ---------------------------------------------------------------
#    
#    er = SXMatrix(3,1,0)  
#    er[0] = x/r;
#    er[1] = y/r;
#    er[2] = z/r;
#
#    
#    # Velocity accross X (cable drag)
#    wp = SXMatrix(3,1,0)  
#    wp = er[0]*we[0] + er[1]*we[1] + er[2]*we[2];
#	
#    
#    wep = SXMatrix(3,1,0)  
#    wep[0] = we[0] - wp*er[0];
#    wep[1] = we[1] - wp*er[1];
#    wep[2] = we[2] - wp*er[2];
    
    # LIFT DIRECTION VECTOR
    # -------------------------
    
    #Relative wind speed in Airfoil's referential 'E' 
    wE = SXMatrix(3,1,0)
    wE[0] = e11*we[0]  + e21*we[1]  + e31*we[2];
    wE[1] = e12*we[0]  + e22*we[1]  + e32*we[2];
    wE[2] = e13*we[0]  + e23*we[1]  + e33*we[2];
    #    wE = SXMatrix(3,1,0)  
    #    wE[0] = (q0*q0 + q1*q1 - q2*q2 - q3*q3)*we[0]  +          (2*q0*q3 + 2*q1*q2)*we[1]  +          (2*q1*q3 - 2*q0*q2)*we[2];
    #    wE[1] =         (2*q1*q2 - 2*q0*q3)*we[0]  +  (q0*q0 - q1*q1 + q2*q2 - q3*q3)*we[1]  +          (2*q0*q1 + 2*q2*q3)*we[2];
    #    wE[2] =         (2*q0*q2 + 2*q1*q3)*we[0]  +          (2*q2*q3 - 2*q0*q1)*we[1]  +  (q0*q0 - q1*q1 - q2*q2 + q3*q3)*we[2];
    
    
    
    #Airfoil's transversal axis in fixed referential 'e'
#    eTe = R[:,1]; # = mul(R.T,[0,1,0])
    eTe = SXMatrix(3,1,0)  
    eTe[0] = e12;
    eTe[1] = e22;
    eTe[2] = e32;
    
    
    # Lift axis ** Normed to we !! **
    eLe = SXMatrix(3,1,0)  
    eLe[0] = - eTe[1]*we[2] + eTe[2]*we[1];
    eLe[1] = - eTe[2]*we[0] + eTe[0]*we[2];
    eLe[2] = - eTe[0]*we[1] + eTe[1]*we[0];
    
    
    # AERODYNAMIC COEEFICIENTS
    # ----------------------------------
    #VT = cross([w1;w2;w3],[-LT;0;0]) + wE;
    VT = SXMatrix(3,1,0)  
    VT[0] =          wE[0];
    VT[1] = -LT*w3 + wE[1];
    VT[2] =  LT*w2 + wE[2];
    
    
    #Note: beta & alphaTail are compensated for the tail motion induced by omega
    alpha = -wE[2]/wE[0];
#    betaTail = VT[1]/sqrt(VT[0]*VT[0] + VT[2]*VT[2]);
#    beta = wE[1]/sqrt(wE[0]*wE[0] + wE[2]*wE[2]);
    betaTail = VT[1]/VT[0];
    beta = wE[1]/wE[0];
    alphaTail = -VT[2]/VT[0];
    
    ur = ua
    up = ue
    
    CL = CLA*alpha + CLe*up + CL0;
    CD = CDA*alpha + CDA2*alpha*alpha + CDB2*beta*beta + CD0;
    CR = -RD*w1 + CRB*betaTail + CRr*ur + CRAB*alphaTail*betaTail;
    CP = -PD*w2 + CPA*alphaTail + CPe*up + CP0;
    CY = -YD*w3 + CYB*betaTail + CYAB*alphaTail*betaTail;

#    Cf = rho*dc*r*VKite/8.0;
         
    
    # THE FRICTION OF THE CABLE :
    # ---------------------------------------------------------------
    
#    Ff = SXMatrix(3,1,0)  
#    Ff[0] = -rho*dc*r*VKite*cc*wep[0]/8.0;
#    Ff[1] = -rho*dc*r*VKite*cc*wep[1]/8.0;
#    Ff[2] = -rho*dc*r*VKite*cc*wep[2]/8.0;

    # LIFT :
    # ---------------------------------------------------------------
    CL = 0.2*CL;
    FL = SXMatrix(3,1,0)  
    FL[0] =  rho*CL*eLe[0]*VKite/2.0;
    FL[1] =  rho*CL*eLe[1]*VKite/2.0;
    FL[2] =  rho*CL*eLe[2]*VKite/2.0;

    # DRAG :
    # ---------------------------------------------------------------
    CD = 0.5*CD;
    FD = SXMatrix(3,1,0)  
    FD[0] = -rho*VKite*CD*we[0]/2.0;
    FD[1] = -rho*VKite*CD*we[1]/2.0; 
    FD[2] = -rho*VKite*CD*we[2]/2.0; 

    
    # FORCES (AERO)
    # ---------------------------------------------------------------
    
    F = SXMatrix(3,1,0)  
    F[0] = FL[0] + FD[0];
    F[1] = FL[1] + FD[1];
    F[2] = FL[2] + FD[2];
    
#    # TORQUES (AERO)
#    # ---------------------------------------------------------------
#    
#    T = SXMatrix(3,1,0)  
#    T[0] =  0.5*rho*VKite2*SPAN*CR;
#    T[1] =  0.5*rho*VKite2*CHORD*CP;
#    T[2] =  0.5*rho*VKite2*SPAN*CY;
#    
    
    # ATTITUDE DYNAMICS
    # -----------------------------------------------------------
    
    
    W = vertcat([horzcat([0,-w3,w2]),
                 horzcat([w3,0,-w1]),
                 horzcat([-w2,w1,0])])
    
    RotPole = 1/2;
    RP = RotPole*mul(R,(inv(mul(R.T,R)) - np.eye(3)));
#    dR = mul(R,W) + RP;
    de11 = e12*w3 - e13*w2 + ddelta*e21;
    de12 = e13*w1 - e11*w3 + ddelta*e22;
    de13 = e11*w2 - e12*w1 + ddelta*e23;
    de21 = e22*w3 - e23*w2 - ddelta*e11;
    de22 = e23*w1 - e21*w3 - ddelta*e12; 
    de23 = e21*w2 - e22*w1 - ddelta*e13;
    de31 = e32*w3 - e33*w2;
    de32 = e33*w1 - e31*w3;
    de33 = e31*w2 - e32*w1; 
    

    de1 = horzcat([de11,de12,de13])
    de2 = horzcat([de21,de22,de23])
    de3 = horzcat([de31,de32,de33])
    dR = vertcat([de1,de2,de3]) + RP; 
    
    #    dq0 = (-q1*w1 - q2*w2 - q3*w3)/2;
    #    dq1 = ( q0*w1 - q3*w2 + q2*w3)/2;
    #    dq2 = ( q3*w1 + q0*w2 - q1*w3)/2;
    #    dq3 = (-q2*w1 + q1*w2 + q0*w3)/2;


    
    ######################################################################
    #                                                                    # 
    #  AUTO-GENERATED EQUATIONS (S. Gros, HIGHWIND, OPTEC, KU Leuven)    # 
    #                                                                    # 
    ###################################################################### 
    
	# Inverse of the Generalized inertia matrix 
    IMA = SXMatrix(3,3,0) ; 
    IMA[0,0] = 1/m; 
    IMA[0,1] = 0; 
    IMA[0,2] = 0; 
    IMA[1,0] = 0; 
    IMA[1,1] = 1/m; 
    IMA[1,2] = 0; 
    IMA[2,0] = 0; 
    IMA[2,1] = 0; 
    IMA[2,2] = 1/m; 
    
    # G1 (right hand-side up)
    G1 = SXMatrix(3,1,0); 
    G1[0,0] = F[0] + (m*(2*ddelta*dy + 2*ddelta*ddelta*rA + 2*ddelta*ddelta*x))/2 + ddelta*dy*m + dddelta*m*y; 
    G1[1,0] = F[1] - (m*(2*ddelta*dx - 2*ddelta*ddelta*y))/2 - (dddelta*m*(2*rA + 2*x))/2 - ddelta*dx*m; 
    G1[2,0] = F[2] - g*mgrav; 
    
    # G2 (right hand-side down) 
    G2 = - dx*dx - dy*dy - dz*dz; 

    # NabG 
    NabG = SXMatrix(3,1,0); 
    NabG[0,0] = x; 
    NabG[1,0] = y; 
    NabG[2,0] = z; 
    
    # Consistency Conditions 
    Const = - r*r/2 + x*x/2 + y*y/2 + z*z/2; 
    dConst = dx*x + dy*y + dz*z; 

    # LambdaFac * lambda_ = lambdaright 
    LambdaFac = mul(NabG.T,mul(IMA,NabG)); 
    
    # lambdaright 
    lambdaright = mul(NabG.T,mul(IMA,G1)) - G2; 
    
    # lambda_ 
    Pole = 1/2;
    lambda_ = (2*Pole*dConst + Pole*Pole*Const + lambdaright)/LambdaFac; 
    
    # ddq (accelerations) 
    ddq = mul(IMA,(G1-mul(NabG,lambda_))); 
    
    ############### END OF AUTO-GENERATED CODE ########################### 
    	     
    # TORQUES (BRIDLE)
    # ---------------------------------------------------------------
    # 	zT = -0.02;
    TB = SXMatrix(3,1,0)  
    TB[0] =  zT*(e12*lambda_*x + e22*lambda_*y + e32*lambda_*z);
    TB[1] = -zT*(e11*lambda_*x + e21*lambda_*y + e31*lambda_*z);
    TB[2] =  0;
    
    # TORQUES (AERO)
    # ---------------------------------------------------------------
    T = SXMatrix(3,1,0)  
    T[0] =  0.5*rho*VKite2*SPAN*CR   + TB[0];
    T[1] =  0.5*rho*VKite2*CHORD*CP  + TB[1];
    T[2] =  0.5*rho*VKite2*SPAN*CY   + TB[2];
    
    
#    IntermediateState dw1, dw2, dw3;
    # 	dw1 = (I31*(T[2] + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3) - (I3*(T[0] - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3); 
    # 	dw2 = (T[1] + w1*(I31*w1 + I3*w3) - w3*(I1*w1 + I31*w3))/I2; 
    # 	dw3 = (I31*(T[0] - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3) - (I1*(T[2] + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3); 
    dw1 = (I31*T[2])/(I31*I31 - I1*I3) - (I3*T[0])/(I31*I31 - I1*I3); 
    dw2 = T[1]/I2; 
    dw3 = (I31*T[0])/(I31*I31 - I1*I3) - (I1*T[2])/(I31*I31 - I1*I3); 
	
    from casadi_vector_ops import dot,cross

    ConstR1 = dot(e1,e1) - 1
    ConstR2 = dot(e1,e2)
    ConstR3 = dot(e1,e3)
    
    ConstR4 = dot(e2,e2) - 1
    ConstR5 = dot(e2,e3)
    
    ConstR6 = dot(e3,e3) - 1
    
    ConstRpos = dot(cross(e1,e2),e3)
    
    
    Fcable = lambda_*r;      
    
    
    # The residual:
    # ------------------------------------------------------------------
#    ["x","y","z","dx","dy","dz","n","Psi","CL","E"] 
    
    res = SXMatrix()
    res.append( dx )
    res.append( dy )
    res.append( dz )
    res.append( ddq[0] )
    res.append( ddq[1] )
    res.append( ddq[2] )
    res.append( dR[0,0] )
    res.append( dR[0,1] )
    res.append( dR[0,2] )
    res.append( dR[1,0] )
    res.append( dR[1,1] )
    res.append( dR[1,2] )
    res.append( dR[2,0] )
    res.append( dR[2,1] )
    res.append( dR[2,2] )
    res.append( dw1 )
    res.append( dw2 )
    res.append( dw3 )
#    res.append( dr )
#    res.append( ddr )
    res.append( ddelta )
    res.append( dddelta )
    #res.append( dua )
    #res.append( due )
    
    # Constraints
#    Const  = - r**2 + x**2 + y**2 + z**2
#    dConst = dx*x + dy*y + dz*z

    ddx = ddq[0]; ddy = ddq[1]; ddz = ddq[2]; 
    ddxIMU = ddx*e11 + ddy*e21 + ddz*e31 - ddelta*ddelta*e11*x - ddelta*ddelta*e21*y + 2*ddelta*dx*e21 - 2*ddelta*dy*e11 + dddelta*e21*rA + dddelta*e21*x - dddelta*e11*y - ddelta*ddelta*e11*rA;
    ddyIMU = ddx*e12 + ddy*e22 + ddz*e32 - ddelta*ddelta*e12*x - ddelta*ddelta*e22*y + 2*ddelta*dx*e22 - 2*ddelta*dy*e12 + dddelta*e22*rA + dddelta*e22*x - dddelta*e12*y - ddelta*ddelta*e12*rA;
    ddzIMU = ddx*e13 + ddy*e23 + ddz*e33 - ddelta*ddelta*e13*x - ddelta*ddelta*e23*y + 2*ddelta*dx*e23 - 2*ddelta*dy*e13 + dddelta*e23*rA + dddelta*e23*x - dddelta*e13*y - ddelta*ddelta*e13*rA;
    
    RIMU = np.loadtxt('../../../../properties/IMU/RIMU.dat');
        
    
    aE = vertcat([ddxIMU,ddyIMU,ddzIMU]); 
    
    w = vertcat([w1,w2,w3]);
    
    aIMU = mul(RIMU,aE);
    wIMU = mul(RIMU,w);
    IMU = vertcat([wIMU[0], wIMU[1], wIMU[2], aIMU[0], aIMU[1], aIMU[2]])
    IMU = vertcat([w1,w2,w3, ddxIMU,ddyIMU,ddzIMU])
    
    
    if flag == "":
        return res
    elif flag == "constraints":
        return Const, dConst, ConstR1, ConstR2, ConstR3, ConstR4, ConstR5, ConstR6, ConstRpos
    elif flag == "lambda":
        return lambda_
    elif flag == "IMU":
        return IMU
    elif flag == "ddX":
        return ddX[0], ddX[1], ddX[2]
    elif flag == "lift":
        return FL1
    elif flag == "wingtip":
        return x + 0.5*ws*eTR1x, y + 0.5*ws*eTR1y, z + 0.5*ws*eTR1z, x - 0.5*ws*eTR1x, y - 0.5*ws*eTR1y, z - 0.5*ws*eTR1z,
    elif flag == "FT":
        return F, T
    elif flag == "CL":
        return CL
    elif flag == "alphabeta":
        return alpha, beta, alphaTail, we
        
    else:
        print "Inexisting flag:", flag
    return

