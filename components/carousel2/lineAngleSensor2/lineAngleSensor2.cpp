#include "lineAngleSensor2.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

LineAngleSensor2::LineAngleSensor2(std::string name):TaskContext(name,PreOperational) 
{
	//log(Error) << "Error in constructor of LineAngleSensor2" << endlog();
}

bool LineAngleSensor2::configureHook()
{
	return true;
}

bool  LineAngleSensor2::startHook()
{
	return true;
}

void  LineAngleSensor2::updateHook()
{
}

void  LineAngleSensor2::stopHook()
{}

void  LineAngleSensor2::cleanupHook()
{}

void  LineAngleSensor2::errorHook()
{}

ORO_CREATE_COMPONENT( LineAngleSensor2 )
