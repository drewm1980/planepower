#include "siemensActuatorsSimulator.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

SiemensActuatorsSimulator::SiemensActuatorsSimulator(std::string name):TaskContext(name,PreOperational) 
{
	//log(Error) << "Error in constructor of SiemensActuatorsSimulator" << endlog();
}

bool SiemensActuatorsSimulator::configureHook()
{
	return true;
}

bool  SiemensActuatorsSimulator::startHook()
{
	return true;
}

void  SiemensActuatorsSimulator::updateHook()
{
}

void  SiemensActuatorsSimulator::stopHook()
{}

void  SiemensActuatorsSimulator::cleanupHook()
{}

void  SiemensActuatorsSimulator::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensActuatorsSimulator )
