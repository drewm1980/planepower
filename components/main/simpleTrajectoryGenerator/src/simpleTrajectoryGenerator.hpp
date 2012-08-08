#ifndef SIMPLE_TRAJECTORY_GENERATOR_HPP
#define SIMPLE_TRAJECTORY_GENERATOR_HPP

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

class SimpleTrajectoryGenerator
	: public TaskContext
{
public:
	SimpleTrajectoryGenerator(const std::string& name);

	virtual ~SimpleTrajectoryGenerator( )
	{}

	virtual bool configureHook( );

	virtual bool startHook( );

	virtual void updateHook( );

	virtual void stopHook( );

	virtual void cleanupHook( );

	virtual void errorHook( );

private:

	bool readDataFromFile(const char* fileName, vector< vector< double > >& data, unsigned numRows = 0, unsigned numCols = 0);

	void printMatrix(string name, vector< vector< double > > data);
	
	string referencesFileName;
	string weightsPFileName;
	
	unsigned N, NX, NU;
	
	vector< vector< double > > references;
	vector< vector< double > > weightsP;
	
	unsigned numOfRefs;
	unsigned numOfWeightsP;
	
	unsigned refCounter;
	
	vector< double > execReferences;
	vector< double > execWeightsP;
	
	OutputPort< vector< double > > portReferences;
	OutputPort< vector< double > > portWeightsP;
	
	InputPort< bool > portTrigger;
	bool trigger;

// 	bool prepareInputData( void );
// 
// 	/// Indicator whether the MPC is ready for outputting the data
// 	bool initialized;
// 
// 	bool firstRefArrived;
// 	bool firstWeightPArrived;
// 
// 	/// Array used explicitly by the NMPC
// 	double feedbackForMPC[ NX ];
// 
// 	//
// 	// Input ports and their associated variables
// 	//
// 
// 	/// Estimates ready trigger
// 	InputPort< bool > portFeedbackReady;
// 	bool feedbackReady;
// 
// 	/// Port state estimate
// 	InputPort< vector< double > > portFeedback;
// 	vector< double > feedback;
// 	FlowStatus statusPortFeedback;
// 
// 	/// Port for references
// 	InputPort< vector< double > > portReferences;
// 	vector< double > references;
// 	FlowStatus statusPortReferences;
// 
// 	/// Terminal cost weighting matrix
// 	InputPort< vector< double > > portWeightingMatrixP;
// 	vector< double > weightingMatrixP;
// 	FlowStatus statusPortWeightingMatrixP;
// 
// 	//
// 	// Output ports and their associated variables
// 	//
// 
// 	/// Controls
// 	// Signals: [ur1, ur2, up]
// 	OutputPort< vector< double > > portControls;
// 	vector< double > controls;
// 	
// 	// Controls for measurement by the MHE
// 	OutputPort< vector< double > > portControlsForMeasurement;
// 	vector< double > controlsForMeasurement;
// 
// 	/// Control rates
// 	// Signals: [dur1, dur2, dur3]
// 	OutputPort< vector< double > > portControlRates;
// 	vector< double > controlRates;
// 
// 	/// KKT tolerance
// 	OutputPort< double > portKKTTolerance;
// 	double kktTolerance;
// 
// 	/// Objective value
// 	OutputPort< double > portObjectiveValue;
// 	double objectiveValue;
// 
// 	/// Number of active set changes, returned by QP solver
// 	OutputPort< int > portNumOfActiveSetChanges;
// 	int numOfActiveSetChanges;
// 
// 	/// Execution times
// 	OutputPort< double > portPreparationPhaseExecTime;
// 	OutputPort< double > portFeedbackPhaseExecTime;
// 	OutputPort< double > portExecutionTime;
// 	RTT::os::TimeService::ticks tickPreparationPhaseBegin;
// 	RTT::os::TimeService::ticks tickFeedbackPhaseBegin;
// 	RTT::os::TimeService::ticks tickMPCBegin;
// 	double timePrepPhase;
// 	double timeFdbPhase;
// 	double timeMPC;
// 
// 	/// Status of the QP solver
// 	OutputPort< int > portQPSolverStatus;
// 	/// Status of the QP solver
// 	int qpSolverStatus;
// 
// 	OutputPort< bool > portDataSizeValid;
// 	/// Data sizes are valid
// 	bool dataSizeValid;
// 
// 	//
// 	// Properties
// 	//
// 
// 	/// Number of SQP iterations
// 	unsigned numSQPIterations;
// 	unsigned sqpIterationsCounter;
// 
// 	// File names for default references and weighting matrices
// 	string refDefaultFileName;
// 	string referencesFileName;
// 	string fileNameWeightsQ;
// 	string fileNameWeightsR;
// 	string fileNameWeightsQF;

	//
	// TODO Logging
	//

	/// Our logging category
//	OCL::logging::Category* logger;

};

#endif // SIMPLE_TRAJECTORY_GENERATOR_HPP
