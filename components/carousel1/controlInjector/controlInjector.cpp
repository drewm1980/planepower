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

	controls.resize(3, 0.0);
	// Prepare ports
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
void ControlInjector::setControlsUnitless(double right_aileron, double left_aileron, double elevator)
{
	vector<double> temp;
	temp.resize(3,0.0);
	temp[0] = right_aileron;
	temp[1] = left_aileron; 
	temp[2] = elevator;
	convert_controls_unitless_to_radians(&temp[0]);
	controls[0]=temp[0];
	controls[1]=temp[1];
	controls[2]=temp[2];
}

ORO_CREATE_COMPONENT( ControlInjector )
