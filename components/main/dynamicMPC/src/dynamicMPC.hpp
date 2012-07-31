#ifndef DYNAMIC_MPC_HPP
#define DYNAMIC_MPC_HPP

#include <rtt/TaskContext.hpp>
#include <rtt/RTT.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>
#include <rtt/Component.hpp>

#include <rtt/base/DataObjectInterface.hpp>

#include <rtt/os/TimeService.hpp>

//#include <ocl/LoggingService.hpp>
//#include <ocl/Category.hpp>

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

#define NX		22		// number of differential states
#define NU		2		// number of controls of the MPC
#define N 		5		// number of control intervals

#define N_OUT	3		// dimension of the output vector of this component

#define SCALE_UR	1.25e6	// Scaling of ur_1, ur_2
#define SCALE_UP	2e5		// Scaling of u_p

//#define SCALE_UR	10000	// Scaling of ur_1, ur_2
//#define SCALE_UP	32767		// Scaling of u_p

#define REF_Z_MIN		-1.0
#define REF_Z_MAX		1.0
#define REF_UR_MIN		-3.2767
#define REF_UR_MAX		3.2767

#define REF_USER_SIZE	2	// Size of the user ref. vector

//
// Initialization
//
// 1: Initialize with single shootin'
// 2: Put the same data to all nodes
#define INIT_VER	2

#define DEEP_DEBUG	0 // Dumping MHE data on ports and so...

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

//	bool setReference(double refZ, double refUr);

	bool setReference(unsigned index);

private:

	bool readDataFromFile(const char* fileName, vector< vector< double > >& data, unsigned numRows = 0, unsigned numCols = 0);

	void printMatrix(string name, vector< vector< double > > data);

	/// Indicator whether the MPC is ready for outputting the data
	bool initialized;

	//
	// Input ports and their associated variables
	//

	/// Estimates ready trigger
	InputPort< bool > portFeedbackReady;
	bool feedbackReady;


	/// Port state estimate
	InputPort< vector< double > > portFeedback;
	vector< double > feedback;

	double feedbackForMPC[ NX ];

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
	// Reference handling
	//
	DataObjectLockFree< vector<double> > refDO;
	vector< double > refIN;
	vector< double > refRT;

	vector< vector< double > > refDefault;

	vector< vector< double > > references;

	unsigned refCounter;
	unsigned numReferences;
	bool refChanged;

	//
	// TODO Logging
	//

	/// Our logging category
//	OCL::logging::Category* logger;

	//
	// Deep debug stuff
	//
#if DEEP_DEBUG

	OutputPort< vector< double > > portFullStateVector;
	vector< double > fullStateVector;

	OutputPort< vector< double > > portFullControlVector;
	vector< double > fullControlVector;

#endif // DEEP_DEBUG
};

#endif // DYNAMIC_MPC_HPP
