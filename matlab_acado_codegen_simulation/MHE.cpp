 /**
 *    SINGLE POWER KITE 
 *    CARTESIAN COORDINATES (ODE FORMULATION)
 *    JULY 2012 SEBASTIEN GROS, MARIO ZANON HIGHWIND, OPTEC
 */

#include <acado_toolkit.hpp>
#include <mhe_export.hpp>

int main( int argc, char * const argv[] )
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
	 
	int i=1;
	while (i < argc) {
		// load the i-th string argument passed to the main function
		char* input = argv[i];
		// parse the string to a double value
		IC[i-1] = atof(input);
		i++;
	}
	
	cout << "Operational Parameters" << endl;
	cout << "------------" << endl;
	for (i = 0; i < argc-1; ++i) {
		cout << i+1 << ":\t" << IC[i] << endl;
	}
	//======================================================================	
	
	double Ncvp  = IC[1]; // Number of control invervals
	double Tc    = IC[2]; // Prediction Horizon [s]
	double r = IC[4]; // Tether length [m]
	 
   USING_NAMESPACE_ACADO
	
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

	double PI = 3.1415926535897932;

#include "sensor_constants.hpp"
#include "model_constants.hpp"
#include "state_definition.hpp"
#include "control_definition.hpp"
#include "model_definition.hpp"
	

		
	// AIRCRAFT REF. FRAME ACCELERATION 
	IntermediateState ddxIMU; 
	IntermediateState ddyIMU; 
	IntermediateState ddzIMU; 

	ddxIMU = ddq(0,0)*e11 + ddq(1,0)*e21 + ddq(2,0)*e31 - ddelta*ddelta*e11*x - ddelta*ddelta*e21*y + 2*ddelta*dx*e21 - 2*ddelta*dy*e11 + dddelta*e21*rA + dddelta*e21*x - dddelta*e11*y - ddelta*ddelta*e11*rA + e31*g;
	ddyIMU = ddq(0,0)*e12 + ddq(1,0)*e22 + ddq(2,0)*e32 - ddelta*ddelta*e12*x - ddelta*ddelta*e22*y + 2*ddelta*dx*e22 - 2*ddelta*dy*e12 + dddelta*e22*rA + dddelta*e22*x - dddelta*e12*y - ddelta*ddelta*e12*rA + e32*g;
	ddzIMU = ddq(0,0)*e13 + ddq(1,0)*e23 + ddq(2,0)*e33 - ddelta*ddelta*e13*x - ddelta*ddelta*e23*y + 2*ddelta*dx*e23 - 2*ddelta*dy*e13 + dddelta*e23*rA + dddelta*e23*x - dddelta*e13*y - ddelta*ddelta*e13*rA + e33*g;	
	
	IntermediateState G1end(3,1); 
	G1end(0,0) = F[0] + (m*(2*ddelta*dy + 2*ddelta*ddelta*rA + 2*ddelta*ddelta*x))/2 + ddelta*dy*m; 
	G1end(1,0) = F[1] - (m*(2*ddelta*dx - 2*ddelta*ddelta*y))/2                      - ddelta*dx*m; 
	G1end(2,0) = F[2] - g*mgrav; 
	
	// lambdaright 
	IntermediateState lambdarightend; 
	lambdarightend = NabGT*IMA*G1end - G2; 

	// lambda 
	IntermediateState lambdaend; 
	lambdaend = lambdarightend/LambdaFac; 

	// ddq (accelerations) 
	IntermediateState ddqend(3,1); 
	ddqend = IMA*(G1end-NabG*lambdaend); 

	IntermediateState ddxIMUend; 
	IntermediateState ddyIMUend; 
	IntermediateState ddzIMUend; 
	ddxIMUend = ddqend(0,0)*e11 + ddqend(1,0)*e21 + ddqend(2,0)*e31 - ddelta*ddelta*e11*x - ddelta*ddelta*e21*y + 2*ddelta*dx*e21 - 2*ddelta*dy*e11 + - ddelta*ddelta*e11*rA + e31*g;
	ddyIMUend = ddqend(0,0)*e12 + ddqend(1,0)*e22 + ddqend(2,0)*e32 - ddelta*ddelta*e12*x - ddelta*ddelta*e22*y + 2*ddelta*dx*e22 - 2*ddelta*dy*e12 + - ddelta*ddelta*e12*rA + e32*g;
	ddzIMUend = ddqend(0,0)*e13 + ddqend(1,0)*e23 + ddqend(2,0)*e33 - ddelta*ddelta*e13*x - ddelta*ddelta*e23*y + 2*ddelta*dx*e23 - 2*ddelta*dy*e13 + - ddelta*ddelta*e13*rA + e33*g;
		
