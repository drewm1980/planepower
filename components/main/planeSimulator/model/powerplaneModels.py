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
    

    varsxd = ["x","y","z","dx","dy","dz","e11","e12","e13","e21","e22","e23","e31","e32","e33","w1","w2","w3","r","dr","delta","ddelta","ua","ue"] 
#    varsxddot = ["xdot","ydot","zdot","dxdot","dydot","dzdot","e11dot","e12dot","e13dot","e21dot","e22dot","e23dot","e31dot","e32dot","e33dot","w1dot","w2dot","w3dot","rdot","drdot","deltadot","ddeltadot"] 
    #    varsxa = ["lambda_"] 
#    varsxa = [] 
    varsu  = ["dddelta","ddr","dua","due"] 
    varsp  = ["w0"] 
     
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
    
    
    
#    Nturn = 2;
#    Ncvp = 40;
#    Torbit = 2*Nturn;
    
    
#    ua = dua;
#    ur = due;
    
    
    # Parameters
    # ------------------------------------------------------------------  
    # TAIL LENGTH
    LT = 0.45;
    
    
    # ROLL DAMPING
    #    RDfac = 1;
    #    RD0 = 1e-2; 
    #    RD = RDfac*RD0;
    RD = 1e-2;
	
    #  PARAMETERS OF THE KITE :
    #  -----------------------------
    mk =  0.463;      #  mass of the kite               #  [ kg    ]
    
    #   PHYSICAL CONSTANTS :
    #  -----------------------------
    g =    9.81;      #  gravitational constant         #  [ m /s^2]
    rho =    1.23;      #  density of the air             #  [ kg/m^3]
    
    #  PARAMETERS OF THE CABLE :
    #  -----------------------------
    rhoc = 1450.00;      #  density of the cable           #  [ kg/m^3]
    cc =   1.00;      #  frictional constant            #  [       ]
    dc = 1e-3;      #  diameter                       #  [ m     ]
    
    
    #CAROUSEL ARM LENGTH
    rA = 1.085; #(dixit Kurt)
    
    #INERTIA MATRIX (Kurt's direct measurements)
    # Note: low sensitivity to I1,2,3... high sensitivity to I31...
    I1 =  0.0163;
    I31 = 0.0006;
    I2 =  0.0078;
    I3 =  0.0229;
    				 
    #WIND-TUNNEL PARAMETERS				 
    #Lift (report p. 67)
    #Sensitivity to CLA error low
    CLA = 5.064;
    #Sensitivity to CLe error low
    CLe = 0.318;
    #Sensitivity to CLr error low
    CLr = 0.85; #?!?!?!?!?
    #HIGH sensitivity to CL0 !!
    CL0 = 0.239;
    					 
    #Drag (report p. 70)
    #Sensitivity to CDA error low
    CDA = -0.195;
    CDA2 = 4.268;
    CDB2 = 0;
    #Sensitivity to CDe error low
    CDe = 0.044;
    #Sensitivity to CDr error low
    CDr = 0.111;
    #Sensitivity to CD0 error low
    CD0 = 0.026;
    					 
    #Roll (report p. 72)
    #HIGH sensitivity to CRB !!
    CRB = -0.062;
    #HIGH sensitivity to CRAB !!
    CRAB = -0.271;
    #Sensitivity to CRr error low
    CRr = -0.244;
    				 
    #Pitch (report p. 74)
    #HIGH sensitivity to CPA !!
    CPA = 0.293;
    #Sensitivity to CPe error low
    CPe = -0.821;
    #Sensitivity to CPr error low
    CPr = -0.647; #?!?!?!?!?
    #HIGH sensitivity to CP0 !!
    CP0 = 0.03;
    					 
    #Yaw (report p. 76)
    #HIGH sensitivity to CYB !!
    CYB = 0.05;
    #HIGH sensitivity to CYAB !!
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
    
    mc      =  rhoc*AQ*r  ;   # mass of the cable
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
    we[0]   = -w + dx;
    we[1]   = dy;
    we[2]   = dz;
    
    VKite2 = (we[0]*we[0] + we[1]*we[1] + we[2]*we[2]); 
    VKite = sqrt(VKite2); 
    
    # CALCULATION OF THE KITE`S AXIS :
    # ---------------------------------------------------------------
    
    er = SXMatrix(3,1,0)  
    er[0] = x/r;
    er[1] = y/r;
    er[2] = z/r;

    
    # Velocity accross X (cable drag)
    wp = SXMatrix(3,1,0)  
    wp = er[0]*we[0] + er[1]*we[1] + er[2]*we[2];
	
    
    wep = SXMatrix(3,1,0)  
    wep[0] = we[0] - wp*er[0];
    wep[1] = we[1] - wp*er[1];
    wep[2] = we[2] - wp*er[2];
    
    # LIFT DIRECTION VECTOR
    # -------------------------
    
    #Relative wind speed in Airfoil's referential 'E'
    wE = mul(R.T,we);
    #    wE = SXMatrix(3,1,0)  
    #    wE[0] = (q0*q0 + q1*q1 - q2*q2 - q3*q3)*we[0]  +          (2*q0*q3 + 2*q1*q2)*we[1]  +          (2*q1*q3 - 2*q0*q2)*we[2];
    #    wE[1] =         (2*q1*q2 - 2*q0*q3)*we[0]  +  (q0*q0 - q1*q1 + q2*q2 - q3*q3)*we[1]  +          (2*q0*q1 + 2*q2*q3)*we[2];
    #    wE[2] =         (2*q0*q2 + 2*q1*q3)*we[0]  +          (2*q2*q3 - 2*q0*q1)*we[1]  +  (q0*q0 - q1*q1 - q2*q2 + q3*q3)*we[2];
    
    
    
    #Airfoil's transversal axis in fixed referential 'e'
    eTe = R[:,1]; # = mul(R.T,[0,1,0])
    #    eTe = SXMatrix(3,1,0)  
    #    eTe[0] =              2*q1*q2 - 2*q0*q3;        
    #    eTe[1] =      q0*q0 - q1*q1 + q2*q2 - q3*q3;
    #    eTe[2] =              2*q0*q1 + 2*q2*q3;  
    
    
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
    
    alpha = -wE[2]/wE[0];
    
    #Note: beta & alphaTail are compensated for the tail motion induced by omega
    beta = VT[1]/sqrt(VT[0]*VT[0] + VT[2]*VT[2]);
    alphaTail = -VT[2]/VT[0];
    
    u1 = ua
    u2 = ue
    
    CL = CLA*alpha + CLe*u2     + CLr*u1 + CL0;
    CD = CDA*alpha + CDA2*alpha*alpha + CDB2*beta*beta + CDe*u2 + CDr*u1 + CD0;
    CR = -RD*w1 + CRB*beta + CRAB*alphaTail*beta + CRr*u1;
    CP = CPA*alphaTail + CPe*u2 + CPr*u1 + CP0;
    CY = CYB*beta + CYAB*alphaTail*beta;

    Cf = rho*dc*r*VKite/8.0;
         
    
    # THE FRICTION OF THE CABLE :
    # ---------------------------------------------------------------
    
    Ff = SXMatrix(3,1,0)  
    Ff[0] = -rho*dc*r*VKite*cc*wep[0]/8.0;
    Ff[1] = -rho*dc*r*VKite*cc*wep[1]/8.0;
    Ff[2] = -rho*dc*r*VKite*cc*wep[2]/8.0;

    # LIFT :
    # ---------------------------------------------------------------
    
    FL = SXMatrix(3,1,0)  
    FL[0] =  rho*CL*eLe[0]*VKite/2.0;
    FL[1] =  rho*CL*eLe[1]*VKite/2.0;
    FL[2] =  rho*CL*eLe[2]*VKite/2.0;

    # DRAG :
    # ---------------------------------------------------------------
    
    FD = SXMatrix(3,1,0)  
    FD[0] = -rho*VKite*CD*we[0]/2.0;
    FD[1] = -rho*VKite*CD*we[1]/2.0; 
    FD[2] = -rho*VKite*CD*we[2]/2.0; 

    
    # FORCES (AERO)
    # ---------------------------------------------------------------
    
    F = SXMatrix(3,1,0)  
    F[0] = FL[0] + FD[0] + Ff[0];
    F[1] = FL[1] + FD[1] + Ff[1];
    F[2] = FL[2] + FD[2] + Ff[2];
    
    # TORQUES (AERO)
    # ---------------------------------------------------------------
    
    T = SXMatrix(3,1,0)  
    T[0] =  0.5*rho*VKite2*SPAN*CR;
    T[1] =  0.5*rho*VKite2*CHORD*CP;
    T[2] =  0.5*rho*VKite2*SPAN*CY;
    
    
    # ATTITUDE DYNAMICS
    # -----------------------------------------------------------
    
    
    W = vertcat([horzcat([0,-w3,w2]),
                 horzcat([w3,0,-w1]),
                 horzcat([-w2,w1,0])])
    
    RotPole = 1/2;
    RP = RotPole*mul(R,(inv(mul(R.T,R)) - np.eye(3)));
    dR = mul(R,W) + RP;
    
    #    dq0 = (-q1*w1 - q2*w2 - q3*w3)/2;
    #    dq1 = ( q0*w1 - q3*w2 + q2*w3)/2;
    #    dq2 = ( q3*w1 + q0*w2 - q1*w3)/2;
    #    dq3 = (-q2*w1 + q1*w2 + q0*w3)/2;


    
    ######################################################################
    #                                                                    # 
    #  AUTO-GENERATED EQUATIONS (S. Gros, HIGHWIND, OPTEC, KU Leuven)    # 
    #                                                                    # 
    ###################################################################### 
    
    # Equations read: 
    # IMA = inv(MA) 
    # ddX = IMA*(Bx - CA*lambda) 
    # lambdaNum = CA^T*IMA*Bx - Blambda 
    # lambdaDen = CA^T*IMA*CA 
    # lambda = lambdaNum/lambdaDen 
    
    
