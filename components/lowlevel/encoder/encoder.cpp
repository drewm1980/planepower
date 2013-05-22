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

Encoder::Encoder(std::string name)
	: TaskContext( name )
{
	//
	// Add ports
	//
	addEventPort("eboxOut", portEboxOut)
		.doc("EBOX encoder port");
	addPort("encoderData", portEncoderData)
		.doc(	"Output port for encoder data:"
				"[timestamp, delta, sin_delta, cos_delta, omega, omega_filtered, omega_rpm].");
	addPort("execTime", portExecTime)
		.doc("Execution time of the component.");
	
	//
	// Add properties
	//
	addProperty("encoderPort", encoderPort);

	//
	// Prepare ports
	//
	encoderData.resize(7, 0.0);
	portEncoderData.setDataSample( encoderData );
	portEncoderData.write( encoderData );
}

bool  Encoder::configureHook()
{
	// TODO If the encoder port is not connected, abort!

	return true;
}

bool  Encoder::startHook()
{
	portEboxOut.read( eboxOut );
	posOld = posNew = eboxOut.encoder[ encoderPort ];
	timeStampOld = TimeService::Instance()->getTicks();

	omegaOld = 0.0;
	posAcc = 0.0;
	
    return true;
}

void  Encoder::updateHook()
{
	tickStart = TimeService::Instance()->getTicks();

	// Read new position and corresponding time-stamp
	// TODO Encoder provides the time stamp, too
	portEboxOut.read( eboxOut );
	timeStampNew = TimeService::Instance()->getTicks();
	posNew = eboxOut.encoder[ encoderPort ];
		
	// Read elapsed time since the last position reading
	elapsedTime = TimeService::Instance()->secondsSince( timeStampOld );
	
	// Knowing that posOld, posNew and posDelta are 32bit signed numbers
	// (int's) there are no sign nor overflow problems. Sign is inverted
	// because of the agreed positive sign of the rotation of the carousel
	posDelta = posOld - posNew;
	
	// Convert encoder ticks to real angle in radians and bound it to -pi.. pi
	posDeltaReal = (double)posDelta * 2.0 * PI / GEAR_RATIO / PULSES_PER_REVOLUTION;
	posAcc += posDeltaReal;
	if (posAcc > PI)
		posAcc -= 2.0 * PI;
	else if (posAcc < -PI)
		posAcc += 2.0 * PI;
	
	// Calculate angular velocity [rad/s]
	omegaNew = posDeltaReal / elapsedTime;
	
	// TODO Apply first order filter to omegaNew
	
	// Invert the signs and fill in the output vector
	encoderData[ 0 ] = timeStampNew;
	encoderData[ 1 ] = posAcc;
	encoderData[ 2 ] = sin( posAcc );
	encoderData[ 3 ] = cos( posAcc );
	encoderData[ 4 ] = omegaNew;
	encoderData[ 5 ] = omegaNew; 
	encoderData[ 6 ] = omegaNew / (2.0 * PI) * 60.0;
	
	// Output data to the port
	portEncoderData.write( encoderData );
	
	// Prepare for the next sampling time.
	posOld = posNew;
	timeStampOld = timeStampNew;
	omegaOld = omegaNew;
	
	// Output execution time of the component
	portExecTime.write(
		TimeService::Instance()->secondsSince( tickStart )
	);
}

void  Encoder::stopHook()
{}

void  Encoder::errorHook()
{}

void  Encoder::cleanupHook()
{}

ORO_CREATE_COMPONENT( Encoder )
