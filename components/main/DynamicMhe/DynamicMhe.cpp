#include "DynamicMhe.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <algorithm>

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
	debugData.S.resize(N * NY * NY, 0.0);
	debugData.SN.resize(NYN * NYN, 0.0);

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

	double weights[ NY ];
	// TODO read weights from the file and populate array
	return false;

	//
	// Configure weights
	//
	for (unsigned blk = 0; blk < N; ++blk)
		for (unsigned el = 0; el < NY; ++el)
			acadoVariables.S[blk * NY * NY + el * NY + el] = weights[ el ];

	for (unsigned el = 0; el < NYN; ++el)
		acadoVariables.SN[el * NYN + el] = weights[ el ];
	
	return true;
}

bool DynamicMhe::startHook()
{
	// TODO Initialize the solver

	runCnt = 0;

	// TODO clean them to zero in conf hook; execY, execYN

	return true;
}

void DynamicMhe::updateHook()
{
	debugData.ts_entry = stateEstimate.ts_entry = TimeService::Instance()->getTicks();
	
	// Read the trigger port
	TIME_TYPE trigger;
	portTrigger.read( trigger );
	debugData.ts_trigger = stateEstimate.ts_trigger = trigger;
	
	// Read sensor data
	if (readInputPorts() == false)
		exception();

	//
	// Prepare the sensor data
	//
	
	unsigned offset = NUM_MARKERS;
	execY[ offset++ ] = encData.cos_theta;
	execY[ offset++ ] = -encData.sin_theta; // !!! Sign is inverted !!!
	// gyro_xyz, accl_xyz
	for (unsigned i = 0; i < 6; ++i)
		execY[ offset++ ] = debugData.imu_avg[ i ];

	// Control surfaces; TODO we might skip this!
	execY[ offset++ ] = debugData.controls_avg[ 0 ]; // TODO check signs
	execY[ offset++ ] = debugData.controls_avg[ 2 ]; // TODO check signs

	// TODO addPropery:

	
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
	// MHE "state-machine"
	//
	
	bool runMhe = false;
	
	if (runCnt < N)
	{
		for (unsigned i = 0; i < NY; ++i)
			acadoVariables.y[runCnt * NY + i] = execY[ i ];

		int ledInd = runCnt - NDELAY;
		if (ledInd >= 0)
		{
			for (unsigned i = 0; i < NUM_MARKERS; ++i)
			{
				acadoVariables.y[ledInd * NY + i] = ledData[ i ];
				acadoVariables.S[ledInd * NY * NY + i * NY + i] = ledWeights[ i ];
			}
		}

		runMhe = false;
		stateEstimate.ready = debugData.ready = false;
		++runCnt;
	}
	else if (runCnt == N)
	{
		for (unsigned i = 0; i < NYN; ++i)
			acadoVariables.yN[ i ] = execYN[ i ];

		int ledInd = runCnt - NDELAY;
		for (unsigned i = 0; i < NUM_MARKERS; ++i)
		{
			acadoVariables.y[ledInd * NY + i] = ledData[ i ];
			acadoVariables.S[ledInd * NY * NY + i * NY + i] = ledWeights[ i ];
		}

		// TODO run preparation step

		runMhe = true;
		++runCnt;
	}
	else
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
		
		// Put new measurements
		for (unsigned el = 0; el < NYN; ++ el)
			acadoVariables.yN[ el ] = execYN[ el ];
		
		// Put new camera data
		int ledInd = N - NDELAY;
		for (unsigned i = 0; i < NUM_MARKERS; ++i)
		{
			acadoVariables.y[ledInd * NY + i] = ledData[ i ];
			acadoVariables.S[ledInd * NY * NY + i * NY + i] = ledWeights[ i ];
		}

		// Shift MHE states and controls
		shiftStates(2, 0, 0);
		shiftControls( 0 );

		runMhe = true;
	}

	int mheStatus = 0;
	if (runMhe == true)
	{
		// Execute an RTI

		preparationStep();
		
		mheStatus = feedbackStep();

		stateEstimate.ready = debugData.ready = mheStatus ? false : true;
		debugData.solver_status = mheStatus;
		debugData.kkt_value = getKKT();
		debugData.obj_value = getObjective();
		debugData.n_asc     = getNWSR();
	}

	// Output the current state estimate
	stateEstimate.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portStateEstimate.write( stateEstimate );

	//
	// Copy all debug data to the debug port
	//

	debugData.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDebugData.write( debugData );

	if ( mheStatus )
		exception();
}

void DynamicMhe::stopHook()
{
	
}

void DynamicMhe::cleanupHook()
{
	
}

void DynamicMhe::errorHook()
{
	
}

bool DynamicMhe::readInputPorts( void )
{	
	// It is assumed that this port will be buffered
	unsigned numImuSamples = 0;
	while((portMcuHandlerData.read( imuData[ numImuSamples ] ) == NewData) &&
		  (numImuSamples < MAX_NUM_IMU_SAMPLES))
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
				ledWeights[ i ] = camData.weights[ i ]; // TODO this has to be from rawesome!!!
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

	return true;
}

bool DynamicMhe::prepareDebugData( void )
{
	debugData.x.assign(acadoVariables.x, acadoVariables.x + (N + 1));
	debugData.u.assign(acadoVariables.u, acadoVariables.u + N * NU);
	debugData.z.assign(acadoVariables.z, acadoVariables.z + N * NXA);
	debugData.y.assign(acadoVariables.y, acadoVariables.y + N * NY);
	debugData.yN.assign(acadoVariables.yN, acadoVariables.yN + NYN);
	debugData.S.assign(acadoVariables.S, acadoVariables.S + N * NY * NY);
	debugData.SN.assign(acadoVariables.SN, acadoVariables.SN + NYN * NYN);

	debugData.enc_data[ 0 ] = encData.theta;
	debugData.enc_data[ 1 ] = encData.sin_theta;
	debugData.enc_data[ 2 ] = encData.cos_theta;

	copy(camData.positions.begin(), camData.positions.end(), debugData.cam_markers.begin());
	copy(camData.pose.begin(), camData.pose.end(), debugData.cam_pose.begin());

	debugData.las_data[ 0 ] = lasData.angle_hor;
	debugData.las_data[ 1 ] = lasData.angle_ver;

	return true;
}

ORO_CREATE_COMPONENT( DynamicMhe )
