#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

#include "dynamicMPC.hpp"

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

}; // end MPCHACK

using namespace MPCHACK;

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

	this->provides()->addOperation("setReference", &DynamicMPC::setReference, this, OwnThread)
			.doc("Function for reference setting.")
			.arg("index",  "Index of the reference in the file; zero based");

	//
	// Set the input ports
	//
	this->addEventPort("portFeedbackReady", portFeedbackReady)
			.doc("Feedback ready trigger.");

	this->addPort("portFeedback", portFeedback)
			.doc("Feedback -- state estimate.");

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

	//
	// Properties
	//
	numSQPIterations = 1;
	this->addProperty("propNumSQPIterations", numSQPIterations)
			.doc("Number of SQP iterations. Default = 1, Max = 10.");
	this->addProperty("propRefDefaultFileName", refDefaultFileName)
			.doc("Name of a file which contains the default full state reference. File should contain one row.");
	this->addProperty("propReferencesFileName", referencesFileName)
			.doc("Name of a file which contains the (point) references. One reference per row");

	this->addProperty("propFileNameWeightsQ", fileNameWeightsQ);
	this->addProperty("propFileNameWeightsR", fileNameWeightsR);
	this->addProperty("propFileNameWeightsQF", fileNameWeightsQF);

	//
	// Misc
	//

	//
	// Deep debug
	//
#if DEEP_DEBUG

	this->addPort("portFullStateVector", portFullStateVector)
			.doc("Full state vector in the MHE.");

	fullStateVector.resize((N + 1) * NX, 0.0);
	portFullStateVector.setDataSample( fullStateVector );
	portFullStateVector.write( fullStateVector );

	this->addPort("portFullControlVector", portFullControlVector);

	fullControlVector.resize(N * NU, 0.0);
	portFullControlVector.setDataSample( fullControlVector );
	portFullControlVector.write( fullControlVector );

#endif

}

bool DynamicMPC::configureHook()
{
	unsigned i, j;

	if (	portFeedback.connected() == false ||
			portFeedbackReady.connected() == false )
	{
		log( Error ) << "At least one of the input ports is not connected" << endlog();

		return false;
	}
	
	//
	// Reset the MPC structures
	//
	memset(&acadoWorkspace, 0, sizeof( acadoWorkspace ));
	memset(&acadoVariables, 0, sizeof( acadoVariables ));

	bool status;
	vector< vector< double > > m;

	//
	// Read the file with the default references
	//
	status = readDataFromFile(refDefaultFileName.c_str(), refDefault, 1, NX);
	if (status == false)
	{
		log( Error ) << "Could not open a file with default reference" << endlog();

		return false;
	}

	for (i = 0; i < N; ++i)
		for (j = 0; j < NX; ++j)
			acadoVariables.xRef[i * NX + j] = refDefault[ 0 ][ j ];

	log( Info ) << "File with default reference successfully loaded" << endlog();
//	printMatrix("refDefault", refDefault);


	status = readDataFromFile(referencesFileName.c_str(), references, 0, NX);
	if (status == false)
	{
		log( Error ) << "Could not open a file with references" << endlog();

		return false;
	}
	log( Info ) << "File with (point) references successfully loaded" << endlog();
//	printMatrix("references", references);

	// Number of reference points
	numReferences = references.size();
//	cout << "numReferences: " << numReferences << endl;

	refCounter = 0;

	//
	// Initialize the real-time data structures for set/get of references
	//
//	refIN.resize(REF_USER_SIZE, 0.0);
//	refRT.resize(REF_USER_SIZE, 0.0);
//
//	refIN[ 0 ] = refDefault[ 0 ][ 2 ];
//	refIN[ 1 ] = refDefault[ 0 ][ 20 ];
//
//	refRT[ 0 ] = refDefault[ 0 ][ 2 ];
//	refRT[ 1 ] = refDefault[ 0 ][ 20 ];
//
//	refDO.data_sample( refIN );
//	refDO.Set( refIN );

	//
	// Load the weights Q, R, QF
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
			acadoVariables.Q[i * NX + j] = m[ i ][ j ];

	log( Info ) << "Q weights successfully loaded" << endlog();
	
	// dx, dy, dz
// 	acadoVariables.Q[ 3 ] *= 10.0;
// 	acadoVariables.Q[ 4 ] *= 10.0;
// 	acadoVariables.Q[ 5 ] *= 10.0;
	
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
			acadoVariables.R[i * NU + j] = m[ i ][ j ];

	log( Info ) << "R weights successfully loaded" << endlog();
//	printMatrix("R", m);

	// QF
	status = readDataFromFile(fileNameWeightsQF.c_str(), m, NX, NX);
	if (status == false)
	{
		log( Error ) << "Error in reading a file with weights QF" << endlog();

		return false;
	}

// 	for (i = 0; i < NX; ++i)
// 		for (j = 0; j < NX; ++j)
// 			acadoVariables.QF[i * NX + j] = m[ i ][ j ];

	log( Info ) << "QF (terminal) weights successfully loaded" << endlog();
//	printMatrix("QF", m);

	//
	// Limit the number of SQP iterations
	//
	if (numSQPIterations > 10)
		numSQPIterations = 10;

	return true;
}

