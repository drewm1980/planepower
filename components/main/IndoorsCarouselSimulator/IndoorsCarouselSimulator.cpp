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

IndoorsCarouselSimulator::IndoorsCarouselSimulator(std::string name)
	: TaskContext(name, PreOperational)
{
	addPort("controls", portControls)
		.doc("Plane's control surfaces values");
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

	controls.reset();

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

	unsigned maxDelay = 0;

	samplingTime = sim_sampling_time;
	addConstant("sim_sampling_time", samplingTime);

	mcu.ts = mcu_ts / sim_sampling_time;
	mcu.td = mcu_td / sim_sampling_time;
	if (mcu.td > maxDelay) maxDelay = mcu.td;

	addConstant("mcu_ts", mcu.ts);
	addConstant("mcu_td", mcu.td);

	enc.ts = enc_ts / sim_sampling_time;
	enc.td = enc_td / sim_sampling_time;
	if (enc.td > maxDelay) maxDelay = enc.td;

	addConstant("enc_ts", enc.ts);
	addConstant("enc_td", enc.td);

	cam.ts = cam_ts / sim_sampling_time;
	cam.td = cam_td / sim_sampling_time;
	if (cam.td > maxDelay) maxDelay = cam.td;

	addConstant("cam_ts", cam.ts);
	addConstant("cam_td", cam.td);

	winch.ts = winch_ts / sim_sampling_time;
	winch.td = winch_td / sim_sampling_time;
	if (winch.td > maxDelay) maxDelay = winch.td;

	addConstant("winch_ts", winch.ts);
	addConstant("winch_td", winch.td);

	mhe.ts = mhe_ts / sim_sampling_time;
	mhe.td = 0;
	addConstant("mhe_trigger_ts", mhe.ts);
	addConstant("mhe_trigger_td", mhe.td);

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
}


bool IndoorsCarouselSimulator::configureHook( )
{
	return true;
}


bool IndoorsCarouselSimulator::startHook( )
{
	mcu.reset();
	enc.reset();
	cam.reset();
	winch.reset();
	mhe.reset();

	// Initialize integrator with steady state
	for (unsigned el = 0; el < NX; integratorIO[ el ] = ss_x[ el ], el++);
	for (unsigned el = 0; el < NXA; integratorIO[NX + el] = ss_z[ el ], el++);
	for (unsigned el = 0; el < NU; integratorIO[NX + NXA + el] = ss_u[ el ], el++);

	firstRun = true;

	return true;
}


void IndoorsCarouselSimulator::updateHook( )
{
	int status = 0;

	trigger = TimeService::Instance()->getTicks();

	// 1) Read and controls
	updateControls();

	// 2) Take the step, i.e. call ACADO generated integrator
	status = integrate(integratorIO.data(), outputs.data(), firstRun == true ? 1 : 0);
	if ( status )
	{
		log( Error ) << "*** SIMULATOR DIED ***" << endlog();
		stop();
	}

	updateControls();
	
	if (firstRun == true)
		firstRun = false;

	// 3) Put outputs to the ports according to the specs (from python codegen)
	updateMcuData();
	updateEncData();
	updateCamData();
	updateWinchData();

	// Update MHE trigger at the end, so that the delayed measurements get sent to corr. port
	updateMheTrigger();

	// 4) Set some debug info
	debugData.ts_trigger = trigger;
	debugData.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDebugData.write( debugData );
}


void IndoorsCarouselSimulator::stopHook( )
{}

void IndoorsCarouselSimulator::updateControls()
{
	// TODO Here we should simulate that servos work @ 50 Hz.

	if (portControls.read( controls ) == NewData)
	{
		if (controls.der_ctrl == true)
		{
			integratorIO[NX + NXA + idx_daileron] = controls.d_ua1;
			integratorIO[NX + NXA + idx_delevator] = controls.d_ue;
		}
		else
		{
			integratorIO[idx_aileron] = controls.ua1;
			integratorIO[idx_elevator] = controls.ue;

			integratorIO[NX + NXA + idx_daileron] = 0.0;
			integratorIO[NX + NXA + idx_delevator] = 0.0;
		}
	}

	// Dodgy way to trim the controls
	double ua1 = integratorIO[idx_aileron];
	double ue  = integratorIO[idx_elevator];

	if (ua1 > aileron_bound) ua1 = aileron_bound; else if (ua1 < -aileron_bound) ua1 = -aileron_bound;
	if (ue > elevator_bound) ue = elevator_bound; else if (ue < -elevator_bound) ue = -elevator_bound;
	
	integratorIO[idx_aileron] = ua1;
	integratorIO[idx_elevator] = ue;
}


void IndoorsCarouselSimulator::updateMheTrigger()
{
	uint64_t mheTrigger = trigger;
	if (mhe.update( mheTrigger ) == true)
		portTrigger.write( mheTrigger );
}


void IndoorsCarouselSimulator::updateMcuData()
{
	mcuData.accl_x = outputs[offset_IMU_acceleration + 0];
	mcuData.accl_y = outputs[offset_IMU_acceleration + 1];
	mcuData.accl_z = outputs[offset_IMU_acceleration + 2];
	
	mcuData.gyro_x = outputs[offset_IMU_angular_velocity + 0];
	mcuData.gyro_y = outputs[offset_IMU_angular_velocity + 1];
	mcuData.gyro_z = outputs[offset_IMU_angular_velocity + 2];

	mcuData.ctrl.ua1 = integratorIO[ idx_aileron ];
	mcuData.ctrl.ua2 = integratorIO[ idx_aileron ];
	mcuData.ctrl.ue  = integratorIO[ idx_elevator ];

	mcuData.ts_trigger = trigger;
	mcuData.ts_elapsed = 0.0;

	if (mcu.update( mcuData ) == true)
		portMcuHandlerData.write( mcuData );
}

void IndoorsCarouselSimulator::updateEncData()
{
	encData.sin_theta = -integratorIO[ idx_sin_delta ]; // !!! Sign is inverted!
	encData.cos_theta = integratorIO[ idx_cos_delta ];
	encData.omega = -integratorIO[ idx_ddelta ]; // !!! Sign is inverted!
	encData.omega_filt_rpm = encData.omega / (2.0 * M_PI) * 60.0;

	encData.ts_trigger = trigger;
	encData.ts_elapsed = 0.0;

	if (enc.update( encData  ) == true)
		portEncoderData.write( encData );
}

void IndoorsCarouselSimulator::updateCamData()
{
	for (unsigned el = 0; el < CAM_DATA_SIZE; ++el)
	{
		double pos = ceil(outputs[offset_marker_positions + el]);
		if (pos < 0. || pos > 1200.)
			pos = -1.;
		double weight = pos > 0. ? 1. : -1.;

		camData.positions[ el ] = pos;
		camData.weights[ el ] = weight;
	}

	camData.ts_trigger = trigger;
	camData.ts_elapsed = 0.0;

	if (cam.update( camData ) == true)
		portLEDTrackerData.write( camData );
}

void IndoorsCarouselSimulator::updateWinchData()
{
	winchData.length = integratorIO[ idx_r ];
	winchData.speed = integratorIO[ idx_dr ];

	winchData.ts_trigger = trigger;
	winchData.ts_elapsed = 0.0;

	if (winch.update( winchData ) == true)
		portWinchData.write( winchData );
}

ORO_CREATE_COMPONENT( IndoorsCarouselSimulator )
