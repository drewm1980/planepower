#include "siemensSensorsSimulator.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

SiemensSensorsSimulator::SiemensSensorsSimulator(std::string name):TaskContext(name,PreOperational) 
{
	//log(Error) << "Error in constructor of SiemensSensorsSimulator" << endlog();
}

bool SiemensSensorsSimulator::configureHook()
{
	return true;
}

bool  SiemensSensorsSimulator::startHook()
{
	return true;
}

void  SiemensSensorsSimulator::updateHook()
{
}

void  SiemensSensorsSimulator::stopHook()
{}

void  SiemensSensorsSimulator::cleanupHook()
{}

void  SiemensSensorsSimulator::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensSensorsSimulator )
