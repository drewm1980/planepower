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

// # delay samples for the cameras; maybe to define this as a property?
#define DELAY_CAM 4

/// Size of the IMU data buffer size
#define MAX_NUM_IMU_SAMPLES 100

/// Check port connection
#define checkPortConnection( port ) \
	if (port.connected() == false) \
	{log( Error ) << "Port " << port.getName() << " is not connected()" << endlog(); return false;}

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
		.doc("Debugging data");
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
	
	return true;
}

bool DynamicMhe::startHook()
{
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
	readInputPorts();

	stateEstimate.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portStateEstimate.write( stateEstimate );

	debugData.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDebugData.write( debugData );
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
			numMarkers += (camData.weights[ i ] > 0.0);

	FlowStatus lasStatus = portLASData.read( lasData );

	//
	// Prepare data for logging
	//
	debugData.imu_first[ 0 ] = imuData[numImuSamples - 1].gyro_x;
	debugData.imu_first[ 1 ] = imuData[numImuSamples - 1].gyro_y;
	debugData.imu_first[ 2 ] = imuData[numImuSamples - 1].gyro_z;
	debugData.imu_first[ 3 ] = imuData[numImuSamples - 1].accl_x;
	debugData.imu_first[ 4 ] = imuData[numImuSamples - 1].accl_y;
	debugData.imu_first[ 5 ] = imuData[numImuSamples - 1].accl_z;

	for (unsigned i = 0; i < debugData.imu_avg.size(); ++i)
		debugData.imu_avg[ i ] = 0.0;
	for (unsigned i = 0; i < numImuSamples; ++i)
	{
		debugData.imu_avg[ 0 ] += imuData[ i ].gyro_x;
		debugData.imu_avg[ 1 ] += imuData[ i ].gyro_y;
		debugData.imu_avg[ 2 ] += imuData[ i ].gyro_z;
		debugData.imu_avg[ 3 ] += imuData[ i ].accl_x;
		debugData.imu_avg[ 4 ] += imuData[ i ].accl_y;
		debugData.imu_avg[ 5 ] += imuData[ i ].accl_z;
	}
	for (unsigned i = 0; i < debugData.imu_avg.size(); ++i)
		debugData.imu_avg[ i ] /= (double)numImuSamples;

	debugData.enc_data[ 0 ] = encData.theta;
	debugData.enc_data[ 1 ] = encData.sin_theta;
	debugData.enc_data[ 2 ] = encData.cos_theta;

	copy(camData.positions.begin(), camData.positions.end(), debugData.cam_markers.begin());
	copy(camData.pose.begin(), camData.pose.end(), debugData.cam_pose.begin());

	debugData.las_data[ 0 ] = lasData.angle_hor;
	debugData.las_data[ 1 ] = lasData.angle_ver;
	
	debugData.num_imu_samples = numImuSamples;
	debugData.num_enc_samples = (encStatus == NewData);
	debugData.num_cam_samples = (numMarkers > 6);
	debugData.num_las_samples = (lasStatus == NewData);

	return true;
}

ORO_CREATE_COMPONENT( DynamicMhe )
