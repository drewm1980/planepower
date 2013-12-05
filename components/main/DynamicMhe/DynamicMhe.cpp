#include "DynamicMhe.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <algorithm>
#include <cmath>

using namespace std;
using namespace RTT;
using namespace RTT::os;

/// Size of the IMU data buffer size
#define MAX_NUM_IMU_SAMPLES 100

/// Check port connection
#define checkPortConnection( port ) \
	if (port.connected() == false) \
	{log( Error ) << "Port " << port.getName() << " is not connected." << endlog(); return false;}

DynamicMhe::DynamicMhe(std::string name)
	: TaskContext(name, PreOperational)
{
	addEventPort("trigger", portTrigger)
		.doc("Trigger port");
	addPort("mcuData", portMcuHandlerData)
		.doc("MCU handler inputs");
	addPort("encoderData",portEncoderData)
		.doc("Encoder data");
	addPort("ledTrackerData", portLEDTrackerData)
		.doc("LED Tracker data");
	addPort("lasData", portLASData)
		.doc("Line angle sensor data");
	addPort("stateEstimate", portStateEstimate)
		.doc("Current state estimate");
	addPort("debugData", portDebugData)
		.doc("Debugging data");

	stateEstimate.x_hat.resize(NX, 0);

	portStateEstimate.setDataSample( stateEstimate );
	portStateEstimate.write( stateEstimate  );

	debugData.x.resize((N + 1) * NX, 0.0);
	debugData.u.resize(N * NU, 0.0);
	debugData.z.resize(N * NXA, 0.0);
	debugData.y.resize(N * NY, 0.0);
	debugData.yN.resize(NYN, 0.0);
//	debugData.S.resize(N * NY * NY, 0.0);
//	debugData.SN.resize(NYN * NYN, 0.0);

	debugData.imu_first.resize(6, 0.0);
	debugData.imu_avg.resize(6, 0.0);
	debugData.enc_data.resize(3, 0.0);
	debugData.cam_markers.resize(12, 0.0);
	debugData.cam_pose.resize(12, 0.0);
	debugData.las_data.resize(2, 0.0);
	debugData.controls_avg.resize(3, 0.0);

	portDebugData.setDataSample( debugData );
	portDebugData.write( debugData );

	imuData.resize( MAX_NUM_IMU_SAMPLES );

	targetCableLength = 0;
	addProperty("targetCableLength", targetCableLength);
}

bool DynamicMhe::configureHook()
{
	if (targetCableLength == 0)
	{
		log( Error ) << "Target cable length must be greater than 0!!!" << endlog();
		return false;
	}

	checkPortConnection( portTrigger );
	checkPortConnection( portMcuHandlerData );
	checkPortConnection( portEncoderData );
	checkPortConnection( portLEDTrackerData );
	checkPortConnection( portLASData );
	
	return true;
}

bool DynamicMhe::startHook()
{
	// Clean the ACADO solver structures
	memset(&acadoWorkspace, 0, sizeof( acadoWorkspace ));
	memset(&acadoVariables, 0, sizeof( acadoVariables ));

	// NOTE: Code below cleans everything from the solver!!!
	initializeSolver();

	//
	// Configure weights, ATM they are fixed, in a header file,
	// which gets exported from rawesome
	//
	
	prepareWeights();

	runCnt = 0;

	return true;
}

void DynamicMhe::updateHook()
{
	TIME_TYPE prepStart, fdbStart;

	debugData.ts_entry = stateEstimate.ts_entry = TimeService::Instance()->getTicks();
	
	// Read the trigger port
	TIME_TYPE trigger;
	portTrigger.read( trigger );
	debugData.ts_trigger = stateEstimate.ts_trigger = trigger;
	
	// Read and prepare sensor data
	if (prepareMeasurements() == false)
		exception();
	
	//
	// MHE "state-machine"
	//
	
	bool runMhe = false;
	
	if (runCnt < N)
	{
		for (unsigned i = 0; i < NY; ++i)
			acadoVariables.y[runCnt * NY + i] = execY[ i ];

		int ledInd = runCnt - mhe_ndelay;
		if (ledInd >= 0)
		{
			for (unsigned i = 0; i < mhe_num_markers; ++i)
				acadoVariables.y[ledInd * NY + i] = ledData[ i ];
			for (unsigned i = 0; i < mhe_num_markers; ++i)
				acadoVariables.S[ledInd * NY * NY + i * NY + i] = ledWeights[ i ];
		}

		if (++runCnt == N)
		{
			// Prepare initial guess
			prepareInitialGuess();
			// Run the preparation step
			preparationStep();
		}

		runMhe = false;
		stateEstimate.ready = debugData.ready = false;
	}
	else if (runCnt >= N)
	{
		// Put new measurements
		for (unsigned i = 0; i < NYN; ++i)
			acadoVariables.yN[ i ] = execYN[ i ];

		int ledInd = N - mhe_ndelay;
		for (unsigned i = 0; i < mhe_num_markers; ++i)
			acadoVariables.y[ledInd * NY + i] = ledData[ i ];
		for (unsigned i = 0; i < mhe_num_markers; ++i)
			acadoVariables.S[ledInd * NY * NY + i * NY + i] = ledWeights[ i ];
		
		runMhe = true;
	}
	
	//
	// Run the MHE
	//

	fdbStart = TimeService::Instance()->getTicks();
	int mheStatus = 0;
	if (runMhe == true)
	{
		//
		// Execute an RTI
		//
		
		// Execute feedback step of the RTI scheme
		mheStatus = feedbackStep();

		stateEstimate.ready = debugData.ready = mheStatus ? false : true;
	
		debugData.solver_status = mheStatus;
		debugData.kkt_value = getKKT();
		debugData.obj_value = getObjective();
		debugData.n_asc = getNWSR();
		
		// Copy the current state estimate to the output port
		for (unsigned i = 0; i < NX; ++i)
			stateEstimate.x_hat[ i ] = acadoVariables.x[N * NX + i];

		//
		// Output the current state estimate
		//
		stateEstimate.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
		portStateEstimate.write( stateEstimate );
	}
	debugData.exec_fdb = TimeService::Instance()->secondsSince( fdbStart );

	//
	// Copy all debug data from the current optimization step to the debug port
	//
	prepareDebugData();

	//
	// Prepare solver for the next time step
	//
	prepStart = TimeService::Instance()->getTicks();
	if (runMhe == true)
	{
		// Shift weighting matrices
		for (unsigned blk = 0; blk < N - 1; ++blk)
			for (unsigned el = 0; el < NY; ++ el)
				acadoVariables.S[blk * NY * NY + el * NY + el] = 
					acadoVariables.S[(blk + 1) * NY * NY + el * NY + el];
		
		// Shift measurements
		for (unsigned blk = 0; blk < N - 1; ++blk)
			for (unsigned el = 0; el < NY; ++ el)
				acadoVariables.y[blk * NY + el] = acadoVariables.y[(blk + 1) * NY + el];
		for (unsigned el = 0; el < NYN; ++ el)
			acadoVariables.y[(N - 1) * NY + el] = acadoVariables.yN[ el ];
		
		// Shift MHE states and controls
		shiftStates(2, 0, 0);
		shiftControls( 0 );

		// Execute preparation step of the RTI scheme
		preparationStep();
	}
	debugData.exec_prep = TimeService::Instance()->secondsSince( prepStart );

	//
	// Output debug data to the port
	//
	debugData.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDebugData.write( debugData );

	if ( mheStatus )
		exception();
}

void DynamicMhe::stopHook()
{}

void DynamicMhe::cleanupHook()
{}

void DynamicMhe::errorHook()
{}

void DynamicMhe::exceptionHook()
{
	log( Error ) << "Exception happened..." << endlog();
	// Try to recover the component ...
	recover();
}

bool DynamicMhe::prepareMeasurements( void )
{	
	//
	// Read sensor ports
	//
	
	// It is assumed that this port will be buffered
	unsigned numImuSamples = 0;

	while ((numImuSamples < MAX_NUM_IMU_SAMPLES)) && (portMcuHandlerData.read( imuData[ numImuSamples ] ) == NewData))
		numImuSamples++;

	FlowStatus encStatus = portEncoderData.read( encData );

	FlowStatus camStatus = portLEDTrackerData.read( camData );
	unsigned numMarkers = 0;
	if (camStatus == NewData)
		for (unsigned i = 0; i < camData.weights.size(); ++i)
		{
			if (camData.weights[ i ] > 0.0)
			{
				++numMarkers;
				ledData[ i ] = camData.positions[ i ];

				// We do not use weights from Andrew, but
				// from rawesome simulation...
				//ledWeights[ i ] = camData.weights[ i ];
				ledWeights[ i ] = weight_marker_positions;
			}
			else
			{
				ledData[ i ] = ledWeights[ i ] = 0.0;
			}
		}
	else
		for (unsigned i = 0; i < camData.weights.size(); ++i)
			ledData[ i ] = ledWeights[ i ] = 0.0;
	
	FlowStatus lasStatus = portLASData.read( lasData );

	//
	// Average data that come from MCU handler component
	//
	debugData.imu_first[ 0 ] = imuData[numImuSamples - 1].gyro_x;
	debugData.imu_first[ 1 ] = imuData[numImuSamples - 1].gyro_y;
	debugData.imu_first[ 2 ] = imuData[numImuSamples - 1].gyro_z;
	debugData.imu_first[ 3 ] = imuData[numImuSamples - 1].accl_x;
	debugData.imu_first[ 4 ] = imuData[numImuSamples - 1].accl_y;
	debugData.imu_first[ 5 ] = imuData[numImuSamples - 1].accl_z;

	for (unsigned i = 0; i < debugData.imu_avg.size(); ++i)
		debugData.imu_avg[ i ] = 0.0;
	for (unsigned i = 0; i < debugData.controls_avg.size(); ++i)
		debugData.controls_avg[ i ] = 0.0;
	for (unsigned i = 0; i < numImuSamples; ++i)
	{
		debugData.imu_avg[ 0 ] += imuData[ i ].gyro_x;
		debugData.imu_avg[ 1 ] += imuData[ i ].gyro_y;
		debugData.imu_avg[ 2 ] += imuData[ i ].gyro_z;
		debugData.imu_avg[ 3 ] += imuData[ i ].accl_x;
		debugData.imu_avg[ 4 ] += imuData[ i ].accl_y;
		debugData.imu_avg[ 5 ] += imuData[ i ].accl_z;

		debugData.controls_avg[ 0 ] += imuData[ i ].ua1;
		debugData.controls_avg[ 1 ] += imuData[ i ].ua2;
		debugData.controls_avg[ 2 ] += imuData[ i ].ue;
	}
	for (unsigned i = 0; i < debugData.imu_avg.size(); ++i)
		debugData.imu_avg[ i ] /= (double)numImuSamples;
	for (unsigned i = 0; i < debugData.controls_avg.size(); ++i)
		debugData.controls_avg[ i ] /= (double)numImuSamples;
	
	debugData.num_imu_samples = numImuSamples;
	debugData.num_enc_samples = (encStatus == NewData);
	debugData.num_cam_samples = (camStatus == NewData);
	debugData.num_las_samples = (lasStatus == NewData);
	
	//
	// Prepare the sensor data
	//
	
	unsigned offset = mhe_num_markers;
	execY[ offset++ ] = encData.cos_theta;
	execY[ offset++ ] = -encData.sin_theta; // !!! Sign is inverted !!!
	// gyro_xyz, accl_xyz
	for (unsigned i = 0; i < 6; ++i)
		execY[ offset++ ] = debugData.imu_avg[ i ];

	// Control surfaces; TODO we might skip this!
	execY[ offset++ ] = debugData.controls_avg[ 0 ]; // TODO check signs
	execY[ offset++ ] = debugData.controls_avg[ 2 ]; // TODO check signs
	
	// r, dr, ddr
	execY[ offset++ ] = targetCableLength;
	execY[ offset++ ] = 0.0;
	execY[ offset++ ] = 0.0;
	
	// controls
	for (unsigned i = 0; i < NU; ++i)
		execY[ offset++ ] = 0.0;

	// Copy to execYN
	for (unsigned i = 0; i < NYN; execYN[ i ] = execY[ i ], i++);

	//
	// Prepare debug data, delays
	// XXX This is not 100% safe way...
	//
	debugData.dbg_cam_delay =
		(int)round((debugData.ts_entry - camData.ts_trigger) * 1e-9 / mhe_sampling_time) * 
		debugData.num_cam_samples;
	debugData.dbg_imu_delay =
		(int)round((debugData.ts_entry - imuData[numImuSamples - 1].ts_trigger)  * 1e-9 / mhe_sampling_time) *
		debugData.num_imu_samples;
	debugData.dbg_enc_delay =
		(int)round((debugData.ts_entry - encData.ts_trigger)  * 1e-9 / mhe_sampling_time) *
		debugData.num_enc_samples;

	return true;
}

bool DynamicMhe::prepareDebugData( void )
{
	debugData.x.assign(acadoVariables.x, acadoVariables.x + (N + 1) * NX);
	debugData.u.assign(acadoVariables.u, acadoVariables.u + N * NU);
	debugData.z.assign(acadoVariables.z, acadoVariables.z + N * NXA);
	debugData.y.assign(acadoVariables.y, acadoVariables.y + N * NY);
	debugData.yN.assign(acadoVariables.yN, acadoVariables.yN + NYN);
//	debugData.S.assign(acadoVariables.S, acadoVariables.S + N * NY * NY);
//	debugData.SN.assign(acadoVariables.SN, acadoVariables.SN + NYN * NYN);

	debugData.enc_data[ 0 ] = encData.theta;
	debugData.enc_data[ 1 ] = encData.sin_theta;
	debugData.enc_data[ 2 ] = encData.cos_theta;

	copy(camData.positions.begin(), camData.positions.end(), debugData.cam_markers.begin());
	copy(camData.pose.begin(), camData.pose.end(), debugData.cam_pose.begin());

	debugData.las_data[ 0 ] = lasData.angle_hor;
	debugData.las_data[ 1 ] = lasData.angle_ver;

	return true;
}

bool DynamicMhe::prepareWeights( void )
{
	// XXX Weights for markers are done in a different way, by dflt = 0.0

	for (unsigned el = 0; el < NY; mheWeights[ el++ ] = 0.0);

	unsigned offset = mhe_num_markers;
	mheWeights[ offset++ ] = weight_cos_delta;
	mheWeights[ offset++ ] = weight_sin_delta;
	for (unsigned el = 0; el < 3; mheWeights[ offset++ ] = weight_IMU_angular_velocity, el++);
	for (unsigned el = 0; el < 3; mheWeights[ offset++ ] = weight_IMU_acceleration, el++);

	mheWeights[ offset++ ] = weight_aileron;
	mheWeights[ offset++ ] = weight_elevator;

	mheWeights[ offset++ ] = weight_r;
	mheWeights[ offset++ ] = weight_dr;
	mheWeights[ offset++ ] = weight_ddr;

	mheWeights[ offset++ ] = weight_daileron;
	mheWeights[ offset++ ] = weight_delevator;

	mheWeights[ offset++ ] = weight_dmotor_torque;
	mheWeights[ offset++ ] = weight_dddr;

	// Here we setup dflt values for weighting matrices
	for (unsigned blk = 0; blk < N; ++blk)
		for (unsigned el = 0; el < NY; ++el)
			acadoVariables.S[blk * NY * NY + el * NY + el] = mheWeights[ el ];

	for (unsigned el = 0; el < NYN; ++el)
		acadoVariables.SN[el * NYN + el] = mheWeights[ el ];

	// XXX A bit off topic, but must be done somewhere :p
	for (unsigned el = 0; el < NY; execY[ el++ ] = 0.0);
	for (unsigned el = 0; el < NYN; execYN[ el++ ] = 0.0);

	return true;
}

bool DynamicMhe::prepareInitialGuess( void )
{
	//
	// Initialize differential variables
	// NOTE Must be called AFTER y is full and BEFORE we get the first yN
	//

	// Last two states are cos_delta and sin_delta which have to be initialized
	// from measurements
	for (unsigned blk = 0; blk < N + 1; ++blk)
		for (unsigned el = 0; el < NX; ++el)
			acadoVariables.x[blk * NX + el] = ss_x[ el ];

	// Initialize cos_delta and sin_delta from measurements:

	// Nodes 0.. N - 1
	for (unsigned blk = 0; blk < N; ++blk)
		for (unsigned el = NX - 2, yIt = mhe_num_markers; el < NX; ++el, ++yIt)
			acadoVariables.x[blk * NX + el] = acadoVariables.y[blk * NY + yIt];

	// Node N
	// Take cos_delta and sin_delta from the last row from y and based on the 
	// steady state speed construct initial guess for the last node
	
	double angle = atan2(acadoVariables.x[(N - 1) * NX + idx_sin_delta],
						 acadoVariables.x[(N - 1) * NX + idx_cos_delta]);
	angle += ss_x[ idx_ddelta ] * mhe_sampling_time;
	acadoVariables.x[N * NX + idx_cos_delta] = cos( angle );
	acadoVariables.x[N * NX + idx_sin_delta] = sin( angle );

//	// Last node must be initialized from yN
//	for (unsigned el = NX - 2, yIt = mhe_num_markers; el < NX; ++el, ++yIt)
//		acadoVariables.x[N * NX + el] = acadoVariables.yN[ yIt ];

	//
	// Initialize algebraic variables
	//
	for (unsigned blk = 0; blk < N; ++blk)
		for (unsigned el = 0; el < NXA; ++el)
			acadoVariables.z[blk * NXA + el] = ss_z[ el ];
	
	//
	// Initialize control variables
	//
	for (unsigned blk = 0; blk < N; ++blk)
		for (unsigned el = 0; el < NU; ++el)
			acadoVariables.u[blk * NU + el] = ss_u[ el ];

	return true;
}

ORO_CREATE_COMPONENT( DynamicMhe )
