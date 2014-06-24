#include "siemensActuators.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

SiemensActuators::SiemensActuators(std::string name):TaskContext(name,PreOperational) 
{
	sender = new SiemensSender;

	addEventPort("controls", portControls)
		.doc("Command to be sent to the Siemens Drives");

#ifdef NONREALTIME_DEBUGGING
	addOperation("setCarouselSpeed", &SiemensSender::send_carousel_calibrated_speed, this->sender, OwnThread)
		.doc("Set the speed reference for the carousel drive. NOTE: This will only have an affect when the carousel drives are fully enabled!")
		.arg("carousel_speed", "Carousel arm rotation speed in Radians/s.");

	addOperation("setWinchSpeed", &SiemensSender::send_winch_calibrated_speed, this->sender, OwnThread)
		.doc("Set the speed reference for the winch drive. NOTE: This will only have an affect when the carousel drives are fully enabled!")
		.arg("winch_speed", "Winch Speed in m/s.");

	addOperation("setBothSpeeds", &SiemensSender::send_calibrated_speeds, this->sender, OwnThread)
		.doc("Set both the winch and carousel speed.  Only has effect when drives are enabled!")
		.arg("winch_speed", "Winch Speed in m/s.")
		.arg("carousel_speed", "Carousel arm rotation speed in Radians/s.");

	addOperation("stopDrives", &SiemensSender::stop_drives, this->sender, OwnThread)
		.doc("Stop the drives!");

#endif
}
SiemensActuators::~SiemensActuators()
{
	delete sender;
}

bool SiemensActuators::configureHook()
{
	return true;
}

bool  SiemensActuators::startHook()
{
	return true;
}

void  SiemensActuators::updateHook()
{
	portControls.read(driveCommand);
	sender->write(driveCommand);
}

void  SiemensActuators::stopHook()
{}

void  SiemensActuators::cleanupHook()
{}

void  SiemensActuators::errorHook()
{}

ORO_CREATE_COMPONENT( SiemensActuators )
