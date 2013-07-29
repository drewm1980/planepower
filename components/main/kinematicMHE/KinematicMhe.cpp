#include "KinematicMhe.hpp"

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

KinematicMhe::KinematicMhe(std::string name)
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
	addPort("debugData", portDebugData)
		.doc("Debugging data");

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

bool  KinematicMhe::configureHook()
{
	Logger::In in( getName() );

	// TODO
	if (portTrigger.connected() == false)
	{
		log( Error ) << "One of the input ports is not connected to a data source" << endlog();
		goto configureHookFailed;
	}

	return true;

configureHookFailed:
	return false;
}

bool KinematicMhe::startHook()
{
	return true;
}

void KinematicMhe::updateHook()
{
	TIME_TYPE trigger;

	debugData.ts_entry = TimeService::Instance()->getTicks();
	
	//
	// Read the data ports
	//
	portTrigger.read( trigger );
	
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

	// Very basic debugging
	debugData.num_imu_samples = numImuSamples;
	debugData.num_enc_samples = (encStatus == NewData);
	debugData.num_cam_samples = (numMarkers > 6);
	debugData.num_las_samples = (lasStatus == NewData);

	debugData.ts_trigger = trigger;
	debugData.ts_elapsed = TimeService::Instance()->secondsSince( trigger );

	portDebugData.write( debugData );
}

void KinematicMhe::stopHook()
{}

void KinematicMhe::cleanupHook()
{}

void KinematicMhe::errorHook()
{}

ORO_CREATE_COMPONENT( KinematicMhe )
