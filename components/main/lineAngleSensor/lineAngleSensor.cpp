#include <stdint.h>
#include <ocl/Component.hpp>

#include "lineAngleSensor.hpp"

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

ORO_CREATE_COMPONENT(LineAngleSensor);

#define TIME_TYPE uint64_t

LineAngleSensor::LineAngleSensor(std::string name):TaskContext(name,PreOperational)
{
	addEventPort("TriggerIn",_TriggerIn).doc("Trigger input/timestamp");
	addPort("TriggerOut",_TriggerOut).doc("Copy of the input trigger timestamp"); 
	addPort("EboxRequestTime",_TriggerOut).doc("Timestamp before reading from Ebox"); 
	addPort("EboxReplyTime",_TriggerOut).doc("Timestamp after reading from Ebox"); 
	addPort("voltages",_voltages).doc("voltages in units from Ebox (V?)"); 
	tempTime = RTT::os::TimeService::Instance()->getTicks(); // Get current time

	_TriggerOut.setDataSample( tempTime );
	_TriggerOut.write( tempTime );
	voltages.push_back(0.0);
	voltages.push_back(0.0);
	_voltages.setDataSample(voltages);
	_voltages.write(voltages);
}

LineAngleSensor::~LineAngleSensor()
{
}

bool  LineAngleSensor::configureHook()
{
	// Connect to the ebox component
	assert(this->hasPeer("soemMaster"));
	assert(this->getPeer("soemMaster")->provides()->hasService("Slave_1001"));
	assert(this->getPeer("soemMaster")->provides()->getService("Slave_1001")
			->provides()->hasOperation("readAnalog"));
	readAnalog = getPeer("soemMaster")->provides()->getService("Slave_1001")
		->provides()->getOperation("readAnalog");
	return true;
}

bool  LineAngleSensor::startHook()
{
	return true;
}

void  LineAngleSensor::updateHook()
{
	_TriggerIn.read(tempTime);
	_TriggerOut.write(tempTime);

	tempTime = RTT::os::TimeService::Instance()->getTicks();
	_EboxRequestTime.write(tempTime);

	for(int i=0; i<2; i++) voltages[i]=readAnalog(i);

	tempTime = RTT::os::TimeService::Instance()->getTicks();
	_EboxReplyTime.write(tempTime);

	_voltages.write(voltages);
}

void  LineAngleSensor::stopHook() {};

