#include <stdint.h>
#include <ocl/Component.hpp>

#include "cameraTrigger.hpp"

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

ORO_CREATE_COMPONENT(CameraTrigger);


#define TIME_TYPE uint64_t

CameraTrigger::CameraTrigger(std::string name):TaskContext(name,PreOperational)
{
	addEventPort("Trigger",_Trigger).doc("Trigger input/timestamp");
	addPort("TriggerTriggeredTime",_TriggerTriggeredTime).doc("Timestamp when trigger was fired"); 
	addPort("TriggerResetTime",_TriggerResetTime).doc("Timestamp when trigger was reset"); 
	tempTime = RTT::os::TimeService::Instance()->getTicks(); // Get current time
	_TriggerTriggeredTime.setDataSample( tempTime );
	_TriggerTriggeredTime.write( tempTime );
	_TriggerResetTime.setDataSample( tempTime );
	_TriggerResetTime.write( tempTime );
}

CameraTrigger::~CameraTrigger()
{
}

bool  CameraTrigger::configureHook()
{
	// Configure a digital output pin on the EBox to use for our trigger
	assert(this->hasPeer("soemMaster"));
	assert(this->getPeer("soemMaster")->provides()->hasService("Slave_1001"));
	assert(this->getPeer("soemMaster")->provides()->getService("Slave_1001")
			->provides()->hasOperation("setBit"));
	setBit = getPeer("soemMaster")->provides()->getService("Slave_1001")
		->provides()->getOperation("setBit");

#if TRIGGER_ACTIVE_HIGH
	pull_trigger_low();
#else
	pull_trigger_high();
#endif

	return true;
}

bool  CameraTrigger::startHook()
{
	return true;
}

void  CameraTrigger::updateHook()
{
	tempTime = RTT::os::TimeService::Instance()->getTicks();
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
	tempTime = RTT::os::TimeService::Instance()->getTicks();
	_TriggerResetTime.write(tempTime);

}

void  CameraTrigger::stopHook()
{
}

void  CameraTrigger::cleanUpHook()
{
}

void CameraTrigger::pull_trigger_high()
{
	setBit(0,1);
}

void CameraTrigger::pull_trigger_low()
{
	setBit(0,0);
}

