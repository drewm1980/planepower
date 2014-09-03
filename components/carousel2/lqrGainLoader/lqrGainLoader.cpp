#include "lqrGainLoader.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

LqrGainLoader::LqrGainLoader(std::string name):TaskContext(name,PreOperational) 
{
	addPort("gains",portLQRGains).doc("Controller Gains");

	// The property loader/marshaller should load the values directly into our
	// gains structure.
	addProperty("Kp", gains.Kp).doc("Proportional gain");
	addProperty("Ki", gains.Ki).doc("Integrator gain");
	addProperty("Kd", gains.Kd).doc("Derivative gain");

	memset(&gains, 0, sizeof( gains ));
}

bool LqrGainLoader::configureHook()
{
	return true;
}

bool  LqrGainLoader::startHook()
{
	return true;
}

void  LqrGainLoader::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	gains.ts_trigger = trigger;
	gains.ts_elapsed = TimeService::Instance()->secondsSince( trigger );

	portLQRGains.write(gains);

}

void  LqrGainLoader::stopHook()
{}

void  LqrGainLoader::cleanupHook()
{}

void  LqrGainLoader::errorHook()
{}

ORO_CREATE_COMPONENT( LqrGainLoader )
