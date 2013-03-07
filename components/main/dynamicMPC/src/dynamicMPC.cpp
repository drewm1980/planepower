#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

//#include <boost/math/special_functions/fpclassify.hpp>

#include "dynamicMPC.hpp"

//
// The worst hack I applied to my code; a big thanx to Andrew.
// XXX Change this ASAP!!!
//

namespace MPCHACK
{

#include "acado.h"
#include "qpoases/solver.hpp"

#include "condensing.c"
#include "gauss_newton_method.c"
#include "integrator.c"
#include "qpoases/solver.cpp"
#include "Bounds.cpp"
#include "Constraints.cpp"
#include "CyclingManager.cpp"
#include "Indexlist.cpp"
#include "MessageHandling.cpp"
#include "QProblemB.cpp"
#include "QProblem.cpp"
#include "SubjectTo.cpp"
#include "Utils.cpp"


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

/// Array used explicitly by the NMPC
double feedbackForMPC[ ACADO_NX ];

}; // end MPCHACK

using namespace MPCHACK;

#define NX		ACADO_NX	// number of differential states
#define NU		ACADO_NU	// number of controls of the MPC
#define N 		ACADO_N	// number of control intervals
#define NP 		ACADO_NP	// number of control intervals

#define N_OUT	3		// dimension of the output vector of this component

//#define N_MULTIPLIERS 5*N + NX  // We have 5 constraints and it appears there are NX extra variables in the struct-variable
#define N_MULTIPLIERS QPOASES_NVMAX + QPOASES_NCMAX
//using namespace boost::math;

//
// Class methods
//
DynamicMPC::DynamicMPC(const std::string& name)
	:
		TaskContext(name, PreOperational)
//		logger(dynamic_cast<OCL::logging::Category*>(&log4cpp::Category::getInstance("dynamicMHE")))
{
	initialized = false;

	//
	// Set the operations
	//
	this->provides()->addOperation("mpcPreparationPhase", &DynamicMPC::mpcPreparationPhase, this, OwnThread)
			.doc( "MPC RTI preparation step." );

	this->provides()->addOperation("mpcFeedbackPhase", &DynamicMPC::mpcFeedbackPhase, this, OwnThread)
			.doc( "MPC RTI feedback step." );

	//
	// Set the input ports
	//
	this->addEventPort("portFeedbackReady", portFeedbackReady)
			.doc("Feedback ready trigger.");

	this->addPort("portFeedback", portFeedback)
			.doc("Feedback -- state estimate.");

	this->addPort("portReferences", portReferences)
			.doc("References -- for the whole horizon");

	this->addPort("portWeightingMatrixP", portWeightingMatrixP)
			.doc("A matrix for the terminal cost");

	this->addPort("portControlInput", portControlInput)
			.doc("A port with control inputs that were acutally applied to the system");

	//
	// Set the output ports
	//
	this->addPort("portControls", portControls)
			.doc("MPC controls, current controls (estimated by MHE).");

	this->addPort("portControlsForMeasurement", portControlsForMeasurement)
			.doc("MPC controls, used by MHE as measurement");

	this->addPort("portControlRates", portControlRates)
			.doc("MPC control rates, used by init. of the MHE.");

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
				.doc("Execution time of the MPC.");

	this->addPort("portQPSolverStatus", portQPSolverStatus)
			.doc("Status of the QP solver; 0 - good, otherwise scratch your head.");

	this->addPort("portDataSizeValid", portDataSizeValid);

	this->addPort("portMultipliers", portMultipliers)
			.doc("Lagrange multipliers");

	//
	// Initialize and output the relevant output ports
	//
	controls.resize(N_OUT, 0.0);
	portControls.setDataSample( controls );
	portControls.write( controls );

	controlsForMeasurement.resize(N_OUT, 0.0);
	portControlsForMeasurement.setDataSample( controlsForMeasurement );
	portControlsForMeasurement.write( controlsForMeasurement );

	controlRates.resize(N_OUT, 0.0);
	portControlRates.setDataSample( controlRates );
	portControlRates.write( controlRates );

	kktTolerance = 0.0;
	portKKTTolerance.write( kktTolerance );

	objectiveValue = 0.0;
	portObjectiveValue.write( objectiveValue );

	numOfActiveSetChanges = 0;
	portNumOfActiveSetChanges.write( numOfActiveSetChanges );

	qpSolverStatus = 0;
	portQPSolverStatus.write( qpSolverStatus );

	dataSizeValid = false;
	portDataSizeValid.write( dataSizeValid );

	multipliers.resize(N_MULTIPLIERS,0.0);
	portMultipliers.write(multipliers);
	portMultipliers.write(multipliers);

	//
	// Properties
	//
	numSQPIterations = 1;
	this->addProperty("propNumSQPIterations", numSQPIterations)
			.doc("Number of SQP iterations. Default = 1, Max = 10.");

	this->addProperty("propFileNameWeightsQ", fileNameWeightsQ);
	this->addProperty("propFileNameWeightsR", fileNameWeightsR);

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
	// Reset the MPC structures
	// XXX Maybe this is not the smartest place to put
	//
	memset(&acadoWorkspace, 0, sizeof( acadoWorkspace ));
	memset(&acadoVariables, 0, sizeof( acadoVariables ));

	//
	// Misc
	//


	this->addPort("portFullStateVector", portFullStateVector)
			.doc("Full state vector in the MHE.");

	fullStateVector.resize((N + 1) * NX, 0.0);
	portFullStateVector.setDataSample( fullStateVector );
	portFullStateVector.write( fullStateVector );

	this->addPort("portFullControlVector", portFullControlVector);

	fullControlVector.resize(N * NU, 0.0);
	portFullControlVector.setDataSample( fullControlVector );
	portFullControlVector.write( fullControlVector );


}

bool DynamicMPC::configureHook()
{
	unsigned i, j;

	if (	portFeedback.connected() == false ||
			portFeedbackReady.connected() == false ||
			portReferences.connected() == false ||
			portWeightingMatrixP.connected() == false )
	{
		log( Error ) << "At least one of the input ports is not connected" << endlog();

		return false;
	}

	bool status;
	vector< vector< double > > m;

	//
	// Load the weights Q, R
	//

	// Q
	status = readDataFromFile(fileNameWeightsQ.c_str(), m, NX, NX);
	if (status == false)
	{
		log( Error ) << "Error in reading a file with weights Q" << endlog();

		return false;
	}

	for (i = 0; i < NX; ++i)
		for (j = 0; j < NX; ++j)
			acadoVariables.QQ[i * NX + j] = m[ i ][ j ];

	log( Info ) << "Q weights successfully loaded" << endlog();
	
//	printMatrix("Q", m);

	// R
	status = readDataFromFile(fileNameWeightsR.c_str(), m, NU, NU);
	if (status == false)
	{
		log( Error ) << "Error in reading a file with weights R" << endlog();

		return false;
	}

	for (i = 0; i < NU; ++i)
		for (j = 0; j < NU; ++j)
			acadoVariables.RR[i * NU + j] = m[ i ][ j ];

	log( Info ) << "R weights successfully loaded" << endlog();
//	printMatrix("R", m);

	//
	// Limit the number of SQP iterations
	//
	if (numSQPIterations > 10)
		log( Warning )
			<< "Number of requested SQP iterations is: "
			<< numSQPIterations << endlog();

	return true;
}

bool DynamicMPC::startHook()
{
	memset(&acadoWorkspace, 0, sizeof( acadoWorkspace ));
	memset(&acadoVariables.x, 0, sizeof( acadoVariables.x ));
	memset(&acadoVariables.u, 0, sizeof( acadoVariables.u ));
	memset(&acadoVariables.xRef, 0, sizeof( acadoVariables.u ));
	memset(&acadoVariables.uRef, 0, sizeof( acadoVariables.u ));
	qpSolverStatus = 0.0; 
	kktTolerance = 0.0;
	numOfActiveSetChanges = 0;
	dataSizeValid = false;
	initialized = false;
	firstRefArrived = false;
	firstWeightPArrived = false;
	return true;
}

void DynamicMPC::updateHook()
{
	tickMPCBegin = TimeService::Instance()->getTicks();

	portFeedbackReady.read( feedbackReady );

	if (feedbackReady == false)
		goto out_done;

	for (sqpIterationsCounter = 0; sqpIterationsCounter < numSQPIterations; sqpIterationsCounter++)
	{
		mpcFeedbackPhase();
		mpcPreparationPhase();
	}

	out_done:

	timeMPC = TimeService::Instance()->secondsSince( tickMPCBegin );
	portExecutionTime.write( timeMPC );
}

void DynamicMPC::stopHook( )
{
	//
	// Reset all controls -- for safety
	//
	for (unsigned i = 0; i < N_OUT; ++i)
	{
		controls[ i ] = 0.0;
		controlsForMeasurement[ i ] = 0.0;
		controlRates[ i ] = 0.0;
	}

	portControls.write( controls );
	portControlsForMeasurement.write( controlsForMeasurement );
	portControlRates.write( controlRates );
}

void DynamicMPC::cleanupHook( )
{
}

void DynamicMPC::errorHook( )
{
	// TODO Implement some wisdom...
}

