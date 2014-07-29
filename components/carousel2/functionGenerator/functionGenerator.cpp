#include "functionGenerator.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;
const double PI = 3.141592653589793238463;

FunctionGenerator::FunctionGenerator(std::string name):TaskContext(name,PreOperational) 
{
	addPort("data",portDriveCommand).doc("Command to the Siemens Drives");
	addPort("refData",portReference).doc("Reference Elevation for the controller");
	addEventPort("triggerIn",portTriggerIn).doc("Event port for driggering the functionGenerator timing off of another component.  IF YOU TRIGGER THIS WAY, REMEMBER TO MAKE THE COMPONENT NON_PERIODIC");

	memset(&driveCommand, 0, sizeof(driveCommand));
	memset(&reference, 0, sizeof(reference));

	// Add properties for what kind of function should be generated
	addProperty("type", type).doc("The type of the generated function. sin wave is 0 square wave is 1");
	addProperty("whichDrive", whichDrive).doc("Which drive to command, winch is 0, carousel is 1");
	addProperty("amplitude", amplitude).doc("The amplitude of the generated function.");
	addProperty("offset", offset).doc("The offset of the generated function.");
	addProperty("phase", phase).doc("The phase of the generated function.");
	addProperty("frequency", frequency).doc("The frequency of the generated function.");
	addProperty("numberOfSines", numberOfSines).doc("The number of sines in the multisine.");
}

bool FunctionGenerator::configureHook()
{
	type = 0;
	whichDrive = 0;
	amplitude = 0;
	offset = 0;
	phase = 0;
	frequency = 0;
	numberOfSines = 0;
	return true;
}

bool  FunctionGenerator::startHook()
{
	startTime = TimeService::Instance()->getTicks();
	return true;
}

void  FunctionGenerator::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();
	double t = (trigger - startTime) * 1e-9;
	double value;
	double sinvalue = 0;
	double wLowest = frequency * 2 * PI;		
	switch (type)
	{
		case 0:
			value = offset + amplitude * sin(t * frequency*(2.0*3.1415) + phase);
			break;
		case 1:
			sinvalue = sin(t * frequency*(2.0*3.1415) + phase);
			value = offset + amplitude * (2*(sinvalue > 0.0)-1.0);
			break;
		case 2:
			for (int k = 1; k <= numberOfSines; k++) {
				double w = k * wLowest;
				double multiPhase = -k * (k-1) * PI / numberOfSines;
				sinvalue += amplitude * sin(w*t + multiPhase);
			}
			value = offset + sinvalue;
			//adding cycle clock find the periods easy
			reference.cycle = (double)(sin(wLowest*t) > 0.0);	
			break;
		default:
			log(Error) << "Unrecognized signal type!" << endl;
			return;
	}

	switch (whichDrive)
	{
		case 0:
			driveCommand.winchSpeedSetpoint = value;
			driveCommand.carouselSpeedSetpoint = 0.0;
			break;
		case 1:
			driveCommand.winchSpeedSetpoint = 0.0;
			driveCommand.carouselSpeedSetpoint = value;
			reference.elevation = value;	
			break;
		default:
			log(Error) << "functionGenerator: whichDrive selection invalid!" << endl;
			return;
	}

	driveCommand.ts_trigger = trigger;
	driveCommand.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDriveCommand.write(driveCommand);
	
	reference.ts_trigger = trigger;
	reference.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portReference.write(reference);

}


void  FunctionGenerator::stopHook()
{
	/*This is not safe!! That will be a hard stop if the offset is high. For stoping the function generator use stopFunctionGenerator() in main.lua
	// Make it so stopping the component stops the drives
	TIME_TYPE trigger = TimeService::Instance()->getTicks();
	driveCommand.winchSpeedSetpoint = 0.0;
	driveCommand.carouselSpeedSetpoint = 0.0;
	driveCommand.ts_trigger = trigger;
	driveCommand.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDriveCommand.write(driveCommand);*/
}

void  FunctionGenerator::cleanupHook()
{}

void  FunctionGenerator::errorHook()
{}

ORO_CREATE_COMPONENT( FunctionGenerator )
