#ifndef DYNAMIC_MPC_HPP
#define DYNAMIC_MPC_HPP

#include <rtt/TaskContext.hpp>
#include <rtt/RTT.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>
#include <rtt/Component.hpp>

#include <rtt/base/DataObjectInterface.hpp>

#include <rtt/os/TimeService.hpp>

//
// Used namespaces
//
using namespace RTT;
using namespace RTT::base;
using namespace RTT::os;
using namespace std;

//
// Define some common stuff
//

#define SCALE_UR	1.25e6	// Scaling of ur_1, ur_2
#define SCALE_UP	2e5		// Scaling of u_p

//
// Initialization
//
// 1: Initialize with single shootin'
// 2: Put the same data to all nodes
#define INIT_VER	1

#define DEEP_DEBUG	0 // Dumping MPC data on ports and so...

class DynamicMPC
	: public TaskContext
{
public:
	DynamicMPC(const std::string& name);

	virtual ~DynamicMPC( )
	{}

	virtual bool configureHook( );

	virtual bool startHook( );

	virtual void updateHook( );

	virtual void stopHook( );

	virtual void cleanupHook( );

	virtual void errorHook( );

	void mpcFeedbackPhase( );

	void mpcPreparationPhase( );

private:

	bool readDataFromFile(const char* fileName, vector< vector< double > >& data, unsigned numRows = 0, unsigned numCols = 0);

	void printMatrix(string name, vector< vector< double > > data);

	bool prepareInputData( void );
	
	bool initializeSolver();

	/// Indicator whether the MPC is ready for outputting the data
	bool initialized;

	bool firstRefArrived;
	bool firstWeightPArrived;

	//
	// Input ports and their associated variables
	//

	/// Estimates ready trigger
	InputPort< bool > portFeedbackReady;
	bool feedbackReady;

	/// Port state estimate
	InputPort< vector< double > > portFeedback;
	vector< double > feedback;
	FlowStatus statusPortFeedback;

	/// Port for references
	InputPort< vector< double > > portReferences;
	vector< double > references;
	FlowStatus statusPortReferences;

	/// Terminal cost weighting matrix
	InputPort< vector< double > > portWeightingMatrixP;
	vector< double > weightingMatrixP;
	FlowStatus statusPortWeightingMatrixP;

	InputPort< vector< double > > portControlInput;
	vector< double > controlInput;
	FlowStatus statusPortControlInput;

	//
	// Output ports and their associated variables
	//

	/// Controls
	// Signals: [ur1, ur2, up]
	OutputPort< vector< double > > portControls;
	vector< double > controls;
	
	// Controls for measurement by the MHE
	OutputPort< vector< double > > portControlsForMeasurement;
	vector< double > controlsForMeasurement;

	/// Control rates
	// Signals: [dur1, dur2, dur3]
	OutputPort< vector< double > > portControlRates;
	vector< double > controlRates;

	/// KKT tolerance
	OutputPort< double > portKKTTolerance;
	double kktTolerance;

	/// Objective value
	OutputPort< double > portObjectiveValue;
	double objectiveValue;

	/// Number of active set changes, returned by QP solver
	OutputPort< int > portNumOfActiveSetChanges;
	int numOfActiveSetChanges;

	/// Execution times
	OutputPort< double > portPreparationPhaseExecTime;
	OutputPort< double > portFeedbackPhaseExecTime;
	OutputPort< double > portExecutionTime;
	RTT::os::TimeService::ticks tickPreparationPhaseBegin;
	RTT::os::TimeService::ticks tickFeedbackPhaseBegin;
	RTT::os::TimeService::ticks tickMPCBegin;
	double timePrepPhase;
	double timeFdbPhase;
	double timeMPC;

	/// Status of the QP solver
	OutputPort< int > portQPSolverStatus;
	/// Status of the QP solver
	int qpSolverStatus;

	OutputPort< bool > portDataSizeValid;
	/// Data sizes are valid
	bool dataSizeValid;

	/// Lagrange multipliers
	OutputPort< vector< double > > portMultipliers;
	vector< double > multipliers;
	//
	// Properties
	//

	/// Number of SQP iterations
	unsigned numSQPIterations;
	unsigned sqpIterationsCounter;

	// File names for default references and weighting matrices
	string refDefaultFileName;
	string referencesFileName;
	string fileNameWeightsQ;
	string fileNameWeightsR;
	string fileNameWeightsQF;

	//
	// TODO Logging
	//

	/// Our logging category
//	OCL::logging::Category* logger;

	//
	// Deep debug stuff
	//

	OutputPort< vector< double > > portFullStateVector;
	vector< double > fullStateVector;

	OutputPort< vector< double > > portFullControlVector;
	vector< double > fullControlVector;

};

#endif // DYNAMIC_MPC_HPP
