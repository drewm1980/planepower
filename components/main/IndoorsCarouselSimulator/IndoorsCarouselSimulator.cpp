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

	winchData.ts_trigger = trigger;
	portWinchData.setDataSample( winchData );

	portTrigger.write( trigger );

	//
	// Add constants for the configuration of the simulator
	// this->addConstant( "pi", pi );
	//

	mcuTime.ts = unsigned(mcu_ts / sim_sampling_time);
	mcuTime.td = unsigned(mcu_td / sim_sampling_time);

	addConstant("mcu_ts", mcuTime.ts);
	addConstant("mcu_td", mcuTime.td);

	encTime.ts = unsigned(enc_ts / sim_sampling_time);
	encTime.td = unsigned(enc_td / sim_sampling_time);

	addConstant("enc_ts", encTime.ts);
	addConstant("enc_td", encTime.td);

	camTime.ts = unsigned(cam_ts / sim_sampling_time);
	camTime.td = unsigned(cam_td / sim_sampling_time);

	addConstant("cam_ts", camTime.ts);
	addConstant("cam_td", camTime.td);

	winchTime.ts = unsigned(winch_ts / sim_sampling_time);
	winchTime.td = unsigned(winch_td / sim_sampling_time);

	addConstant("winch_ts", winchTime.ts);
	addConstant("winch_td", winchTime.td);;
}


bool IndoorsCarouselSimulator::configureHook( )
{
	return true;
}


bool IndoorsCarouselSimulator::startHook( )
{
	mcuTime.reset();
	encTime.reset();
	camTime.reset();
	winchTime.reset();

	return true;
}


void IndoorsCarouselSimulator::updateHook( )
{
	// 1) Take the step

	// 2) Put outputs to the ports according to the specs (from python codegen)
	updateMcuData();
	updateEncData();
	updateCamData();
	updateWinchData();
}


void IndoorsCarouselSimulator::stopHook( )
{}


void IndoorsCarouselSimulator::cleanupHook( )
{}


void IndoorsCarouselSimulator::errorHook( )
{}


void IndoorsCarouselSimulator::exceptionHook()
{}

void IndoorsCarouselSimulator::updateMcuData()
{

}

void IndoorsCarouselSimulator::updateEncData()
{

}

void IndoorsCarouselSimulator::updateCamData()
{

}

void IndoorsCarouselSimulator::updateWinchData()
{

}

ORO_CREATE_COMPONENT( IndoorsCarouselSimulator )
