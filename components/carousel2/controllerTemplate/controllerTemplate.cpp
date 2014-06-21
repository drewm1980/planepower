#include "controllerTemplate.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

ControllerTemplate::ControllerTemplate(std::string name):TaskContext(name,PreOperational) 
{
	//log(Error) << "Error in constructor of ControllerTemplate" << endlog();
}

bool ControllerTemplate::configureHook()
{
	return true;
}

bool  ControllerTemplate::startHook()
{
	return true;
}

void  ControllerTemplate::updateHook()
{
}

void  ControllerTemplate::stopHook()
{}

void  ControllerTemplate::cleanupHook()
{}

void  ControllerTemplate::errorHook()
{}

ORO_CREATE_COMPONENT( ControllerTemplate )
