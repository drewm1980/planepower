#include "IndoorsCarouselSimulator.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <algorithm>
#include <cmath>

// We need this for loading some constants
#include "LEDTracker/LEDTracker.hpp"

using namespace std;
using namespace RTT;
using namespace RTT::os;

IndoorsCarouselSimulator::IndoorsCarouselSimulator(std::string name)
	: TaskContext(name, PreOperational)
{
	addPort("trigger", portTrigger)
		.doc("Trigger port");
	addPort("mcuData", portMcuHandlerData)
		.doc("MCU handler inputs");
	addPort("encoderData",portEncoderData)
		.doc("Encoder data");
	addPort("ledTrackerData", portLEDTrackerData)
		.doc("LED Tracker data");
	addPort("lasData", portLASData)
		.doc("Line angle sensor data");
	addPort("winchData", portWinchData)
		.doc("Winch data");

	//
	// Set data samples
	//

	trigger = TimeService::Instance()->getTicks();

	mcuData.ts_trigger = trigger;
	portMcuHandlerData.setDataSample( mcuData );

	encData.ts_trigger = trigger;
	portEncoderData.setDataSample( encData );

	camData.positions.resize(CAMERA_COUNT * LED_COUNT * 2, 0.0);
	camData.weights.resize(CAMERA_COUNT * LED_COUNT * 2, 0.0);
	camData.pose.resize(NPOSE, 0.0);
	camData.ts_trigger = trigger;
	portLEDTrackerData.setDataSample( camData );

	lasData.ts_trigger = trigger;
	portLASData.setDataSample( lasData );

	winchData.ts_trigger = trigger;
	portWinchData.setDataSample( winchData );

	portTrigger.write( trigger );

	//
	// Add constants for the configuration of the simulator
	// this->addConstant( "pi", pi );
	//

}


bool IndoorsCarouselSimulator::configureHook( )
{
	return true;
}


bool IndoorsCarouselSimulator::startHook( )
{
	return true;
}


void IndoorsCarouselSimulator::updateHook( )
{
	// 1) Take the step

	// 2) Put outputs to the ports according to the specs (from python codegen)
}


void IndoorsCarouselSimulator::stopHook( )
{}


void IndoorsCarouselSimulator::cleanupHook( )
{}


void IndoorsCarouselSimulator::errorHook( )
{}


void IndoorsCarouselSimulator::exceptionHook()
{}
