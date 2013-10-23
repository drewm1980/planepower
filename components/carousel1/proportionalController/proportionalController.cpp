#include "proportionalController.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

ProportionalController::ProportionalController(std::string name):TaskContext(name,PreOperational) 
{
	addPort("controls", portControls)
		.doc("Control values");
	controls.resize(3, 0.0);
	portControls.setDataSample( controls );
	portControls.write( controls );
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
