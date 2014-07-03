#include "armboneLisaSensors.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

ArmboneLisaSensors::ArmboneLisaSensors(std::string name):TaskContext(name,PreOperational) 
{
	//log(Error) << "Error in constructor of ArmboneLisaSensors" << endlog();

//	addPort("driveState",portDriveState).doc("Siemens Drives Measurements");
//	addPort("lineAngles",portLineAngles).doc("Line Angle Sensor Measurements");
//	addPort("data",portData).doc("Resampled measurements from all sensors");

//	memset(&driveState, 0, sizeof( driveState ));
//	memset(&lineAngles, 0, sizeof( driveState ));
//	memset(&resampledMeasurements, 0, sizeof( resampledMeasurements ));
}

bool ArmboneLisaSensors::configureHook()
{
	return true;
}

bool  ArmboneLisaSensors::startHook()
{
	return true;
}

void  ArmboneLisaSensors::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();
	//portDriveState.read(driveState);

	//resampledMeasurements.ts_trigger = trigger;
	//resampledMeasurements.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	//portData.write(resampledMeasurements);

}

void  ArmboneLisaSensors::stopHook()
{}

void  ArmboneLisaSensors::cleanupHook()
{}

void  ArmboneLisaSensors::errorHook()
{}

ORO_CREATE_COMPONENT( ArmboneLisaSensors )
