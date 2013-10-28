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
	addPort("encoderData",portEncoderData)
		.doc("Encoder data");

	// Set up Output port
	addPort("controls", portControls)
		.doc("Control values");
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
	float az = lasData.angle_hor;
	float el = lasData.angle_ver;
	controls[0] = 0.0;
	controls[1] = 0.0;
	//controls[0] += 
	controls[2] = controls[1];
	portControls.write(controls);
}

void  ProportionalController::stopHook()
{
	controls[ 0 ] = controls[ 1 ] = controls[ 2 ] = 0.0;
	portControls.write( controls );
}

void  ProportionalController::cleanupHook()
{}

void  ProportionalController::errorHook()
{}

ORO_CREATE_COMPONENT( ProportionalController )
