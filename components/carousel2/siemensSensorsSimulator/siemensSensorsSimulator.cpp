#include <math.h>

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include "siemensSensorsSimulator.hpp"
#include "time.h"

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

SiemensSensorsSimulator::SiemensSensorsSimulator(std::string name):TaskContext(name,PreOperational) 
{
	addPort("data", portData).doc("The output data port");
}

bool SiemensSensorsSimulator::configureHook()
{
	return true;
}

bool  SiemensSensorsSimulator::startHook()
{
	return true;
}

void  SiemensSensorsSimulator::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	usleep(7000); // Simulate waiting for the udp packet
	state.winchSpeedSmoothed = -0.5 + 1.2 * sin(.2*3.1415 * trigger + 1.8);
	state.winchEncoderPosition = -0.4 + 0.7 * sin(.1*3.1415 * trigger + 1.8);
	state.carouselSpeedSmoothed = -0.4 + 0.7 * sin(.1*3.1415 * trigger + 1.8);
	state.carouselEncoderPosition = -0.4 + 0.7 * sin(.1*3.1415 * trigger + 1.8);
	state.winchTorque = -0.4 + 0.7 * sin(.1*3.1415 * trigger + 1.8);

	state.ts_trigger = trigger;
	state.ts_elapsed = TimeService::Instance()->secondsSince( trigger );

	portData.write(state);

	this->getActivity()->trigger(); // This makes the component re-trigger automatically

}

void  SiemensSensorsSimulator::stopHook()
{}

void  SiemensSensorsSimulator::cleanupHook()
{}

void  SiemensSensorsSimulator::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensSensorsSimulator )
