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
	//log(Error) << "Error in constructor of ArmboneLisaSensors" << endlog();

	addPort("GyroState",portGyroState).doc("Gyro Measurements");
	addPort("MagState",portMagState).doc("Mag Measurements");
	addPort("AccelState",portAccelState).doc("Accel Measurements");
//	addPort("driveState",portDriveState).doc("Siemens Drives Measurements");
//	addPort("lineAngles",portLineAngles).doc("Line Angle Sensor Measurements");
//	addPort("data",portData).doc("Resampled measurements from all sensors");

//	memset(&driveState, 0, sizeof( driveState ));
//	memset(&lineAngles, 0, sizeof( driveState ));
//	memset(&resampledMeasurements, 0, sizeof( resampledMeasurements ));
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
	receiver.read(&imuGyro, &imuMag, &imuAccel);
	TIME_TYPE trigger = TimeService::Instance()->getTicks();
	//portDriveState.read(driveState);

	//resampledMeasurements.ts_trigger = trigger;
	//resampledMeasurements.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	//portData.write(resampledMeasurements);

	imuGyro.ts_trigger = trigger;
	imuMag.ts_trigger = trigger;
	imuAccel.ts_trigger = trigger;
	imuGyro.ts_elapsed = TimeService::Instance()->secondsSince(trigger);
	imuMag.ts_elapsed = TimeService::Instance()->secondsSince(trigger);
	imuAccel.ts_elapsed = TimeService::Instance()->secondsSince(trigger);
	portGyroState.write(imuGyro);
	portMagState.write(imuMag);
	portAccelState.write(imuAccel);

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
