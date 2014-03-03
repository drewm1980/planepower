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
	addPort("debugData", portDebugData)
		.doc("Debug data");

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
	//

	mcuTime.ts = mcu_ts / sim_sampling_time);
	mcuTime.td = mcu_td / sim_sampling_time);

	addConstant("mcu_ts", mcuTime.ts);
	addConstant("mcu_td", mcuTime.td);

	encTime.ts = enc_ts / sim_sampling_time);
	encTime.td = enc_td / sim_sampling_time);

	addConstant("enc_ts", encTime.ts);
	addConstant("enc_td", encTime.td);

	camTime.ts = cam_ts / sim_sampling_time);
	camTime.td = cam_td / sim_sampling_time);

	addConstant("cam_ts", camTime.ts);
	addConstant("cam_td", camTime.td);

	winchTime.ts = winch_ts / sim_sampling_time);
	winchTime.td = winch_td / sim_sampling_time);

	addConstant("winch_ts", winchTime.ts);
	addConstant("winch_td", winchTime.td);

	//
	// Set-up ACADO stuff
	//

	integratorIO.resize(NX + NXA + NU, 0.0);
	outputs.resize(ACADO_NOUT[ 0 ] * (1 + NX + NU), 0.0);

	//
	// Debug data conf
	//
	debugData.ts_trigger = trigger;
	debugData.ts_elapsed = 0.0;
	portDebugData.write( debugData );
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

	// Initialize integrator with steady state
	for (unsigned el = 0; el < NX; integratorIO[ el ] = ss_x[ el ], el++);
	for (unsigned el = NX; el < NX + NXA; integratorIO[ el ] = ss_z[ el ], el++);
	for (unsigned el = NX + NXA; el < NX + NXA+ NU; integratorIO[ el ] = ss_u[ el ], el++);

	firstRun = true;

	return true;
}


void IndoorsCarouselSimulator::updateHook( )
{
	int status;

	trigger = TimeService::Instance()->getTicks();

	// 1) Take the step, i.e. call ACADO generated integrator
	status = integrate(integratorIO.data(), outputs.data(), firstRun == true ? 1 : 0);
	if ( status )
		exception();
	
	if (firstRun == true)
		firstRun = false;

	// 2) Put outputs to the ports according to the specs (from python codegen)
	updateMcuData();
	updateEncData();
	updateCamData();
	updateWinchData();

	// 3) Set some debug info
	debugData.ts_trigger = trigger;
	debugData.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDebugData.write( debugData );
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
	if (++mcuTime.cnt_ts >= mcuTime.ts)
	{
		mcuTime.cnt_ts = 0;

		mcuData.accl_x = outputs[offset_IMU_acceleration + 0];
		mcuData.accl_y = outputs[offset_IMU_acceleration + 1];
		mcuData.accl_z = outputs[offset_IMU_acceleration + 2];

		mcuData.gyro_x = outputs[offset_IMU_angular_velocity + 0];
		mcuData.gyro_y = outputs[offset_IMU_angular_velocity + 1];
		mcuData.gyro_z = outputs[offset_IMU_angular_velocity + 2];

		mcuData.ua1 = integratorIO[ idx_aileron ];
		mcuData.ua2 = integratorIO[ idx_aileron ];
		mcuData.ue  = integratorIO[ idx_elevator ];

		mcuData.ts_trigger = trigger;
		mcuData.ts_elapsed = 0.0;

		mcuTime.samples.push_back( mcuData );
	}

	if (++mcuTime.cnt_td >= mcuTime.td)
	{
		mcuTime.cnt_td = 0;

		if (mcuTime.samples.size() > 0)
		{
			portMcuHandlerData.write( mcuTime.samples.front() );
			mcuTime.samples.pop_front();
		}
	}
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
