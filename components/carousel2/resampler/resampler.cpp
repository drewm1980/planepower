#include "resampler.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

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
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	portDriveState.read(driveState);
	portLineAngles.read(lineAngles);

	// TODO: Actually keep a buffer of previous measurements,
	// and implement a proper downsampling filter of appropriate length.
	// For now, just send the most recent measurements from each sensor.
	resampledMeasurements.winchSpeedSmoothed = driveState.winchSpeedSmoothed;
	resampledMeasurements.winchEncoderPosition = driveState.winchEncoderPosition;
	resampledMeasurements.carouselSpeedSmoothed = driveState.carouselSpeedSmoothed;
	resampledMeasurements.carouselEncoderPosition = driveState.carouselEncoderPosition;

	resampledMeasurements.azimuth = lineAngles.azimuth;
	resampledMeasurements.elevation = lineAngles.elevation;

	resampledMeasurements.ts_trigger = trigger;
	resampledMeasurements.ts_elapsed = TimeService::Instance()->secondsSince( trigger );

	portData.write(resampledMeasurements);
}

void  Resampler::stopHook()
{}

void  Resampler::cleanupHook()
{}

void  Resampler::errorHook()
{}

ORO_CREATE_COMPONENT( Resampler )
