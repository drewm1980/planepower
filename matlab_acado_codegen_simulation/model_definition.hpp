
// OTHER VARIABLES :
// ------------------------
	
	IntermediateState     mc;      //  mass of the cable
	IntermediateState     m ;      //  effective inertial mass
	IntermediateState  mgrav;      //  gravific mass

// ORIENTATION AND FORCES :
// ------------------------
	
	IntermediateState wind               ;      //  the wind at altitude 	
	
	IntermediateState Cf              ;      //  cable drag
	IntermediateState CD              ;      //  the aerodynamic drag coefficient
	IntermediateState CL              ;      //  the aerodynamic lift coefficient
	IntermediateState CR              ;      //  the aerodynamic roll coefficient
	IntermediateState CP              ;      //  the aerodynamic pitch coefficient
	IntermediateState CY              ;      //  the aerodynamic yaw coefficient
	
	IntermediateState F           [ 3];      //  aero forces + gravity
	IntermediateState FL          [ 3];      //  the lift force
	IntermediateState FD          [ 3];      //  the drag force
// 	IntermediateState Ff          [ 3];      //  the frictional force
// 	IntermediateState Fcable          ;      //  force in the cable
	
// 	IntermediateState er          [ 3];      // X normed to 1
	IntermediateState eTe         [ 3];      //unrotated transversal vector (psi = 0)
	IntermediateState eLe         [ 3];      //unrotated lift vector (psi = 0)
	IntermediateState we          [ 3];      //  effective wind vector
	IntermediateState wE          [ 3];      //  effective wind vector
	IntermediateState wp              ;		
// 	IntermediateState wep         [ 3];		// effective wind projected in the plane orthogonal to X
	
	IntermediateState VKite           ;     //Kite (relative) speed
	IntermediateState VKite2          ;     //Squared (relative) kite speed
		
	IntermediateState Vp;
	IntermediateState VT			[3];
	IntermediateState alpha;
	IntermediateState beta;
	IntermediateState betaTail;
	IntermediateState alphaTail;
	IntermediateState T			    [3];
	IntermediateState TB		    [3];
	
	
	// TERMS ON RIGHT-HAND-SIDE
	// OF THE DIFFERENTIAL
	// EQUATIONS              :
	// ------------------------
	
	IntermediateState de11;
	IntermediateState de12;
	IntermediateState de13;
	IntermediateState de21;
	IntermediateState de22;
	IntermediateState de23;
	IntermediateState de31;
	IntermediateState de32;
	IntermediateState de33;
	
	
//                        MODEL EQUATIONS :
// ===============================================================

	// CROSS AREA OF THE CABLE :
	// ---------------------------------------------------------------
	
// 	AQ      =  PI*dc*dc/4.0                                       ;
	
	// THE EFECTIVE MASS' :
	// ---------------------------------------------------------------
	
	mc      =  0*rhoc*AQ*r  ;   // mass of the cable
	m       =  mk + mc/3.0;   // effective inertial mass
	mgrav   =  mk + mc/2.0;   // effective inertial mass
	
	// -----------------------------   // ----------------------------
	//   dm      =  (rhoc*AQ/ 3.0)*dr;   // time derivative of the mass
	
	
	// WIND SHEAR MODEL :
	// ---------------------------------------------------------------
	
	wind       =  0.;
	
	
	// EFFECTIVE WIND IN THE KITE`S SYSTEM :
	// ---------------------------------------------------------------
	
	we[0]   = -wind + dx - ddelta*y;
	we[1]   =		  dy + ddelta*rA + ddelta*x;
	we[2]   =		  dz;
	
	VKite2 = (we[0]*we[0] + we[1]*we[1] + we[2]*we[2]); 
	VKite = sqrt(VKite2); 
	
	// CALCULATION OF THE FORCES :
	// ---------------------------------------------------------------
	
// 	// er
//     er[0] = x/r;
// 	er[1] = y/r;
// 	er[2] = z/r;
// 	
// 	//Velocity accross X (cable drag)
// 	wp = er[0]*we[0] + er[1]*we[1] + er[2]*we[2];
// 	wep[0] = we[0] - wp*er[0];
// 	wep[1] = we[1] - wp*er[1];
// 	wep[2] = we[2] - wp*er[2];
	
	//Aero coeff.
	
	// LIFT DIRECTION VECTOR
	//Relative wind speed in Airfoil's referential 'E'
	// The following Mapping sends vector (we) from arm tip frame to aircraft frame (wE).
	// rE = transpose(e) * we
	wE[0] = e11*we[0]  + e21*we[1]  + e31*we[2];
	wE[1] = e12*we[0]  + e22*we[1]  + e32*we[2];
	wE[2] = e13*we[0]  + e23*we[1]  + e33*we[2];

	//Airfoil's transversal axis in fixed referential 'e'
	eTe[0] = e12;
	eTe[1] = e22;
	eTe[2] = e32;

	// Lift axis ** Normed to we !! **
	eLe[0] = - eTe[1]*we[2] + eTe[2]*we[1];
	eLe[1] = - eTe[2]*we[0] + eTe[0]*we[2];
	eLe[2] = - eTe[0]*we[1] + eTe[1]*we[0];

	// AERODYNAMIC COEEFICIENTS
	// ----------------------------------
	//VT = cross([w1;w2;w3],[-LT;0;0]) + wE;

	VT[0] =          wE[0];
	VT[1] = -LT*w3 + wE[1];
	VT[2] =  LT*w2 + wE[2];

	alpha = -wE[2]/wE[0];

	//Note: beta & alphaTail are compensated for the tail motion induced by omega
