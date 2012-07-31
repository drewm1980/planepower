 /**
 *    SINGLE POWER KITE 
 *    CARTESIAN COORDINATES (ODE FORMULATION)
 *    JULY 2012 SEBASTIEN GROS, MARIO ZANON HIGHWIND, OPTEC
 */

#include <acado_toolkit.hpp>

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

	double PI = 3.1415926535897932;
	
#include "model_constants.hpp"
#include "state_definition.hpp"
#include "control_definition.hpp"
#include "model_definition.hpp"

    // DEFINE AN OPTIMAL CONTROL PROBLEM:
    // ----------------------------------
    OCP ocp( 0.0, Tc, Ncvp );
    
    ExportVariable QQ( "QQ",n_XD ,n_XD );
    ExportVariable RR( "RR",n_U ,n_U ); 
    ExportVariable QT( "QT",n_XD ,n_XD );
    
    ocp.minimizeLSQ( QQ, RR );
    ocp.minimizeLSQEndTerm( QT );
	
    ocp.subjectTo( f );
	
	// BOUNDS:
	// ---------------------------------
	
	// CONTROL
	double AccRate = 30*PI/180;
	ocp.subjectTo( -AccRate <= dddelta <= AccRate );
    ocp.subjectTo( dddelta == 0 );
	ocp.subjectTo(  -1 <= up  <= 1 );
	ocp.subjectTo(  -3.2767 <= ur  <= 3.2767 );
	ocp.subjectTo( -10 <= dup <= 10 );
	ocp.subjectTo( -10 <= dur <= 10 );
	
	// STATE
	ocp.subjectTo( -1 <= e11 <= 1 );
	ocp.subjectTo( -1 <= e12 <= 1 );
	ocp.subjectTo( -1 <= e13 <= 1 );
	ocp.subjectTo( -1 <= e21 <= 1 );
	ocp.subjectTo( -1 <= e22 <= 1 );
	ocp.subjectTo( -1 <= e23 <= 1 );
	ocp.subjectTo( -1 <= e31 <= 1 );
	ocp.subjectTo( -1 <= e32 <= 1 );
	ocp.subjectTo( -1 <= e33 <= 1 );
	
	// Export Code 
	// ---------------------------------
	
	printf("EXPORTING LINUX/QPOASES CODE\n");
	MPCexport mpc( ocp );

	mpc.set( HESSIAN_APPROXIMATION,       GAUSS_NEWTON    );
	mpc.set( DISCRETIZATION_TYPE,   MULTIPLE_SHOOTING );
	mpc.set( QP_SOLVER,             QP_QPOASES      );
	mpc.set( HOTSTART_QP,           YES              );
    
	mpc.set( INTEGRATOR_TYPE,             INT_IRK_GL2    );
	mpc.set( NUM_INTEGRATOR_STEPS,        30              );
	
	mpc.set( IMPLICIT_INTEGRATOR_NUM_ITS, 3				);
	mpc.set( IMPLICIT_INTEGRATOR_NUM_ITS_INIT, 0		);
	mpc.set( LINEAR_ALGEBRA_SOLVER,		  HOUSEHOLDER_QR );
	mpc.set( UNROLL_LINEAR_SOLVER,        NO	      );
	mpc.set( IMPLICIT_INTEGRATOR_MODE, IFTR );
	
	mpc.set(SPARSE_QP_SOLUTION, CONDENSING);
    
	mpc.set( GENERATE_TEST_FILE,          YES            );
	mpc.set( GENERATE_SIMULINK_INTERFACE, YES );
	mpc.set( GENERATE_MAKE_FILE,          NO            );

	mpc.exportCode( "code_export_MPC" );
	mpc.printDimensionsQP();

    return 0;
}