// 	ddyIMU = ddX(1,0)*(q0*q0 - q1*q1 + q2*q2 - q3*q3) + (ddX(2,0) + g)*(2*q0*q1 + 2*q2*q3) - ddX(0,0)*(2*q0*q3 - 2*q1*q2); 
// 	ddzIMU = ddX(0,0)*(2*q0*q2 + 2*q1*q3) - ddX(1,0)*(2*q0*q1 - 2*q2*q3) + (ddX(2,0) + g)*(q0*q0 - q1*q1 - q2*q2 + q3*q3); 
// 	w1IMU = w1; 
// 	w2IMU = w2; 
// 	w3IMU = w3;
		
	IntermediateState ConstR1, ConstR2, ConstR3, ConstR4, ConstR5, ConstR6;
	ConstR1 = e11*e11 + e12*e12 + e13*e13 - 1;
	ConstR2 = e11*e21 + e12*e22 + e13*e23;
	ConstR3 = e11*e31 + e12*e32 + e13*e33;
	ConstR4 = e21*e21 + e22*e22 + e23*e23 - 1;
	ConstR5 = e21*e31 + e22*e32 + e23*e33;
	ConstR6 = e31*e31 + e32*e32 + e33*e33 - 1;

	
	// DEFINE A MEASUREMENT FUNCTION:
	// ------------------------------

	// The camera measurement is given by: [u;v;s;] = Proj*Rp*Translation(pos)*Rotation(q)*pos_marker_body;

	IntermediateState Rotation(4,4);
	Rotation = eye(4);
	Rotation(0,0) = e11;
	Rotation(0,1) = e12;
	Rotation(0,2) = e13;
	Rotation(1,0) = e21;
	Rotation(1,1) = e22;
	Rotation(1,2) = e23;
	Rotation(2,0) = e31;
	Rotation(2,1) = e32;
	Rotation(2,2) = e33;

	IntermediateState Translation(4,4);
	Translation = eye(4);
	Translation(0,3) = x;//pos[0];
	Translation(1,3) = y;//pos[1];
	Translation(2,3) = z;//pos[2];
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

#if (CODEGEN_FOR_OROCOS == 1)
	Matrix RPC1 = readFromFile( "../../../../matlab_acado_codegen_simulation/CameraCalibration/RPC1.txt" );
	Matrix RPC2 = readFromFile( "../../../../matlab_acado_codegen_simulation/CameraCalibration/RPC2.txt" );
	Matrix ProjC1 = readFromFile( "../../../../matlab_acado_codegen_simulation/CameraCalibration/PC1.txt" );
	Matrix ProjC2 = readFromFile( "../../../../matlab_acado_codegen_simulation/CameraCalibration/PC2.txt" );
	Matrix pos_marker_body1 = readFromFile( "../../../../matlab_acado_codegen_simulation/CameraCalibration/pos_marker_body1.txt" );
	Matrix pos_marker_body2 = readFromFile( "../../../../matlab_acado_codegen_simulation/CameraCalibration/pos_marker_body2.txt" );
	Matrix pos_marker_body3 = readFromFile( "../../../../matlab_acado_codegen_simulation/CameraCalibration/pos_marker_body3.txt" );
#else
	Matrix RPC1 = readFromFile( "CameraCalibration/RPC1.txt" );
	Matrix RPC2 = readFromFile( "CameraCalibration/RPC2.txt" );
	Matrix ProjC1 = readFromFile( "CameraCalibration/PC1.txt" );
	Matrix ProjC2 = readFromFile( "CameraCalibration/PC2.txt" );
	Matrix pos_marker_body1 = readFromFile( "CameraCalibration/pos_marker_body1.txt" );
	Matrix pos_marker_body2 = readFromFile( "CameraCalibration/pos_marker_body2.txt" );
	Matrix pos_marker_body3 = readFromFile( "CameraCalibration/pos_marker_body3.txt" );
#endif
	
// 	pos_marker_body1.print("pos_marker_body1:");
	
	Matrix RpC1Temp(4,4), RpC2Temp(4,4);
	RpC1Temp = eye(4);
	RpC1Temp(0,0) = RPC1(0,0);
	RpC1Temp(0,1) = RPC1(1,0);
	RpC1Temp(0,2) = RPC1(2,0);
	RpC1Temp(1,0) = RPC1(0,1);
	RpC1Temp(1,1) = RPC1(1,1);
	RpC1Temp(1,2) = RPC1(2,1);
	RpC1Temp(2,0) = RPC1(0,2);
	RpC1Temp(2,1) = RPC1(1,2);
	RpC1Temp(2,2) = RPC1(2,2);
	
	RpC2Temp = eye(4);
	RpC2Temp(0,0) = RPC2(0,0);
	RpC2Temp(0,1) = RPC2(1,0);
	RpC2Temp(0,2) = RPC2(2,0);
	RpC2Temp(1,0) = RPC2(0,1);
	RpC2Temp(1,1) = RPC2(1,1);
	RpC2Temp(1,2) = RPC2(2,1);
	RpC2Temp(2,0) = RPC2(0,2);
	RpC2Temp(2,1) = RPC2(1,2);
	RpC2Temp(2,2) = RPC2(2,2);
	
	RpC1Temp(0,3) = -(RPC1(0,0)*RPC1(0,3) + RPC1(1,0)*RPC1(1,3) + RPC1(2,0)*RPC1(2,3));
	RpC1Temp(1,3) = -(RPC1(0,1)*RPC1(0,3) + RPC1(1,1)*RPC1(1,3) + RPC1(2,1)*RPC1(2,3));
	RpC1Temp(2,3) = -(RPC1(0,2)*RPC1(0,3) + RPC1(1,2)*RPC1(1,3) + RPC1(2,2)*RPC1(2,3));
	
	RpC2Temp(0,3) = -(RPC2(0,0)*RPC2(0,3) + RPC2(1,0)*RPC2(1,3) + RPC2(2,0)*RPC2(2,3));
	RpC2Temp(1,3) = -(RPC2(0,1)*RPC2(0,3) + RPC2(1,1)*RPC2(1,3) + RPC2(2,1)*RPC2(2,3));
	RpC2Temp(2,3) = -(RPC2(0,2)*RPC2(0,3) + RPC2(1,2)*RPC2(1,3) + RPC2(2,2)*RPC2(2,3));
	
	Matrix RpC1Full(4,4), RpC2Full(4,4);
	RpC1Full = RpC1Temp;
	RpC2Full = RpC2Temp;
	
	Matrix RpC1(3,4), RpC2(3,4);
	RpC1(0,0) = RpC1Full(0,0); 	RpC1(0,1) = RpC1Full(0,1); 	RpC1(0,2) = RpC1Full(0,2); 	RpC1(0,3) = RpC1Full(0,3); 
	RpC1(1,0) = RpC1Full(1,0); 	RpC1(1,1) = RpC1Full(1,1); 	RpC1(1,2) = RpC1Full(1,2); 	RpC1(1,3) = RpC1Full(1,3); 
	RpC1(2,0) = RpC1Full(2,0); 	RpC1(2,1) = RpC1Full(2,1); 	RpC1(2,2) = RpC1Full(2,2); 	RpC1(2,3) = RpC1Full(2,3); 
	
	RpC2(0,0) = RpC2Full(0,0); 	RpC2(0,1) = RpC2Full(0,1); 	RpC2(0,2) = RpC2Full(0,2); 	RpC2(0,3) = RpC2Full(0,3); 
	RpC2(1,0) = RpC2Full(1,0); 	RpC2(1,1) = RpC2Full(1,1); 	RpC2(1,2) = RpC2Full(1,2); 	RpC2(1,3) = RpC2Full(1,3); 
	RpC2(2,0) = RpC2Full(2,0); 	RpC2(2,1) = RpC2Full(2,1); 	RpC2(2,2) = RpC2Full(2,2); 	RpC2(2,3) = RpC2Full(2,3); 
	
	IntermediateState uvsC1M1(3,1), uvsC2M2(3,1), uvsC1M3(3,1), uvsC2M1(3,1), uvsC1M2(3,1), uvsC2M3(3,1);
	uvsC1M1 = ProjC1*RpC1*Translation*Rotation*pos_marker_body1;
	uvsC1M2 = ProjC1*RpC1*Translation*Rotation*pos_marker_body2;
	uvsC1M3 = ProjC1*RpC1*Translation*Rotation*pos_marker_body3;
	uvsC2M1 = ProjC2*RpC2*Translation*Rotation*pos_marker_body1;
	uvsC2M2 = ProjC2*RpC2*Translation*Rotation*pos_marker_body2;
	uvsC2M3 = ProjC2*RpC2*Translation*Rotation*pos_marker_body3;
	
	IntermediateState uvC1M1(2,1), uvC2M2(2,1), uvC1M3(2,1), uvC2M1(2,1), uvC1M2(2,1), uvC2M3(2,1);
	uvC1M1(0,0) = uvsC1M1(0,0)/uvsC1M1(2,0);	uvC1M1(1,0) = uvsC1M1(1,0)/uvsC1M1(2,0);
	uvC1M2(0,0) = uvsC1M2(0,0)/uvsC1M2(2,0);	uvC1M2(1,0) = uvsC1M2(1,0)/uvsC1M2(2,0);
	uvC1M3(0,0) = uvsC1M3(0,0)/uvsC1M3(2,0);	uvC1M3(1,0) = uvsC1M3(1,0)/uvsC1M3(2,0);
	
	uvC2M1(0,0) = uvsC2M1(0,0)/uvsC2M1(2,0);	uvC2M1(1,0) = uvsC2M1(1,0)/uvsC2M1(2,0);
	uvC2M2(0,0) = uvsC2M2(0,0)/uvsC2M2(2,0);	uvC2M2(1,0) = uvsC2M2(1,0)/uvsC2M2(2,0);
	uvC2M3(0,0) = uvsC2M3(0,0)/uvsC2M3(2,0);	uvC2M3(1,0) = uvsC2M3(1,0)/uvsC2M3(2,0);
	
#if (CODEGEN_FOR_OROCOS == 1)
	Matrix RIMU = readFromFile( "../../../../matlab_acado_codegen_simulation/IMU/RIMU.txt" );
#else
	Matrix RIMU = readFromFile( "IMU/RIMU.txt" );
#endif
	
// 	RIMU = eye(3);
	
	IntermediateState aE(3,1), aEend(3,1);
	aE(0,0) = ddxIMU;
	aE(1,0) = ddyIMU;
	aE(2,0) = ddzIMU;
	aEend(0,0) = ddxIMUend;
	aEend(1,0) = ddyIMUend;
	aEend(2,0) = ddzIMUend;
	
	IntermediateState Omega(3,1);
	Omega(0,0) = w1;
	Omega(1,0) = w2;
	Omega(2,0) = w3;
	
	IntermediateState aSHIFT(3,1);
	aSHIFT(0,0) = XIMU3*dw2 - XIMU2*dw3 + w2*(XIMU2*w1 - XIMU1*w2) + w3*(XIMU3*w1 - XIMU1*w3);
    aSHIFT(1,0) = XIMU1*dw3 - XIMU3*dw1 - w1*(XIMU2*w1 - XIMU1*w2) + w3*(XIMU3*w2 - XIMU2*w3);
    aSHIFT(2,0) = XIMU2*dw1 - XIMU1*dw2 - w1*(XIMU3*w1 - XIMU1*w3) - w2*(XIMU3*w2 - XIMU2*w3);
	
	IntermediateState aIMU, aIMUend, wIMU;
// 	aIMU = RIMU*(aE + aSHIFT);
	aIMU = RIMU*aE;
	aIMUend = RIMU*aEend;
	wIMU = RIMU*Omega;
	
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
	CostLSQend << wIMU(0,0);
	CostLSQend << wIMU(1,0);
	CostLSQend << wIMU(2,0);
	CostLSQend << aIMUend(0,0);
	CostLSQend << aIMUend(1,0);
	CostLSQend << aIMUend(2,0);
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

	CostLSQx << uvC1M1(0,0);
	CostLSQx << uvC1M1(1,0);
	CostLSQx << uvC1M2(0,0);
	CostLSQx << uvC1M2(1,0);
	CostLSQx << uvC1M3(0,0);
	CostLSQx << uvC1M3(1,0);
	CostLSQx << uvC2M1(0,0);
	CostLSQx << uvC2M1(1,0);
	CostLSQx << uvC2M2(0,0);
	CostLSQx << uvC2M2(1,0);
	CostLSQx << uvC2M3(0,0);
	CostLSQx << uvC2M3(1,0);
// 	CostLSQx << w1;
// 	CostLSQx << w2;
// 	CostLSQx << w3;
// 	CostLSQx << ddxIMU;
// 	CostLSQx << ddyIMU;
// 	CostLSQx << ddzIMU;
	CostLSQx << wIMU(0,0);
	CostLSQx << wIMU(1,0);
	CostLSQx << wIMU(2,0);
	CostLSQx << aIMU(0,0);
	CostLSQx << aIMU(1,0);
	CostLSQx << aIMU(2,0);
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
// 	Matrix Q = readFromFile( "MHE_Q.txt" );
// 	Matrix Qend = readFromFile( "MHE_Qend.txt" );
// 	Matrix Qx = readFromFile( "MHE_Qx.txt" );
// 	Matrix Qu = readFromFile( "MHE_Qu.txt" );
	
	
	
       cout << "f dim: " << f.getDim() << endl;
	   cout << "CostLSQend dim: " << CostLSQend.getDim() << endl;
       cout << "CostLSQend #u: " << CostLSQend.getNU() << endl;
       cout << "CostLSQ dim: " << CostLSQ.getDim() << endl;
       cout << "CostLSQ  #u: " << CostLSQ.getNU() << endl;
	
	
	// TERMINAL COST FUNCTION
	//---------------
	/*
       VariablesGrid Last;
    Last = readFromFile( "Measurements.txt" );
	Vector x_F(9);
	// Get the last reference
// 	for ( i=0; i<3; i++ ){
// 		x_F(i) = Last(Ncvp,i);
// 	};
	// Get the last reference
	for ( i=0; i<9; i++ ){
		x_F(i) = Last(Ncvp,i+12);
	};
// 	x_F.print();
*/

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
    OCP ocp( 0.0, Tc, Ncvp );
//     ocp.minimizeLSQ( Q,CostLSQ, "Measurements.txt" );
// 	ocp.minimizeLSQEndTerm( Qend,CostLSQend, x_F );
// 	ocp.minimizeMayerTerm( ACost );
// 	ocp.minimizeMayerTerm( SLACK );
	
	ExportVariable QQ, QQend;
	
	ocp.minimizeLSQ(QQ, CostLSQ);
	ocp.minimizeLSQEndTerm(QQend, CostLSQend);
	
	
	
    ocp.subjectTo( f );

	
	// TERMINAL CONSTRAINTS
	// ---------------------------------
	ocp.subjectTo( AT_END, ConstR1 == 0 );
	ocp.subjectTo( AT_END, ConstR2 == 0 );
	ocp.subjectTo( AT_END, ConstR3 == 0 );
	ocp.subjectTo( AT_END, ConstR4 == 0 );
	ocp.subjectTo( AT_END, ConstR5 == 0 );
	ocp.subjectTo( AT_END, ConstR6 == 0 );
	
	ocp.subjectTo( AT_END,  Const == 0 );
	ocp.subjectTo( AT_END, dConst == 0 );

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
	//double AccRate = 30*PI/180;
	//ocp.subjectTo( -AccRate <= dddelta <= AccRate );
	//ocp.subjectTo(  -1 <= up  <= 1 );
	//ocp.subjectTo(  -3.2767 <= ur  <= 3.2767 );
	//ocp.subjectTo( -10 <= dup <= 10 );
	//ocp.subjectTo( -10 <= dur <= 10 );
	
	// STATE
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
	
	
	printf("EXPORTING LINUX/QPOASES CODE\n");
	MHEexport mhe( ocp );

	mhe.set( HESSIAN_APPROXIMATION, GAUSS_NEWTON    );
	mhe.set( DISCRETIZATION_TYPE,   MULTIPLE_SHOOTING );
	//	mhe.set( DISCRETIZATION_TYPE,   SINGLE_SHOOTING );
	
	// 	mhe.set( INTEGRATOR_TYPE,             INT_RK4    );
	// 	mhe.set( NUM_INTEGRATOR_STEPS,        100              );
	
	mhe.set( INTEGRATOR_TYPE,             INT_IRK_GL2     );
	mhe.set( NUM_INTEGRATOR_STEPS,        30               );
	
	mhe.set( IMPLICIT_INTEGRATOR_NUM_ITS, 3				);
	mhe.set( IMPLICIT_INTEGRATOR_NUM_ITS_INIT, 0		);
	mhe.set( LINEAR_ALGEBRA_SOLVER,		  HOUSEHOLDER_QR );
	mhe.set( UNROLL_LINEAR_SOLVER,        NO	      );
	mhe.set( IMPLICIT_INTEGRATOR_MODE, IFTR );
	
	mhe.set(SPARSE_QP_SOLUTION, CONDENSING);
	
	mhe.set( QP_SOLVER,             QP_QPOASES      );
	mhe.set( HOTSTART_QP,           YES              );
    mhe.set( GENERATE_TEST_FILE,    YES              );
	// 	mpc.set( GENERATE_MAKE_FILE,    YES              );
	
	mhe.set( CG_USE_C99,    YES              );
	
	mhe.set(PRINTLEVEL, HIGH);

#if ( CODEGEN_FOR_OROCOS == 1 )

	mhe.set( CG_USE_VARIABLE_WEIGHTING_MATRIX, YES);

#endif

	printf("Stigao do ovde\n");
	
	mhe.exportCode( "code_export_mhe" );
	mhe.printDimensionsQP();

    return 0;
}



