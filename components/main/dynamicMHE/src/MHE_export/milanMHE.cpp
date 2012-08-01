/*
 *    This file is part of ACADO Toolkit.
 *
 *    ACADO Toolkit -- A Toolkit for Automatic Control and Dynamic Optimization.
 *    Copyright (C) 2008-2009 by Boris Houska and Hans Joachim Ferreau, K.U.Leuven.
 *    Developed within the Optimization in Engineering Center (OPTEC) under
 *    supervision of Moritz Diehl. All rights reserved.
 *
 *    ACADO Toolkit is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    ACADO Toolkit is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with ACADO Toolkit; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/**
 *    SINGLE POWER KITE START-UP WITH PROPELLER
 *    CARTESIAN COORDINATES (ODE FORMULATION)
 *    MARCH 2012 SEBASTIEN GROS & MARIO ZANON, HIGHWIND, OPTEC
 *    SEBASTIEN GROS & MARIO ZANON
 */

#include <acado_toolkit.hpp>
#include <mhe_export.hpp>

int main(int argc, char * const argv[])
{

	//======================================================================
	// PARSE THE INPUT ARGUMENTS:
	// ----------------------------------
	double IC[4];

	/* arguments are passed to the main function by string
	 *  there are 'int argc' arguments
	 *		the first one is the name of the program
	 *		the next ones are arguments passed in the call like
	 *			program number1 number2
	 *  in this stupid simple parser, we directly read doubles from the arguments
	 */

	int i = 1;
	while (i < argc)
	{
		// load the i-th string argument passed to the main function
		char* input = argv[i];
		// parse the string to a double value
		IC[i - 1] = atof(input);
		i++;
	}

	cout << "Initial Conditions" << endl;
	cout << "------------" << endl;
	for (i = 0; i < argc - 1; ++i)
	{
		cout << i + 1 << ":\t" << IC[i] << endl;
	}
	//======================================================================	

	double Ncvp = IC[1];
	double Tc = IC[2];
	int MaxSteps = IC[3];

	USING_NAMESPACE_ACADO

// DIFFERENTIAL STATES :
// -------------------------

	DifferentialState x; // Position
	DifferentialState y; //
	DifferentialState z; //
// -------------------------      //  -------------------------------------------

	DifferentialState dx; //  Speed
	DifferentialState dy; //
	DifferentialState dz; //

	DifferentialState e11;
	DifferentialState e12;
	DifferentialState e13;
	DifferentialState e21;
	DifferentialState e22;
	DifferentialState e23;
	DifferentialState e31;
	DifferentialState e32;
	DifferentialState e33;

	DifferentialState w1;
	DifferentialState w2;
	DifferentialState w3;
// -------------------------      //  -------------------------------------------
// 	DifferentialState      r;      //  Kite distance
// 	DifferentialState     dr;      //  Kite distance / dt

//-------------------------      //  -------------------------------------------

	DifferentialState delta; //  Carousel
	DifferentialState ddelta; //
	DifferentialState ur;
	DifferentialState up; //  Ailerons

	// Collect all the states in a vector
// 	const int n_XD = 24; // Number of states
//
// 	IntermediateState XD[n_XD];    // Vector collecting all states
// 	XD[0]  = x;
// 	XD[1]  = y;
// 	XD[2]  = z;
// 	XD[3]  = dx;
// 	XD[4]  = dy;
// 	XD[5]  = dz;
// 	XD[6]  = e11;
// 	XD[7]  = e12;
// 	XD[8]  = e13;
// 	XD[9]  = e21;
// 	XD[10] = e22;
// 	XD[11] = e23;
// 	XD[12] = e31;
// 	XD[13] = e32;
// 	XD[14] = e33;
// 	XD[15] = w1;
// 	XD[16] = w2;
// 	XD[17] = w3;
// 	XD[18] = r;
// 	XD[19] = dr;
// 	XD[20] = delta;
// 	XD[21] = ddelta;
// 	XD[22] = ur;
// 	XD[23] = up;

// CONTROL :
// -------------------------
	Control dddelta; //  Carousel acceleration
// 	Control             ddr;
	Control dur;
	Control dup; //  Ailerons

	// Collect all the controls in a vector
	const int n_U = 3; // Number of controls

	IntermediateState U[n_U]; // Vector collecting all states
	U[0] = dddelta;
// 	U[1] = ddr;
	U[1] = dur;
	U[2] = dup;

// PERTURBATIONS :
// -------------------------
// 	Control             FF1;      //  Ficticious forces and torques
// 	Control             FF2;
// 	Control             FF3;
// 	Control             FT1;
// 	Control             FT2;
// 	Control             FT3;
// 	const int n_W = 6; // Number of controls
//
// 	IntermediateState W[n_W];    // Vector collecting all states
// 	W[0] = FF1;
// 	W[1] = FF2;
// 	W[2] = FF3;
// 	W[3] = FT1;
// 	W[4] = FT2;
// 	W[5] = FT3;

// PARAMETERS
// -----------------------
// 	Parameter SlackCLmax;
// 	Parameter SlackLambda;
// 	Control dW; // Unknown wind modeled as a control

	Parameter r;
	Parameter mk;
	Parameter g;
	Parameter rho;
	Parameter rhoc;
	Parameter dc;
	Parameter AQ;
	Parameter rA;
	Parameter zT;
	Parameter I1;
	Parameter I31;
	Parameter I2;
	Parameter I3;

	Parameter XIMU1;
	Parameter XIMU2;
	Parameter XIMU3;
	Parameter LT;
	Parameter RD;
	Parameter PD;
	Parameter YD;
	Parameter CLA;
	Parameter CLe;
	Parameter CL0;

	Parameter CDA;
	Parameter CDA2;//
	Parameter CDB2;
	Parameter CD0;

	Parameter CRB;
	Parameter CRAB;
	Parameter CRr;

	Parameter CPA;
	Parameter CPe;

	Parameter CP0;

	Parameter CYB;
	Parameter CYAB;
	Parameter SPAN;
	Parameter CHORD;

	Parameter CL_scaling;
	Parameter CD_scaling;


	// DEFINITION OF PI :
	// ------------------------

	double PI = 3.1415926535897932;

//	// CONSTANTS :
//	// ------------------------
//	//  PARAMETERS OF THE KITE :
//	//  -----------------------------


//	double r = 1.2;
//
//
//	double mk =  0.626;      //  mass of the kite               //  [ kg    ]
//
//
//	//  A =  0.2;      //  effective area                 //  [ m^2   ]
//
//	//   PHYSICAL CONSTANTS :
//	//  -----------------------------
//	double   g =    9.81;      //  gravitational constant         //  [ m /s^2]
//	double rho =    1.23;      //  density of the air             //  [ kg/m^3]
//
//
//
//
//	//  PARAMETERS OF THE CABLE :
//	//  -----------------------------
//		double rhoc = 1450.00;      //  density of the cable           //  [ kg/m^3]
//// 		double cc =   1.00;      //  frictional constant            //  [       ]
//		double dc = 1e-3;      //  diameter                       //  [ m     ]
//
//
//		double AQ      =  PI*dc*dc/4.0;
//
//
//
//	//CAROUSEL ARM LENGTH
//		double rA = 1.085; //(dixit Kurt)
//
//
//
//		double zT = -0.02;
//
//
//// 		double XT = [0;0;-0.01];
//
//// 		double ZT = 0;
//	//             YT = 0.005;
//
//	//INERTIA MATRIX (Kurt's direct measurements)
//		double I1 = 0.0163;
//		double I31 = 0.0006;
//		double I2 = 0.0078;
//		double I3 = 0.0229;
//
//
//
//	//IMU POSITION & ANGLE
//		double XIMU1 = 0.0246;
//		double XIMU2 = -0.0116;
//		double XIMU3 = -0.0315;
//// 		double XIMU = [0.0246;-0.0116;-0.0315];
//// 		double alphaIMU = 0*pi/180;//4
//// 		double betaIMU = 0*pi/180;
//// 		double deltaIMU = 0*pi/180;
//
//
//
//// 		double alpha0 = -0*PI/180;
//
//	//TAIL LENGTH
//	double LT = 0.4;
//
//	//ROLL DAMPING
//	double RD = 1e-2;
//	double PD = 0 * 5e-3;
//	double YD = 0 * 5e-3;
//	//WIND-TUNNEL PARAMETERS
//
//
//
//	//Lift (report p. 67)
//	double CLA = 5.064;
//
//
//	double CLe = -1.924; //e-5;//0.318;//
//
//
//	double CL0 = 0.239;
//
//
//	//Drag (report p. 70)
//	double CDA = -0.195;
//	double CDA2 = 4.268;
//	double CDB2 = 5; //0;//
//// 		double CDe = 0.044;
//// 		double CDr = 0.111;
//	double CD0 = 0.026;
//
//
//
//	//Roll (report p. 72)
//	double CRB = -0.062;
//	double CRAB = -0.271;
//	double CRr = -5.637e-1; //e-6;//-0.244;//
//
//
//
//
//	//Pitch (report p. 74)
//	double CPA = 0.293;
//	double CPe = -4.9766e-1; //e-6;//-0.821;//
//
//
//	double CP0 = 0.03;
//
//
//
//	//Yaw (report p. 76)
//	double CYB = 0.05;
//	double CYAB = 0.229;
//
//	double SPAN = 0.96;
//	double CHORD = 0.1;
//
//
//
//	double CL_scaling = 0.2;
//	double CD_scaling = 0.5;

// OTHER VARIABLES :
// ------------------------

	IntermediateState mc; //  mass of the cable
	IntermediateState m; //  effective inertial mass
	IntermediateState mgrav; //  gravific mass
	//  IntermediateState     dmc;      //  first  derivative of m     with respect to t

// ORIENTATION AND FORCES :
// ------------------------

	IntermediateState wind; //  the wind at altitude

	IntermediateState Cf; //  cable drag
	IntermediateState CD; //  the aerodynamic drag coefficient
	IntermediateState CL; //  the aerodynamic lift coefficient
	IntermediateState CR; //  the aerodynamic roll coefficient
	IntermediateState CP; //  the aerodynamic pitch coefficient
	IntermediateState CY; //  the aerodynamic yaw coefficient

	IntermediateState F[3]; //  aero forces + gravity
	IntermediateState FL[3]; //  the lift force
	IntermediateState FD[3]; //  the drag force
// 	IntermediateState Ff          [ 3];      //  the frictional force
// 	IntermediateState Fcable          ;      //  force in the cable

// 	IntermediateState er          [ 3];      // X normed to 1
	IntermediateState eTe[3]; //unrotated transversal vector (psi = 0)
	IntermediateState eLe[3]; //unrotated lift vector (psi = 0)
	IntermediateState we[3]; //  effective wind vector
	IntermediateState wE[3]; //  effective wind vector
	IntermediateState wp;
// 	IntermediateState wep         [ 3];		// effective wind projected in the plane orthogonal to X

	IntermediateState VKite; //Kite (relative) speed
	IntermediateState VKite2; //Squared (relative) kite speed

	IntermediateState Vp;
	IntermediateState VT[3];
	IntermediateState alpha;
	IntermediateState beta;
	IntermediateState betaTail;
	IntermediateState alphaTail;
	IntermediateState T[3];
	IntermediateState TB[3];

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

	mc = 0 * rhoc * AQ * r; // mass of the cable
	m = mk + mc / 3.0; // effective inertial mass
	mgrav = mk + mc / 2.0; // effective inertial mass

	// -----------------------------   // ----------------------------
	//   dm      =  (rhoc*AQ/ 3.0)*dr;   // time derivative of the mass

	// WIND SHEAR MODEL :
	// ---------------------------------------------------------------

	wind = 0.;

	// EFFECTIVE WIND IN THE KITE`S SYSTEM :
	// ---------------------------------------------------------------

	we[0] = -wind + dx - ddelta * y;
	we[1] = dy + ddelta * rA + ddelta * x;
	we[2] = dz;

	VKite2 = (we[0] * we[0] + we[1] * we[1] + we[2] * we[2]);
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
	// -------------------------

	//Relative wind speed in Airfoil's referential 'E'
// 	wE[0] = e11*we[0]  + e12*we[1]  + e13*we[2];
// 	wE[1] = e21*we[0]  + e22*we[1]  + e23*we[2];
// 	wE[2] = e31*we[0]  + e32*we[1]  + e33*we[2];
	wE[0] = e11 * we[0] + e21 * we[1] + e31 * we[2];
	wE[1] = e12 * we[0] + e22 * we[1] + e32 * we[2];
	wE[2] = e13 * we[0] + e23 * we[1] + e33 * we[2];

	//Airfoil's transversal axis in fixed referential 'e'
// 	eTe[0] = e21;
// 	eTe[1] = e22;
// 	eTe[2] = e23;
	eTe[0] = e12;
	eTe[1] = e22;
	eTe[2] = e32;

	// Lift axis ** Normed to we !! **
	eLe[0] = -eTe[1] * we[2] + eTe[2] * we[1];
	eLe[1] = -eTe[2] * we[0] + eTe[0] * we[2];
	eLe[2] = -eTe[0] * we[1] + eTe[1] * we[0];

	// AERODYNAMIC COEEFICIENTS
	// ----------------------------------
	//VT = cross([w1;w2;w3],[-LT;0;0]) + wE;

	VT[0] = wE[0];
	VT[1] = -LT * w3 + wE[1];
	VT[2] = LT * w2 + wE[2];

	alpha = -wE[2] / wE[0];

	//Note: beta & alphaTail are compensated for the tail motion induced by omega
// 	beta = VT[1]/sqrt(VT[0]*VT[0] + VT[2]*VT[2]);
	betaTail = VT[1] / VT[0];
//     betaTail = wE(2)/sqrt(wE(1)*wE(1) + wE(3)*wE(3));
	beta = wE[1] / wE[0];
	alphaTail = -VT[2] / VT[0];

// 	CL = CLA*alpha + CLe*up     + CLr*ur + CL0;
// 	CD = CDA*alpha + CDA2*alpha*alpha + CDB2*beta*beta + CDe*up + CDr*ur + CD0;
// 	CR = -RD*w1 + CRB*betaTail + CRAB*alphaTail*betaTail + CRr*ur;
// 	CP = CPA*alphaTail + CPe*up + CPr*ur + CP0;
// 	CY = CYB*betaTail + CYAB*alphaTail*betaTail;
	CL = CLA * alpha + CLe * up + CL0;
	CD = CDA * alpha + CDA2 * alpha * alpha + CDB2 * beta * beta + CD0;
	CR = -RD * w1 + CRB * betaTail + CRr * ur + CRAB * alphaTail * betaTail;
	CP = -PD * w2 + CPA * alphaTail + CPe * up + CP0;
	CY = -YD * w3 + CYB * betaTail + CYAB * alphaTail * betaTail;

// 	Cf = rho*dc*r*VKite/8.0;

	// THE FRICTION OF THE CABLE :
	// ---------------------------------------------------------------

// 	Ff[0] = -rho*dc*r*VKite*cc*wep[0]/8.0;
// 	Ff[1] = -rho*dc*r*VKite*cc*wep[1]/8.0;
// 	Ff[2] = -rho*dc*r*VKite*cc*wep[2]/8.0;

	// LIFT :
	// ---------------------------------------------------------------

	CL = CL * CL_scaling;
	CD = CD * CD_scaling;
	FL[0] = rho * CL * eLe[0] * VKite / 2.0;
	FL[1] = rho * CL * eLe[1] * VKite / 2.0;
	FL[2] = rho * CL * eLe[2] * VKite / 2.0;

	// DRAG :
	// ---------------------------------------------------------------

	FD[0] = -rho * VKite * CD * we[0] / 2.0;
	FD[1] = -rho * VKite * CD * we[1] / 2.0;
	FD[2] = -rho * VKite * CD * we[2] / 2.0;

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
	de11 = e12 * w3 - e13 * w2 + ddelta * e21;
	de12 = e13 * w1 - e11 * w3 + ddelta * e22;
	de13 = e11 * w2 - e12 * w1 + ddelta * e23;
	de21 = e22 * w3 - e23 * w2 - ddelta * e11;
	de22 = e23 * w1 - e21 * w3 - ddelta * e12;
	de23 = e21 * w2 - e22 * w1 - ddelta * e13;
	de31 = e32 * w3 - e33 * w2;
	de32 = e33 * w1 - e31 * w3;
	de33 = e31 * w2 - e32 * w1;

	////////////////////////////////////////////////////////////////////////
	//                                                                    //
	//  AUTO-GENERATED EQUATIONS (S. Gros, HIGHWIND, OPTEC, KU Leuven)    //
	//                                                                    //
	////////////////////////////////////////////////////////////////////////

	// Inverse of the Generalized inertia matrix
	IntermediateState IMA(3, 3);
	IMA(0, 0) = 1 / m;
	IMA(0, 1) = 0;
	IMA(0, 2) = 0;
	IMA(1, 0) = 0;
	IMA(1, 1) = 1 / m;
	IMA(1, 2) = 0;
	IMA(2, 0) = 0;
	IMA(2, 1) = 0;
	IMA(2, 2) = 1 / m;

	// G1 (right hand-side up)
	IntermediateState G1(3, 1);
// 	G1(0,0) = F[0] + ddelta*dy*m + dddelta*m*y;
// 	G1(1,0) = F[1] - (dddelta*m*(2*rA + 2*x))/2 - ddelta*dx*m;
// 	G1(2,0) = F[2] - g*mgrav;
	G1(0, 0) = F[0]
			+ (m
					* (2 * ddelta * dy + 2 * ddelta * ddelta * rA
							+ 2 * ddelta * ddelta * x)) / 2 + ddelta * dy * m
			+ dddelta * m * y;
	G1(1, 0) = F[1] - (m * (2 * ddelta * dx - 2 * ddelta * ddelta * y)) / 2
			- (dddelta * m * (2 * rA + 2 * x)) / 2 - ddelta * dx * m;
	G1(2, 0) = F[2] - g * mgrav;

	// G2 (right hand-side down)
	IntermediateState G2;
	G2 = -dx * dx - dy * dy - dz * dz;

	// NabG
	IntermediateState NabG(3, 1);
	NabG(0, 0) = x;
	NabG(1, 0) = y;
	NabG(2, 0) = z;

	// NabGT
	IntermediateState NabGT(1, 3);
	NabGT(0, 0) = x;
	NabGT(0, 1) = y;
	NabGT(0, 2) = z;

	// LambdaFac * lambda = lambdaright
	IntermediateState LambdaFac;
	LambdaFac = NabGT * IMA * NabG;

	// lambdaright
	IntermediateState lambdaright;
	lambdaright = NabGT * IMA * G1 - G2;

	// lambda
	IntermediateState lambda;
	lambda = lambdaright / LambdaFac;

	// ddq (accelerations)
	IntermediateState ddq(3, 1);
	ddq = IMA * (G1 - NabG * lambda);

	// Consistency Conditions
	IntermediateState Const, dConst;
	Const = -r * r / 2 + x * x / 2 + y * y / 2 + z * z / 2;
	dConst = dx * x + dy * y + dz * z;

	// AIRCRAFT REF. FRAME ACCELERATION
	IntermediateState ddxIMU;
	IntermediateState ddyIMU;
	IntermediateState ddzIMU;
// 	IntermediateState w1IMU;
// 	IntermediateState w2IMU;
// 	IntermediateState w3IMU;

// 	ddxIMU = e11*ddq(0,0) + e21*ddq(1,0) + e31*(ddq(2,0) + g);
// 	ddyIMU = e12*ddq(0,0) + e22*ddq(1,0) + e32*(ddq(2,0) + g);
// 	ddzIMU = e13*ddq(0,0) + e23*ddq(1,0) + e33*(ddq(2,0) + g);
	ddxIMU = ddq(0, 0) * e11 + ddq(1, 0) * e21 + ddq(2, 0) * e31
			- ddelta * ddelta * e11 * x - ddelta * ddelta * e21 * y
			+ 2 * ddelta * dx * e21 - 2 * ddelta * dy * e11 + dddelta * e21 * rA
			+ dddelta * e21 * x - dddelta * e11 * y
			- ddelta * ddelta * e11 * rA;
	ddyIMU = ddq(0, 0) * e12 + ddq(1, 0) * e22 + ddq(2, 0) * e32
			- ddelta * ddelta * e12 * x - ddelta * ddelta * e22 * y
			+ 2 * ddelta * dx * e22 - 2 * ddelta * dy * e12 + dddelta * e22 * rA
			+ dddelta * e22 * x - dddelta * e12 * y
			- ddelta * ddelta * e12 * rA;
	ddzIMU = ddq(0, 0) * e13 + ddq(1, 0) * e23 + ddq(2, 0) * e33
			- ddelta * ddelta * e13 * x - ddelta * ddelta * e23 * y
			+ 2 * ddelta * dx * e23 - 2 * ddelta * dy * e13 + dddelta * e23 * rA
			+ dddelta * e23 * x - dddelta * e13 * y
			- ddelta * ddelta * e13 * rA;

// 	IntermediateState ddxAend, ddyAend;
// 	ddxAend = ddelta*ddelta*rA*sin(delta);
// 	ddyAend = - rA*cos(delta)*ddelta*ddelta;
// // 	ddxAend = ddxA;
// // 	ddyAend = ddyA;

	IntermediateState G1end(3, 1);
// 	G1end(0,0) = F[0] + ddelta*dy*m;
// 	G1end(1,0) = F[1] - ddelta*dx*m;
// 	G1end(2,0) = F[2] - g*mgrav;
	G1end(0, 0) = F[0]
			+ (m
					* (2 * ddelta * dy + 2 * ddelta * ddelta * rA
							+ 2 * ddelta * ddelta * x)) / 2 + ddelta * dy * m;
	G1end(1, 0) = F[1] - (m * (2 * ddelta * dx - 2 * ddelta * ddelta * y)) / 2
			- ddelta * dx * m;
	G1end(2, 0) = F[2] - g * mgrav;

	// lambdaright
	IntermediateState lambdarightend;
	lambdarightend = NabGT * IMA * G1end - G2;

	// lambda
	IntermediateState lambdaend;
	lambdaend = lambdarightend / LambdaFac;

	// ddq (accelerations)
	IntermediateState ddqend(3, 1);
	ddqend = IMA * (G1end - NabG * lambdaend);

	IntermediateState ddxIMUend;
	IntermediateState ddyIMUend;
	IntermediateState ddzIMUend;
// 	ddxIMUend = e11*ddqend(0,0) + e21*ddqend(1,0) + e31*(ddqend(2,0) + g);
// 	ddyIMUend = e12*ddqend(0,0) + e22*ddqend(1,0) + e32*(ddqend(2,0) + g);
// 	ddzIMUend = e13*ddqend(0,0) + e23*ddqend(1,0) + e33*(ddqend(2,0) + g);
	ddxIMUend = ddqend(0, 0) * e11 + ddqend(1, 0) * e21 + ddqend(2, 0) * e31
			- ddelta * ddelta * e11 * x - ddelta * ddelta * e21 * y
			+ 2 * ddelta * dx * e21 - 2 * ddelta * dy * e11
			+ -ddelta * ddelta * e11 * rA;
	ddyIMUend = ddqend(0, 0) * e12 + ddqend(1, 0) * e22 + ddqend(2, 0) * e32
			- ddelta * ddelta * e12 * x - ddelta * ddelta * e22 * y
			+ 2 * ddelta * dx * e22 - 2 * ddelta * dy * e12
			+ -ddelta * ddelta * e12 * rA;
	ddzIMUend = ddqend(0, 0) * e13 + ddqend(1, 0) * e23 + ddqend(2, 0) * e33
			- ddelta * ddelta * e13 * x - ddelta * ddelta * e23 * y
			+ 2 * ddelta * dx * e23 - 2 * ddelta * dy * e13
			+ -ddelta * ddelta * e13 * rA;

// 	ddyIMU = ddX(1,0)*(q0*q0 - q1*q1 + q2*q2 - q3*q3) + (ddX(2,0) + g)*(2*q0*q1 + 2*q2*q3) - ddX(0,0)*(2*q0*q3 - 2*q1*q2);
// 	ddzIMU = ddX(0,0)*(2*q0*q2 + 2*q1*q3) - ddX(1,0)*(2*q0*q1 - 2*q2*q3) + (ddX(2,0) + g)*(q0*q0 - q1*q1 - q2*q2 + q3*q3);
// 	w1IMU = w1;
// 	w2IMU = w2;
// 	w3IMU = w3;

	///////////////////////////// END OF AUTO-GENERATED CODE //////////////////////////////////////////////////////

	// TORQUES (BRIDLE)
	// ---------------------------------------------------------------
// 	zT = -0.02;
	TB[0] = zT * (e12 * lambda * x + e22 * lambda * y + e32 * lambda * z);
	TB[1] = -zT * (e11 * lambda * x + e21 * lambda * y + e31 * lambda * z);
	TB[2] = 0;

	// TORQUES (AERO)
	// ---------------------------------------------------------------

	T[0] = 0.5 * rho * VKite2 * SPAN * CR + TB[0];
	T[1] = 0.5 * rho * VKite2 * CHORD * CP + TB[1];
	T[2] = 0.5 * rho * VKite2 * SPAN * CY + TB[2];

	IntermediateState dw1, dw2, dw3;
// 	dw1 = (I31*(T[2] + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3) - (I3*(T[0] - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3);
// 	dw2 = (T[1] + w1*(I31*w1 + I3*w3) - w3*(I1*w1 + I31*w3))/I2;
// 	dw3 = (I31*(T[0] - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3) - (I1*(T[2] + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3);
	dw1 = (I31 * T[2]) / (I31 * I31 - I1 * I3)
			- (I3 * T[0]) / (I31 * I31 - I1 * I3);
	dw2 = T[1] / I2;
	dw3 = (I31 * T[0]) / (I31 * I31 - I1 * I3)
			- (I1 * T[2]) / (I31 * I31 - I1 * I3);

	IntermediateState ConstR1, ConstR2, ConstR3, ConstR4, ConstR5, ConstR6;
	ConstR1 = e11 * e11 + e12 * e12 + e13 * e13 - 1;
	ConstR2 = e11 * e21 + e12 * e22 + e13 * e23;
	ConstR3 = e11 * e31 + e12 * e32 + e13 * e33;
	ConstR4 = e21 * e21 + e22 * e22 + e23 * e23 - 1;
	ConstR5 = e21 * e31 + e22 * e32 + e23 * e33;
	ConstR6 = e31 * e31 + e32 * e32 + e33 * e33 - 1;

// 	Fcable = lambda*r;

// THE "RIGHT-HAND-SIDE" OF THE ODE:
// ---------------------------------------------------------------
	DifferentialEquation f(0.0, Tc);

	f << dot(x) == dx;
	f << dot(y) == dy;
	f << dot(z) == dz;
	f << dot(dx) == 1 * ddq(0, 0);
	f << dot(dy) == 1 * ddq(1, 0);
	f << dot(dz) == 1 * ddq(2, 0);

	f << dot(e11) == de11;
	f << dot(e12) == de12;
	f << dot(e13) == de13;
	f << dot(e21) == de21;
	f << dot(e22) == de22;
	f << dot(e23) == de23;
	f << dot(e31) == de31;
	f << dot(e32) == de32;
	f << dot(e33) == de33;

	f << dot(w1) == dw1;
	f << dot(w2) == dw2;
	f << dot(w3) == dw3;

// 	f  << dot(r)      ==  dr                             ;
// 	f  << dot(dr)     ==  ddr                            ;
	f << dot(delta) == ddelta;
	f << dot(ddelta) == dddelta;
	f << dot(ur) == dur;
	f << dot(up) == dup;

	// ===============================================================
	//                        END OF MODEL EQUATIONS
	// ===============================================================

	// DEFINE A MEASUREMENT FUNCTION:
	// ------------------------------

	// The camera measurement is given by: [u;v;s;] = Proj*Rp*Translation(pos)*Rotation(q)*pos_marker_body;

	IntermediateState Rotation(4, 4);
	Rotation = eye(4);
	Rotation(0, 0) = e11;
	Rotation(0, 1) = e12;
	Rotation(0, 2) = e13;
	Rotation(1, 0) = e21;
	Rotation(1, 1) = e22;
	Rotation(1, 2) = e23;
	Rotation(2, 0) = e31;
	Rotation(2, 1) = e32;
	Rotation(2, 2) = e33;

	IntermediateState Translation(4, 4);
	Translation = eye(4);
	Translation(0, 3) = x; //pos[0];
	Translation(1, 3) = y; //pos[1];
	Translation(2, 3) = z; //pos[2];
//
// 	Matrix TrAnch(4,4);
// 	TrAnch = eye(4);
// 	TrAnch(0,3) = -1.085;
// 	TrAnch(1,3) = 0.0;
// 	TrAnch(2,3) = 0.0;
//
// 	IntermediateState Rdelta(4,4);
// 	Rdelta = eye(4);
// 	Rdelta(0,0) =  cos(-delta - PI/2);
// 	Rdelta(0,1) = -sin(-delta - PI/2);
// 	Rdelta(1,0) =  sin(-delta - PI/2);
// 	Rdelta(1,1) =  cos(-delta - PI/2);
//
	Matrix RPC1 = readFromFile("CameraCalibration/cameras/RPC1.dat");

	Matrix RPC2 = readFromFile("CameraCalibration/cameras/RPC2.dat");


	Matrix ProjC1_dat = readFromFile("CameraCalibration/cameras/PC1.dat");
	Matrix ProjC1(3,3);
	ProjC1(0,0) = ProjC1_dat(0,0); ProjC1(0,1) = 0.0; ProjC1(0,2) = ProjC1_dat(2,0);
	ProjC1(1,0) = 0.0; ProjC1(1,1) = ProjC1_dat(1,0); ProjC1(1,2) = ProjC1_dat(3,0);
	ProjC1(2,0) = ProjC1_dat(0,0); ProjC1(2,1) = 0.0; ProjC1(2,2) = 1.0;

	Matrix ProjC2_dat = readFromFile("CameraCalibration/cameras/PC2.dat");
	Matrix ProjC2(3,3);
	ProjC2(0,0) = ProjC2_dat(0,0); ProjC2(0,1) = 0.0; ProjC2(0,2) = ProjC2_dat(2,0);
	ProjC2(1,0) = 0.0; ProjC2(1,1) = ProjC2_dat(1,0); ProjC2(1,2) = ProjC2_dat(3,0);
	ProjC2(2,0) = ProjC2_dat(0,0); ProjC2(2,1) = 0.0; ProjC2(2,2) = 1.0;


	Matrix pos_marker_body1_dat = readFromFile(
			"pos_marker_body1.dat");
	Matrix pos_marker_body1(4,1);
	pos_marker_body1(0,0) = pos_marker_body1_dat(0,0);
	pos_marker_body1(1,0) = pos_marker_body1_dat(1,0);
	pos_marker_body1(2,0) = pos_marker_body1_dat(2,0);
	pos_marker_body1(3,0) = 1.0;

	Matrix pos_marker_body2_dat = readFromFile(
			"pos_marker_body2.dat");
	Matrix pos_marker_body2(4,1);
	pos_marker_body2(0,0) = pos_marker_body2_dat(0,0);
	pos_marker_body2(1,0) = pos_marker_body2_dat(1,0);
	pos_marker_body2(2,0) = pos_marker_body2_dat(2,0);
	pos_marker_body2(3,0) = 1.0;

	Matrix pos_marker_body3_dat = readFromFile(
			"pos_marker_body3.dat");
	Matrix pos_marker_body3(4,1);
	pos_marker_body3(0,0) = pos_marker_body3_dat(0,0);
	pos_marker_body3(1,0) = pos_marker_body3_dat(1,0);
	pos_marker_body3(2,0) = pos_marker_body3_dat(2,0);
	pos_marker_body3(3,0) = 1.0;
	ProjC1.print("ProjC1: ");
	ProjC2.print("ProjC2: ");
 	pos_marker_body1.print("pos_marker_body1:");
 	pos_marker_body2.print("pos_marker_body2:");
 	pos_marker_body3.print("pos_marker_body3:");

	Matrix RpC1Temp(4, 4), RpC2Temp(4, 4);
	RpC1Temp = eye(4);
	RpC1Temp(0, 0) = RPC1(0, 0);
	RpC1Temp(0, 1) = RPC1(1, 0);
	RpC1Temp(0, 2) = RPC1(2, 0);
	RpC1Temp(1, 0) = RPC1(0, 1);
	RpC1Temp(1, 1) = RPC1(1, 1);
	RpC1Temp(1, 2) = RPC1(2, 1);
	RpC1Temp(2, 0) = RPC1(0, 2);
	RpC1Temp(2, 1) = RPC1(1, 2);
	RpC1Temp(2, 2) = RPC1(2, 2);

	RpC2Temp = eye(4);
	RpC2Temp(0, 0) = RPC2(0, 0);
	RpC2Temp(0, 1) = RPC2(1, 0);
	RpC2Temp(0, 2) = RPC2(2, 0);
	RpC2Temp(1, 0) = RPC2(0, 1);
	RpC2Temp(1, 1) = RPC2(1, 1);
	RpC2Temp(1, 2) = RPC2(2, 1);
	RpC2Temp(2, 0) = RPC2(0, 2);
	RpC2Temp(2, 1) = RPC2(1, 2);
	RpC2Temp(2, 2) = RPC2(2, 2);

	RpC1Temp(0, 3) = -(RPC1(0, 0) * RPC1(0, 3) + RPC1(1, 0) * RPC1(1, 3)
			+ RPC1(2, 0) * RPC1(2, 3));
	RpC1Temp(1, 3) = -(RPC1(0, 1) * RPC1(0, 3) + RPC1(1, 1) * RPC1(1, 3)
			+ RPC1(2, 1) * RPC1(2, 3));
	RpC1Temp(2, 3) = -(RPC1(0, 2) * RPC1(0, 3) + RPC1(1, 2) * RPC1(1, 3)
			+ RPC1(2, 2) * RPC1(2, 3));

	RpC2Temp(0, 3) = -(RPC2(0, 0) * RPC2(0, 3) + RPC2(1, 0) * RPC2(1, 3)
			+ RPC2(2, 0) * RPC2(2, 3));
	RpC2Temp(1, 3) = -(RPC2(0, 1) * RPC2(0, 3) + RPC2(1, 1) * RPC2(1, 3)
			+ RPC2(2, 1) * RPC2(2, 3));
	RpC2Temp(2, 3) = -(RPC2(0, 2) * RPC2(0, 3) + RPC2(1, 2) * RPC2(1, 3)
			+ RPC2(2, 2) * RPC2(2, 3));

	Matrix RpC1Full(4, 4), RpC2Full(4, 4);
	RpC1Full = RpC1Temp;
	RpC2Full = RpC2Temp;

	Matrix RpC1(3, 4), RpC2(3, 4);
	RpC1(0, 0) = RpC1Full(0, 0);
	RpC1(0, 1) = RpC1Full(0, 1);
	RpC1(0, 2) = RpC1Full(0, 2);
	RpC1(0, 3) = RpC1Full(0, 3);
	RpC1(1, 0) = RpC1Full(1, 0);
	RpC1(1, 1) = RpC1Full(1, 1);
	RpC1(1, 2) = RpC1Full(1, 2);
	RpC1(1, 3) = RpC1Full(1, 3);
	RpC1(2, 0) = RpC1Full(2, 0);
	RpC1(2, 1) = RpC1Full(2, 1);
	RpC1(2, 2) = RpC1Full(2, 2);
	RpC1(2, 3) = RpC1Full(2, 3);

	RpC2(0, 0) = RpC2Full(0, 0);
	RpC2(0, 1) = RpC2Full(0, 1);
	RpC2(0, 2) = RpC2Full(0, 2);
	RpC2(0, 3) = RpC2Full(0, 3);
	RpC2(1, 0) = RpC2Full(1, 0);
	RpC2(1, 1) = RpC2Full(1, 1);
	RpC2(1, 2) = RpC2Full(1, 2);
	RpC2(1, 3) = RpC2Full(1, 3);
	RpC2(2, 0) = RpC2Full(2, 0);
	RpC2(2, 1) = RpC2Full(2, 1);
	RpC2(2, 2) = RpC2Full(2, 2);
	RpC2(2, 3) = RpC2Full(2, 3);

	IntermediateState uvsC1M1(3, 1), uvsC2M2(3, 1), uvsC1M3(3, 1), uvsC2M1(3,
			1), uvsC1M2(3, 1), uvsC2M3(3, 1);
	uvsC1M1 = ProjC1 * RpC1 * Translation * Rotation * pos_marker_body1;
	uvsC1M2 = ProjC1 * RpC1 * Translation * Rotation * pos_marker_body2;
	uvsC1M3 = ProjC1 * RpC1 * Translation * Rotation * pos_marker_body3;
	uvsC2M1 = ProjC2 * RpC2 * Translation * Rotation * pos_marker_body1;
	uvsC2M2 = ProjC2 * RpC2 * Translation * Rotation * pos_marker_body2;
	uvsC2M3 = ProjC2 * RpC2 * Translation * Rotation * pos_marker_body3;

	IntermediateState uvC1M1(2, 1), uvC2M2(2, 1), uvC1M3(2, 1), uvC2M1(2, 1),
			uvC1M2(2, 1), uvC2M3(2, 1);
	uvC1M1(0, 0) = uvsC1M1(0, 0) / uvsC1M1(2, 0);
	uvC1M1(1, 0) = uvsC1M1(1, 0) / uvsC1M1(2, 0);
	uvC1M2(0, 0) = uvsC1M2(0, 0) / uvsC1M2(2, 0);
	uvC1M2(1, 0) = uvsC1M2(1, 0) / uvsC1M2(2, 0);
	uvC1M3(0, 0) = uvsC1M3(0, 0) / uvsC1M3(2, 0);
	uvC1M3(1, 0) = uvsC1M3(1, 0) / uvsC1M3(2, 0);

	uvC2M1(0, 0) = uvsC2M1(0, 0) / uvsC2M1(2, 0);
	uvC2M1(1, 0) = uvsC2M1(1, 0) / uvsC2M1(2, 0);
	uvC2M2(0, 0) = uvsC2M2(0, 0) / uvsC2M2(2, 0);
	uvC2M2(1, 0) = uvsC2M2(1, 0) / uvsC2M2(2, 0);
	uvC2M3(0, 0) = uvsC2M3(0, 0) / uvsC2M3(2, 0);
	uvC2M3(1, 0) = uvsC2M3(1, 0) / uvsC2M3(2, 0);

	Matrix RIMU = readFromFile("IMU/RIMU.dat");
// 	RIMU = eye(3);

	IntermediateState aE(3, 1), aEend(3, 1);
	aE(0, 0) = ddxIMU;
	aE(1, 0) = ddyIMU;
	aE(2, 0) = ddzIMU;
	aEend(0, 0) = ddxIMUend;
	aEend(1, 0) = ddyIMUend;
	aEend(2, 0) = ddzIMUend;

	IntermediateState Omega(3, 1);
	Omega(0, 0) = w1;
	Omega(1, 0) = w2;
	Omega(2, 0) = w3;

	IntermediateState aSHIFT(3, 1);
	aSHIFT(0, 0) = XIMU3 * dw2 - XIMU2 * dw3 + w2 * (XIMU2 * w1 - XIMU1 * w2)
			+ w3 * (XIMU3 * w1 - XIMU1 * w3);
	aSHIFT(1, 0) = XIMU1 * dw3 - XIMU3 * dw1 - w1 * (XIMU2 * w1 - XIMU1 * w2)
			+ w3 * (XIMU3 * w2 - XIMU2 * w3);
	aSHIFT(2, 0) = XIMU2 * dw1 - XIMU1 * dw2 - w1 * (XIMU3 * w1 - XIMU1 * w3)
			- w2 * (XIMU3 * w2 - XIMU2 * w3);

	IntermediateState aIMU, aIMUend, wIMU;
// 	aIMU = RIMU*(aE + aSHIFT);
	aIMU = RIMU * aE;
	aIMUend = RIMU * aEend;
	wIMU = RIMU * Omega;

	Function CostLSQend;

// 	CostLSQend << x;
// 	CostLSQend << y;
// 	CostLSQend << z;
// // 	CostLSQ << dx;
// // 	CostLSQ << dy;
// // 	CostLSQ << dz;
// 	CostLSQend << e11;
// 	CostLSQend << e12;
// 	CostLSQend << e13;
// 	CostLSQend << e21;
// 	CostLSQend << e22;
// 	CostLSQend << e23;
// 	CostLSQend << e31;
// 	CostLSQend << e32;
// 	CostLSQend << e33;

// 	CostLSQend << w1;
// 	CostLSQend << w2;
// 	CostLSQend << w3;
// 	CostLSQend << ddxIMUend;
// 	CostLSQend << ddyIMUend;
// 	CostLSQend << ddzIMUend;
	CostLSQend << wIMU(0, 0);
	CostLSQend << wIMU(1, 0);
	CostLSQend << wIMU(2, 0);
	CostLSQend << aIMUend(0, 0);
	CostLSQend << aIMUend(1, 0);
	CostLSQend << aIMUend(2, 0);
// 	CostLSQend << r;
// 	CostLSQend << dr;
	CostLSQend << delta;
// 	CostLSQend << ddelta;
// 	CostLSQend << ur/32767;
// 	CostLSQend << up/10000;
	CostLSQend << ur;
	CostLSQend << up;

	// COST FUNCTION
	//---------------
	Function CostLSQx;

	int j = 0;

// 	CostLSQx << x;
// 	CostLSQx << y;
// 	CostLSQx << z;
// // // 	CostLSQ << dx;
// // // 	CostLSQ << dy;
// // // 	CostLSQ << dz;
// 	CostLSQx << e11;
// 	CostLSQx << e12;
// 	CostLSQx << e13;
// 	CostLSQx << e21;
// 	CostLSQx << e22;
// 	CostLSQx << e23;
// 	CostLSQx << e31;
// 	CostLSQx << e32;
// 	CostLSQx << e33;

	CostLSQx << uvC1M1(0, 0);
	CostLSQx << uvC1M1(1, 0);
	CostLSQx << uvC1M2(0, 0);
	CostLSQx << uvC1M2(1, 0);
	CostLSQx << uvC1M3(0, 0);
	CostLSQx << uvC1M3(1, 0);
	CostLSQx << uvC2M1(0, 0);
	CostLSQx << uvC2M1(1, 0);
	CostLSQx << uvC2M2(0, 0);
	CostLSQx << uvC2M2(1, 0);
	CostLSQx << uvC2M3(0, 0);
	CostLSQx << uvC2M3(1, 0);
// 	CostLSQx << w1;
// 	CostLSQx << w2;
// 	CostLSQx << w3;
// 	CostLSQx << ddxIMU;
// 	CostLSQx << ddyIMU;
// 	CostLSQx << ddzIMU;
	CostLSQx << wIMU(0, 0);
	CostLSQx << wIMU(1, 0);
	CostLSQx << wIMU(2, 0);
	CostLSQx << aIMU(0, 0);
	CostLSQx << aIMU(1, 0);
	CostLSQx << aIMU(2, 0);
// 	CostLSQx << r;
// 	CostLSQx << dr;
	CostLSQx << delta;
// 	CostLSQx << ddelta;
	CostLSQx << ur;
	CostLSQx << up;

	Function CostLSQu;
	// Controls :
	// -------------------------
// 	for ( j=0; j < n_U; j++ ){
// 		CostLSQu <<   U[j];
// 	}
	CostLSQu << dddelta;
	CostLSQu << dur;
	CostLSQu << dup;
	// Perturbations :
	// -------------------------
// 	for ( j=0; j < n_W; j++ ){
// 		CostLSQu <<   W[j];
// 	}

	Expression ExprCostLSQx;
	CostLSQx.getExpression(ExprCostLSQx);
	Expression ExprCostLSQu;
	CostLSQu.getExpression(ExprCostLSQu);

	Function CostLSQ;
	CostLSQ << ExprCostLSQx;
	CostLSQ << ExprCostLSQu;

	// Matrix of weights for the LSQ cost function
	Matrix Q = readFromFile("./MHE_Q.txt");
	Matrix Qend = readFromFile("./MHE_Qend.txt");
// 	Matrix Qx = readFromFile( "MHE_Qx.txt" );
// 	Matrix Qu = readFromFile( "MHE_Qu.txt" );

	cout << "f dim: " << f.getDim() << endl;
	cout << "CostLSQend dim: " << CostLSQend.getDim() << endl;
	cout << "CostLSQend #u: " << CostLSQend.getNU() << endl;
	cout << "CostLSQ dim: " << CostLSQ.getDim() << endl;
	cout << "CostLSQ  #u: " << CostLSQ.getNU() << endl;

	// TERMINAL COST FUNCTION
	//---------------
//	VariablesGrid Last;
//	Last = readFromFile( "Measurements.txt" );
//	Vector x_F(9);
//	// Get the last reference
//// 	for ( i=0; i<3; i++ ){
//// 		x_F(i) = Last(Ncvp,i);
//// 	};
//	// Get the last reference
//	for ( i=0; i<9; i++ ){
//		x_F(i) = Last(Ncvp,i+12);
//	};
// 	x_F.print();

//
// 	Matrix QF = readFromFile( "QArrival.txt" );
// 	// 	Multiply the terminal cost
// 	double WF = 1;
// 	for ( i=0; i<n_XD; i++ ){
// 		for ( j=0; j<n_XD; j++ ){
// 			QF(i,j) = WF*QF(i,j);
// 		}
// 	};
//
// 	// Build up an expression for the arrival cost
// 	IntermediateState ACost = 0;
//
// 	// X_N^T * QF * X_N
// 	for ( i=0; i<n_XD; i++ ){
// 		for ( j=0; j<n_XD; j++ ){
// 			ACost += (XD[i]-Last(Ncvp,i) )*(XD[j]-Last(Ncvp,j) )*QF(i,j);
// 		}
// 	}

	// L1 PENALTY ON SLACKS
	//---------------
// 	double WsCL = 1e1;
// 	double WsL  = 1e1;
// 	IntermediateState SLACK = 0;
// 	SLACK += WsCL*SlackCLmax;
// 	SLACK += WsL*SlackLambda;

	// DEFINE AN OPTIMAL CONTROL PROBLEM:
	// ----------------------------------
	OCP ocp(0.0, 1.0, 10);

	ExportVariable QQ, QQend;

	ocp.minimizeLSQ(QQ, CostLSQ);
	ocp.minimizeLSQEndTerm(QQend, CostLSQend);
// 	ocp.minimizeMayerTerm( ACost );
// 	ocp.minimizeMayerTerm( SLACK );

	ocp.subjectTo(f);

	// TERMINAL CONSTRAINTS
	// ---------------------------------
	ocp.subjectTo(AT_END, ConstR1 == 0);
	ocp.subjectTo(AT_END, ConstR2 == 0);
	ocp.subjectTo(AT_END, ConstR3 == 0);
	ocp.subjectTo(AT_END, ConstR4 == 0);
	ocp.subjectTo(AT_END, ConstR5 == 0);
	ocp.subjectTo(AT_END, ConstR6 == 0);

	ocp.subjectTo(AT_END, Const == 0);
	ocp.subjectTo(AT_END, dConst == 0);

	// INITIAL VALUE CONSTRAINTS:
	// ---------------------------------

	// Read the initial state
// 	VariablesGrid x_0;
//     x_0 = readFromFile( "NMPC_X0.txt" );
// 	// Impose the initial constraint on all states
// 	for ( i=0; i < n_XD; i++ ) {
// 		ocp.subjectTo( AT_START,   XD[i] ==  x_0(0,i) );
// 	}

	// BOUNDS:
	// ---------------------------------

	// CONTROL
	double AccRate = 30 * PI / 180;
	ocp.subjectTo(-AccRate <= dddelta <= AccRate);
// 	ocp.subjectTo(  -0.5 <= ddr <= 0.5 );

	ocp.subjectTo(-1 <= up <= 1);
	ocp.subjectTo(-3.2767 <= ur <= 3.2767);
	ocp.subjectTo(-10 <= dup <= 10);
	ocp.subjectTo(-10 <= dur <= 10);

// 	ocp.subjectTo( -0.2 <= up <= 0.2 );
// 	ocp.subjectTo( -0.2 <= ur <= 0.2 );
// 	ocp.subjectTo( -20.0 <= dup <= 20.0 );
// 	ocp.subjectTo( -20.0 <= dur <= 20.0 );

	// STATE
//	ocp.subjectTo( -1 <= z );

//	ocp.subjectTo( -1 <= e11 <= 1 );
//	ocp.subjectTo( -1 <= e12 <= 1 );
//	ocp.subjectTo( -1 <= e13 <= 1 );
//	ocp.subjectTo( -1 <= e21 <= 1 );
//	ocp.subjectTo( -1 <= e22 <= 1 );
//	ocp.subjectTo( -1 <= e23 <= 1 );
//	ocp.subjectTo( -1 <= e31 <= 1 );
//	ocp.subjectTo( -1 <= e32 <= 1 );
//	ocp.subjectTo( -1 <= e33 <= 1 );

// 	for ( j=0; j < n_W; j++ ){
// 		ocp.subjectTo(  W[j] == 0 );
// 	}

	// PATH CONSTRAINTS
	// ---------------------------------
// 	ocp.subjectTo( -0.1 <= CL <= 1.1 );
// 	ocp.subjectTo( -0.1 <= CL + SlackCLmax );
// 	ocp.subjectTo(        CL - SlackCLmax <= 1.1 );
// 	ocp.subjectTo( 0 <= SlackCLmax <= 0.4 );

	// lambda > 10
// 	ocp.subjectTo( 1 <= lambda/10 );
// 	ocp.subjectTo( 1 <= lambda/10 + SlackLambda );
// 	ocp.subjectTo( 0 <= SlackLambda <= 1 );

// 	ocp.subjectTo( -10*PI/180 <= beta <= 10*PI/180 );
// 	ocp.subjectTo( -10*PI/180 <= alpha <= 10*PI/180 );
// 	ocp.subjectTo( -15*PI/180 <= alphaTail <= 15*PI/180 );

// 	ocp.subjectTo( -30 <= dW <= 30 );
// 	ocp.subjectTo(  dW == 0 );

	cout << "f dim: " << f.getDim() << endl;
	cout << "CostLSQx dim: " << CostLSQend.getDim() << endl;
	cout << "CostLSQx #u: " << CostLSQend.getNU() << endl;
	cout << "CostLSQ dim: " << CostLSQ.getDim() << endl;
	cout << "CostLSQ  #u: " << CostLSQ.getNU() << endl;
	cout << "CostLSQend dim: " << CostLSQend.getDim() << endl;
	cout << "CostLSQend  #u: " << CostLSQend.getNU() << endl;

	MHEexport mhe(ocp);

	mhe.set(HESSIAN_APPROXIMATION, GAUSS_NEWTON);
	mhe.set(DISCRETIZATION_TYPE, MULTIPLE_SHOOTING);
	//	mhe.set( DISCRETIZATION_TYPE,   SINGLE_SHOOTING );

	// 	mpc.set( INTEGRATOR_TYPE,             INT_RK4    );
	// 	mpc.set( NUM_INTEGRATOR_STEPS,        100              );

	mhe.set(INTEGRATOR_TYPE, INT_IRK_GL2);
	mhe.set(NUM_INTEGRATOR_STEPS, 30);

	mhe.set(IMPLICIT_INTEGRATOR_NUM_ITS, 3);
	mhe.set(IMPLICIT_INTEGRATOR_NUM_ITS_INIT, 0);
	mhe.set(LINEAR_ALGEBRA_SOLVER, HOUSEHOLDER_QR);
	mhe.set(UNROLL_LINEAR_SOLVER, NO);
	mhe.set(IMPLICIT_INTEGRATOR_MODE, IFTR);

	mhe.set(SPARSE_QP_SOLUTION, CONDENSING);

	mhe.set(QP_SOLVER, QP_QPOASES);
	mhe.set(HOTSTART_QP, YES);
	//	mhe.set( GENERATE_TEST_FILE,    YES              );
	// 	mpc.set( GENERATE_MAKE_FILE,    YES              );

	mhe.set(CG_USE_C99, YES);

	mhe.set(PRINTLEVEL, HIGH);

	mhe.set(CG_USE_VARIABLE_WEIGHTING_MATRIX, YES);

	mhe.exportCode("kite_export");
	mhe.printDimensionsQP();

//	Q.getNumCols()

	return 0;
}

