#include "pidGainLoader.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

PidGainLoader::PidGainLoader(std::string name):TaskContext(name,PreOperational) 
{
	addPort("gains",portPIDGains).doc("Controller Gains");

	// The property loader/marshaller should load the values directly into our
	// gains structure.
	addProperty("Kp", gains.Kp).doc("Proportional gain");
	addProperty("Ki", gains.Ki).doc("Integrator gain");
	addProperty("Kd", gains.Kd).doc("Derivative gain");

	memset(&gains, 0, sizeof( gains ));
}

bool PidGainLoader::configureHook()
{
	return true;
}

bool  PidGainLoader::startHook()
{
	return true;
}

void  PidGainLoader::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	gains.ts_trigger = trigger;
	gains.ts_elapsed = TimeService::Instance()->secondsSince( trigger );

	portPIDGains.write(gains);

}

void  PidGainLoader::stopHook()
{}

void  PidGainLoader::cleanupHook()
{}

void  PidGainLoader::errorHook()
{}

ORO_CREATE_COMPONENT( PidGainLoader )
