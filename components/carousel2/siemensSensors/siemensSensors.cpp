#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include "siemensSensors.hpp"

using namespace std;
using namespace RTT;
using namespace RTT::os;

SiemensSensors::SiemensSensors(std::string name):TaskContext(name,PreOperational) 
{
}
SiemensSensors::~SiemensSensors()
{
}

bool SiemensSensors::configureHook()
{
	receiver = new SiemensReceiver;
	state = new SiemensDriveState;
	return true;
}

bool  SiemensSensors::startHook()
{
	return true;
}

void  SiemensSensors::updateHook()
{
        receiver->read(state);
}

void  SiemensSensors::stopHook()
{
}

void  SiemensSensors::cleanupHook()
{
	delete receiver;
	delete state;
}

void  SiemensSensors::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensSensors )
