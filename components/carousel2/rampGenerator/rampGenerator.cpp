#include "rampGenerator.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

RampGenerator::RampGenerator(std::string name):TaskContext(name,PreOperational) 
{
	//log(Error) << "Error in constructor of RampGenerator" << endlog();

	addPort("driveState",portDriveState).doc("Siemens Drives Measurements");
//	addPort("lineAngles",portLineAngles).doc("Line Angle Sensor Measurements");
	addPort("driveCommand",portDriveCommand).doc("Resampled measurements from all sensors");

	memset(&driveState, 0, sizeof( driveState ));
	memset(&driveCommand, 0, sizeof( driveCommand ));
//	memset(&lineAngles, 0, sizeof( driveState ));
//	memset(&resampledMeasurements, 0, sizeof( resampledMeasurements ));

	addProperty("acceleration", acceleration).doc("The acceleration (slope) of the ramp");
	addProperty("targetSpeed", acceleration).doc("target Speed of the ramp");
}

bool RampGenerator::configureHook()
{
	acceleration = 0;
	targetSpeed = 0;
	return true;
}

bool  RampGenerator::startHook()
{
//	TIME_TYPE startTime = TimeService::Instance()->getTicks();
	return true;
}

void  RampGenerator::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();
	//portDriveState.read(driveState);

	//resampledMeasurements.ts_trigger = trigger;
	//resampledMeasurements.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	//portData.write(resampledMeasurements);

	double softlimit = 3.1415;
	double currentSetpoint;
	double currentSpeed;
	double nextSetpoint;
        double dt = .5; // s
        double threshold = acceleration; // Rad/s
        int retrys = 10;

        if (abs(targetSpeed) > softlimit) {
                log(Error) << "Requested speed is outside the soft limit!" << endl;
        }
	while (true) {
		portDriveState.read(driveState);
		currentSetpoint = driveState.carouselSpeedSetpoint;
		currentSpeed = driveState.carouselSpeedSmoothed;
		nextSetpoint = currentSetpoint;
		//check if targetspeed is reached
        	if (abs(currentSetpoint - targetSpeed) < threshold) {
                	nextSetpoint = targetSpeed;
        	        log(Info) << "Ramp goal achieved!" << endl;
     	 		return;
        	}
   	     	// check if setpoint is reached
       	 	if (abs(currentSetpoint - currentSpeed) < threshold) {
	        	if (currentSetpoint > targetSpeed) {
                  		// "Ramping down..."
                        	nextSetpoint = max(targetSpeed, currentSetpoint - dt*acceleration);
                	}
			else {
                        	//print "Ramping up..."
                        	nextSetpoint = min(targetSpeed, currentSetpoint + dt*acceleration);
                	}
	        	retrys = 10;
		}
        	else {
                	log(Warning) << "Current setpoint not reached! Retrying(" << retrys << ")" << endl;
                	retrys--;
        	}
        	// check if ramp got stuck
        	if (retrys <= 0) {
                	log(Error) << "Aborting ramp! Current setpoint = " << currentSetpoint << endl;
                       	return;
		}
        	else {
                	//print "Target Speed:  Current Speed: Next Speed: 
                 	sleep(dt);
        	}	

		driveCommand.carouselSpeedSetpoint = nextSetpoint;
		driveCommand.ts_trigger = trigger;
		driveCommand.ts_elapsed = TimeService::Instance()->secondsSince(trigger);
		portDriveCommand.write(driveCommand);
	}
}

void  RampGenerator::stopHook()
{}

void  RampGenerator::cleanupHook()
{}

void  RampGenerator::errorHook()
{}

ORO_CREATE_COMPONENT( RampGenerator )
