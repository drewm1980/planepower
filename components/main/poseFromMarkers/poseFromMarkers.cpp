#include "poseFromMarkers.hpp"

#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

#include "pose_from_markers.h"

PoseFromMarkers::PoseFromMarkers(string name)
	: TaskContext(name, PreOperational)
{
	//
	// Add ports
	//
	addEventPort("camData", portCamData)
		.doc("LED tracker data");
	addPort("data", portData)
		.doc("The body pose w.r.t. the anchorpoint frame, "
			 "based on marker positions in the cameras: x, y, z, e11, ..., e33");

	data.pose.resize(NPOSE, 0.0);
	portData.setDataSample( data );
	portData.write( data );
	
	cInput[ 0 ]  = markers;
	cOutput[ 0 ] = pose;
}

bool PoseFromMarkers::configureHook()
{
	Logger::In in( getName() );

	if (portCamData.connected() == false)
	{
		log( Error ) << "Input port is not connected" << endlog();
		return false;
	}

	return true;
}

bool PoseFromMarkers::startHook()
{
	return true;
}

void PoseFromMarkers::updateHook()
{
	TimeService::ticks ts_entry = TimeService::Instance()->getTicks();  
	bool proceed = true;
	int cStatus = 1;

	// Read the input port
	portCamData.read( camData );
	// Check if data is OK
	for (unsigned i = 0; i < NMARKERPOSITIONS; i++)
		if (camData.weights[ i ] == 0)
		{
			proceed = false;
			break;
		}

	// Process data
	if (proceed == true)
	{
		copy(camData.positions.begin(), camData.positions.end(), cInput[ 0 ]);

		cStatus = pose_from_markers(cInput, cOutput);
	}
	
	// If status is NOT OK, clear temporary data buffer
	if ( cStatus )
	{
		memset(pose, 0.0, NPOSE * sizeof( double ));
	}
	
	// Assign data to output buffer
	data.pose.assign(pose, pose + NPOSE);

	// Send data out!
	data.ts_trigger = camData.ts_trigger;
	data.ts_entry   = ts_entry; 
	data.ts_elapsed = TimeService::Instance()->secondsSince( ts_entry );
	portData.write( data );
}

void PoseFromMarkers::stopHook()
{}

void PoseFromMarkers::errorHook()
{}

void PoseFromMarkers::cleanupHook()
{}

ORO_CREATE_COMPONENT( PoseFromMarkers )
