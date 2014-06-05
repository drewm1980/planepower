#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include "siemensDrives.hpp"
#include "siemensSensors.hpp"

using namespace std;
using namespace RTT;
using namespace RTT::os;

SiemensSensors::SiemensSensors(std::string name):TaskContext(name,PreOperational) 
{
	//log(Error) << "Error in constructor of SiemensSensors" << endlog();
}

bool SiemensSensors::configureHook()
{
	return true;
}

bool  SiemensSensors::startHook()
{
	return true;
}

void  SiemensSensors::updateHook()
{
        receiver.read(&state);
}

void  SiemensSensors::stopHook()
{
}

void  SiemensSensors::cleanupHook()
{}

void  SiemensSensors::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensSensors )
