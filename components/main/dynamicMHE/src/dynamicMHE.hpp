#ifndef DYNAMIC_MHE_HPP
#define DYNAMIC_MHE_HPP

#include <rtt/TaskContext.hpp>
#include <rtt/RTT.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>
#include <rtt/Component.hpp>

#include <rtt/base/DataObjectInterface.hpp>

#include <rtt/os/TimeService.hpp>

//#include <ocl/LoggingService.hpp>
//#include <ocl/Category.hpp>

#include "acado.h"

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

#define NX		ACADO_NX	// number of differential states
#define NU		ACADO_NU	// number of control inputs
#define NP		ACADO_NP	// number of user parameters
#define NY		ACADO_NY	// number of measurements
#define NYN		ACADO_NYN	// number of measurements, last node
#define N 		ACADO_N		// number of estimation intervals

#define NY_POSE	12		// # of measurements from pose component / or markers component; in the later case it is number of markers
#define NY_MARK NY_POSE

#define NY_IMU	6		// # of IMU measurements
#define NY_ENC	2		// # of encoder measurements, angle true, ang. velocity fake, some averaging; vel. used only for initialization
#define NY_CTRL	3		// # of control measurements; ur1, ur2 and up

#define POSE_VEC_SIZE	(2 * NY_POSE)	// Samples + Cov. matrix diag. elements
#define IMU_VEC_SIZE	(4 * NY_IMU)	// Samples + Cov. matrix diag. elements

#define DELAY_POSE		2 // samples
#define	DELAY_IMU		1 // samples

//
// Initialization
//
// 1: Initialize with single shootin'
// 2: Put the same data to all nodes
#define INIT_VER	1

#define DEEP_DEBUG	0 // Dumping MHE data on ports and so...

#define SCALE_OBJ	0.01 	// Scaling of the objective

#define SCALE_UR	1.25e6	// Scaling of ur_1, ur_2
#define SCALE_UP	2e5		// Scaling of u_p

class DynamicMHE
	: public TaskContext
{
public:
	DynamicMHE(const std::string& name);
	
	virtual ~DynamicMHE( )
	{}
	
	virtual bool configureHook( );
	
	virtual bool startHook( );
	
	virtual void updateHook( );
	
	virtual void stopHook( );
	
	virtual void cleanupHook( );
	
	virtual void errorHook();

	void mheFeedbackPhase( );
	
	void mhePreparationPhase( );
	
private:
	
	bool prepareMeasurements( void );

	/// Indicator whether the buffer of first N measurements is ready
	bool initialized;
	
	/// Needed for initialization
	unsigned sampleCounter;
	
	/// Last measurement vector
//	double lastMeasurementVector[ NY ];

	/// Contains last NYN measurements, dependent only on states; this one goes
	/// directly to the feedback function
	double lastMeasurementForMHE[ NYN ];

	double initPose[ NY_POSE ];

	double initDdelta;

	vector< double > oldMeasurementsCtrl;

	bool statusMeasurements;

	//
	// Input ports and their associated variables
	//
	
	/// 12 Markers + 12 cov. diag. matrix elements
	InputPort< vector< double > > portMeasurementsMarkers;
	vector< double > measurementsMarkers;
	FlowStatus statusMeasurementsMarkers;
	
	/// Measurements: x, y z, e11, ...,e33 + covariance diag elements matrix
	InputPort< vector< double > > portMeasurementsPose;
	vector< double > measurementsPose;
	FlowStatus statusMeasurementsPose;

	/// Measurements: wx, wy, wz, ax, ay, az + covariance matrix
	InputPort< vector< double > > portMeasurementsIMU;
	vector< double > measurementsIMU;
	FlowStatus statusMeasurementsIMU;

	/// Measurements: delta, ddelta
	InputPort< vector< double > > portMeasurementsEncoder;
	vector< double > measurementsEncoder;
	FlowStatus statusMeasurementsEncoder;

	/// Measurements: ur, up
	InputPort< vector< double > > portMeasurementsCtrl;
	vector< double > measurementsCtrl;
	FlowStatus statusMeasurementsCtrl;

	/// Measurements: dur, dup
	InputPort< vector< double > > portMeasurementsCtrlRates;
	vector< double > measurementsCtrlRates;
	FlowStatus statusMeasurementsCtrlRates;

	//
	// Output ports and their associated variables
	//
	
	/// ready flag
	OutputPort< bool > portReady;
	bool ready;
	
	/// state estimates
	OutputPort< vector< double > > portStateEstimate;
	vector< double > stateEstimate;
	
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
	RTT::os::TimeService::ticks tickMHEBegin;
	double timePrepPhase;
	double timeFdbPhase;
	double timeMHE;

	/// Status of the QP solver
	OutputPort< int > portQPSolverStatus;
	/// Status of the QP solver
	int qpSolverStatus;

	OutputPort< bool > portDataSizeValid;
	/// Data sizes are valid
	bool dataSizeValid;

	OutputPort< vector< double > > portOneStepPrediction;
	vector< double > oneStepPrediction;

	// Ports for reporting everything on the same port

	OutputPort<vector<double> > portStateAndControl;
	vector<double> StateAndControl;

	vector<double> statePredicted;

	OutputPort< vector< double > > portFullStateVector;
	vector< double > fullStateVector;
	
	OutputPort< vector< double > > portFullControlVector;
	vector< double > fullControlVector;

	// Inputport for the reference of the controller. For reporting purposes only.
	InputPort<vector<double> > portStateReference;
	vector<double> StateReference; 

	//
	// Properties
	//

	/// Number of SQP iterations
	unsigned numSQPIterations;
	unsigned sqpIterationsCounter;

	//
	// TODO Logging
	//

	/// Our logging category
//	OCL::logging::Category* logger;

	//
	// Deep debug stuff
	//
#if DEEP_DEBUG

	OutputPort< vector< double > > portWeightingCoefficients;
	vector< double > weightingCoefficients;

#endif // DEEP_DEBUG
};

#endif // DYNAMIC_MHE_HPP
