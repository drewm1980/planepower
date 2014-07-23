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
	addPort("gains",portPIDControllerGains).doc("Controller Gains");
	addPort("gainsOut",portGainsOut).doc("Controller Gains");
	addPort("data",portDriveCommand).doc("Command to the Siemens Drives");
	addPort("reference",portReference).doc("Reference elevation");

	memset(&resampledMeasurements, 0, sizeof(resampledMeasurements));
	memset(&driveCommand, 0, sizeof(driveCommand));
	memset(&gains, 0, sizeof(gains));
	
}

bool ControllerTemplate::configureHook()
{
	
	FlowStatus gainsStatus = portPIDControllerGains.read(gains);
	if(gainsStatus == NewData) 
	{
		return true;
	}
	lookup_steady_state_speed = getPeer("carouselSimulator")->provides()->getOperation("lookup_steady_state_speed");

	log(Error) << "Cannot configure the controller; gains must be loaded first!" << endlog();
	return false;
}

bool  ControllerTemplate::startHook()
{
	portResampledMeasurements.read(resampledMeasurements);
	FlowStatus refStatus = portReference.read(reference);
	if (refStatus != NewData) 
	{
		log(Error) << "Controller cannot start without reference elevation!" << endlog();
		return false;
	}	
	el_ref = reference.elevation;
	//error = el_ref - resampledMeasurements.elevation;
	//last_error = error;
	//ierror = 0;
	return true;
}

void  ControllerTemplate::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	FlowStatus measurementStatus = portResampledMeasurements.read(resampledMeasurements);
	if (measurementStatus != NewData) 
	{
		log(Warning) << "No new measurment data!! " << endlog();
		return;
	}	

	// C++ trick to make shorter names
	//double & az = resampledMeasurements.azimuth;
	double & el = resampledMeasurements.elevation;
	//ControllerGains& g = gains;
	PIDControllerGains& g = gains;
	
	// Controller Implementation goes here!
	//  gain matrix meaning:
	//  [winchSpeedSetpoint carouselSpeedSetpoint]' = [k11 k12; k21 k22] * [azimuth elevation]'
	//driveCommand.winchSpeedSetpoint =     g.k11*az + g.k12*el;
	//driveCommand.carouselSpeedSetpoint =  g.k21*az + g.k22*el;
	portReference.read(reference);
	el_ref = reference.elevation;
	
	error = el_ref - el;
	//ierror += error;
	//derror = ( error - last_error ) / getPeriod();
	//last_error = error;
	
	driveCommand.carouselSpeedSetpoint = lookup_steady_state_speed(el_ref) 
										+ g.Kp * error ;
										//+ g.Ki * ierror 
										//+ g.Kd * derror

	driveCommand.ts_trigger = trigger;
	driveCommand.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDriveCommand.write(driveCommand);

	// Write out the gains, for reporting.
	// Reporting the output of gainLoader would also be an option
	// if bandwidth gets constrained, but this is probably more
	// convenient for plotting, and more reliable.
	portGainsOut.write(gains);

	// Load in new gains if they are available
	PIDControllerGains tempGains;
	FlowStatus gainsStatus = portPIDControllerGains.read(tempGains);
	if(gainsStatus == NewData) { gains = tempGains; }
}


void  ControllerTemplate::stopHook()
{}

void  ControllerTemplate::cleanupHook()
{}

void  ControllerTemplate::errorHook()
{}

ORO_CREATE_COMPONENT( ControllerTemplate )
