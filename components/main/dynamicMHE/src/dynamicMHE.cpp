#include <string.h>

#include "dynamicMHE.hpp"

//
// Global variables for the real-time algorithm
//
ACADOvariables acadoVariables;
ACADOworkspace acadoWorkspace;

//
// Global variables for the QP solver
//
Vars vars;
Params params;

//
// Quick hack to get # of active set changes
//
extern int logNWSR;

//
// Class methods
//

DynamicMHE::DynamicMHE(const std::string& name)
	:
		TaskContext(name, PreOperational)
//		logger(dynamic_cast<OCL::logging::Category*>(&log4cpp::Category::getInstance("dynamicMHE")))
{
	initialized = false;
	
	sampleCounter = 0;
	
	//
	// Set the operations
	//
	this->provides()->addOperation( "mhePreparationPhase", &DynamicMHE::mhePreparationPhase, this, OwnThread )
			.doc( "MHE RTI preparation step." );
		
	this->provides()->addOperation( "mheFeedbackPhase", &DynamicMHE::mheFeedbackPhase, this, OwnThread )
			.doc( "MHE RTI feedback step." );

	//
	// TODO Set the input ports
	//
	this->addPort("portMeasurementsMarkers", portMeasurementsMarkers)
			.doc("Port that contains measurements from the 'markers' component: uv1 .. uv12.");

	this->addPort("portMeasurementsPose", portMeasurementsPose)
				.doc("Port that contains measurements from the 'pose' component: x, y, z, e11, ..., e33.");

	this->addPort("portMeasurementsIMU", portMeasurementsIMU)
			.doc("Port that contains measurements from the 'IMU' component: wx, wy, wz, ax, ay, az");

	this->addEventPort("portMeasurementsEncoder", portMeasurementsEncoder)
		.doc("Port that contains measurements from encoder: delta, ddelta.");

	this->addPort("portMeasurementsCtrl", portMeasurementsCtrl)
			.doc("Port that contains measurements from controller: ur1, ur2, up");
	
	this->addPort("portMeasurementsCtrlRates", portMeasurementsCtrlRates)
			.doc("Port that contains control rate measurements from controller: dur1, dur2, dup");

	//
	// Set the output ports
	//
	this->addPort("portReady", portReady)
			.doc("Indicates whether the estimator is ready. "
					"User component should always read this first before reading the state estimate");
	
	this->addPort("portStateEstimate", portStateEstimate)
			.doc("State estimate.");
	
	this->addPort("portControlEstimate", portControlEstimate)
			.doc("Control estimate.");
	
	this->addPort("portKKTTolerance", portKKTTolerance)
			.doc("KKT tolerance.");

	this->addPort("portObjectivevalue", portObjectiveValue)
				.doc("Objective value.");

	this->addPort("portNumOfActiveSetChanges", portNumOfActiveSetChanges)
			.doc("Number of active set changes of the QP solver.");

	this->addPort("portPreparationPhaseExecTime", portPreparationPhaseExecTime)
			.doc("Preparation phase execution time.");
	this->addPort("portFeedbackPhaseExecTime", portFeedbackPhaseExecTime)
			.doc("Feedback phase execution time.");
	this->addPort("portExecutionTime", portExecutionTime)
				.doc("Execution time of the MHE.");

	this->addPort("portQPSolverStatus", portQPSolverStatus)
			.doc("Status of the QP solver; 0 - good, otherwise scratch your head.");

	this->addPort("portDataSizeValid", portDataSizeValid);

	this->addPort("portOneStepPrediction", portOneStepPrediction)
			.doc("One step prediction of the MHE, after the shifting");

	this->addPort("portStateAndControl", portStateAndControl)
			.doc("Port with states and controls, measurements, ... ");

	this->addPort("portStateReference",portStateReference)
			.doc("Port with reference of controller");

	this->addPort("portFullStateVector", portFullStateVector)
			.doc("Full state vector in the MHE.");
	
	this->addPort("portFullControlVector", portFullControlVector);
			
	//
	// Initialize and output the relevant output ports
	//	
	stateEstimate.resize(NX, 0.0);
	portStateEstimate.setDataSample( stateEstimate );
	portStateEstimate.write( stateEstimate );
	
	controlEstimate.resize(NU, 0.0);
	portControlEstimate.setDataSample( controlEstimate );
	portControlEstimate.write( controlEstimate );
	
	kktTolerance = 0.0;
	portKKTTolerance.write( kktTolerance );
	
	objectiveValue = 0.0;
	portObjectiveValue.write( objectiveValue );

	numOfActiveSetChanges = 0;
	portNumOfActiveSetChanges.write( numOfActiveSetChanges );
	
	ready = false;
	portReady.write( ready );

	qpSolverStatus = 0;
	portQPSolverStatus.write( qpSolverStatus );

	dataSizeValid = false;
	portDataSizeValid.write( dataSizeValid );

	oneStepPrediction.resize(NX, 0.0);
	portOneStepPrediction.setDataSample( oneStepPrediction );
	portOneStepPrediction.write( oneStepPrediction );

	StateAndControl.resize(5*NX+2*NU+NY_MARK+NY_IMU+NY_POSE+NY_ENC+3*(N + 1) * NX,0.0);
	portStateAndControl.setDataSample( StateAndControl );
	portStateAndControl.write( StateAndControl );

	statePredicted.resize(NX,0.0);
	StateReference.resize(NX,0.0);

	fullStateVector.resize((N + 1) * NX, 0.0);
	portFullStateVector.setDataSample( fullStateVector );
	portFullStateVector.write( fullStateVector );
	
	fullControlVector.resize(N * NU, 0.0);
	portFullControlVector.setDataSample( fullControlVector );
	portFullControlVector.write( fullControlVector );

	//
	// Size of input ports
	//
	measurementsCtrlRates.resize(NY_CTRL, 0.0);

	//
	// Properties
	//
	numSQPIterations = 1;
	this->addProperty("propNumSQPIterations", numSQPIterations)
			.doc("Number of SQP iterations. Default = 1, Max = 10.");

	this->addProperty("propSigma_delta", sigma_delta)
			.doc("The standard deviation of the measurement of delta. Default = 1e-2");
	sigma_delta = 1e-2;

	this->addProperty("propSigma_ur", sigma_ur)
			.doc("The standard deviation of the measurement of ur. Default = 1e-2");
	sigma_ur = 1e-2;

	this->addProperty("propSigma_up", sigma_up)
			.doc("The standard deviation of the measurement of up. Default = 1e-2");
	sigma_up = 1e-2;

	this->addProperty("propSigma_dddelta", sigma_dddelta)
			.doc("The standard deviation of the measurement of dddelta. Default = 0.03162");
	sigma_dddelta = 0.03162;

	this->addProperty("propSigma_dur", sigma_dur)
			.doc("The standard deviation of the measurement of dur. Default = 0.03162");
	sigma_dur = 0.03162;

	this->addProperty("propSigma_dup", sigma_dup)
			.doc("The standard deviation of the measurement of dup. Default = 0.03162");
	sigma_dup = 0.03162;

	this->addProperty("propScale_Obj", SCALE_OBJ)
			.doc("Scaling of the objective. Default = 0.01");
	SCALE_OBJ = 0.01;

	// ACADO properties
	this->addProperty("mk", acadoVariables.p[ 0 ]);
	this->addProperty("g", acadoVariables.p[ 1 ]);
	this->addProperty("rho", acadoVariables.p[ 2 ]);
	this->addProperty("rhoc", acadoVariables.p[ 3 ]);
	this->addProperty("dc", acadoVariables.p[ 4 ]);
	this->addProperty("AQ", acadoVariables.p[ 5 ]);
	this->addProperty("rA", acadoVariables.p[ 6 ]);
	this->addProperty("zT", acadoVariables.p[ 7 ]);
	this->addProperty("I1", acadoVariables.p[ 8 ]);
	this->addProperty("I31", acadoVariables.p[ 9 ]);
	this->addProperty("I2", acadoVariables.p[ 10 ]);
	this->addProperty("I3", acadoVariables.p[ 11 ]);
	this->addProperty("LT", acadoVariables.p[ 12 ]);
	this->addProperty("RD", acadoVariables.p[ 13 ]);
	this->addProperty("PD", acadoVariables.p[ 14 ]);
	this->addProperty("YD", acadoVariables.p[ 15 ]);
	this->addProperty("CLA", acadoVariables.p[ 16 ]);
	this->addProperty("CLe", acadoVariables.p[ 17 ]);
	this->addProperty("CL0", acadoVariables.p[ 18 ]);
	this->addProperty("CDA", acadoVariables.p[ 19 ]);
	this->addProperty("CDA2", acadoVariables.p[ 20 ]);
	this->addProperty("CDB2", acadoVariables.p[ 21 ]);
	this->addProperty("CD0", acadoVariables.p[ 22 ]);
	this->addProperty("CRB", acadoVariables.p[ 23 ]);
	this->addProperty("CRAB", acadoVariables.p[ 24 ]);
	this->addProperty("CRr", acadoVariables.p[ 25 ]);
	this->addProperty("CPA", acadoVariables.p[ 26 ]);
	this->addProperty("CPe", acadoVariables.p[ 27 ]);
	this->addProperty("CP0", acadoVariables.p[ 28 ]);
	this->addProperty("CYB", acadoVariables.p[ 29 ]);
	this->addProperty("CYAB", acadoVariables.p[ 30 ]);
	this->addProperty("SPAN", acadoVariables.p[ 31 ]);
	this->addProperty("CHORD", acadoVariables.p[ 32 ]);
	this->addProperty("CL_scaling", acadoVariables.p[ 33 ]);
	this->addProperty("CD_scaling", acadoVariables.p[ 34 ]);

	//
	// Misc
	//

	oldMeasurementsCtrl.resize(NY_CTRL, 0.0);

	//
	// Deep debug
	//
#if DEEP_DEBUG

	this->addPort("portWeightingCoefficients", portWeightingCoefficients);

	weightingCoefficients.resize(N * NY + NYN, 0.0);
	portWeightingCoefficients.setDataSample( weightingCoefficients );
	portWeightingCoefficients.write( weightingCoefficients );

#endif

	//
	// Reset the MHE structures
	// TODO This is not the smartest place for the parameter...
	//
	memset(&acadoWorkspace, 0, sizeof( acadoWorkspace ));
	memset(&acadoVariables, 0, sizeof( acadoVariables ));

}

