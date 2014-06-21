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
	//log(Error) << "Error in constructor of GainLoader" << endlog();

//	addPort("driveState",portDriveState).doc("Siemens Drives Measurements");
//	addPort("lineAngles",portLineAngles).doc("Line Angle Sensor Measurements");
//	addPort("data",portData).doc("Resampled measurements from all sensors");

//	memset(&driveState, 0, sizeof( driveState ));
//	memset(&lineAngles, 0, sizeof( driveState ));
//	memset(&resampledMeasurements, 0, sizeof( resampledMeasurements ));
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
	//portDriveState.read(driveState);

	//resampledMeasurements.ts_trigger = trigger;
	//resampledMeasurements.ts_elapsed = TimeService::Instance()->secondsSince( trigger 
	//portData.write(resampledMeasurements);

}

void  GainLoader::stopHook()
{}

void  GainLoader::cleanupHook()
{}

void  GainLoader::errorHook()
{}

ORO_CREATE_COMPONENT( GainLoader )
