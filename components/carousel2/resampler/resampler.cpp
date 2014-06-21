#include "resampler.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

Resampler::Resampler(std::string name):TaskContext(name,PreOperational) 
{
	addPort("driveState",portDriveState).doc("Siemens Drives Measurements");
	addPort("lineAngles",portLineAngles).doc("Line Angle Sensor Measurements");
	addPort("data",portData).doc("Resampled measurements from all sensors");

	memset(&driveState, 0, sizeof( driveState ));
	memset(&lineAngles, 0, sizeof( driveState ));
	memset(&resampledMeasurements, 0, sizeof( resampledMeasurements ));
}

bool Resampler::configureHook()
{
	return true;
}

bool  Resampler::startHook()
{
	return true;
}

void  Resampler::updateHook()
{
	portDriveState.read(driveState);
	portLineAngles.read(lineAngles);

	portData.write(resampledMeasurements);
}

void  Resampler::stopHook()
{}

void  Resampler::cleanupHook()
{}

void  Resampler::errorHook()
{}

ORO_CREATE_COMPONENT( Resampler )