bool DynamicMHE::configureHook( )
{	
	if (	portMeasurementsPose.connected() == false ||
			portMeasurementsMarkers.connected() == false ||
			portMeasurementsIMU.connected() == false ||
			portMeasurementsEncoder.connected() == false ||
			portMeasurementsCtrl.connected() == false )
	{
		log( Error ) << "At least one of the input ports is not connected" << endlog();

		return false;
	}
	
// 	for (unsigned i = 0; i < NP; ++i)
//         cout << acadoVariables.p[ i ] << endl;

	return true;
}

bool DynamicMHE::startHook( )
{
	unsigned i;
	
	//
	// Misc.
	//
	memset(&acadoWorkspace, 0, sizeof( acadoWorkspace ));
	memset(&acadoVariables.x, 0, sizeof( acadoVariables.x ));
	memset(&acadoVariables.u, 0, sizeof( acadoVariables.u ));

	kktTolerance = 0.0;
	numOfActiveSetChanges = 0;
	
	sampleCounter = 0;
	
	ready = false;
	initialized = false;

	//
	// Initialize the weighting coeff. for delta, ur, up
	//
	for (i = 0; i < N; ++i)
	{
		// delta
		acadoVariables.S[i * NY * NY + 18 * NY + 18] = SCALE_OBJ * 1.0/sigma_delta/sigma_delta; // delta

		// ur, up
		acadoVariables.S[i * NY * NY + 19 * NY + 19] = SCALE_OBJ * 1.0/sigma_ur/sigma_ur; // ur
		acadoVariables.S[i * NY * NY + 20 * NY + 20] = SCALE_OBJ * 1.0/sigma_up/sigma_up; // up

		// controls
		acadoVariables.S[i * NY * NY + 21 * NY + 21] = SCALE_OBJ * 1.0/sigma_dddelta/sigma_dddelta; // dddelta
		acadoVariables.S[i * NY * NY + 22 * NY + 22] = SCALE_OBJ * 1.0/sigma_dur/sigma_dur; // dur
		acadoVariables.S[i * NY * NY + 23 * NY + 23] = SCALE_OBJ * 1.0/sigma_dup/sigma_dup; // dup
	}

	acadoVariables.SN[6 * NYN + 6] = SCALE_OBJ * 1.0/sigma_delta/sigma_delta; // delta
	acadoVariables.SN[7 * NYN + 7] = SCALE_OBJ * 1.0/sigma_ur/sigma_ur; // ur
	acadoVariables.SN[8 * NYN + 8] = SCALE_OBJ * 1.0/sigma_up/sigma_up; // up

	return true;
}

