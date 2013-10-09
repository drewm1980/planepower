#include "siemensDrives.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

SiemensDrives::SiemensDrives(std::string name):TaskContext(name,PreOperational) 
{
	log(Error) << "foooo" << endlog();
}

bool SiemensDrives::configureHook()
{
	return true;
}

bool  SiemensDrives::startHook()
{
	return true;
}

void  SiemensDrives::updateHook()
{
}

void  SiemensDrives::stopHook()
{}

void  SiemensDrives::cleanupHook()
{}

void  SiemensDrives::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensDrives )
