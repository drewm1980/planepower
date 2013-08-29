#include "DynamicMhe.hpp"

#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <algorithm>

using namespace std;
using namespace RTT;
using namespace RTT::os;

#define DELAY_CAM 4 // samples

DynamicMhe::DynamicMhe(std::string name)
	: TaskContext(name, PreOperational)
{
	
}

bool DynamicMhe::configureHook()
{
	return true;
}

bool DynamicMhe::startHook()
{
	return true;
}

void DynamicMhe::updateHook()
{
	preparationStep();
}

void DynamicMhe::stopHook()
{
	
}

void DynamicMhe::cleanupHook()
{
	
}

void DynamicMhe::errorHook()
{
	
}