void DynamicMPC::mpcPreparationPhase()
{
	unsigned i, j;

	unsigned indexU = (1 + NX + NU) * NX;

	tickPreparationPhaseBegin = TimeService::Instance()->getTicks();

	if (dataSizeValid == false)
	{
		return;
	}

	if (initialized == true && sqpIterationsCounter == (numSQPIterations - 1))
	{
		//
		// Shift the shooting nodes and integrate the last one, OK
		//

		for (i = 0; i < NX; ++i)
			acadoWorkspace.state[ i ] = acadoVariables.x[N * NX + i];
		for (i = 0; i < NU; ++i)
			acadoWorkspace.state[ indexU + i ] = acadoVariables.u[(N - 1) * NU + i];
		for (i = 0; i < NP; ++i)
			acadoWorkspace.state[ indexU + NU + i ] = acadoVariables.p[i];

		integrate( acadoWorkspace.state );

		shiftStates( acadoWorkspace.state );
		shiftControls( 0 );
	}
	else if (	sqpIterationsCounter == (numSQPIterations - 1) &&
				firstRefArrived == true &&
				firstWeightPArrived == true )
	{
		//
		// i.e. here we are still not initialized
		// Initialize the first node
		//

		// Initialize with the feedback from the estimator
		for (i = 0; i < NX; ++i)
			acadoVariables.x[ i ] = feedbackForMPC[ i ];

		// XXX Initialize with the "default" reference (stable equilibrium)
//		for (i = 0; i < NX; ++i)
//			acadoVariables.x[ i ] = refDefault[ 0 ][ i ];

		//
		// Initialize all other nodes
		//

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
				acadoWorkspace.state[ indexU + j ] = 0.0;
			for (j = 0; j < NP; ++j)
				acadoWorkspace.state[ indexU + NU + j ] = acadoVariables.p[ j ];


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

	if (initialized == true)
	{
		preparationStep( );
	}

	timePrepPhase = TimeService::Instance()->secondsSince( tickPreparationPhaseBegin );
	portPreparationPhaseExecTime.write( timePrepPhase );
}

void DynamicMPC::mpcFeedbackPhase()
{
	unsigned i;
	//unsigned j;
	bool isFinite;

	tickFeedbackPhaseBegin = TimeService::Instance()->getTicks();

	prepareInputData();

	if (initialized == true && dataSizeValid == true)
	{
		//
		// Run the controller if we are initialized
		//

		// Run the NMPC
		qpSolverStatus = feedbackStep( feedbackForMPC );

		// First check if states && controls are NaN
		// XXX isfinite is something compiler dependent and should be tested...
		// XXX If we have NaN we should shutdown everything!!!
		isFinite = true;
		for (i = 0; i < NX * (N + 1); ++i)
		{
			if (isfinite( static_cast< double >( acadoVariables.x[ i ] ) ) == false)
			{
				isFinite = false;

				break;
			}
		}
		if (isFinite == true)
		{
			for (i = 0; i < NU * N; ++i)
			{
				if (isfinite( static_cast< double >( acadoVariables.u[ i ] ) ) == false)
				{
					isFinite = false;

					break;
				}
			}
		}


		// Now check for QP solver status
		if (qpSolverStatus == 0 && isFinite == true)
		{
			// HUH, we are so lucky today

			if (sqpIterationsCounter == (numSQPIterations - 1))
			{
				// Set the output to the port, ur1, ur2, up, and scale them

				controls[ 0 ] = SCALE_UR * acadoVariables.x[ 20 ];
				controls[ 1 ] = SCALE_UR * acadoVariables.x[ 20 ];
				controls[ 2 ] = SCALE_UP * acadoVariables.x[ 21 ];

				controlsForMeasurement[ 0 ] = SCALE_UR * acadoVariables.x[NX + 20];
				controlsForMeasurement[ 1 ] = SCALE_UR * acadoVariables.x[NX + 20];
				controlsForMeasurement[ 2 ] = SCALE_UP * acadoVariables.x[NX + 21];

				controlRates[ 0 ] = SCALE_UR * acadoVariables.u[ 1 ];
				controlRates[ 1 ] = SCALE_UR * acadoVariables.u[ 1 ];
				controlRates[ 2 ] = SCALE_UP * acadoVariables.u[ 2 ];
			}
		}
		else
		{
			// XXX Implement some wisdom for the case NMPC wants to output some rubbish
			// Stop the component is case we are not lucky today
			cout << "MPC want to trow garbage. stopping it.. " << endl;
			stop();
		}

		if (sqpIterationsCounter == (numSQPIterations - 1))
		{
			portControls.write( controls );
			portControlsForMeasurement.write( controlsForMeasurement );
			portControlRates.write( controlRates );
		}

		copy(vars.y, vars.y+N_MULTIPLIERS, multipliers.begin());
		portMultipliers.write(multipliers);
		portQPSolverStatus.write( qpSolverStatus );

		kktTolerance = getKKT();

		objectiveValue = getObjectiveValue();

		portKKTTolerance.write( kktTolerance );
		portObjectiveValue.write( objectiveValue );

		numOfActiveSetChanges = logNWSR;
		portNumOfActiveSetChanges.write( numOfActiveSetChanges );

		// Copy the full state vector over the full horizon and write it to a port
		copy(acadoVariables.x, acadoVariables.x + (N + 1) * NX, fullStateVector.begin());
		portFullStateVector.write( fullStateVector );
		// Copy the full control vector over the full horizon and write it to a port
		copy(acadoVariables.u, acadoVariables.u + N * NU, fullControlVector.begin());
		portFullControlVector.write( fullControlVector );
	}

	// Write the info about data sizes
	portDataSizeValid.write( dataSizeValid );

	// Write the exec time
	timeFdbPhase = TimeService::Instance()->secondsSince( tickFeedbackPhaseBegin );
	portFeedbackPhaseExecTime.write( timeFdbPhase );
}

bool DynamicMPC::prepareInputData( void )
{
	register unsigned i, j;

	if (sqpIterationsCounter == 0)
	{
		dataSizeValid = true;

		// Read the feedback
		statusPortFeedback = portFeedback.read( feedback );
		if (feedback.size() != NX)
		{
			dataSizeValid = false;
		}

		// Read the references
		statusPortReferences = portReferences.read( references );
		if (references.size() != (NX * N + NU * N))
		{
			dataSizeValid = false;
		}

		// Read the weighting matrix P
		statusPortWeightingMatrixP = portWeightingMatrixP.read( weightingMatrixP );
		if (weightingMatrixP.size() != (NX * NX))
		{
			dataSizeValid = false;
		}

		// Read the control input
		statusPortControlInput = portControlInput.read( controlInput );
		if (controlInput.size() != NU)
		{
			dataSizeValid = false;
		}

		//
		// If all data sizes are correct we can proceed
		//
		if (dataSizeValid == true)
		{
			// Set the feedback for the controls to controlinput. Do this such that we don't use the controls estimated by MHE, since MHE does not do a very good job at estimating them yet.
			feedback[20] = controlInput[0]/SCALE_UR; // ur
			feedback[21] = controlInput[2]/SCALE_UP; // up

			// References
			if (statusPortReferences == NewData)
			{
//				copy(references.begin(), references.end(), acadoVariables.xRef);

				for (i = 0; i < N; ++i)
					for (j = 0; j < NX; ++j)
						acadoVariables.xRef[i * NX + j] = references[i * (NX + NU) + j];
				for (i = 0; i < N; ++i)
					for (j = 0; j < NU; ++j)
						acadoVariables.uRef[i * NU + j] = references[i * (NX + NU) + NX + j];

				if (initialized == false && firstRefArrived == false)
				{
					firstRefArrived = true;
				}
			}

			// Terminal weighting matrix
			if (statusPortWeightingMatrixP == NewData)
			{
				copy(weightingMatrixP.begin(), weightingMatrixP.end(), acadoVariables.QT);

				if (initialized == false && firstWeightPArrived == false)
				{
					firstWeightPArrived = true;
				}
			}

			// Full state feedback
			copy(feedback.begin(), feedback.end(), feedbackForMPC);
		}
	}

	return dataSizeValid;
}

bool DynamicMPC::readDataFromFile(const char* fileName, vector< vector< double > >& data, unsigned numRows, unsigned numCols)
{
	ifstream file( fileName );
	string line;

	if ( file.is_open() )
	{
		data.clear();

		while( getline(file, line) )
		{
			istringstream linestream( line );
			vector< double > linedata;
			double number;

			while( linestream >> number )
			{
				linedata.push_back( number );
			}

			if (linedata.size() != numCols && numCols > 0)
			{
				file.close();

				return false;
			}

			data.push_back( linedata );
		}

		file.close();

		if (data.size() != numRows && numRows > 0)
			return false;
	}
	else
		return false;

	return true;
}

void DynamicMPC::printMatrix(string name, vector< vector< double > > data)
{
	cout << name << ": " << endl;
	for(unsigned i = 0; i < data.size(); ++i)
	{
		for(unsigned j = 0; j < data[ i ].size(); ++j)
			cout << data[ i ][ j ] << " ";

		cout << endl;
	}
}

ORO_CREATE_COMPONENT( DynamicMPC )
