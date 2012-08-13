

#include "mex.h"

#include "acado.h"

#include "auxiliary_functions.c"


/* SOME CONVENIENT DEFINTIONS: */
/* --------------------------------------------------------------- */
   #define NX          22     /* number of differential states  */
   #define NU          3      /* number of control inputs       */
   #define N           10     /* number of control intervals    */
   #define NUM_STEPS   1      /* number of real time iterations */
   #define VERBOSE     1      /* show iterations: 1, silent: 0  */
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
    int nRx0, nCx0; /*dimensions of first input argument*/
    double *dPx0; /*data pointer for the first input argument*/
    int nRxInit, nCxInit; 
    double *dPxInit; 
    int nRuInit, nCuInit; 
    double *dPuInit; 
    int nRin_xRef, nCin_xRef; 
    double *dPin_xRef; 
    int nRin_uRef, nCin_uRef; 
    double *dPin_uRef; 
    int nR_Q, nC_Q; 
    double *dP_Q; 
    int nR_R, nC_R; 
    double *dP_R;
    int nR_QT, nC_QT; 
    double *dP_QT; 
    int nR_lb, nC_lb; 
    double *dP_lb; 
    int nR_ub, nC_ub; 
    double *dP_ub; 
    /* Output */
    double *dPoutU; /*data pointer for the output argument*/
    double *dPoutX; 
    
    /* Initial State */
    nRx0      = mxGetM(prhs[0]);
    nCx0      = mxGetN(prhs[0]);
    dPx0      = mxGetPr(prhs[0]);
    /* Initial guess */
    nRxInit   = mxGetM(prhs[1]);
    nCxInit   = mxGetN(prhs[1]);
    dPxInit   = mxGetPr(prhs[1]);
    nRuInit   = mxGetM(prhs[2]);
    nCuInit   = mxGetN(prhs[2]);
    dPuInit   = mxGetPr(prhs[2]);
    /* Reference */
    nRin_xRef = mxGetM(prhs[3]);
    nCin_xRef = mxGetN(prhs[3]);
    dPin_xRef = mxGetPr(prhs[3]);
    nRin_uRef = mxGetM(prhs[4]);
    nCin_uRef = mxGetN(prhs[4]);
    dPin_uRef = mxGetPr(prhs[4]);
    /* Weighting matrices */
    nR_Q      = mxGetM(prhs[5]);
    nC_Q      = mxGetN(prhs[5]);
    dP_Q      = mxGetPr(prhs[5]);
    nR_R      = mxGetM(prhs[6]);
    nC_R      = mxGetN(prhs[6]);
    dP_R      = mxGetPr(prhs[6]);
    nR_QT     = mxGetM(prhs[7]);
    nC_QT     = mxGetN(prhs[7]);
    dP_QT     = mxGetPr(prhs[7]);
    
    
    /*checking input dimensions*/
    if ( nRx0*nCx0 != NX ){
        mexPrintf( "First argument must be a matrix of size: %d \n", NX );
        mexErrMsgTxt( "Check the dimensions of the first input array (initial state)" );
    }
    if ( ( nRxInit != N+1 ) || ( nCxInit != NX ) ){
        mexPrintf( "Second argument must be a matrix of size: %d by %d \n", N+1, NX );
        mexErrMsgTxt( "Check the dimensions of the second input array (initial guess for the states)" );
    }
    if ( ( nRuInit != N ) || ( nCuInit != NU ) ){
        mexPrintf( "Third argument must be a matrix of size: %d by %d \n", N, NU );
        mexErrMsgTxt( "Check the dimensions of the third input array (initial guess for the controls)" );
    }
    if ( ( nRin_xRef != N ) || ( nCin_xRef != NX ) ){
        mexPrintf( "Fourth argument must be a matrix of size: %d by %d \n", N, NX );
        mexErrMsgTxt( "Check the dimensions of the fourth input array (reference for the states)" );
    }
    if ( ( nRin_uRef != N ) || ( nCin_uRef != NU ) ){
        mexPrintf( "Fifth argument must be a matrix of size: %d by %d \n", N, NU );
        mexErrMsgTxt( "Check the dimensions of the fifth input array (reference for the controls)" );
    }
    if ( ( nR_Q != NX ) || ( nC_Q != NX ) ){
        mexPrintf( "Sixth argument must be a matrix of size: %d by %d \n", NX, NX );
        mexErrMsgTxt( "Check the dimensions of the sixth input array (state weighting matrix)" );
    }
    if ( ( nR_R != NU ) || ( nC_R != NU ) ){
        mexPrintf( "Seventh argument must be a matrix of size: %d by %d \n", NU, NU );
        mexErrMsgTxt( "Check the dimensions of the seventh input array (control weighting matrix)" );
    }
    if ( ( nR_QT != NX ) || ( nC_QT != NX ) ){
        mexPrintf( "Eighth argument must be a matrix of size: %d by %d \n", NX, NX );
        mexErrMsgTxt( "Check the dimensions of the eighth input array (terminal cost weighting matrix)" );
    }
    
    
    
    
   /* INTRODUCE AUXILIARY VAIRABLES: */
   /* ------------------------------ */
      int    i, j, iter        ;
      real_t measurement[NX];

    /* for( i = 0; i < NX*(N+1)  ; ++i ) mexPrintf( "%d %e \n", i, dPxInit[i] ); */
   /* INITIALIZE THE STATES AND CONTROLS: */
   /* ---------------------------------------- */
    for( j = 0; j < N+1; ++j ) {
        for( i = 0; i < NX  ; ++i ) {
            acadoVariables.x[j*NX + i] = dPxInit[j + i*(N+1)];
            /* mexPrintf( "%d %e \n", j + i*(N+1), dPxInit[j + i*(N+1)] ); */
        }
    }
    for( j = 0; j < N; ++j ) {
        for( i = 0; i < NU; ++i ) {
            acadoVariables.u[j*NU + i] = dPuInit[j + i*N];
        }
    }

   /* INITIALIZE THE STATES AND CONTROL REFERENCE: */
   /* -------------------------------------------- */
    for( j = 0; j < N; ++j ) {
        for( i = 0; i < NX  ; ++i ) {
            acadoVariables.xRef[j*NX + i] = dPin_xRef[j + i*N];
            /* mexPrintf( "%d %e \n", j + i*(N), dPin_xRef[j + i*(N)] ); */
        }
    }
    for( j = 0; j < N; ++j ) {
        for( i = 0; i < NU; ++i ) {
            acadoVariables.uRef[j*NU + i] = dPin_uRef[j + i*N];
        }
    }
      /* for( i = 0; i < NX*N; ++i )  acadoVariables.xRef[i]  =  dPin_xRef[i]; */
      /* for( i = 0; i < NU*N; ++i )  acadoVariables.uRef[i]  =  dPin_uRef[i]; */


   /* SETUP THE FIRST STATE MEASUREMENT: */
   /* ------------------------------------------------ */
    for( i = 0; i < NX; ++i ) {
      measurement[i] = dPx0[i];
          /* mexPrintf( "%e \n", dPx0[i] );*/
    }

   /* SETUP THE WEIGHTING MATRICES: */
   /* ------------------------------------------------ */
    for( j = 0; j < NX; ++j ) {
        for( i = 0; i < NX; ++i ) {
            acadoVariables.QQ[j*NX + i] = dP_Q[j + i*NX];
        }
    }
    for( j = 0; j < NU; ++j ) {
        for( i = 0; i < NU; ++i ) {
            acadoVariables.RR[j*NU + i] = dP_R[j + i*NU];
            /* mexPrintf( "%d %d %e \n", j, i, acadoVariables.RR[j*NU + i] ); */ 
        }
    }
    for( j = 0; j < NX; ++j ) {
        for( i = 0; i < NX; ++i ) {
            acadoVariables.QT[j*NX + i] = dP_QT[j + i*NX];
        }
    }
      
      
      
      /* if( VERBOSE ) printHeader(); */

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
      for( iter = 0; iter < NUM_STEPS; ++iter ){

        /* TAKE A MEASUREMENT: */
        /* ----------------------------- */
           /* meausrement = ... */

        /* PERFORM THE FEEDBACK STEP: */
        /* ----------------------------- */
           status = feedbackStep( measurement );
           
           if (  status )
               mexPrintf("WARNING: QP solver returned the error code: %d \n", status);

           t2 = getTime();
        /* APPLY THE NEW CONTROL IMMEDIATELY TO THE PROCESS: */
        /* ------------------------------------------------- */
           /* send first piece of acadoVariables.u to process; */
           if( VERBOSE ) mexPrintf("=================================================================\n\n" );
           if( VERBOSE ) mexPrintf("   MPC:  Real-Time Iteration %d:  KKT Tolerance = %.3e\n", iter, getKKT() );
           /* if( VERBOSE ) mexPrintf("\n=================================================================\n" );*/

        /* OPTIONAL: SHIFT THE INITIALIZATION: */
        /* ----------------------------------- */
           /* shiftControls( acadoVariables.uRef ); */
           /* shiftStates  ( acadoVariables.xRef ); */

        /* PREPARE NEXT STEP: */
        /* ------------------ */
           /* preparationStep(); */
      }
      /* if( VERBOSE ) mexPrintf("\n\n              END OF THE REAL-TIME LOOP. \n\n\n"); */


   /* GET THE TIME AT THE END OF THE LOOP: */
   /* ---------------------------------------------- */
      real_t t3 = getTime();

    /* if( VERBOSE ) mexPrintf("=================================================================\n\n" ); */
    if( VERBOSE ) mexPrintf("      Preparation step: %.3e s\n", t1-t0 );
    if( VERBOSE ) mexPrintf("      Feedback step:    %.3e s\n", t2-t1 );
    if( VERBOSE ) mexPrintf("\n=================================================================\n" );


   /* PRINT DURATION AND RESULTS: */
   /* -------------------------------------------------------------------------------------------------- */
      if( !VERBOSE )
      mexPrintf("\n\n AVERAGE DURATION OF ONE REAL-TIME ITERATION:   %.3g μs\n\n", 1e6*(t2-t1)/NUM_STEPS );

     /* printStates(); */
     /* printControls(); */
      
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
    /* for ( i=0; i<N*NU; ++i ) mexPrintf( "%d %e \n", i, dPoutU[i] ); */
    /* mexPrintf("%e \n", getKKT()); */
}
