#include "lineAngleSensor2Simulator.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <math.h>

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

	lineAngles.azimuth = -0.5 + 1.2 * sin(.2*3.1415 * trigger + 1.8);
	lineAngles.elevation = -0.4 + 0.7 * sin(.2*3.1415 * trigger + 1.8);

	lineAngles.ts_trigger = trigger;
	lineAngles.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portData.write(lineAngles);

}

void  LineAngleSensor2Simulator::stopHook()
{}

void  LineAngleSensor2Simulator::cleanupHook()
{}

void  LineAngleSensor2Simulator::errorHook()
{}

ORO_CREATE_COMPONENT( LineAngleSensor2Simulator )
