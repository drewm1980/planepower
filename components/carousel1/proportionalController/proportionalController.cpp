#include "proportionalController.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

ProportionalController::ProportionalController(std::string name):TaskContext(name,PreOperational) 
{
	// Set up Input ports
	addEventPort("lasData", portLASData)
		.doc("Line angle sensor data in Radians");

	// Set up Output port
	addPort("controls", portControls)
		.doc("Control values");
	addPort("triggerOut", portTriggerOut)
		.doc("Currently meaningless trigger value");
	portTriggerOut.write(0);
	controls.resize(3, 0.0);
	portControls.setDataSample( controls );
	portControls.write( controls );
	
	// Properties
	addProperty("azimuthElevator",   azimuthElevator);
	addProperty("elevationElevator", elevationElevator);
	addProperty("azimuthAileron",    azimuthAileron);
	addProperty("elevationAileron",  elevationAileron);
}

bool ProportionalController::configureHook()
{
	// Default values for the gains.  Good for testing!
	azimuthAileron = 1.0;
	azimuthElevator = 0.0;
	elevationAileron = 0.0;
	elevationElevator = 1.0;
	return true;
}

bool  ProportionalController::startHook()
{
	return true;
}

void  ProportionalController::updateHook()
{
	portLASData.read(lasData);
	//portEncoderData.read(encoderData);
	float azimuth = lasData.angle_hor; 
	float elevation = lasData.angle_ver;
	controls[0] = azimuth*azimuthAileron + elevation*elevationAileron;
	controls[1] = controls[0];
	controls[2] = azimuth*azimuthElevator + elevation*elevationElevator;
	portControls.write(controls);
	portTriggerOut.write(0);
}

void  ProportionalController::stopHook()
{
	controls[ 0 ] = controls[ 1 ] = controls[ 2 ] = 0.0;
	portControls.write( controls );
	portTriggerOut.write(0);
}

void  ProportionalController::cleanupHook()
{}

void  ProportionalController::errorHook()
{}

ORO_CREATE_COMPONENT( ProportionalController )