void DynamicMHE::updateHook( )
{
	tickMHEBegin = TimeService::Instance()->getTicks();

	for (sqpIterationsCounter = 0; sqpIterationsCounter < numSQPIterations; sqpIterationsCounter++)
	{
		mheFeedbackPhase();
		mhePreparationPhase();
	}

	timeMHE = TimeService::Instance()->secondsSince( tickMHEBegin );
	portExecutionTime.write( timeMHE );
}

void DynamicMHE::stopHook( )
{
}

void DynamicMHE::cleanupHook( )
{
}

void DynamicMHE::errorHook( )
{
	// TODO Implement some wisdom...
}

void DynamicMHE::mhePreparationPhase( )
{
	unsigned i, j;
	
	unsigned indexU = (1 + NX + NU) * NX;
	unsigned indexP = (1 + NX + NU) * NX + NU;
	
	tickPreparationPhaseBegin = TimeService::Instance()->getTicks();

	if (dataSizeValid == false)
	{
		return;
	}

	if (initialized == true && sqpIterationsCounter == (numSQPIterations - 1))
	{
		//
		// Shift the shooting nodes and integrate from the last one
		//

		// Fill the states
		for (i = 0; i < NX; ++i)
			acadoWorkspace.state[ i ] = acadoVariables.x[N * NX + i];

		// Read MHE controls (control rates in reality)
		statusMeasurementsCtrlRates = portMeasurementsCtrlRates.read( measurementsCtrlRates );

		// Fill the controls
		if (statusMeasurementsCtrlRates == NewData && measurementsCtrlRates.size() == NU)
		{
			// 0: dddelta, always zero
			// 1: dur, used
			// 2: dup, used
			// TODO This should not be hardcoded, should be programmed in more elegnat way

			if(isfinite(measurementsCtrlRates[0]) && isfinite(measurementsCtrlRates[2])){
				acadoWorkspace.state[ indexU + 1 ] = measurementsCtrlRates[ 0 ] / SCALE_UR;
				acadoWorkspace.state[ indexU + 2 ] = measurementsCtrlRates[ 2 ] / SCALE_UP;
			}
			else{
				acadoWorkspace.state[ indexU + 1 ] = 0.0; 
				acadoWorkspace.state[ indexU + 2 ] = 0.0;
			}
		}
		else
		{
			for (i = 0; i < NU; ++i)
				acadoWorkspace.state[ indexU + i ] = acadoVariables.u[(N - 1) * NU + i];
		}

		// Fill the parameters
		for (i = 0; i < NP; ++i)
			acadoWorkspace.state[ indexP + i ] = acadoVariables.p[ i ];

		// Integrate
		integrate( acadoWorkspace.state );

		for(i=0; i< NX; i++){
			statePredicted[i] = acadoWorkspace.state[i];
		}

		// Shift the states and controls
		shiftStates( acadoWorkspace.state );

		if (statusMeasurementsCtrlRates == NewData && measurementsCtrlRates.size() == NU)
		{
			shiftControls( acadoWorkspace.state + indexU );
		}
		else
		{
			shiftControls( 0 );
		}

		// Write the one step prediction to the port.
		copy(acadoWorkspace.state, acadoWorkspace.state + NX, oneStepPrediction.begin());
		portOneStepPrediction.write( oneStepPrediction );

		//
		// Shift the weighting matrices, OK
		//

		// Nodes i: 0, ..., N - 1
		for (i = 0; i < N - 1; ++i)
			for(j = 0; j < NY * NY; ++j)
				acadoVariables.S[i * NY * NY + j] = acadoVariables.S[(i + 1) * NY * NY + j];

		// Move SN matrix to the last block of the S matrix
		for (i = 0; i < NYN; ++i)
			for(j = 0; j < NYN; ++j)
				acadoVariables.S[(N - 1) * NY * NY + (NY_POSE + i) * NY + NY_POSE + j] =
						acadoVariables.SN[i * NYN + j];

		//
		// Shift the measurements
		//

		for (i = 0; i < N - 1; ++i)
			for (j = 0; j < NY; ++j)
				acadoVariables.yMeas[i * NY + j] = acadoVariables.yMeas[(i + 1) * NY + j];

		// Set the last "vector" of measurements
		for (j = 0; j < NYN; ++j)
			acadoVariables.yMeas[(N - 1) * NY + NY_POSE + j] = lastMeasurementForMHE[ j ];
	}
	else if (sqpIterationsCounter == (numSQPIterations - 1))
	{
		//
		// Just before enabling the estimator, initialize the nodes
		// NOTE: it is assumed the estimator works in MS mode
		//

		if (sampleCounter == N)
		{
			//
			// Initialize the 1. node (node #0)
			//

			// x, y, z
			acadoVariables.x[ 0 ] = initPose[ 0 ];
			acadoVariables.x[ 1 ] = initPose[ 1 ];
			acadoVariables.x[ 2 ] = initPose[ 2 ];

			// dx, dy, dz, do not touch them, they are zero anyway

			// e11, ..., e33
			acadoVariables.x[ 6 ] = initPose[ 3 ];
			acadoVariables.x[ 7 ] = initPose[ 4 ];
			acadoVariables.x[ 8 ] = initPose[ 5 ];
			acadoVariables.x[ 9 ] = initPose[ 6 ];
			acadoVariables.x[ 10 ] = initPose[ 7 ];
			acadoVariables.x[ 11 ] = initPose[ 8 ];
			acadoVariables.x[ 12 ] = initPose[ 9 ];
			acadoVariables.x[ 13 ] = initPose[ 10 ];
			acadoVariables.x[ 14 ] = initPose[ 11 ];

			// delta, ddelta
			acadoVariables.x[ 18 ] = acadoVariables.yMeas[ 18 ];
			acadoVariables.x[ 19 ] = initDdelta;

			// V1 w1, w2, w3
//			acadoVariables.x[ 15 ] = acadoVariables.yMeas[ 12 ];
//			acadoVariables.x[ 16 ] = acadoVariables.yMeas[ 13 ];
//			acadoVariables.x[ 17 ] = acadoVariables.yMeas[ 14 ];

			// V2, w1, w2, w3
			acadoVariables.x[ 15 ] = initPose[ 9 ] * initDdelta;
			acadoVariables.x[ 16 ] = initPose[ 10 ] * initDdelta;
			acadoVariables.x[ 17 ] = initPose[ 11 ] * initDdelta;
			// ur, up
//			acadoVariables.x[ 20 ] = acadoVariables.yMeas[ 19 ];
//			acadoVariables.x[ 21 ] = acadoVariables.yMeas[ 20 ];

			acadoVariables.x[ 20 ] = 0.0;
			acadoVariables.x[ 21 ] = 0.0;

#if INIT_VER == 1

			//
			// Initialize all other nodes by forward simulation
			// NOTE: This is tested with implicit integrator
			//
			for (i = 0; i < N; ++i)
			{
				// set the states
				for (j = 0; j < NX; ++j)
					acadoWorkspace.state[ j ] = acadoVariables.x[i * NX + j];

				// We can assume they are zero for starters
				for (j = 0; j < NU; ++j)
					acadoWorkspace.state[ indexU + j ] = acadoVariables.u[i * NU + j];

				for (j = 0; j < NP; ++j)
					acadoWorkspace.state[ indexP + j ] = acadoVariables.p[ j ];

				// set the parameters

				integrate( acadoWorkspace.state );

				// Write the new states
				for (j = 0; j < NX; ++j)
					acadoVariables.x[(i + 1) * NX + j] = acadoWorkspace.state[ j ];
			}

#elif INIT_VER == 2

			//
			// Put the same data on all nodes
			//
			for (i = 0; i < N; ++i)
				for (j = 0; j < NX; ++j)
					acadoVariables.x[(i + 1) * NX + j] = acadoVariables.x[ j ];

#else
	#error "Option is not supported."
#endif

			initialized = true;
		}
	}

	if (initialized == true)
	{
		preparationStep( );
	}

#if DEEP_DEBUG

	for (i = 0; i < N; ++i)
		for (j = 0; j < NY; ++j)
			weightingCoefficients[i * NY + j] = acadoVariables.S[i * NY * NY + j * NY + j];

	for (i = 0; i < NYN; ++i)
		weightingCoefficients[N * NY + i] = acadoVariables.SN[i * NYN + i];

	portWeightingCoefficients.write( weightingCoefficients );

#endif

	timePrepPhase = TimeService::Instance()->secondsSince( tickPreparationPhaseBegin );
	portPreparationPhaseExecTime.write( timePrepPhase );
}

