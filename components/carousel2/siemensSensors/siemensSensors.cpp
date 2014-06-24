#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include "siemensSensors.hpp"

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

SiemensSensors::SiemensSensors(std::string name):TaskContext(name,PreOperational) 
{
	addPort("data", portData).doc("The output data port");
}
SiemensSensors::~SiemensSensors()
{
}

bool SiemensSensors::configureHook()
{
	memset(&state, 0, sizeof( SiemensDriveState ));

	return true;
}

bool  SiemensSensors::startHook()
{
	receiver.read(&state); // Stay in stopped state until data actually arrives
	keepRunning = true;
	return true;
}

void  SiemensSensors::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	receiver.read(&state); // Blocking
	//cout << "winchSpeedSmoothed: " << state.winchSpeedSmoothed << endl;

	state.ts_trigger = trigger;
	state.ts_elapsed = TimeService::Instance()->secondsSince( trigger );

	portData.write(state);
	if(keepRunning) this->getActivity()->trigger(); // This makes the component re-trigger automatically
}

void  SiemensSensors::stopHook()
{
	keepRunning = false;
}

void  SiemensSensors::cleanupHook()
{
}

void  SiemensSensors::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensSensors )
