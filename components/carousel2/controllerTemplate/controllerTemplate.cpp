#include "controllerTemplate.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

void clamp(double & x, double lb, double ub)
{
	if (x < lb) x = lb;
	if (x > ub) x = ub;
}

void simple_lowpass(double dt, double tau, double *state, double input)
{
	double emdt = exp(-dt/tau);
	*state = (*state)*emdt + input*(1-emdt);
}

ControllerTemplate::ControllerTemplate(std::string name):TaskContext(name,PreOperational) 
{
	addEventPort("resampledMeasurements",portResampledMeasurements).doc("Resampled measurements from all sensors");
	addPort("gains",portPIDControllerGains).doc("Controller Gains");
	addPort("gainsOut",portGainsOut).doc("Controller Gains");
	addPort("debug",portDebug).doc("PID controller debug info");
	addPort("data",portDriveCommand).doc("Command to the Siemens Drives");
	addPort("reference",portReference).doc("Reference elevation");

	addProperty("freezeFeedForwardTerm", freezeFeedForwardTerm).doc("Set to true to lock current value of the feedforward term.");

	memset(&resampledMeasurements, 0, sizeof(resampledMeasurements));
	memset(&driveCommand, 0, sizeof(driveCommand));
	memset(&gains, 0, sizeof(gains));

	// We NEED to have feedforward on during any big steps
	freezeFeedForwardTerm = false; 
	feedForwardTermAsSpeed = 0.0;
	feedForwardTermAsAngle = 0.0;
	feedForwardTermHasBeenSet = false;

	derivativeLowpassFilterState = 0.0;
	trigger = TimeService::Instance()->getTicks();
	trigger_last = TimeService::Instance()->getTicks();
	trigger_last_is_valid = false;
	derivativeFilterReady = false;

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
	derivativeLowpassFilterState = 0.0;

	freezeFeedForwardTerm = false;

	portResampledMeasurements.read(resampledMeasurements);
	FlowStatus refStatus = portReference.read(reference);
	if (refStatus != NewData) 
	{
		log(Error) << "controllerTemplate: Cannot start without reference elevation!" << endlog();
		return false;
	}	
	referenceElevation = reference.elevation;
	//integral initialisation for i and d gains 
	error = referenceElevation - resampledMeasurements.elevation;
	ierror = 0;

	return true;
}

void  ControllerTemplate::updateHook()
{
	trigger_last = trigger; 
	trigger = TimeService::Instance()->getTicks();

	FlowStatus measurementStatus = portResampledMeasurements.read(resampledMeasurements);
	if (measurementStatus != NewData) 
	{
		log(Warning) << "controllerTemplate: No new measurment data!! " << endlog();
		return;
	}	

	// C++ trick to make shorter names
	//double & az = resampledMeasurements.azimuth;
	double & elevation = resampledMeasurements.elevation;
	//ControllerGains& g = gains;
	PIDControllerGains& g = gains;
	
	// Controller Implementation goes here!
	//  gain matrix meaning:
	//  [winchSpeedSetpoint carouselSpeedSetpoint]' = [k11 k12; k21 k22] * [azimuth elevation]'
	//driveCommand.winchSpeedSetpoint =     g.k11*az + g.k12*elevation;
	//driveCommand.carouselSpeedSetpoint =  g.k21*az + g.k22*elevation;
	portReference.read(reference);
	referenceElevation = reference.elevation;

	// Look up the steady state speed for our reference elevation
	double referenceSpeed = lookup_steady_state_speed(referenceElevation);
	if (isnan(referenceSpeed))
	{
		log(Warning) << "controllerTemplate: Line angle sensor is out of range of the lookup table so cannot look up a reference speed!" << endlog();
		return;
	}

	if(freezeFeedForwardTerm)
	{
		if (!feedForwardTermHasBeenSet)
		{
			log(Error) << "controllerTemplate: updateHook has been called before feedForwardTerm has had a chance to be set, while freezeFeedForwardTerm is set!" << endlog();
		}
	}
	else 
	{
		feedForwardTermAsSpeed = referenceSpeed; // Rad/s
		feedForwardTermAsAngle = referenceElevation; // Rad
		feedForwardTermHasBeenSet = true;
	}

	// Update our derivative and integral filters
	if(trigger_last_is_valid)
	{
		double dt = (trigger - trigger_last)*1.0e-9; // seconds
		double tau = 0.2;
		double d_elevation = (elevation - lastElevation)/dt;
		simple_lowpass(dt, tau, &derivativeLowpassFilterState, d_elevation);
	
		ierror += error*dt; //Rad*s Elevetion integration
	} else {
		trigger_last_is_valid = true;
	}
	error = referenceElevation - elevation; // Radians
	derror = 0.0 - derivativeLowpassFilterState; // Radians / s.  This is an error if d/dt of reference = 0

	lastElevation = elevation; // Now that we're done using elevation, save it for next time.

	// Bound the controller to referenceSpeed +/- 
	const double speedBand = .05; // Rad/s

	//cout << "Looked up value is " << referenceSpeed << endl;
	double pTerm = g.Kp * error;
	double iTerm = g.Ki * ierror;
	double dTerm = g.Kd * derror;

	double iTermBound = 0.1; //Rad elevation
	clamp(ierror, -iTermBound/g.Ki, iTermBound/g.Ki); 
	double pidTerm  = pTerm + iTerm + dTerm;

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
	double ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	driveCommand.ts_elapsed = ts_elapsed;
	portDriveCommand.write(driveCommand);

	// Write out the gains, for reporting.
	// Reporting the output of gainLoader would also be an option
	// if bandwidth gets constrained, but this is probably more
	// convenient for plotting, and more reliable.
	portGainsOut.write(gains);

	// Write out all of our debug info
	debug.Kp = gains.Kp;
	debug.Ki = gains.Ki;
	debug.Kd = gains.Kd;
	debug.derivativeLowpassFilterState = derivativeLowpassFilterState;
	debug.feedForwardTermAsAngle = feedForwardTermAsAngle;
	debug.feedForwardTermAsSpeed = feedForwardTermAsSpeed;
	debug.ierror = ierror;
	debug.derror = derror;
	debug.ts_trigger = trigger;
	debug.ts_elapsed = ts_elapsed;
	portDebug.write(debug);

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
