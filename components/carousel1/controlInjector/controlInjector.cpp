#include "controlInjector.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

ControlInjector::ControlInjector(std::string name):TaskContext(name,PreOperational) 
{
	addEventPort("trigger", portTrigger, boost::bind(&ControlInjector::applyControls,this))
		        .doc("Trigger the component to write the controls");
	addPort("portControls", portControls)
		        .doc("Trigger the component to write the controls");
	addOperation("setControlsUnitless", &ControlInjector::setControlsUnitless, this, OwnThread)  
			.doc("Set the values for the flight surfaces, scaled from -1 to 1")
			.arg("right_aileron", "Right Aileron, scaled from -1 to 1, positive is DOWN")
			.arg("left_aileron", "Left Aileron, scaled from -1 to 1, positive is UP")
			.arg("elevator", "Elevator, scaled from -1 to 1, positive is UP");
	
	// Prepare ports
	controls.reset();
	portControls.setDataSample( controls );
	portControls.write( controls );

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
}

void  ControlInjector::applyControls()
{
	portControls.write( controls );
}

void  ControlInjector::stopHook()
{}

void  ControlInjector::cleanupHook()
{}

void  ControlInjector::errorHook()
{}

// Warning, this function NOT hard real-time safe.
// But, this should be fine if you're running this component
// in it's own thread.
void ControlInjector::setControlsUnitless(float right_aileron, float left_aileron, float elevator)
{
	convertControlsUnitlessRadians(right_aileron, left_aileron, elevator);
	controls.ua1 = right_aileron;
	controls.ua2 = left_aileron;
	controls.ue = elevator;

	controls.d_ua1 = controls.d_ua1 = controls.d_ue = 0.0;
}

ORO_CREATE_COMPONENT( ControlInjector )