// 	beta = VT[1]/sqrt(VT[0]*VT[0] + VT[2]*VT[2]);
	betaTail = VT[1]/VT[0];
//     betaTail = wE(2)/sqrt(wE(1)*wE(1) + wE(3)*wE(3));
    beta = wE[1]/wE[0];
	alphaTail = -VT[2]/VT[0];

// 	CL = CLA*alpha + CLe*up     + CLr*ur + CL0;
// 	CD = CDA*alpha + CDA2*alpha*alpha + CDB2*beta*beta + CDe*up + CDr*ur + CD0;
// 	CR = -RD*w1 + CRB*betaTail + CRAB*alphaTail*betaTail + CRr*ur;
// 	CP = CPA*alphaTail + CPe*up + CPr*ur + CP0;
// 	CY = CYB*betaTail + CYAB*alphaTail*betaTail;
	CL = CLA*alpha + CLe*up + CL0;
	CD = CDA*alpha + CDA2*alpha*alpha + CDB2*beta*beta + CD0;
	CR = -RD*w1 + CRB*betaTail + CRr*ur + CRAB*alphaTail*betaTail;
	CP = -PD*w2 + CPA*alphaTail + CPe*up + CP0;
	CY = -YD*w3 + CYB*betaTail + CYAB*alphaTail*betaTail;
	
	
	
// 	Cf = rho*dc*r*VKite/8.0;

	// THE FRICTION OF THE CABLE :
	// ---------------------------------------------------------------

// 	Ff[0] = -rho*dc*r*VKite*cc*wep[0]/8.0;
// 	Ff[1] = -rho*dc*r*VKite*cc*wep[1]/8.0;
// 	Ff[2] = -rho*dc*r*VKite*cc*wep[2]/8.0;

	// LIFT :
	// ---------------------------------------------------------------
    CL = 0.2*CL;
	FL[0] =  rho*CL*eLe[0]*VKite/2.0;
	FL[1] =  rho*CL*eLe[1]*VKite/2.0;
	FL[2] =  rho*CL*eLe[2]*VKite/2.0;

	// DRAG :
	// ---------------------------------------------------------------
    CD = 0.5*CD;
	FD[0] = -rho*VKite*CD*we[0]/2.0;
	FD[1] = -rho*VKite*CD*we[1]/2.0; 
	FD[2] = -rho*VKite*CD*we[2]/2.0; 


	// FORCES (AERO)
	// ---------------------------------------------------------------

	F[0] = FL[0] + FD[0];
	F[1] = FL[1] + FD[1];
	F[2] = FL[2] + FD[2];


	// ATTITUDE DYNAMICS
	// -----------------------------------------------------------

// 	de11 = e21*w3 - e31*w2 + ddelta*e12; 
// 	de21 = e31*w1 - e11*w3 + ddelta*e22; 
// 	de31 = e11*w2 - e21*w1 + ddelta*e32; 
// 	de12 = e22*w3 - e32*w2 - ddelta*e11; 
// 	de22 = e32*w1 - e12*w3 - ddelta*e21; 
// 	de32 = e12*w2 - e22*w1 - ddelta*e31; 
// 	de13 = e23*w3 - e33*w2; 
// 	de23 = e33*w1 - e13*w3; 
// 	de33 = e13*w2 - e23*w1;
    de11 = e12*w3 - e13*w2 + ddelta*e21;
    de12 = e13*w1 - e11*w3 + ddelta*e22;
    de13 = e11*w2 - e12*w1 + ddelta*e23;
    de21 = e22*w3 - e23*w2 - ddelta*e11;
    de22 = e23*w1 - e21*w3 - ddelta*e12; 
    de23 = e21*w2 - e22*w1 - ddelta*e13;
    de31 = e32*w3 - e33*w2;
    de32 = e33*w1 - e31*w3;
    de33 = e31*w2 - e32*w1; 



	//////////////////////////////////////////////////////////////////////// 
	//                                                                    // 
	//  AUTO-GENERATED EQUATIONS (S. Gros, HIGHWIND, OPTEC, KU Leuven)    // 
	//                                                                    // 
	//////////////////////////////////////////////////////////////////////// 

	// Inverse of the Generalized inertia matrix 
	IntermediateState IMA(3,3); 
	IMA(0,0) = 1/m; 
	IMA(0,1) = 0; 
	IMA(0,2) = 0; 
	IMA(1,0) = 0; 
	IMA(1,1) = 1/m; 
	IMA(1,2) = 0; 
	IMA(2,0) = 0; 
	IMA(2,1) = 0; 
	IMA(2,2) = 1/m; 

	// G1 (right hand-side up) 
	IntermediateState G1(3,1); 