#    xA = rA*cos(delta); 
#    dxA = -ddelta*rA*sin(delta); 
#    ddxA = (-dddelta*rA*sin(delta) - ddelta*ddelta*rA*cos(delta)); 
#    yA = rA*sin(delta); 
#    dyA = ddelta*rA*cos(delta); 
#    ddyA = -rA*sin(delta)*ddelta*ddelta + dddelta*rA*cos(delta); 
    xA = -rA*sin(delta); 
    dxA = -ddelta*rA*cos(delta); 
    ddxA = -(dddelta*rA*cos(delta) - ddelta*ddelta*rA*sin(delta)); 
    yA = rA*cos(delta); 
    dyA = -ddelta*rA*sin(delta); 
    ddyA = - rA*cos(delta)*ddelta*ddelta - dddelta*rA*sin(delta); 
    
    # BUILD CONSTRAINTS 
    Const = - r*r/2 + x*x/2 - x*xA + xA*xA/2 + y*y/2 - y*yA + yA*yA/2 + z*z/2; 
    dConst = dx*x - dr*r - dxA*x - dx*xA + dxA*xA + dy*y - dyA*y - dy*yA + dyA*yA + dz*z; 
    
    # BUILD DYNAMICS 
    lambdaNum = ddxA*xA - 2*dy*dyA - ddr*r - ddxA*x - 2*dx*dxA - ddyA*y + ddyA*yA - dr*dr + dx*dx + dxA*dxA + dy*dy + dyA*dyA + dz*dz + (F[0]*(x - xA))/m + (F[1]*(y - yA))/m + (z*(F[2] - g*mgrav))/m; 
    
    lambdaDen = x*x/m + xA*xA/m + y*y/m + yA*yA/m + z*z/m - (2*x*xA)/m - (2*y*yA)/m; 
    
    #lambda_ = lambdaNum/lambdaDen; 
    Pole = 1/2;
    lambda_ = (2*Pole*dConst + Pole*Pole*Const + lambdaNum)/lambdaDen; 
    
    ddX = SXMatrix(6,1,0)
    ddX[0] = (F[0] - lambda_*(x - xA))/m; 
    ddX[1] = (F[1] - lambda_*(y - yA))/m; 
    ddX[2] = -(g*mgrav - F[2] + lambda_*z)/m; 
    ddX[3] = (I31*(T[2] + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3) - (I3*(T[0] - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3); 
    ddX[4] = (T[1] + w1*(I31*w1 + I3*w3) - w3*(I1*w1 + I31*w3))/I2; 
    ddX[5] = (I31*(T[0] - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3) - (I1*(T[2] + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3); 
    
    
    ############### END OF AUTO-GENERATED CODE ########################### 
    	     
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
    res.append( ddX[0] )
    res.append( ddX[1] )
    res.append( ddX[2] )
    res.append( dR[0,0] )
    res.append( dR[0,1] )
    res.append( dR[0,2] )
    res.append( dR[1,0] )
    res.append( dR[1,1] )
    res.append( dR[1,2] )
    res.append( dR[2,0] )
    res.append( dR[2,1] )
    res.append( dR[2,2] )
    res.append( ddX[3] )
    res.append( ddX[4] )
    res.append( ddX[5] )
    res.append( dr )
    res.append( ddr )
    res.append( ddelta )
    res.append( dddelta )
    res.append( dua )
    res.append( due )
    
    # Constraints
#    Const  = - r**2 + x**2 + y**2 + z**2
#    dConst = dx*x + dy*y + dz*z
    
    if flag == "":
        return res
    elif flag == "constraints":
        return Const, dConst, ConstR1, ConstR2, ConstR3, ConstR4, ConstR5, ConstR6, ConstRpos
    elif flag == "lambda":
        return lambda_
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

