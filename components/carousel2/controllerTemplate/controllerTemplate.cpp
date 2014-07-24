#include "controllerTemplate.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

void clamp(double & x, double lb, double ub)
{
	if (x < lb) x = lb;
	if (x > ub) x = ub;
}

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

	// We NEED to have feedforward on during any big steps
	freezeFeedForwardTerm = false; 
	feedForwardTermAsSpeed = 0.0;
	feedForwardTermAsAngle = 0.0;
}

bool ControllerTemplate::configureHook()
{
	
	FlowStatus gainsStatus = portPIDControllerGains.read(gains);
	if(gainsStatus != NewData) 
	{
		log(Error) << "controllerTemplate: Cannot configure; gains are needed!" << endlog();
		return false;
	}

	if(! hasPeer("carouselSimulator"))
	{
		log(Error) << "controllerTemplate: Cannot configure; carouselSimulator needs to be a peer!" << endlog();
		return false;
	}
	lookup_steady_state_speed = getPeer("carouselSimulator")->provides()->getOperation("lookup_steady_state_speed");
	
	// Try reading a measurement, since it seems we never get newData
	// the first time updateHook is called...
	FlowStatus measurementStatus = portResampledMeasurements.read(resampledMeasurements);
	if (measurementStatus != NewData) 
	{
		log(Info) << "controllerTemplate: First read to measurements port was indeed not newData!" << endlog();
	}	

	return true;
}

bool  ControllerTemplate::startHook()
{
	freezeFeedForwardTerm = false;

	portResampledMeasurements.read(resampledMeasurements);
	FlowStatus refStatus = portReference.read(reference);
	if (refStatus != NewData) 
	{
		log(Error) << "controllerTemplate: Cannot start without reference elevation!" << endlog();
		return false;
	}	
	referenceElevation = reference.elevation;
	//error = referenceElevation - resampledMeasurements.elevation;
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
		log(Warning) << "controllerTemplate: No new measurment data!! " << endlog();
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
	referenceElevation = reference.elevation;

	// Look up the steady state speed for our reference elevation
	double referenceSpeed = lookup_steady_state_speed(referenceElevation);
	if (isnan(referenceSpeed))
	{
		log(Warning) << "controllerTemplate: Line angle sensor is out of range of the lookup table so cannot look up a reference speed!" << endlog();
		return;
	}

	if(!freezeFeedForwardTerm)
	{
		feedForwardTermAsSpeed = referenceSpeed; // Rad/s
		feedForwardTermAsAngle = referenceElevation; // Rad
	}

	error = referenceElevation - el; // Radians
	//ierror += error;
	//derror = ( error - last_error ) / getPeriod();
	//last_error = error;
	

	// Bound the controller to referenceSpeed +/- 
	const double speedBand = .2; // Rad/s

	//cout << "Looked up value is " << referenceSpeed << endl;
	double pTerm = g.Kp * error;
	//double iTerm = g.Ki * ierror;
	//double dTerm = g.Kd * derror;

	double pidTerm  = pTerm; // + iTerm + dTerm;

#define USE_MORITZ_IDEA 1
#if USE_MORITZ_IDEA	
	double pidControlAsSpeed = lookup_steady_state_speed(feedForwardTermAsAngle+pidTerm);
	if (isnan(pidControlAsSpeed))
	{
		log(Warning) << "controllerTemplate: Control (as elevation) is out of range of the lookup table so cannot look up the Control (as a speed)!" << endlog();
		return;
	}
	double control = pidControlAsSpeed; // Rad/s
#else
	double control = feedForwardTermAsSpeed + pidTerm; // Rad/s
#endif

	clamp(control,referenceSpeed-speedBand,referenceSpeed+speedBand);


	driveCommand.carouselSpeedSetpoint = control;

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
