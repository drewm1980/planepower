#include "siemensActuatorsSimulator.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

SiemensActuatorsSimulator::SiemensActuatorsSimulator(std::string name):TaskContext(name,PreOperational) 
{
	addEventPort("controls", portControls)
		.doc("Command to be sent to the Siemens Drives");
}

bool SiemensActuatorsSimulator::configureHook()
{
	return true;
}

bool  SiemensActuatorsSimulator::startHook()
{
	return true;
}

void  SiemensActuatorsSimulator::updateHook()
{
	portControls.read(driveCommand);
	// Simulate nothing; just read the controls from the port and sit on them.
}

void  SiemensActuatorsSimulator::stopHook()
{}

void  SiemensActuatorsSimulator::cleanupHook()
{}

void  SiemensActuatorsSimulator::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensActuatorsSimulator )
