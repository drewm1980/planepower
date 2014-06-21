#include "resampler.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

Resampler::Resampler(std::string name):TaskContext(name,PreOperational) 
{
	//log(Error) << "Error in constructor of Resampler" << endlog();
}

bool Resampler::configureHook()
{
	return true;
}

bool  Resampler::startHook()
{
	return true;
}

void  Resampler::updateHook()
{
}

void  Resampler::stopHook()
{}

void  Resampler::cleanupHook()
{}

void  Resampler::errorHook()
{}

ORO_CREATE_COMPONENT( Resampler )
