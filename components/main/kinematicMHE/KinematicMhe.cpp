#include "KinematicMhe.hpp"

#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

#define MAX_NUM_IMU_SAMPLES 100

KinematicMhe::KinematicMhe(std::string name)
	: TaskContext(name, PreOperational)
//	  , portMcuHandlerData(ConnPolicy::buffer( MAX_NUM_IMU_SAMPLES ))
{
	addEventPort("trigger", portTrigger)
		.doc("Trigger port");
	addPort("mcuData", portMcuHandlerData)
		.doc("MCU handler inputs");
	addPort("encoderData",portEncoderData)
		.doc("Encoder data");
	addPort("ledTrackerData", portLEDTrackerData)
		.doc("LED Tracker data");
	addPort("debugData", portDebugData)
		.doc("Debugging data");

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
	while(portMcuHandlerData.read( imuData[ numImuSamples ] ) == NewData)
		numImuSamples++;

	FlowStatus encStatus = portEncoderData.read( encData );

	FlowStatus camStatus = portLEDTrackerData.read( camData );
	unsigned numMarkers = 0;
	if (camStatus == NewData)
		for (unsigned i = 0; i < camData.weights.size(); ++i)
			numMarkers += (camData.weights[ i ] > 0.0);

	// Very basic debugging
	debugData.num_imu_samples = numImuSamples;
	debugData.num_enc_samples = (encStatus == NewData);
	debugData.num_cam_samples = (numMarkers > 6);

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
