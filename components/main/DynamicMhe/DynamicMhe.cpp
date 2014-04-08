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
/// Maxumum number of failures when we start the MHE
#define MAX_NUM_FAILURES (2 * ACADO_N)

/// Check port connection
#define checkPortConnection( port ) \
	if (port.connected() == false) \
	{log( Error ) << "Port " << port.getName() << " is not connected." << endlog(); return false;}

/// For internal use
enum DynamicMheErrorCodes
{
	ERR_OK = 0,
	ERR_NANS,
	ERR_QP_STATUS,
	ERR_MEAS_FAILED
};

static bool isNaN(double* array, unsigned dim)
{
	for (unsigned i = 0; i < dim; ++i)
		if ((array[ i ] != array[ i ]) || (array[ i ] < -1e12) || (array[ i ] > 1e12))
			return true;

	return false;	
}

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
	addPort("winchData", portWinchData)
		.doc("Winch data");
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
//	debugData.W.resize(N * NY * NY, 0.0);
//	debugData.WN.resize(NYN * NYN, 0.0);

	debugData.imu_first.resize(6, 0.0);
	debugData.imu_avg.resize(6, 0.0);
	debugData.enc_data.resize(3, 0.0);
	debugData.cam_markers.resize(12, 0.0);
	debugData.cam_pose.resize(12, 0.0);
	debugData.las_data.resize(2, 0.0);
	debugData.winch_data.resize(3, 0.0);
	debugData.controls_avg.resize(3, 0.0);

	portDebugData.setDataSample( debugData );
	portDebugData.write( debugData );

	imuData.resize( MAX_NUM_IMU_SAMPLES );
}

bool DynamicMhe::configureHook()
{
	checkPortConnection( portTrigger );
	checkPortConnection( portMcuHandlerData );
	checkPortConnection( portEncoderData );
	checkPortConnection( portLEDTrackerData );
	checkPortConnection( portLASData );
	checkPortConnection( portWinchData );
	
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

	numOfFailures = 0;

	return true;
}

void DynamicMhe::updateHook()
{
	TIME_TYPE prepStart, fdbStart;
	bool runMhe = false;

	debugData.ts_entry = TimeService::Instance()->getTicks();
	
	// Reset the error codes
	errorCode = ERR_OK;
	mheStatus = 0;

	// Read the trigger port
	TIME_TYPE trigger;
	portTrigger.read( trigger );
	debugData.ts_trigger = stateEstimate.ts_trigger = trigger;
	
	// Read and prepare sensor data
	if (prepareMeasurements() == false)
	{
		if (runCnt > 0 and ++numOfFailures > MAX_NUM_FAILURES)
		{
			errorCode = ERR_MEAS_FAILED;
			stop();
		}
		else
			goto DynamicMheUpdateHookExit;
	}
	
	//
	// MHE "state-machine"
	//
	
	if (runCnt < N)
	{
		for (unsigned i = 0; i < NY; ++i)
			acadoVariables.y[runCnt * NY + i] = execY[ i ];

		int ledInd = runCnt - mhe_ndelay;
		if (ledInd >= 0)
		{
			for (unsigned i = 0, el = offset_marker_positions; i < mhe_num_markers; ++i, ++el)
				acadoVariables.y[ledInd * NY + el] = ledData[ i ];
			for (unsigned i = 0, el = offset_marker_positions; i < mhe_num_markers; ++i, ++el)
				acadoVariables.W[ledInd * NY * NY + el * NY + el] = ledWeights[ i ];
		}

		int cableInd = runCnt - debugData.dbg_winch_delay;
		if (cableInd >= 0)
		{
			acadoVariables.y[cableInd * NY + offset_r] = cableLength;
			acadoVariables.W[cableInd * NY * NY + offset_r * NY + offset_r] = cableLengthWeight;
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
		// Embed new measurements, the "default ones"
		for (unsigned i = 0; i < NYN; ++i)
			acadoVariables.yN[ i ] = execYN[ i ];

		// Embed marker positions
		// TODO In principle, we do not need mhe_ndelay, it is calculated
		int ledInd = N - mhe_ndelay;
		for (unsigned i = 0, el = offset_marker_positions; i < mhe_num_markers; ++i, ++el)
			acadoVariables.y[ledInd * NY + el] = ledData[ i ];
		for (unsigned i = 0, el = offset_marker_positions; i < mhe_num_markers; ++i, ++el)
			acadoVariables.W[ledInd * NY * NY + el * NY + el] = ledWeights[ i ];

		// Embed winch measurement
		int cableInd = N - debugData.dbg_winch_delay;
		if (cableInd < N)
		{
			acadoVariables.y[cableInd * NY + offset_r] = cableLength;
			acadoVariables.W[cableInd * NY * NY + offset_r * NY + offset_r] = cableLengthWeight;
			acadoVariables.WN[offset_r * NYN + offset_r] = 0.0;
		}
		else
		{
			acadoVariables.yN[ offset_r ] = cableLength;
			acadoVariables.WN[offset_r * NYN + offset_r] = cableLengthWeight;
		}
		
		runMhe = true;
	}
	
	//
	// Run the MHE
	//

	fdbStart = TimeService::Instance()->getTicks();
	mheStatus = 0;
	if (runMhe == true)
	{
		//
		// Execute an RTI
		//
		
		// Execute feedback step of the RTI scheme
		mheStatus = feedbackStep();

		if (isNaN(acadoVariables.x, NX * (N + 1)) == true || isNaN(acadoVariables.u, NU * N) == true)
		{
			mheStatus = -100;
			errorCode = ERR_NANS;
		}

		stateEstimate.ready = debugData.ready = mheStatus ? false : true;
			
		// Copy the current state estimate to the output port
		for (unsigned i = 0; i < NX; ++i)
			stateEstimate.x_hat[ i ] = acadoVariables.x[N * NX + i];

		//
		// Output the current state estimate
		//
		stateEstimate.ts_trigger = TimeService::Instance()->getTicks();
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
				acadoVariables.W[blk * NY * NY + el * NY + el] = 
					acadoVariables.W[(blk + 1) * NY * NY + el * NY + el];

		for (unsigned el = 0; el < NYN; ++el)
			acadoVariables.W[(N - 1) * NY * NY + el * NY + el] =
				acadoVariables.WN[el * NYN + el];
		
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

DynamicMheUpdateHookExit:

	//
	// Output debug data to the port
	//
	debugData.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDebugData.write( debugData );

	if (mheStatus and errorCode == ERR_OK)
		errorCode = ERR_QP_STATUS;
	if (errorCode != ERR_OK)
		stop();
}

void DynamicMhe::stopHook()
{
	Logger::In in( getName() );

	switch( errorCode )
	{
	case ERR_OK:
		log( Info ) << "Successfully stopped." << endlog();
		break;
	case ERR_NANS:
		log( Error ) << "Runtime counter " << runCnt
					 << ": NANs detected in the estimator structures." << endlog();
		break;
	case ERR_QP_STATUS:
		log( Error ) << "Runtime counter " << runCnt
					 << ": mHE returned error code " << mheStatus << "." << endlog();
		break;
	case ERR_MEAS_FAILED:
		log( Error ) << "Runtime counter " << runCnt
					 << ": gathering of measurements failed." << endlog();
		break;
	default:
		log( Error ) << "Runtime counter " << runCnt
					 << ": unknown error." << endlog();
	}
}

void DynamicMhe::cleanupHook()
{}

void DynamicMhe::errorHook()
{}

void DynamicMhe::exceptionHook()
{
	Logger::In in( getName() );
	
	log( Error ) << "Unknown exception happened..." << endlog();
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

	while ((numImuSamples < MAX_NUM_IMU_SAMPLES) && (portMcuHandlerData.read( imuData[ numImuSamples ] ) == NewData))
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

	FlowStatus winchStatus = portWinchData.read( winchData );
	cableLength = winchData.length * (double)(winchStatus == NewData);
	cableLengthWeight = (winchStatus == NewData) ? weight_r : 0.0;
	
	debugData.winch_data[ 0 ] = cableLength;

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
	
	double prevTimestamp = debugData.ts_trigger - mhe_sampling_time * 1e9;
	unsigned numImuSamplesReal = 0;
	for (unsigned i = 0; i < numImuSamples; ++i)
	{
		// There can be some garbage in the buffer
		if (imuData[ i ].ts_trigger < prevTimestamp)
			continue;

		debugData.imu_avg[ 0 ] += imuData[ i ].gyro_x;
		debugData.imu_avg[ 1 ] += imuData[ i ].gyro_y;
		debugData.imu_avg[ 2 ] += imuData[ i ].gyro_z;
		debugData.imu_avg[ 3 ] += imuData[ i ].accl_x;
		debugData.imu_avg[ 4 ] += imuData[ i ].accl_y;
		debugData.imu_avg[ 5 ] += imuData[ i ].accl_z;

		debugData.controls_avg[ 0 ] += imuData[ i ].ctrl.ua1;
		debugData.controls_avg[ 1 ] += imuData[ i ].ctrl.ua2;
		debugData.controls_avg[ 2 ] += imuData[ i ].ctrl.ue;

		++numImuSamplesReal;
	}
	for (unsigned i = 0; i < debugData.imu_avg.size(); ++i)
		debugData.imu_avg[ i ] /= (double)numImuSamplesReal;
	for (unsigned i = 0; i < debugData.controls_avg.size(); ++i)
		debugData.controls_avg[ i ] /= (double)numImuSamplesReal;

	//
	// Some debug stuff, record number of measurements we got
	//
	
	debugData.num_imu_samples = numImuSamplesReal;
	debugData.num_enc_samples = (encStatus == NewData);
	debugData.num_cam_samples = (camStatus == NewData);
	debugData.num_las_samples = (lasStatus == NewData);
	debugData.num_winch_samples = (winchStatus == NewData);

	//
	// If we got no measurements from the IMU, abort
	//
	if (numImuSamplesReal == 0)
		return false;
	
	//
	// Prepare the sensor data
	//
	
	execY[ offset_cos_delta ] = encData.cos_theta;
	execY[ offset_sin_delta ] = -encData.sin_theta; // !!! Sign is inverted !!!
	// gyro_xyz, accl_xyz
	for (unsigned i = 0, el = offset_IMU_angular_velocity; i < 3; ++i, ++el)
		execY[ el ] = debugData.imu_avg[ i ];
	for (unsigned i = 3, el = offset_IMU_acceleration; i < 6; ++i, ++el)
		execY[ el ] = debugData.imu_avg[ i ];

	// Control surfaces; TODO we might skip this!
	execY[ offset_aileron  ] = debugData.controls_avg[ 0 ]; // TODO check signs
	execY[ offset_elevator ] = debugData.controls_avg[ 2 ]; // TODO check signs
	
	// r, dr, ddr
	// Measurement for cable length "r" is embedded the same way we do
	// for cameras, calculating the delays...
	execY[ offset_r ] = 0.0;
	execY[ offset_dr ] = 0.0;
	execY[ offset_ddr ] = 0.0;
	
	// controls
	execY[ offset_daileron ] = 0.0;
	execY[ offset_delevator ] = 0.0;
	execY[ offset_dmotor_torque ] = 0.0;
	execY[ offset_dddr ] = 0.0;

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

	debugData.dbg_winch_delay =
		(int)round((debugData.ts_entry - winchData.ts_trigger)  * 1e-9 / mhe_sampling_time) *
		debugData.num_winch_samples;

	return true;
}

bool DynamicMhe::prepareDebugData( void )
{
	debugData.solver_status = mheStatus;
	debugData.kkt_value     = getKKT();
	debugData.obj_value     = getObjective();
	debugData.n_asc         = getNWSR();

	debugData.x.assign(acadoVariables.x, acadoVariables.x + (N + 1) * NX);
	debugData.u.assign(acadoVariables.u, acadoVariables.u + N * NU);
	debugData.z.assign(acadoVariables.z, acadoVariables.z + N * NXA);
	debugData.y.assign(acadoVariables.y, acadoVariables.y + N * NY);
	debugData.yN.assign(acadoVariables.yN, acadoVariables.yN + NYN);
//	debugData.S.assign(acadoVariables.W, acadoVariables.W + N * NY * NY);
//	debugData.SN.assign(acadoVariables.WN, acadoVariables.WN + NYN * NYN);

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
	// TODO Now that we generate offsets, remove offset inc stuff

	for (unsigned el = 0; el < NY; mheWeights[ el++ ] = 0.0);

	mheWeights[ offset_cos_delta ] = weight_cos_delta;
	mheWeights[ offset_sin_delta ] = weight_sin_delta;
	for (unsigned el = offset_IMU_angular_velocity; el < offset_IMU_angular_velocity + 3; mheWeights[ el++ ] = weight_IMU_angular_velocity);
	for (unsigned el = offset_IMU_acceleration; el < offset_IMU_acceleration + 3; mheWeights[ el++ ] = weight_IMU_acceleration);

	mheWeights[ offset_aileron ] = weight_aileron;
	mheWeights[ offset_elevator ] = weight_elevator;

	// Weight for cable length is dynamically updated
	mheWeights[ offset_r ] = 0.0;
	mheWeights[ offset_dr ] = weight_dr;
	mheWeights[ offset_ddr ] = weight_ddr;

	mheWeights[ offset_daileron ] = weight_daileron;
	mheWeights[ offset_delevator ] = weight_delevator;

	mheWeights[ offset_dmotor_torque ] = weight_dmotor_torque;
	mheWeights[ offset_dddr ] = weight_dddr;

	mheWeights[ offset_dt1_disturbance ] = weight_dt1_disturbance;
	mheWeights[ offset_dt2_disturbance ] = weight_dt2_disturbance;
	mheWeights[ offset_dt3_disturbance ] = weight_dt3_disturbance;

	// Here we setup dflt values for weighting matrices
	for (unsigned blk = 0; blk < N; ++blk)
		for (unsigned el = 0; el < NY; ++el)
			acadoVariables.W[blk * NY * NY + el * NY + el] = mheWeights[ el ];

	for (unsigned el = 0; el < NYN; ++el)
		acadoVariables.WN[el * NYN + el] = mheWeights[ el ];

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

	// Initialize cos_delta and sin_delta by simulation from the first
	// measurement using precomputed steady-state speed
	double ddelta_ss = ss_x[ idx_ddelta ];
	double angle = atan2(acadoVariables.y[ offset_sin_delta ], acadoVariables.y[ offset_cos_delta ]);

	for (unsigned blk = 0; blk < N + 1; ++blk)
	{
		acadoVariables.x[blk * NX + idx_cos_delta] = cos( angle );
		acadoVariables.x[blk * NX + idx_sin_delta] = sin( angle );
		angle += mhe_sampling_time * ddelta_ss;
	}

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

ORO_LIST_COMPONENT_TYPE( DynamicMhe )
//ORO_CREATE_COMPONENT( DynamicMhe )