bool DynamicMPC::startHook()
{
	unsigned i, j;

	kktTolerance = 0.0;
	numOfActiveSetChanges = 0;

	initialized = false;

	if (numSQPIterations > 10)
		numSQPIterations = 10;

	refCounter = 0;
	refChanged = false;

	//
	// (Re-)set the references
	//
	for (i = 0; i < N; ++i)
		for (j = 0; j < NX; ++j)
			acadoVariables.xRef[i * NX + j] = refDefault[ 0 ][ j ];

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
	unsigned i;

	for (i = 0; i < N_OUT; ++i)
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

		integrate( acadoWorkspace.state );

		shiftStates( acadoWorkspace.state );
		shiftControls( 0 );
	}
	else if (sqpIterationsCounter == (numSQPIterations - 1))
	{
		//
		// Initialize the first node
		//

		// Initialize with the feedback from the estimator
//		for (i = 0; i < NX; ++i)
//			acadoVariables.x[ i ] = feedback[ i ];

		// Initialize with the "default" reference (stable equilibrium)
		for (i = 0; i < NX; ++i)
			acadoVariables.x[ i ] = refDefault[ 0 ][ i ];

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

			// Set the controls
			// We can assume they are zero for starters
//				for (j = 0; j < NU; ++j)
//					acadoWorkspace.state[ indexU + j ] = acadoVariables.u[i * NU + j];

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
	unsigned i, j;

	tickFeedbackPhaseBegin = TimeService::Instance()->getTicks();

	if (sqpIterationsCounter == 0)
	{
		//
		// Read the feedback
		//

		dataSizeValid = true;

		portFeedback.read( feedback );

		if (feedback.size() != NX)
		{
			dataSizeValid = false;
		}

		if (refChanged == true && refCounter < numReferences)
		{
			for (i = 0; i < N; ++i)
				for (j = 0; j < NX; ++j)
					acadoVariables.xRef[i * NX + j] = references[ refCounter ][ j ];

			refChanged = false;
		}
	}

//	//
//	// Read the reference and set it, just write the values on the whole horizon
//	//
//	refDO.Get( refRT );
//
//	for (i = 0; i < N; ++i)
//	{
//		acadoVariables.xRef[i * NX + 2 ] = refRT[ 0 ];
//		acadoVariables.xRef[i * NX + 20] = refRT[ 1 ];
//	}

	if (initialized == true && dataSizeValid == true)
	{
		//
		// Run the controller if we are initialized
		//

		for(i = 0; i < NX; ++i)
			feedbackForMPC[ i ] = feedback[ i ];

		// Run the NMPC
		qpSolverStatus = feedbackStep( feedbackForMPC );

		if (qpSolverStatus == 0)
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

				controlRates[ 0 ] = SCALE_UR * acadoVariables.u[ 0 ];
				controlRates[ 1 ] = SCALE_UR * acadoVariables.u[ 0 ];
				controlRates[ 2 ] = SCALE_UP * acadoVariables.u[ 1 ];
			}
		}
		else
		{
			// TODO Implement some wisdom for the case NMPC wants to output some rubbish

			for (i = 0; i < N_OUT; ++i)
			{
				controls[ i ] = 0.0;
				controlsForMeasurement[ i ] = 0.0;
				controlRates[ i ] = 0.0;
			}
		}

		if (sqpIterationsCounter == (numSQPIterations - 1))
		{
			portControls.write( controls );
			portControlsForMeasurement.write( controlsForMeasurement );
			portControlRates.write( controlRates );
		}

		portQPSolverStatus.write( qpSolverStatus );

		kktTolerance = getKKT();

		objectiveValue = getObjectiveValue();

		portKKTTolerance.write( kktTolerance );
		portObjectiveValue.write( objectiveValue );

		numOfActiveSetChanges = logNWSR;
		portNumOfActiveSetChanges.write( numOfActiveSetChanges );
	}

	// Write the info about data sizes
	portDataSizeValid.write( dataSizeValid );

	// Write the exec time
	timeFdbPhase = TimeService::Instance()->secondsSince( tickFeedbackPhaseBegin );
	portFeedbackPhaseExecTime.write( timeFdbPhase );
}

//bool DynamicMPC::setReference(double _refZ, double _refUr)
//{
//	if (	_refZ < REF_Z_MIN ||
//			_refZ > REF_Z_MAX ||
//			_refUr < REF_UR_MIN ||
//			_refUr > REF_UR_MAX )
//		return false;
//
//	refIN[ 0 ] = _refZ;
//	refIN[ 1 ] = _refUr;
//
//	refDO.Set( refIN );
//
//	return true;
//}

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

bool DynamicMPC::setReference(unsigned index)
{
	if (index > numReferences)
		return false;

	refCounter = index;

	refChanged = true;

	return true;
}

ORO_CREATE_COMPONENT( DynamicMPC )
