#include "gainLoader.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

GainLoader::GainLoader(std::string name):TaskContext(name,PreOperational) 
{
	addPort("gains",portGains).doc("Controller Gains");

	// The property loader/marshaller should load the values directly into our
	// gains structure.
	addProperty("k11", gains.k11).doc("An entry in the controller gain matrix");
	addProperty("k12", gains.k12).doc("An entry in the controller gain matrix");
	addProperty("k21", gains.k21).doc("An entry in the controller gain matrix");
	addProperty("k22", gains.k22).doc("An entry in the controller gain matrix");

	memset(&gains, 0, sizeof( gains ));
}

bool GainLoader::configureHook()
{
	return true;
}

bool  GainLoader::startHook()
{
	return true;
}

void  GainLoader::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	gains.ts_trigger = trigger;
	gains.ts_elapsed = TimeService::Instance()->secondsSince( trigger );

	portGains.write(gains);

}

void  GainLoader::stopHook()
{}

void  GainLoader::cleanupHook()
{}

void  GainLoader::errorHook()
{}

ORO_CREATE_COMPONENT( GainLoader )
