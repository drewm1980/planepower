#include "DynamicMpc.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <algorithm>

using namespace std;
using namespace RTT;
using namespace RTT::os;

static bool isNaN(double* array, unsigned dim)
{
	for (unsigned i = 0; i < dim; ++i)
		if ((array[ i ] != array[ i ]) || (array[ i ] < -1e12) || (array[ i ] > 1e12))
			return true;

	return false;	
}

DynamicMpc::DynamicMpc(std::string name)
	: TaskContext(name, PreOperational)
{
//	initialized = false;

	//
	// Set the input ports
	//
	addEventPort("feedback", portFeedback)
		.doc("Feedback -- state estimate.");

	//
	// Set the output ports
	//
	addPort("controls", portControls)
		.doc("MPC controls, current controls (estimated by MHE).");

	//
	// Initialize and output the relevant output ports
	//

	//
	// Properties
	//
	numSqpIterations = 1;
	addProperty("numSqpIterations", numSqpIterations)
		.doc("Number of SQP iterations. Default = 1, Max = 10.");
	
	//
	// Reset the MPC structures
	//
}

bool DynamicMpc::configureHook()
{
	memset(&acadoWorkspace, 0, sizeof( acadoWorkspace ));
	memset(&acadoVariables, 0, sizeof( acadoVariables ));

	unsigned i, j;

	if (portFeedback.connected() == false)
	{
		log( Error ) << "At least one of the input ports is not connected" << endlog();

		return false;
	}

	if (numSqpIterations > 10)
		log( Warning )
			<< "Number of requested SQP iterations is: "
			<< numSqpIterations << endlog();

	return true;
}

bool DynamicMpc::startHook()
{
	initializeSolver();

	return true;
}

void DynamicMpc::updateHook()
{
	// tickMPCBegin = TimeService::Instance()->getTicks();

	// portFeedbackReady.read( feedbackReady );

	// if (feedbackReady == false)
	// 	goto out_done;

	// for (sqpIterationsCounter = 0; sqpIterationsCounter < numSQPIterations; sqpIterationsCounter++)
	// {
	// 	mpcFeedbackPhase();
	// 	mpcPreparationPhase();
	// }

	// out_done:

	// timeMPC = TimeService::Instance()->secondsSince( tickMPCBegin );
	// portExecutionTime.write( timeMPC );
}

void DynamicMpc::stopHook( )
{
	//
	// Reset all controls -- for safety
	//
	// for (unsigned i = 0; i < N_OUT; ++i)
	// {
	// 	controls[ i ] = 0.0;
	// 	controlsForMeasurement[ i ] = 0.0;
	// 	controlRates[ i ] = 0.0;
	// }

	// portControls.write( controls );
	// portControlsForMeasurement.write( controlsForMeasurement );
	// portControlRates.write( controlRates );
}

void DynamicMpc::cleanupHook( )
{}

void DynamicMpc::errorHook( )
{
	// TODO Implement some wisdom...
}

//void DynamicMpc::mpcPreparationPhase()
//{
// 	unsigned i, j;

// 	unsigned indexU = (1 + NX + NU) * NX;

// 	tickPreparationPhaseBegin = TimeService::Instance()->getTicks();

// 	if (dataSizeValid == false)
// 	{
// 		return;
// 	}

// 	if (initialized == true && sqpIterationsCounter == (numSQPIterations - 1))
// 	{
// 		//
// 		// Shift the shooting nodes and integrate the last one, OK
// 		//

// 		for (i = 0; i < NX; ++i)
// 			acadoWorkspace.state[ i ] = acadoVariables.x[N * NX + i];
// 		for (i = 0; i < NU; ++i)
// 			acadoWorkspace.state[ indexU + i ] = acadoVariables.u[(N - 1) * NU + i];
// 		for (i = 0; i < NP; ++i)
// 			acadoWorkspace.state[ indexU + NU + i ] = acadoVariables.p[i];

// 		integrate(acadoWorkspace.state, 1);

// 		shiftStates( acadoWorkspace.state );
// 		shiftControls( 0 );
// 	}
// 	else if (	sqpIterationsCounter == (numSQPIterations - 1) &&
// 				firstRefArrived == true &&
// 				firstWeightPArrived == true )
// 	{
// 		//
// 		// i.e. here we are still not initialized
// 		// Initialize the first node
// 		//

// 		// Initialize with the feedback from the estimator
// 		for (i = 0; i < NX; ++i)
// 			acadoVariables.x[ i ] = feedbackForMPC[ i ];

// 		// XXX Initialize with the "default" reference (stable equilibrium)
// //		for (i = 0; i < NX; ++i)
// //			acadoVariables.x[ i ] = refDefault[ 0 ][ i ];

// 		//
// 		// Initialize all other nodes
// 		//

// #if INIT_VER == 1

