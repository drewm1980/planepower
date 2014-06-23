#include "lineAngleSensor2.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <unistd.h>

using namespace std;
using namespace RTT;
using namespace RTT::os;

LineAngleSensor2::LineAngleSensor2(std::string name):TaskContext(name,PreOperational) 
{
}

bool LineAngleSensor2::configureHook()
{
	return true;
}

bool  LineAngleSensor2::startHook()
{
	// Trigger updateHook to be called at least once
	keepRunning = true;
	this->getActivity()->trigger(); 
	return true;
}

void  LineAngleSensor2::updateHook()
{
	// Do blocking wait for new line angle measurements here

	usleep(500000); // make sure we don't burn spin the cpu in a high priority thread during testing.

	// Re-trigger this comonent internally
	if(keepRunning) this->getActivity()->trigger(); 
}

void  LineAngleSensor2::stopHook()
{
	keepRunning = false;
}

void  LineAngleSensor2::cleanupHook()
{}

void  LineAngleSensor2::errorHook()
{}

ORO_CREATE_COMPONENT( LineAngleSensor2 )
