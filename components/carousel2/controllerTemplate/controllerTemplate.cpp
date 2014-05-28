#include "controllerTemplate.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

ControllerTemplate::ControllerTemplate(std::string name):TaskContext(name,PreOperational) 
{
	addEventPort("resampledMeasurements",portResampledMeasurements).doc("Resampled measurements from all sensors");
	addPort("gains",portControllerGains).doc("Controller Gains");
	addPort("gainsOut",portGainsOut).doc("Controller Gains");
	addPort("data",portDriveCommand).doc("Command to the Siemens Drives");

	memset(&resampledMeasurements, 0, sizeof(resampledMeasurements));
	memset(&driveCommand, 0, sizeof(driveCommand));
	memset(&gains, 0, sizeof(gains));
	
}

bool ControllerTemplate::configureHook()
{
	FlowStatus gainsStatus = portControllerGains.read(gains);
	if(gainsStatus == NewData) 
	{
		return true;
	} 
	log(Error) << "Cannot configure the controller; gains must be loaded first!" << endlog();
	return false;
}

bool  ControllerTemplate::startHook()
{
	return true;
}

void  ControllerTemplate::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	portResampledMeasurements.read(resampledMeasurements);

	// C++ trick to make shorter names
	double & az = resampledMeasurements.azimuth;
	double & el = resampledMeasurements.elevation;
	ControllerGains& g = gains;
	
	// Controller Implementation goes here!
	driveCommand.winchSpeedReference =     g.k11*az + g.k12*el;
	driveCommand.carouselSpeedReference =  g.k21*az + g.k22*el;

	driveCommand.ts_trigger = trigger;
	driveCommand.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDriveCommand.write(driveCommand);

	// Write out the gains, for reporting.
	// Reporting the output of gainLoader would also be an option
	// if bandwidth gets constrained, but this is probably more
	// convenient for plotting, and more reliable.
	portGainsOut.write(gains);

	// Load in new gains if they are available
	ControllerGains tempGains;
	FlowStatus gainsStatus = portControllerGains.read(tempGains);
	if(gainsStatus == NewData) { gains = tempGains; }
}


void  ControllerTemplate::stopHook()
{}

void  ControllerTemplate::cleanupHook()
{}

void  ControllerTemplate::errorHook()
{}

ORO_CREATE_COMPONENT( ControllerTemplate )
