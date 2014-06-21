#include "siemensDrivesSimulator.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

SiemensDrivesSimulator::SiemensDrivesSimulator(std::string name):TaskContext(name,PreOperational) 
{
	//log(Error) << "Error in constructor of SiemensDrivesSimulator" << endlog();
}

bool SiemensDrivesSimulator::configureHook()
{
	return true;
}

bool  SiemensDrivesSimulator::startHook()
{
	return true;
}

void  SiemensDrivesSimulator::updateHook()
{
}

void  SiemensDrivesSimulator::stopHook()
{}

void  SiemensDrivesSimulator::cleanupHook()
{}

void  SiemensDrivesSimulator::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensDrivesSimulator )
