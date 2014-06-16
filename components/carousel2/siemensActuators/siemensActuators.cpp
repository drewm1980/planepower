#include "siemensActuators.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

SiemensActuators::SiemensActuators(std::string name):TaskContext(name,PreOperational) 
{
	sender = new SiemensSender;
}
SiemensActuators::~SiemensActuators()
{
	delete sender;
}

bool SiemensActuators::configureHook()
{
	return true;
}

bool  SiemensActuators::startHook()
{
	return true;
}

void  SiemensActuators::updateHook()
{
}

void  SiemensActuators::stopHook()
{}

void  SiemensActuators::cleanupHook()
{}

void  SiemensActuators::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensActuators )
