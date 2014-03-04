#include "IndoorsCarouselSimulator.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <algorithm>
#include <cmath>

using namespace std;
using namespace RTT;
using namespace RTT::os;

#define CAM_DATA_SIZE 12
#define TRIGGER_OFFSET 1

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

	// NOTE: Dimensions are hard-coded, should be compatible with LEDTracker component
	camData.positions.resize(CAM_DATA_SIZE, 0.0);
	camData.weights.resize(CAM_DATA_SIZE, 0.0);
	camData.pose.resize(CAM_DATA_SIZE, 0.0);
	camData.ts_trigger = trigger;
	portLEDTrackerData.setDataSample( camData );

	winchData.ts_trigger = trigger;
	portWinchData.setDataSample( winchData );

	portTrigger.write( trigger );

	//
	// Add constants for the configuration of the simulator
	//

	samplingTime = sim_sampling_time;
	addConstant("sim_sampling_time", samplingTime);

	mcuTime.ts = mcu_ts / sim_sampling_time;
	mcuTime.td = mcu_td / sim_sampling_time;

	addConstant("mcu_ts", mcuTime.ts);
	addConstant("mcu_td", mcuTime.td);

	encTime.ts = enc_ts / sim_sampling_time;
	encTime.td = enc_td / sim_sampling_time;

	addConstant("enc_ts", encTime.ts);
	addConstant("enc_td", encTime.td);

	camTime.ts = cam_ts / sim_sampling_time;
	camTime.td = cam_td / sim_sampling_time;

	addConstant("cam_ts", camTime.ts);
	addConstant("cam_td", camTime.td);

	winchTime.ts = winch_ts / sim_sampling_time;
	winchTime.td = winch_td / sim_sampling_time;

	addConstant("winch_ts", winchTime.ts);
	addConstant("winch_td", winchTime.td);

	trigger_ts = mhe_ts / sim_sampling_time;
	// The delay is the max of all delays, here camTime
	trigger_td = camTime.td + TRIGGER_OFFSET;
	trigger_enable = false;
	
	addConstant("trigger_ts", trigger_ts);

	//
	// Set-up ACADO stuff
	//

	integratorIO.resize(NX + NXA + NU, 0.0);
	outputs.resize(ACADO_NOUT[ 0 ], 0.0);

	//
	// Debug data conf
	//
	debugData.ts_trigger = trigger;
	debugData.ts_elapsed = 0.0;
	portDebugData.write( debugData );
	
	trigger_cnt = 0;
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
	for (unsigned el = 0; el < NXA; integratorIO[NX + el] = ss_z[ el ], el++);
	for (unsigned el = 0; el < NU; integratorIO[NX + NXA + el] = ss_u[ el ], el++);

	firstRun = true;

	trigger_cnt = trigger_td;
	trigger_enable =  false;

	return true;
}


void IndoorsCarouselSimulator::updateHook( )
{
	int status = 0;

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

	// Update trigegr at the end, so that the delayed measurements get sent to corr. port
	updateTrigger();

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


void IndoorsCarouselSimulator::updateTrigger()
{
	if (trigger_enable == false && --trigger_cnt <= 0)
	{
		trigger_enable = true;
	}

	if (trigger_enable == true && --trigger_cnt <= 0)
	{
		trigger_cnt = trigger_ts;

		portTrigger.write( trigger );
	}
}


void IndoorsCarouselSimulator::updateMcuData()
{
	if (--mcuTime.cnt_ts <= 0)
	{
		mcuTime.cnt_ts = mcuTime.ts;

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

		if (mcuTime.cnt_td_enable == false)
		{
			mcuTime.cnt_td_enable = true;
		}
	}

	if (mcuTime.cnt_td_enable == true && --mcuTime.cnt_td <= 0)
	{
		mcuTime.cnt_td = mcuTime.ts;

		if (mcuTime.samples.size() > 0)
		{
			portMcuHandlerData.write( mcuTime.samples.front() );
			mcuTime.samples.pop_front();
		}
	}
}

void IndoorsCarouselSimulator::updateEncData()
{
	if (--encTime.cnt_ts <= 0)
	{
		encTime.cnt_ts = encTime.ts;

		encData.sin_theta = -integratorIO[ idx_sin_delta ]; // !!! Sign is inverted!
		encData.cos_theta = integratorIO[ idx_cos_delta ];
		encData.omega = integratorIO[ idx_ddelta ];
		encData.omega_filt_rpm = encData.omega / (2.0 * M_PI) * 60.0;

		encData.ts_trigger = trigger;
		encData.ts_elapsed = 0.0;

		encTime.samples.push_back( encData );

		if (encTime.cnt_td_enable == false)
		{
			encTime.cnt_td_enable = true;
		}
	}

	if (encTime.cnt_td_enable == true && --encTime.cnt_td <= 0)
	{
		encTime.cnt_td = encTime.ts;

		if (encTime.samples.size() > 0)
		{
			portEncoderData.write( encTime.samples.front() );
			encTime.samples.pop_front();

			// It's not implemented yet, but MHE requires connection to this source
			portLASData.write( lasData );
		}
	}
}

void IndoorsCarouselSimulator::updateCamData()
{
	if (--camTime.cnt_ts <= 0)
	{
		camTime.cnt_ts = camTime.ts;

		for (unsigned el = 0; el < CAM_DATA_SIZE; ++el)
			camData.positions[ el ] = outputs[offset_marker_positions + el];

		camData.ts_trigger = trigger;
		camData.ts_elapsed = 0.0;

		camTime.samples.push_back( camData );

		if (camTime.cnt_td_enable == false)
		{
			camTime.cnt_td_enable = true;
		}
	}

	if (camTime.cnt_td_enable == true && --camTime.cnt_td <= 0)
	{
		camTime.cnt_td = camTime.ts;

		if (camTime.samples.size() > 0)
		{
			portLEDTrackerData.write( camTime.samples.front() );
			camTime.samples.pop_front();
		}
	}
}

void IndoorsCarouselSimulator::updateWinchData()
{
	if (--winchTime.cnt_ts <= 0)
	{
		winchTime.cnt_ts = winchTime.ts;

		winchData.length = integratorIO[ idx_r ];
		winchData.speed = integratorIO[ idx_dr ];

		winchData.ts_trigger = trigger;
		winchData.ts_elapsed = 0.0;

		winchTime.samples.push_back( winchData );

		if (winchTime.cnt_td_enable == false)
		{
			winchTime.cnt_td_enable = true;
		}
	}

	if (winchTime.cnt_td_enable == true && --winchTime.cnt_td <= 0)
	{
		winchTime.cnt_td = winchTime.ts;

		if (winchTime.samples.size() > 0)
		{
			portWinchData.write( winchTime.samples.front() );
			winchTime.samples.pop_front();
		}
	}
}

ORO_CREATE_COMPONENT( IndoorsCarouselSimulator )
