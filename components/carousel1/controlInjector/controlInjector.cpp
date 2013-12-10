#include "controlInjector.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

ControlInjector::ControlInjector(std::string name):TaskContext(name,PreOperational) 
{
	addEventPort("trigger", portTrigger)
		        .doc("Trigger the component to write the controls");
	addPort("portControls", portControls)
		        .doc("Trigger the component to write the controls");
	addOperation("setControlsUnitless", &McuHandler::setControlsUnitless, this, OwnThread)  
			.doc("Set the values for the flight surfaces, scaled from -1 to 1")
			.arg("right_aileron", "Right Aileron, scaled from -1 to 1, positive is DOWN")
			.arg("left_aileron", "Left Aileron, scaled from -1 to 1, positive is UP")
			.arg("elevator", "Elevator, scaled from -1 to 1, positive is UP");

	controls.resize(3, 0.0);
	// Prepare ports
	portControls.setDataSample( controls );
	portControls.write( controls );

	log(Error) << "foooo" << endlog();
}

bool ControlInjector::configureHook()
{
	return true;
}

bool  ControlInjector::startHook()
{
	return true;
}

void  ControlInjector::updateHook()
{
	portControls.write( controls );
	convert_controls_radians_to_unitless(&controls[0])
}

void  ControlInjector::stopHook()
{}

void  ControlInjector::cleanupHook()
{}

void  ControlInjector::errorHook()
{}

void ControlInjector::setControlsUnitless(double right_aileron, double left_aileron, double elevator)
{
	    controls[0] = right_aileron;
	    controls[1] = left_aileron; 
	    controls[2] = elevator;
}

ORO_CREATE_COMPONENT( ControlInjector )
