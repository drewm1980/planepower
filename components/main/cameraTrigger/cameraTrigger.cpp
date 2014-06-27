#include "cameraTrigger.hpp"

#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

#define TRIGGER_ACTIVE_HIGH 0

#define SLEEP_TIME_MILLISECONDS 10

CameraTrigger::CameraTrigger(std::string name)
	: TaskContext(name, PreOperational)
{
	TIME_TYPE tempTime;

	addEventPort("Trigger", _Trigger)
		.doc( "Trigger input/timestamp" );
	addPort("TriggerTriggeredTime", _TriggerTriggeredTime)
		.doc( "Timestamp when trigger was fired" ); 
	addPort("TriggerResetTime", _TriggerResetTime)
		.doc( "Timestamp when trigger was reset" );
 
	tempTime = TimeService::Instance()->getTicks();

	_TriggerTriggeredTime.setDataSample( tempTime );
	_TriggerTriggeredTime.write( tempTime );
	_TriggerResetTime.setDataSample( tempTime );
	_TriggerResetTime.write( tempTime );
}

bool  CameraTrigger::configureHook()
{
	Logger::In in( getName() );

	if (this->hasPeer("soemMaster") == false)
	{
		log( Error ) << "The is no soemMaster peer" << endlog();
		goto configureHookFailed;
	}

	if (this->getPeer("soemMaster")->provides()->hasService("Slave_1001") == false)
	{
		log( Error ) << "The is no Slave_1001 service" << endlog();
		goto configureHookFailed;
	}

	if (this->getPeer("soemMaster")->provides()->getService("Slave_1001")
		->provides()->hasOperation("setBit") == false)
	{
		log( Error ) << "The is no operation setBit" << endlog();
		goto configureHookFailed;
	}

	setBit = getPeer("soemMaster")->provides()->getService("Slave_1001")
		->provides()->getOperation("setBit");

#if TRIGGER_ACTIVE_HIGH
	pull_trigger_low();
#else
	pull_trigger_high();
#endif

	return true;

configureHookFailed:
	return false;
}

bool  CameraTrigger::startHook()
{
	return true;
}

void  CameraTrigger::updateHook()
{
	TIME_TYPE tempTime;

	tempTime = TimeService::Instance()->getTicks();
	_TriggerTriggeredTime.write(tempTime);
#if TRIGGER_ACTIVE_HIGH
	pull_trigger_high();
	usleep(SLEEP_TIME_MILLISECONDS*1e3);
	pull_trigger_low();
#else
	pull_trigger_low();
	usleep(SLEEP_TIME_MILLISECONDS*1e3);
	pull_trigger_high();
#endif
	tempTime = TimeService::Instance()->getTicks();
	_TriggerResetTime.write(tempTime);
}

void  CameraTrigger::stopHook()
{}

void  CameraTrigger::cleanupHook()
{}

void  CameraTrigger::errorHook()
{}

void CameraTrigger::pull_trigger_high()
{
	setBit(0, 1);
}

void CameraTrigger::pull_trigger_low()
{
	setBit(0, 0);
}

ORO_CREATE_COMPONENT( CameraTrigger )
