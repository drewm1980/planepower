#include "lineAngleSensor2Simulator.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <math.h>

#include "time.h"

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

LineAngleSensor2Simulator::LineAngleSensor2Simulator(std::string name):TaskContext(name,PreOperational) 
{
	addPort("data", portData).doc("The output data port");
}

bool LineAngleSensor2Simulator::configureHook()
{
	memset(&lineAngles, 0, sizeof( LineAngles ));
	return true;
}

bool  LineAngleSensor2Simulator::startHook()
{
	return true;
}

void  LineAngleSensor2Simulator::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	usleep(20000); // Simulate waiting for the udp packet
	lineAngles.azimuth = 1 * sin(.2*3.1415 * trigger * 1e-9 );
	lineAngles.elevation = 2 * sin(.1*3.1415 * trigger * 1e-9 );

	lineAngles.ts_trigger = trigger;
	lineAngles.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portData.write(lineAngles);
	this->getActivity()->trigger(); // This makes the component re-trigger automatically

}

void  LineAngleSensor2Simulator::stopHook()
{}

void  LineAngleSensor2Simulator::cleanupHook()
{}

void  LineAngleSensor2Simulator::errorHook()
{}

ORO_CREATE_COMPONENT( LineAngleSensor2Simulator )
