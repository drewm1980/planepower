#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include "siemensSensors.hpp"

using namespace std;
using namespace RTT;
using namespace RTT::os;

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
	return true;
}

void  SiemensSensors::updateHook()
{
	receiver.read(&state); // Blocking
	//cout << "winchSpeedSmoothed: " << state.winchSpeedSmoothed << endl;
	portData.write(state);
	this->getActivity()->trigger(); // This makes the component re-trigger automatically
}

void  SiemensSensors::stopHook()
{
}

void  SiemensSensors::cleanupHook()
{
}

void  SiemensSensors::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensSensors )
