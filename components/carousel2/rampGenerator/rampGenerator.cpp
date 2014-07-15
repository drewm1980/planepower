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
	info = "";
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
	
	bool debug = true;
	std::string debuginfo;
	if (debug) {
		ostringstream d;
		d << endl << "targetSpeed: " << targetSpeed << " acceleration: " << acceleration << endl;
		d << "cSetpoint: " << currentSetpoint << " cSpeed: " << currentSpeed << " nextSetpoint: " << nextSetpoint << endl;
	 	debuginfo = d.str();
	

	}

        threshold = 0.1; // Rad/s

        if (abs(targetSpeed) > softlimit) {
                info = "Requested speed is outside the soft limit!";
        }
	else {
		portDriveState.read(driveState);
		currentSetpoint = driveState.carouselSpeedSetpoint;
		currentSpeed = driveState.carouselSpeedSmoothed;
		nextSetpoint = currentSetpoint;
   	     	// check if setpoint is reached
       	 	if (abs(currentSetpoint - currentSpeed) < threshold) {
			//check if targetspeed is reached
        		if (abs(currentSetpoint - targetSpeed) < threshold) {
                		nextSetpoint = targetSpeed;
        	        	info = "Ramp goal achieved! Stoping rampGenerator...";
		//		stop();
        		}
			else {
	        		if (currentSetpoint > targetSpeed) {
                  			info = "Ramping down...";
                        		nextSetpoint = max(targetSpeed, currentSetpoint - dt*acceleration);
                		}
				else {
                        		info = "Ramping up...";
                        		nextSetpoint = min(targetSpeed, currentSetpoint + dt*acceleration);
                		}
			}
	        	retrys = 10;
		}
        	else {
			ostringstream s;
			s << "Current setpoint not reached! Retrying(" << retrys << ")" ;
			info = s.str();
               		retrys--;
        	}
        	// check if ramp got stuck
        	if (retrys <= 0) {
               		ostringstream st;
			st << "Aborting ramp! Current setpoint, speed = " << currentSetpoint << ", " << currentSpeed;
		 	targetSpeed = 0;	
                       	acceleration = 0.1;
			nextSetpoint = max(targetSpeed, currentSetpoint - dt*acceleration);
			info = st.str();
		}
        	else {}	
		
		info += debuginfo;
		portInfo.write(info);
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
