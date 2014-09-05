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
	
	addProperty("R_control", gains.R_control).doc("Control Penalty");
	addProperty("Q_alpha", gains.Q_alpha).doc("Alpha Penalty");
	addProperty("Q_dalpha", gains.Q_dalpha).doc("Dalpha Penalty");
	addProperty("K0", gains.K0);
	addProperty("K1", gains.K1);
	addProperty("K2", gains.K2);
	addProperty("K3", gains.K3);
	addProperty("K4", gains.K4);
	addProperty("K5", gains.K5);
	addProperty("K6", gains.K6);
	addProperty("K7", gains.K7);
	addProperty("K8", gains.K8);
	memset(&gains, 0, sizeof( gains ));

	addProperty("delta_motor", stateHolder.delta_motor);
	addProperty("delta_arm", stateHolder.delta_arm);
	addProperty("alpha", stateHolder.alpha);
	addProperty("beta", stateHolder.beta);
	addProperty("ddelta_motor", stateHolder.ddelta_motor);
	addProperty("ddelta_arm", stateHolder.ddelta_arm);
	addProperty("dalpha", stateHolder.dalpha);
	addProperty("dbeta", stateHolder.dbeta);
	addProperty("ddelta_motor_setpoint", stateHolder.ddelta_motor_setpoint);
	memset(&stateHolder, 0, sizeof( gains ));

	memset(&xss, 0, sizeof( State ));
	memset(&xss0, 0, sizeof( State ));
	memset(&xss1, 0, sizeof( State ));

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