// 		//
// 		// Initialize all other nodes by forward simulation
// 		// NOTE: This is tested with implicit integrator
// 		//
// 		for (i = 0; i < N; ++i)
// 		{
// 			// set the states
// 			for (j = 0; j < NX; ++j)
// 				acadoWorkspace.state[ j ] = acadoVariables.x[i * NX + j];
// 			// We can assume they are zero for starters			
// 			for (j = 0; j < NU; ++j)
// 				acadoWorkspace.state[ indexU + j ] = 0.0;
// 			for (j = 0; j < NP; ++j)
// 				acadoWorkspace.state[ indexU + NU + j ] = acadoVariables.p[ j ];


// 			integrate(acadoWorkspace.state, 1);

// 			// Write the new states
// 			for (j = 0; j < NX; ++j)
// 				acadoVariables.x[(i + 1) * NX + j] = acadoWorkspace.state[ j ];
// 		}

// #elif INIT_VER == 2

// 			//
// 			// Put the same data on all nodes
// 			//
// 			for (i = 0; i < N; ++i)
// 				for (j = 0; j < NX; ++j)
// 					acadoVariables.x[(i + 1) * NX + j] = acadoVariables.x[ j ];

// #else
// 	#error "Option is not supported."
// #endif

// 			initialized = true;
// 	}

// 	if (initialized == true)
// 	{
// 		preparationStep( );
		
// 		if (isNaN(acadoVariables.x, (N + 1) * NX))
// 			log( Debug ) << "Preparation step: acadoVariables.x is NaN" << endlog();
			
// 		if (isNaN(acadoVariables.u, N * NU))
// 			log( Debug ) << "Preparation step: acadoVariables.u is NaN" << endlog();
// 	}

// 	timePrepPhase = TimeService::Instance()->secondsSince( tickPreparationPhaseBegin );
// 	portPreparationPhaseExecTime.write( timePrepPhase );
//}

//void DynamicMpc::mpcFeedbackPhase()
// {
// 	unsigned i;
// 	//unsigned j;
// 	bool isFinite;

// 	tickFeedbackPhaseBegin = TimeService::Instance()->getTicks();

// 	prepareInputData();

// 	if (initialized == true && dataSizeValid == true)
// 	{
// 		//
// 		// Run the controller if we are initialized
// 		//

// 		// Run the NMPC
// 		qpSolverStatus = feedbackStep( feedbackForMPC );

// 		kktTolerance = getKKT();

// 		objectiveValue = getObjectiveValue();

// 		portKKTTolerance.write( kktTolerance );
// 		portObjectiveValue.write( objectiveValue );

// 		numOfActiveSetChanges = logNWSR;
// 		portNumOfActiveSetChanges.write( numOfActiveSetChanges );
// 		// First check if states && controls are NaN
// 		// XXX isfinite is something compiler dependent and should be tested...
// 		// XXX If we have NaN we should shutdown everything!!!
// 		isFinite = true;
// 		if (isNaN(acadoVariables.x, NX * (N + 1)) == true)
// 		{
// 			isFinite = false;
// 		}
// 		if (isNaN(acadoVariables.u, NU * N) == true)
// 		{
// 			isFinite = false;
// 		}

// /*		for (i = 0; i < NX * (N + 1); ++i)
// 		{
// 			if (isfinite( static_cast< double >( acadoVariables.x[ i ] ) ) == false)
// 			{
// 				isFinite = false;

// 				break;
// 			}
// 		}
// 		if (isFinite == true)
// 		{
// 			for (i = 0; i < NU * N; ++i)
// 			{
// 				if (isfinite( static_cast< double >( acadoVariables.u[ i ] ) ) == false)
// 				{
// 					isFinite = false;

// 					break;
// 				}
// 			}
// 		} */


// 		// Now check for QP solver status
// 		if ((qpSolverStatus == 0 || qpSolverStatus == 58) && isFinite == true)
// 		{
// 			// HUH, we are so lucky today

// 			if (sqpIterationsCounter == (numSQPIterations - 1))
// 			{
// 				// Set the output to the port, ur1, ur2, up, and scale them

// 				controls[ 0 ] = SCALE_UR * acadoVariables.x[ 20 ];
// 				controls[ 1 ] = SCALE_UR * acadoVariables.x[ 20 ];
// 				controls[ 2 ] = SCALE_UP * acadoVariables.x[ 21 ];

// 				controlsForMeasurement[ 0 ] = SCALE_UR * acadoVariables.x[NX + 20];
// 				controlsForMeasurement[ 1 ] = SCALE_UR * acadoVariables.x[NX + 20];
// 				controlsForMeasurement[ 2 ] = SCALE_UP * acadoVariables.x[NX + 21];

// 				controlRates[ 0 ] = SCALE_UR * acadoVariables.u[ 1 ];
// 				controlRates[ 1 ] = SCALE_UR * acadoVariables.u[ 1 ];
// 				controlRates[ 2 ] = SCALE_UP * acadoVariables.u[ 2 ];
// 			}
// 		}
// 		else
// 		{
// 			// XXX Implement some wisdom for the case NMPC wants to output some rubbish
// 			// Stop the component is case we are not lucky today
// //			log( Error )  << "MPC want to trow garbage. stopping it.. " << endlog();
			
// 			stop();

// //			goto feedbackStepExit;
// 		}

// 		if (sqpIterationsCounter == (numSQPIterations - 1))
// 		{
// 			portControls.write( controls );
// 			portControlsForMeasurement.write( controlsForMeasurement );
// 			portControlRates.write( controlRates );
// 		}

// 		copy(vars.y, vars.y+N_MULTIPLIERS, multipliers.begin());
// 		portMultipliers.write(multipliers);
// 		portQPSolverStatus.write( qpSolverStatus );


// 		// Copy the full state vector over the full horizon and write it to a port
// 		copy(acadoVariables.x, acadoVariables.x + (N + 1) * NX, fullStateVector.begin());
// 		portFullStateVector.write( fullStateVector );
// 		// Copy the full control vector over the full horizon and write it to a port
// 		copy(acadoVariables.u, acadoVariables.u + N * NU, fullControlVector.begin());
// 		portFullControlVector.write( fullControlVector );
// 	}

// 	feedbackStepExit:

// 	// Write the info about data sizes
// 	portDataSizeValid.write( dataSizeValid );

// 	// Write the exec time
// 	timeFdbPhase = TimeService::Instance()->secondsSince( tickFeedbackPhaseBegin );
// 	portFeedbackPhaseExecTime.write( timeFdbPhase );
// }

// bool DynamicMpc::prepareInputData( void )
// {
// 	register unsigned i, j;

// 	if (sqpIterationsCounter == 0)
// 	{
// 		dataSizeValid = true;

// 		// Read the feedback
// 		statusPortFeedback = portFeedback.read( feedback );
// 		if (feedback.size() != NX || statusPortFeedback != NewData)
// 		{
// 			dataSizeValid = false;
// 		}
// 		if (isNaN(&feedback[ 0 ], NX))
// 		{
// 			log( Debug ) << "feedback is NaN" << endlog();
// 		}

// 		// Read the references
// 		statusPortReferences = portReferences.read( references );
// 		if (references.size() != (NX * N + NU * N))
// 		{
// 			dataSizeValid = false;
// 		}
// 		if (isNaN(&references[ 0 ], NX * N + NU * N))
// 		{
// 			log( Debug ) << "references is NaN" << endlog();
// 		}

// 		// Read the weighting matrix P
// 		statusPortWeightingMatrixP = portWeightingMatrixP.read( weightingMatrixP );
// 		if (weightingMatrixP.size() != (NX * NX))
// 		{
// 			dataSizeValid = false;
// 		}
// 		if (isNaN(&weightingMatrixP[ 0 ], NX * NX))
// 		{
// 			log( Debug ) << "weightingMatrixP is NaN" << endlog();
// 		}

// 		// Read the control input
// 		statusPortControlInput = portControlInput.read( controlInput );
// 		if (controlInput.size() != NU)
// 		{
// 			dataSizeValid = false;
// 		}
// 		if (isNaN(&controlInput[ 0 ], NU))
// 		{
// 			log( Debug ) << "controlInput is NaN" << endlog();
// 		}

// 		//
// 		// If all data sizes are correct we can proceed
// 		//
// 		if (dataSizeValid == true)
// 		{
// 			// Set the feedback for the controls to controlinput. Do this such that we don't use the controls estimated by MHE, since MHE does not do a very good job at estimating them yet.
// 			if (isfinite(controlInput[ 0 ]) == false || isfinite(controlInput[ 1 ]) == false)
// 			{
// 				feedback[ 20 ] = feedback[ 21 ] = 0.0;
// 			}
// 			else
// 			{
// 				feedback[20] = controlInput[0]/SCALE_UR; // ur
// 				feedback[21] = controlInput[2]/SCALE_UP; // up
// 			}

// 			// References
// 			if (statusPortReferences == NewData)
// 			{
// //				copy(references.begin(), references.end(), acadoVariables.xRef);

// 				for (i = 0; i < N; ++i)
// 					for (j = 0; j < NX; ++j)
// 						acadoVariables.xRef[i * NX + j] = references[i * (NX + NU) + j];
// 				for (i = 0; i < N; ++i)
// 					for (j = 0; j < NU; ++j)
// 						acadoVariables.uRef[i * NU + j] = references[i * (NX + NU) + NX + j];

// 				if (initialized == false && firstRefArrived == false)
// 				{
// 					firstRefArrived = true;
// 				}
// 			}

// 			// Terminal weighting matrix
// 			if (statusPortWeightingMatrixP == NewData)
// 			{
// 				copy(weightingMatrixP.begin(), weightingMatrixP.end(), acadoVariables.QT);

// 				if (initialized == false && firstWeightPArrived == false)
// 				{
// 					firstWeightPArrived = true;
// 				}
// 			}

// 			// Full state feedback
// 			copy(feedback.begin(), feedback.end(), feedbackForMPC);
// 		}
// 	}

// 	return dataSizeValid;
// }

ORO_CREATE_COMPONENT( DynamicMpc )
