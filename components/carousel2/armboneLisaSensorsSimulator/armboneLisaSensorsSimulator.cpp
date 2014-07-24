#include "armboneLisaSensorsSimulator.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

ArmboneLisaSensorsSimulator::ArmboneLisaSensorsSimulator(std::string name):TaskContext(name,PreOperational) 
{
//	addPort("driveState",portDriveState).doc("Siemens Drives Measurements");
	addPort("GyroState",portGyroState).doc("Gyro Measurements");
	addPort("MagState",portMagState).doc("Mag Measurements");
	addPort("AccelState",portAccelState).doc("Accel Measurements");

}

bool ArmboneLisaSensorsSimulator::configureHook()
{
	memset(&imuGyro, 0, sizeof( ImuGyro));	
	memset(&imuMag, 0, sizeof( ImuMag));	
	memset(&imuAccel, 0, sizeof( ImuAccel));	
	return true;
}

bool  ArmboneLisaSensorsSimulator::startHook()
{
	return true;
}

void  ArmboneLisaSensorsSimulator::updateHook()
{

	usleep(7000); // Simulate waiting for the udp packet

	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	// Go ahead and write all the ports.
	// Note, in the REAL component these messages actually
	// arrive at slightly different times.
	
	// TODO: Add some more interesting dummy data in here...
	//state.winchSpeedSmoothed = 3.0 + .001* sin(.2*3.1415 * trigger * 1e-9 + 1.8);

	imuGyro.ts_elapsed = TimeService::Instance()->secondsSince(trigger);
	portGyroState.write(imuGyro);
	imuMag.ts_elapsed = TimeService::Instance()->secondsSince(trigger);
	portMagState.write(imuMag);
	imuAccel.ts_elapsed = TimeService::Instance()->secondsSince(trigger);
	portAccelState.write(imuAccel);

	this->getActivity()->trigger(); // This makes the component re-trigger automatically

}

void  ArmboneLisaSensorsSimulator::stopHook()
{}

void  ArmboneLisaSensorsSimulator::cleanupHook()
{}

void  ArmboneLisaSensorsSimulator::errorHook()
{}

ORO_CREATE_COMPONENT( ArmboneLisaSensorsSimulator )