// 	G1(0,0) = F[0] + ddelta*dy*m + dddelta*m*y; 
// 	G1(1,0) = F[1] - (dddelta*m*(2*rA + 2*x))/2 - ddelta*dx*m; 
// 	G1(2,0) = F[2] - g*mgrav; 
	G1(0,0) = F[0] + (m*(2*ddelta*dy + 2*ddelta*ddelta*rA + 2*ddelta*ddelta*x))/2 + ddelta*dy*m + dddelta*m*y; 
	G1(1,0) = F[1] - (m*(2*ddelta*dx - 2*ddelta*ddelta*y))/2 - (dddelta*m*(2*rA + 2*x))/2 - ddelta*dx*m; 
	G1(2,0) = F[2] - g*mgrav; 

	// G2 (right hand-side down) 
	IntermediateState G2; 
	G2 = - dx*dx - dy*dy - dz*dz; 

	// NabG 
	IntermediateState NabG(3,1); 
	NabG(0,0) = x; 
	NabG(1,0) = y; 
	NabG(2,0) = z; 

	// NabGT 
	IntermediateState NabGT(1,3); 
	NabGT(0,0) = x; 
	NabGT(0,1) = y; 
	NabGT(0,2) = z; 

	// LambdaFac * lambda = lambdaright 
	IntermediateState LambdaFac; 
	LambdaFac = NabGT*IMA*NabG; 

	// lambdaright 
	IntermediateState lambdaright; 
	lambdaright = NabGT*IMA*G1 - G2; 

	// lambda 
	IntermediateState lambda; 
	lambda = lambdaright/LambdaFac; 

	// ddq (accelerations) 
	IntermediateState ddq(3,1); 
	ddq = IMA*(G1-NabG*lambda); 

	// Consistency Conditions 
	IntermediateState Const, dConst; 
	Const = - r*r/2 + x*x/2 + y*y/2 + z*z/2; 
	dConst = dx*x + dy*y + dz*z; 
	
	// TORQUES (BRIDLE)
	// ---------------------------------------------------------------
	// This is an approximation !!!
	TB[0] =  zT*(e12*lambda*x + e22*lambda*y + e32*lambda*z);
	TB[1] = -zT*(e11*lambda*x + e21*lambda*y + e31*lambda*z);
	TB[2] =  0;
	
	// TORQUES (AERO)
	// ---------------------------------------------------------------

	T[0] =  0.5*rho*VKite2*SPAN*CR   + TB[0];
	T[1] =  0.5*rho*VKite2*CHORD*CP  + TB[1];
	T[2] =  0.5*rho*VKite2*SPAN*CY   + TB[2];


	IntermediateState dw1, dw2, dw3;
// 	dw1 = (I31*(T[2] + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3) - (I3*(T[0] - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3); 
// 	dw2 = (T[1] + w1*(I31*w1 + I3*w3) - w3*(I1*w1 + I31*w3))/I2; 
// 	dw3 = (I31*(T[0] - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3) - (I1*(T[2] + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3); 
	dw1 = (I31*T[2])/(I31*I31 - I1*I3) - (I3*T[0])/(I31*I31 - I1*I3); 
	dw2 = T[1]/I2; 
	dw3 = (I31*T[0])/(I31*I31 - I1*I3) - (I1*T[2])/(I31*I31 - I1*I3); 
	
	

	
// THE "RIGHT-HAND-SIDE" OF THE ODE:
// ---------------------------------------------------------------
	DifferentialEquation f( 0.0, Tc );


	f  << dot(x)     ==  dx                             ;
	f  << dot(y)	 ==  dy                             ;
	f  << dot(z)	 ==  dz                             ;
	f  << dot(dx)    ==  1*ddq(0,0)                            ;
	f  << dot(dy)	 ==  1*ddq(1,0)                            ;
	f  << dot(dz)	 ==  1*ddq(2,0)                            ;

	f  << dot(e11)	 ==  de11;
	f  << dot(e12)	 ==  de12;
	f  << dot(e13)	 ==  de13;
	f  << dot(e21)	 ==  de21;
	f  << dot(e22)	 ==  de22;
	f  << dot(e23)	 ==  de23;
	f  << dot(e31)	 ==  de31;
	f  << dot(e32)	 ==  de32;
	f  << dot(e33)	 ==  de33;

	f  << dot(w1)	 ==  dw1                            ;
	f  << dot(w2)	 ==  dw2                            ;
	f  << dot(w3)	 ==  dw3                            ;

	f  << dot(delta)  ==  ddelta                         ;
	f  << dot(ddelta) ==  dddelta                        ;
	f  << dot(ur)     ==  dur;
	f  << dot(up)     ==  dup;

	// ===============================================================
	//                        END OF MODEL EQUATIONS DREW
	// ===============================================================
