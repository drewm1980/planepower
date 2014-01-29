#include "encoder.hpp"

#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <cmath>

using namespace std;
using namespace RTT;
using namespace RTT::os;
using namespace soem_ebox;

/// Gear ratio of the gearbox of the carousel
#define GEAR_RATIO 32.8
/// Number of pulses per revolution of the encoder
#define PULSES_PER_REVOLUTION 4096.0
/// PI
#define PI 3.14159265358979323846264338327950288419716939937510

/// Cut-off Frequency of the filter [Hz]
#define DFILTER_FC 0.5

/// Digital filter coefficient calculation
#define DFILTER_K(fc, fs) \
	exp( -2.0 * M_PI * (fc) / (fs) )
/// 1-order low-pass digital filter
#define DFILTER(Output, OldOutput, Input, fc, fs) \
	Output = ( 1 - DFILTER_K(fc, fs) ) * Input + DFILTER_K(fc, fs) * OldOutput;


Encoder::Encoder(std::string name)
	: TaskContext(name, PreOperational)
{
	//
	// Add ports
	//
	addEventPort("eboxOut", portEboxOut)
		.doc("EBOX encoder port");
	addPort("eboxAnalog", portEboxAnalog)
		.doc("EBOX analog data port");
	addPort("data", portEncoderData)
		.doc("Output port for encoder data");
		
	//
	// Add properties
	//
	addProperty("encoderPort", encoderPort);
	addProperty("Ts", Ts);

	//
	// Prepare ports
	//
	portEncoderData.setDataSample( encoderData );
	portEncoderData.write( encoderData );
}

bool  Encoder::configureHook()
{
	Logger::In in( getName() );

	if (portEboxOut.connected() == false)
	{
		log( Error ) << "Encoder port is not connected" << endlog();
		return false;
	}

	return true;
}

bool  Encoder::startHook()
{
	portEboxOut.read( eboxOut );
	posOld = posNew = eboxOut.encoder[ encoderPort ];
	timeStampOld = TimeService::Instance()->getTicks();

	omegaFiltNew = omegaFiltOld = 0.0;
	posAcc = posRaw = 0.0;

	analogConn = portEboxAnalog.connected();
	encoderData.dbg_speed_voltage = 0.0;
	
    return true;
}

void  Encoder::updateHook()
{
	tickStart = TimeService::Instance()->getTicks();

	// Read new position and corresponding time-stamp
	// TODO Encoder provides the time stamp, too
	portEboxOut.read( eboxOut );
	timeStampNew = TimeService::Instance()->getTicks();
	
	// Read voltage sent to the carousel for the debugging purposes
	if (analogConn == true)
	{
		portEboxAnalog.read( eboxAnalog );
		encoderData.dbg_speed_voltage = eboxAnalog.analog[ 0 ];
	}

	// Read and log the new, raw, position
	posNew = eboxOut.encoder[ encoderPort ];
	encoderData.dbg_raw_angle = posNew;
		
	// Read elapsed time since the last position reading
	elapsedTime = TimeService::Instance()->secondsSince( timeStampOld );
	
	// Knowing that posOld, posNew and posDelta are 32bit signed numbers
	// (int's) there are no sign nor overflow problems. Sign is inverted
	// because of the agreed positive sign of the rotation of the carousel
	posDelta = posOld - posNew;
	
	// Convert encoder ticks to real angle in radians and bound it to -pi.. pi
	posDeltaReal = (double)posDelta * 2.0 * M_PI / GEAR_RATIO / PULSES_PER_REVOLUTION;

	// Just for debugging, log the raw angle
	posRaw += posDeltaReal;
	encoderData.dbg_angle = posRaw;

	// Accumulate and limit the angle
	posAcc += posDeltaReal;
	if (posAcc > M_PI)
		posAcc -= 2.0 * M_PI;
	else if (posAcc < -PI)
		posAcc += 2.0 * M_PI;
	
	// Calculate angular velocity [rad/s], and filter it through low-pass filter
	omegaNew = posDeltaReal / elapsedTime;
	DFILTER(omegaFiltNew, omegaFiltOld, omegaNew, DFILTER_FC, (1 / Ts));
	
	// Fill in the output vector
	encoderData.theta = posAcc;
	encoderData.sin_theta = sin( posAcc );
	encoderData.cos_theta = cos( posAcc );
	encoderData.omega = omegaNew;
	encoderData.omega_filt_rpm = omegaFiltNew / (2.0 * PI) * 60.0;
	
	// Prepare for the next sampling time.
	posOld = posNew;
	timeStampOld = timeStampNew;
	omegaFiltOld = omegaFiltNew;
	
	// Output data to the port
	encoderData.ts_trigger = timeStampNew;
	encoderData.ts_elapsed = TimeService::Instance()->secondsSince( tickStart );

	portEncoderData.write( encoderData );
}

void  Encoder::stopHook()
{}

void  Encoder::errorHook()
{}

void  Encoder::cleanupHook()
{}

ORO_LIST_COMPONENT_TYPE( Encoder )
