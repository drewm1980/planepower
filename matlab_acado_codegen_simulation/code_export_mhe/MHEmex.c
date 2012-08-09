


#include "mex.h"

#include "acado.h"

/* #include "auxiliary_functions.c" */

real_t getTime(  ){
real_t current_time = 0.000000e+00;
struct timeval theclock;
gettimeofday( &theclock,0 );
current_time = 1.0*theclock.tv_sec + 1.0e-6*theclock.tv_usec;
return current_time;
}


/* SOME CONVENIENT DEFINTIONS: */
/* --------------------------------------------------------------- */
   #define NX          ACADO_NX     /* number of differential states  */
   #define NU          ACADO_NU     /* number of control inputs       */
   #define NY          ACADO_NY     /* number of measurements         */
   #define NYN         ACADO_NYN    /* number of measurements, last node         */
   #define N           ACADO_N      /* number of estimation intervals    */
   #define NUM_STEPS   1            /* number of real time iterations */
   #define VERBOSE     1            /* show iterations: 1, silent: 0  */
/* --------------------------------------------------------------- */


/* GLOBAL VARIABLES FOR THE ACADO REAL-TIME ALGORITHM: */
/* --------------------------------------------------- */
   ACADOvariables acadoVariables;
   ACADOworkspace acadoWorkspace;

/* GLOBAL VARIABLES FOR THE QP SOLVER: */
/* ----------------------------------- */
   Vars         vars;
   Params       params;


/* A TEMPLATE FOR TESTING THE REAL-TIME IMPLEMENTATION: */
/* ---------------------------------------------------- */
void mexFunction( int            nlhs, 
                  mxArray        *plhs[], 
                  int            nrhs, 
                  const mxArray  *prhs[]){

    
    /* Input */
    int nRxInit, nCxInit; 
    double *dPxInit; 
    int nRuInit, nCuInit; 
    double *dPuInit; 
    int nRin_Y, nCin_Y; 
    double *dPin_Y; 
    int nRin_YN, nCin_YN; 
    double *dPin_YN; 
    int nR_S, nC_S; 
    double *dP_S; 
    int nR_SN, nC_SN; 
    double *dP_SN;	
    /* Output */
    double *dPoutU; /*data pointer for the output argument*/
    double *dPoutX; 
    
    /* Initial guess */
    nRxInit    = mxGetM(prhs[0]);
    nCxInit    = mxGetN(prhs[0]);
    dPxInit    = mxGetPr(prhs[0]);
    nRuInit    = mxGetM(prhs[1]);
    nCuInit    = mxGetN(prhs[1]);
    dPuInit    = mxGetPr(prhs[1]);
    /* Measurements */
    nRin_Y  = mxGetM(prhs[2]);
    nCin_Y  = mxGetN(prhs[2]);
    dPin_Y  = mxGetPr(prhs[2]);
    nRin_YN = mxGetM(prhs[3]);
    nCin_YN = mxGetN(prhs[3]);
    dPin_YN = mxGetPr(prhs[3]);
    /* Weighting matrices */
    nR_S       = mxGetM(prhs[4]);
    nC_S       = mxGetN(prhs[4]);
    dP_S       = mxGetPr(prhs[4]);
    nR_SN      = mxGetM(prhs[5]);
    nC_SN      = mxGetN(prhs[5]);
    dP_SN      = mxGetPr(prhs[5]);
    
    
    /*checking input dimensions*/
    if ( ( nRxInit != N+1 ) || ( nCxInit != NX ) ){
        mexPrintf( "First argument must be a vector of size: %d by %d \n", N+1, NX );
        mexErrMsgTxt( "Check the dimensions of the first input array (initial guess for the states)" );
    }
    if ( ( nRuInit != N ) || ( nCuInit != NU ) ){
        mexPrintf( "Second argument must be a vector of size: %d by %d \n", N, NU );
        mexErrMsgTxt( "Check the dimensions of the second input array (initial guess for the controls)" );
    }
    if ( ( nRin_Y != N ) || ( nCin_Y != NY ) ){
        mexPrintf( "Third argument must be a vector of size: %d by %d \n", N, NY );
        mexErrMsgTxt( "Check the dimensions of the third input array (reference for the states)" );
    }
    if ( ( nRin_YN != 1 ) || ( nCin_YN != NYN ) ){
        mexPrintf( "Fourth argument must be a vector of size: %d by %d \n", 1, NYN );
        mexErrMsgTxt( "Check the dimensions of the fourth input array (reference for the controls)" );
    }
    if ( ( nR_S != NY ) || ( nC_S != NY ) ){
        mexPrintf( "Fifth argument must be a vector of size: %d by %d \n", NY, NY );
        mexErrMsgTxt( "Check the dimensions of the fifth input array (weighting matrix)" );
    }
    if ( ( nR_SN != NYN ) || ( nC_SN != NYN ) ){
        mexPrintf( "Sixth argument must be a vector of size: %d by %d \n", NYN, NYN );
        mexErrMsgTxt( "Check the dimensions of the sixth input array (end weighting matrix)" );
    }
    
    
    
    
   /* INTRODUCE AUXILIARY VAIRABLES: */
   /* ------------------------------ */
      int    i, j, iter        ;
      real_t lastMeasurement[NYN];

   /* INITIALIZE THE STATES AND CONTROLS: */
   /* ---------------------------------------- */
    for( j = 0; j < N+1; ++j ) {
        for( i = 0; i < NX  ; ++i ) {
            acadoVariables.x[j*NX + i] = dPxInit[j + i*(N+1)];
            /*mexPrintf( "%d %d %e \n", j, i, acadoVariables.yMeas[j*NY + i] - acadoVariables.x[j*NX + i]);*/
        }
    }
    for( j = 0; j < N; ++j ) {
        for( i = 0; i < NU; ++i ) {
            acadoVariables.u[j*NU + i] = dPuInit[j + i*N];
        }
    }

   /* INITIALIZE THE MEASUREMENTS: */
   /* -------------------------------------------- */
    for( j = 0; j < N; ++j ) {
        for( i = 0; i < NY  ; ++i ) {
            acadoVariables.yMeas[j*NY + i] = dPin_Y[j + i*N];
//             mexPrintf( "%d %e \n", j*NY + i, acadoVariables.yMeas[j*NY + i]);
//             mexPrintf( "%e \t", acadoVariables.yMeas[j*NY + i]);
        }
//         mexPrintf( "\n" );
    }
	for( i = 0; i < NYN; ++i ) {
		lastMeasurement[i] = dPin_YN[i];
//         mexPrintf( "%d %e \n", i, lastMeasurement[i] );
	}
    

   /* SETUP THE WEIGHTING MATRICES: */
   /* ------------------------------------------------ */
    for( j = 0; j < NY; ++j ) {
        for( i = 0; i < NY; ++i ) {
            acadoVariables.S[j*NY + i] = dP_S[j + i*NY];
            /*mexPrintf( "%d %e \n", j*NY + i, acadoVariables.S[j*NY + i] );*/
//             mexPrintf( "%d %d %e \n", j, i, acadoVariables.S[j*NY + i] );
        }
    }
    for( j = 0; j < NYN; ++j ) {
        for( i = 0; i < NYN; ++i ) {
            acadoVariables.SN[j*NYN + i] = dP_SN[j + i*NYN];
//             mexPrintf( "%d %d %e \n", j, i, acadoVariables.SN[j*NYN + i] );
        }
    }
      
     
//      printStates();
//      printControls();
      
// int run01,run02;
// printf( "acadoVariables.SN = \n" );
// for( run01=0; run01<NYN; ++run01 ){
//   for( run02=0; run02<NYN; ++run02 )
//     printf( "%e \t", acadoVariables.SN[run02*NYN+run01] );
//   printf( "\n" );
// }
     
	int status;


      real_t t0 = getTime();
   /* PREPARE FIRST STEP: */
   /* ------------------- */
      preparationStep();


   /* GET THE TIME BEFORE START THE LOOP: */
   /* ---------------------------------------------- */
      real_t t1 = getTime();
      real_t t2;


   /* THE REAL-TIME ITERATION LOOP: */
   /* ---------------------------------------------- */
      for( iter = 0; iter < 1; ++iter ){

        /* TAKE A MEASUREMENT: */
        /* ----------------------------- */
           /* meausrement = ... */

        /* PERFORM THE FEEDBACK STEP: */
        /* ----------------------------- */
           status = feedbackStep( lastMeasurement );
           
		if (  status )
			mexPrintf("WARNING: QP solver returned the error code: %d \n", status);
           
           t2 = getTime();

        /* APPLY THE NEW CONTROL IMMEDIATELY TO THE PROCESS: */
        /* ------------------------------------------------- */
           /* send first piece of acadoVariables.u to process;*/
           mexPrintf("=================================================================\n\n" );
           mexPrintf("   MHE:  Real-Time Iteration %d:  KKT Tolerance = %.3e\n", iter, getKKT() );
           /*mexPrintf("\n=================================================================\n" );*/

        /* OPTIONAL: SHIFT THE INITIALIZATION: */
        /* ----------------------------------- */
           /* shiftControls( acadoVariables.uRef );*/
           /* shiftStates  ( acadoVariables.xRef );*/

        /* PREPARE NEXT STEP: */
        /* ------------------ */
//            preparationStep();
      }
      /*mexPrintf("\n\n              END OF THE REAL-TIME LOOP. \n\n\n");*/


   /* GET THE TIME AT THE END OF THE LOOP: */
   /* ---------------------------------------------- */
      real_t t3 = getTime();

    /*mexPrintf("=================================================================\n\n" ); */
    mexPrintf("      Preparation step: %.3e s\n", t1-t0 );
    mexPrintf("      Feedback step:    %.3e s\n", t2-t1 );
    mexPrintf("\n=================================================================\n" );

//      printStates(); 
//      printControls();
      
   /* ASSIGN THE STATES AND CONTROLS TO THE OUTPUTS: */
   /* ---------------------------------------- */
    plhs[0] = mxCreateDoubleMatrix(NU, N, mxREAL);
    dPoutU = mxGetPr(plhs[0]);
    plhs[1] = mxCreateDoubleMatrix(NX, N+1, mxREAL);
    dPoutX = mxGetPr(plhs[1]);
    plhs[2] = mxCreateDoubleScalar( getKKT() );
    /* plhs[3] = mxCreateDoubleScalar( t2-t1 ); */

    for( j = 0; j < N+1; ++j ) {
        for( i = 0; i < NX; ++i ) {
            dPoutX[j + i*(N+1)] = acadoVariables.x[j + i*(N+1)];
        }
    }
    /* for ( i=0; i<N*NU; ++i ) mexPrintf( "%d %e \n", i, acadoVariables.u[i] ); */
    for( j = 0; j < N; ++j ) {
        for( i = 0; i < NU; ++i ) {
            dPoutU[j + i*N] = acadoVariables.u[j + i*N];
        }
    }
}
