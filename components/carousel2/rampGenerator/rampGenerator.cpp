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
	addPort("info",portInfo).doc("Information about the ramping");
	
	memset(&driveState, 0, sizeof( driveState ));
	memset(&driveCommand, 0, sizeof( driveCommand ));
//	memset(&lineAngles, 0, sizeof( driveState ));
//	memset(&resampledMeasurements, 0, sizeof( resampledMeasurements ));

	addProperty("acceleration", acceleration).doc("The acceleration (slope) of the ramp");
	addProperty("targetSpeed", targetSpeed).doc("target Speed of the ramp");
}

bool RampGenerator::configureHook()
{
	acceleration = 0;
	targetSpeed = 0;
	softlimit = 3.1415;
	currentSetpoint = 0;
	nextSetpoint = 0;
        dt = getPeriod(); // s
        retrys = 10;
        threshold = 0.05; // Rad/s
	info = "";
	state = 0;
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
	int oldstate = state;
	bool debug = false;
	std::string debuginfo;
	if (debug) {
		ostringstream d;
		d << endl << "targetSpeed: " << targetSpeed << " acceleration: " << acceleration << endl;
		d << "cSetpoint: " << currentSetpoint << " cSpeed: " << currentSpeed << " nextSetpoint: " << nextSetpoint << endl;
	 	debuginfo = d.str();
	

	}

	stepheight = dt*acceleration;
        if (fabs(targetSpeed) > softlimit) {
                info = "Requested speed is outside the soft limit!";
		state = 1;
        }
	else {
		FlowStatus stat = portDriveState.read(driveState);
		if (stat == RTT::NewData) {
			currentSetpoint = driveState.carouselSpeedSetpoint;
			currentSpeed = driveState.carouselSpeedSmoothed;
		}
		nextSetpoint = currentSetpoint;
   	     	// check if setpoint is reached
       	 	if (fabs(currentSetpoint - currentSpeed) < threshold) {
			//check if targetspeed is reached
        		if (fabs(currentSetpoint - targetSpeed) < stepheight) {
                		nextSetpoint = targetSpeed;
        	        	info = "Ramp goal achieved! Stoping rampGenerator..."; // if you change this you also have to change it in experiment_helpers.lua
				state = 2;	
        		}
			else if (currentSetpoint > targetSpeed) {
				info = "Ramping down...";
				state = 3;
				nextSetpoint = max(targetSpeed, currentSetpoint - stepheight );
			}
			else {
				info = "Ramping up...";
				state = 4;
				nextSetpoint = min(targetSpeed, currentSetpoint + stepheight);
			}

	        	retrys = 10;
		}
		// check if current speed is more than a stepheight closer to target speed than current setpoint is.
        	else if ( fabs(currentSetpoint - targetSpeed) - fabs(currentSpeed - targetSpeed) > stepheight) {
			nextSetpoint = currentSpeed;
		}
		else {
			ostringstream s;
			s << "Current setpoint not reached! Retrying(" << retrys << ")" ;
			state = 5;
			info = s.str();
               		retrys--;
        	}
        	// check if ramp got stuck
        	if (retrys < -1) {
               		ostringstream st;
			st << "Aborting ramp! Current setpoint = " << currentSetpoint << ", current speed = " << currentSpeed;
		 	state = 6;
			targetSpeed = 0;	
                       	acceleration = 0.1;
			nextSetpoint = max(targetSpeed, currentSetpoint - stepheight);
			info = st.str();
		}
        	else {}	
		
		if (oldstate != state || state==5) {
			info += debuginfo;
			portInfo.write(info);
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
