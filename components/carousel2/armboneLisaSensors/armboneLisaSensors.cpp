#include "armboneLisaSensors.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <unistd.h>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

ArmboneLisaSensors::ArmboneLisaSensors(std::string name):TaskContext(name,PreOperational) 
{
	addPort("GyroState",portGyroState).doc("Gyro Measurements");
	addPort("MagState",portMagState).doc("Mag Measurements");
	addPort("AccelState",portAccelState).doc("Accel Measurements");

}

bool ArmboneLisaSensors::configureHook()
{
	memset(&imuGyro, 0, sizeof( ImuGyro));	
	memset(&imuMag, 0, sizeof( ImuMag));	
	memset(&imuAccel, 0, sizeof( ImuAccel));	
	return true;
}

bool  ArmboneLisaSensors::startHook()
{	
	receiver.read(&imuGyro, &imuMag, &imuAccel);
	keepRunning = true;
	this->getActivity()->trigger();
	return true;
}

void  ArmboneLisaSensors::updateHook()
{	
	int value_type = receiver.read(&imuGyro, &imuMag, &imuAccel);
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	switch(value_type)
	{
		case 1:	imuGyro.ts_trigger = trigger;
			imuGyro.ts_elapsed = TimeService::Instance()->secondsSince(trigger);
			portGyroState.write(imuGyro);
			break;
		case 2: imuMag.ts_trigger = trigger;
			imuMag.ts_elapsed = TimeService::Instance()->secondsSince(trigger);
			portMagState.write(imuMag);
			break;
		case 3: imuAccel.ts_trigger = trigger;
			imuAccel.ts_elapsed = TimeService::Instance()->secondsSince(trigger);
			portAccelState.write(imuAccel);
			break;
	}
	
	if(keepRunning) this->getActivity()->trigger();
}

void  ArmboneLisaSensors::stopHook()
{
	keepRunning = false;
}

void  ArmboneLisaSensors::cleanupHook()
{}

void  ArmboneLisaSensors::errorHook()
{}

ORO_CREATE_COMPONENT( ArmboneLisaSensors )
