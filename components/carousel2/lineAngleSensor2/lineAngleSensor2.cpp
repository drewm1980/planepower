#include "lineAngleSensor2.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <unistd.h>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

LineAngleSensor2::LineAngleSensor2(std::string name):TaskContext(name,PreOperational) 
{
	addPort("data", portData).doc("The output data port");
}


bool LineAngleSensor2::configureHook()
{
	memset(&lineAngles, 0, sizeof( LineAngles ));
	return true;
}

bool  LineAngleSensor2::startHook()
{	
	receiver.read(&lineAngles);
	// Trigger updateHook to be called at least once
	keepRunning = true;
	this->getActivity()->trigger(); 
	return true;
        
}

void  LineAngleSensor2::updateHook()
{
	receiver.read(&lineAngles);
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	// Do blocking wait for new line angle measurements here
	// usleep(50000); // make sure we don't burn spin the cpu in a high priority thread during testing.


	//lineAngles.azimuth = -0.5 + 1.2 * sin(.2*3.1415 * trigger + 1.8);
	//lineAngles.elevation = -0.4 + 0.7 * sin(.1*3.1415 * trigger + 1.8);

	lineAngles.ts_trigger = trigger;
	lineAngles.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portData.write(lineAngles);

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