void DynamicMHE::mheFeedbackPhase( )
{
	unsigned i;

	tickFeedbackPhaseBegin = TimeService::Instance()->getTicks();

	if (sqpIterationsCounter == 0)
	{
		statusMeasurements = prepareMeasurements();
	}

	if (initialized == true && statusMeasurements == true)
	{
		// Run the estimator
		qpSolverStatus = feedbackStep( lastMeasurementForMHE );

		if (qpSolverStatus == 0)
		{
			// HUH, we are so lucky today

			if (sqpIterationsCounter == 0)
			{
				// Set the ready trigger to true after the first SQP iteration

				ready = true;
			}
			else
			{
				ready = false;
			}
		}
		else
		{
			// TODO Houston, we have a problem...

			ready = false;
		}

		portQPSolverStatus.write( qpSolverStatus );

		// Write the data to the output port
		for (i = 0; i < NX; ++i)
			stateEstimate[ i ] = acadoVariables.x[N * NX + i];

		for (i = 0; i < NU; ++i)
			controlEstimate[ i ] = acadoVariables.u[(N-1) * NU + i]; 

		portStateEstimate.write( stateEstimate );
		portControlEstimate.write( controlEstimate );

		kktTolerance = getKKT();

		objectiveValue = getObjectiveValue();

		portKKTTolerance.write( kktTolerance );
		portObjectiveValue.write( objectiveValue );

		numOfActiveSetChanges = logNWSR;
		portNumOfActiveSetChanges.write( numOfActiveSetChanges );

		if(sqpIterationsCounter==0){
			for(i = 0; i < NX; i++){
				StateAndControl[i] = stateEstimate[i];
			}
		}
		if(sqpIterationsCounter==1){
			for(i = 0; i<NX; i++){
				StateAndControl[NX+i] = stateEstimate[i];
			}
		}
		if(sqpIterationsCounter==2){
			for(i = 0; i<  NX; i++){
				StateAndControl[2*NX+i] = stateEstimate[i];
			}
		}
		if(sqpIterationsCounter==0){ // This is the predicted state. This was computed at the previous time step, so by doing it now, it is integrated to the current time.
			for(i = 0; i < NX; i++){
				StateAndControl[3*NX+i] = statePredicted[i];
			}
		}
		if(sqpIterationsCounter == 0){
			if(portStateReference.read(StateReference) == NoData){
				StateReference.resize(NX,0.0);
			}
			for(i = 0; i < NX; i++){
				StateAndControl[4*NX+i] = StateReference[i];
			}
		}
		if(sqpIterationsCounter == 2){
			for(i = 0; i < NU; i++){
				if(measurementsCtrl.size() == NU){
					StateAndControl[5*NX+i] = measurementsCtrl[i];
				}
				else{
					StateAndControl[5*NX+i] = 0.0;
				}
			}
		}
		if(sqpIterationsCounter == 2){
			for(i = 0; i < NU; i++){
				if(measurementsCtrl.size() == NU){
					StateAndControl[5*NX+NU+i] = measurementsCtrlRates[i];
				}
				else{
					StateAndControl[5*NX+NU+i] = 0.0;
				}
			}
		}
		if(sqpIterationsCounter == 0){
			for(i = 0; i < NY_MARK; i++){
				StateAndControl[5*NX+2*NU+i] = measurementsMarkers[i];
			}
		}
		if(sqpIterationsCounter == 0){
			for(i = 0; i < NY_IMU; i++){
				StateAndControl[5*NX+2*NU+NY_MARK+i] = measurementsIMU[i];
			}
		}
		if(sqpIterationsCounter == 0){
			for(i = 0; i < NY_POSE; i++){
				StateAndControl[5*NX+2*NU+NY_MARK+NY_IMU+i] = measurementsPose[i];
			}
		}
		if(sqpIterationsCounter == 0){
			for(i = 0; i < NY_ENC; i++){
				StateAndControl[5*NX+2*NU+NY_MARK+NY_IMU+NY_POSE+i] = measurementsEncoder[i];
			}
		}

		// Copy the full state vector over the full horizon and write it to a port
		copy(acadoVariables.x, acadoVariables.x + (N + 1) * NX, fullStateVector.begin());
		portFullStateVector.write( fullStateVector );
		// Copy the full control vector over the full horizon and write it to a port
		copy(acadoVariables.u, acadoVariables.u + N * NU, fullControlVector.begin());
		portFullControlVector.write( fullControlVector );

		if(sqpIterationsCounter==0){
			for(i = 0; i < (N + 1)*NX; i++){
				StateAndControl[5*NX+2*NU+NY_MARK+NY_IMU+NY_POSE+NY_ENC+i] = fullStateVector[i];
			}
		}
		if(sqpIterationsCounter==1){
			for(i = 0; i < (N + 1)*NX; i++){
				StateAndControl[5*NX+2*NU+NY_MARK+NY_IMU+NY_POSE+NY_ENC+(N + 1)*NX+i] = fullStateVector[i];
			}
		}
		if(sqpIterationsCounter==2){
			for(i = 0; i < (N + 1)*NX; i++){
				StateAndControl[5*NX+2*NU+NY_MARK+NY_IMU+NY_POSE+NY_ENC+2*(N + 1)*NX+i] = fullStateVector[i];
			}
		}

		if(sqpIterationsCounter==0){
			portStateAndControl.write(StateAndControl);
		}
	}
	
	// The user component should always read _first_ whether the MHE is ready
	portReady.write( ready );

	// Write the info about data sizes
	portDataSizeValid.write( dataSizeValid );

	// Write the exec time
	timeFdbPhase = TimeService::Instance()->secondsSince( tickFeedbackPhaseBegin );
	portFeedbackPhaseExecTime.write( timeFdbPhase );
}

bool DynamicMHE::prepareMeasurements( void )
{
	unsigned i;

	dataSizeValid = false;

	//
	// OK, so, first read all data
	//
	statusMeasurementsMarkers = portMeasurementsMarkers.read( measurementsMarkers );
	statusMeasurementsIMU = portMeasurementsIMU.read( measurementsIMU );
	statusMeasurementsEncoder = portMeasurementsEncoder.read( measurementsEncoder );
	statusMeasurementsCtrl = portMeasurementsCtrl.read( measurementsCtrl );
	statusMeasurementsCtrlRates = portMeasurementsCtrlRates.read( measurementsCtrlRates );
	statusMeasurementsPose = portMeasurementsPose.read( measurementsPose );

	//
	// Check if the controls are valid (not Nan's)
	//
	for(i=0; i<measurementsCtrl.size(); i++){
		if(not(isfinite(measurementsCtrl[i]))){
			measurementsCtrl[i] = 0.0;
		}
	}
	for(i=0; i<measurementsCtrlRates.size(); i++){
		if(not(isfinite(measurementsCtrlRates[i]))){
			measurementsCtrlRates[i] = 0.0;
		}
	}

	//
	// Read the measurements of controls
	//
	if (statusMeasurementsCtrl == NewData)
	{
		if (measurementsCtrl.size() != NY_CTRL)
			return false;

		oldMeasurementsCtrl = measurementsCtrl;

		// Scale inputs
		oldMeasurementsCtrl[ 0 ] /= SCALE_UR;
		oldMeasurementsCtrl[ 1 ] /= SCALE_UR;
		oldMeasurementsCtrl[ 2 ] /= SCALE_UP;
	}

	//
	// Read the measurements of control rates
	//
	if (statusMeasurementsCtrlRates != NoData)
	{
		if (measurementsCtrlRates.size() != NY_CTRL)
			return false;
		measurementsCtrlRates[ 0 ] /= SCALE_UR;
		measurementsCtrlRates[ 1 ] /= SCALE_UR;
		measurementsCtrlRates[ 2 ] /= SCALE_UP;
	}
	else
	{
		for (i = 0; i < NY_CTRL; ++i)
			measurementsCtrlRates[ i ] = 0.0;
	}

	//
	// Process new data; ALL data vectors have to be new
	//

//		// Check for data dimensions
//		dataSizeValid = true;
//		if (	measurementsMarkers.size() != POSE_VEC_SIZE ||
//				measurementsIMU.size() != IMU_VEC_SIZE ||
//				measurementsEncoder.size() != NY_ENC ||
//				 )
//		{
//			// TODO Some serious error happened, implement some wisdom here
//
//			dataSizeValid = false;
//
//			return;
//		}

	//
	// Process fresh data
	//

	// TODO Check for sizes, too....

	if (initialized == false)
	{
		if (statusMeasurementsEncoder != NewData)
		{
			return false;
		}

		//
		// IMU measurements
		//
		if (sampleCounter >= DELAY_IMU)
		{
			if (statusMeasurementsIMU != NewData)
			{
				return false;
			}

			// Stack IMU measurements
			for (i = 0; i < NY_IMU; ++i)
				acadoVariables.yMeas[
				                      (sampleCounter - DELAY_IMU) * NY
				                      + NY_POSE + i] =
				                    		  measurementsIMU[2 * NY_IMU + i];

			// Stack IMU cov. coefficients
			for (i = 0; i < NY_IMU; ++i)
				acadoVariables.S[
				                 (sampleCounter - DELAY_IMU) * NY * NY
				                 + (NY_POSE + i) * NY + NY_POSE + i] =
				                		 SCALE_OBJ * measurementsIMU[3 * NY_IMU + i];
		}

		//
		// Markers measurements
		//
		if (sampleCounter >= DELAY_POSE)
		{
			if (statusMeasurementsMarkers != NewData)
			{
				return false;
			}

			if (sampleCounter == DELAY_POSE)
			{
				//
				// Catch initial pose
				//
				if (statusMeasurementsPose != NewData)
				{
					return false;
				}

				for (i = 0; i < NY_POSE; ++i)
					initPose[ i ] = measurementsPose[ i ];
			}

			// Stack markers measurements
			for (i = 0; i < NY_POSE; ++i)
				acadoVariables.yMeas[
				                     (sampleCounter - DELAY_POSE) * NY
				                     + i] =
				                    		 measurementsMarkers[ i ];

			// Stack markers cov. coefficients
			for (i = 0; i < NY_POSE; ++i)
				acadoVariables.S[
				                 (sampleCounter - DELAY_POSE) * NY * NY
				                 + i * NY + i] =
				                		 SCALE_OBJ * measurementsMarkers[ NY_POSE + i ];
		}

		//
		// Stack encoder measurements
		//
		acadoVariables.yMeas[sampleCounter * NY + 18] = measurementsEncoder[ 0 ];

		// Catch initial speed
		if (sampleCounter == 0)
		{
			initDdelta = measurementsEncoder[ 1 ];
		}

		//
		// Stack ctrl measurements
		//
		acadoVariables.yMeas[sampleCounter * NY + 19] = oldMeasurementsCtrl[ 0 ];
		acadoVariables.yMeas[sampleCounter * NY + 20] = oldMeasurementsCtrl[ 2 ];

		//
		// Stack MHE control measurements:
		// 0: dddelta, always 0
		// 1: dur
		// 2: dup
		//
		acadoVariables.yMeas[sampleCounter * NY + 21] = 0.0;
		acadoVariables.yMeas[sampleCounter * NY + 22] = measurementsCtrlRates[ 0 ];
		acadoVariables.yMeas[sampleCounter * NY + 23] = measurementsCtrlRates[ 2 ];

		++sampleCounter;
	}
	else
	{
		//
		// initialized == true
		//

		if (	statusMeasurementsMarkers != NewData || measurementsMarkers.size() != POSE_VEC_SIZE ||
				statusMeasurementsIMU != NewData || measurementsIMU.size() != IMU_VEC_SIZE ||
				statusMeasurementsEncoder != NewData || measurementsEncoder.size() != NY_ENC)
		{
			return false;
		}

		//
		// Initialized, prepare data for the feedback phase.
		//

		//
		// MARKERS
		//

		// Markers data, two samples late
		for (i = 0; i < NY_POSE; ++i)
			acadoVariables.yMeas[
			                     (N - 2) * NY
			                     + i] =
			                    		 measurementsMarkers[ i ];

		// Markers cov. matrix, diag elements, two sample late
		for (i = 0; i < NY_POSE; ++i)
			acadoVariables.S[
			                 (N - 2) * NY * NY
			                 + i * NY + i] =
			                		 SCALE_OBJ * measurementsMarkers[NY_POSE + i];

		//
		// IMU
		//

		// IMU data, node N
		for (i = 0; i < NY_IMU; ++i)
			lastMeasurementForMHE[ i ] = measurementsIMU[ i ];

		// IMU Cov. matrix, node N
		for (i = 0; i < NY_IMU; ++i)
			acadoVariables.SN[
			                  i * NYN +
			                  + i] =
			                		  SCALE_OBJ * measurementsIMU[NY_IMU + i];

		// IMU data, node N - 1
		for (i = 0; i < NY_IMU; ++i)
			acadoVariables.yMeas[
			                     (N - 1) * NY
			                     + NY_POSE + i] =
			                    		 measurementsIMU[2 * NY_IMU  + i];

		// IMU Cov. matrix, node N - 1
		for (i = 0; i < NY_IMU; ++i)
			acadoVariables.S[
			                 (N - 1) * NY * NY
			                 + (NY_POSE + i) * NY + NY_POSE + i] =
			                		 SCALE_OBJ * measurementsIMU[3 * NY_IMU + i];

		//
		// DELTA
		//

		lastMeasurementForMHE[NY_IMU + 0] = measurementsEncoder[ 0 ];

		//
		// UR, UP
		//

		lastMeasurementForMHE[NY_IMU + 1] = oldMeasurementsCtrl[ 0 ];
		lastMeasurementForMHE[NY_IMU + 2] = oldMeasurementsCtrl[ 2 ];
	}

	dataSizeValid = true;

	return true;
}

ORO_CREATE_COMPONENT( DynamicMHE )
