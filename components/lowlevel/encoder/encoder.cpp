#include "encoder.hpp"

#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <cmath>

using namespace std;
using namespace RTT;
using namespace RTT::os;

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
	addEventPort("trigger", portTrigger)
		.doc("Trigger port for the component.");
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
	if (this->hasPeer("soemMaster") == false)
	{
		log( Error ) << "There is no peer" << "soemMaster" << endlog();
		goto configureHookFail;
	}
	
	if (this->getPeer("soemMaster")->provides()->hasService("Slave_1001") == false)
	{
		log( Error ) << "There is no service" << "Slave_1001" << endlog();
		goto configureHookFail;
	}
	
	if (this->getPeer("soemMaster")->provides()
		->getService("Slave_1001")->provides()->hasOperation("readEncoder") == false)
	{
		log( Error ) << "There is no operation" << "readEncoder" << endlog();
		goto configureHookFail;
	}
	
	readEncoder = getPeer("soemMaster")->provides()
		->getService("Slave_1001")->provides()->getOperation("readEncoder");
		
	return true;
	
	configureHookFail:
	
	return false;
}

bool  Encoder::startHook()
{
	posOld = readEncoder( encoderPort );
	posOld = readEncoder( encoderPort );
	posOld = readEncoder( encoderPort );

	omegaOld = 0.0;
	timeStampOld = TimeService::Instance()->getTicks();
	posAcc = 0.0;
	
    return true;
}

void  Encoder::updateHook()
{
	static unsigned firstRun = 1;

// 	TimeService::ticks tickStart = TimeService::Instance()->getTicks();

	// Read time stamp
	portTrigger.read( triggerTimeStamp );
	
	// Read new position and corresponding time-stamp
	posNew = readEncoder( encoderPort );
	timeStampNew = TimeService::Instance()->getTicks();

	if ( firstRun )
	{
		firstRun = 0;
		posOld = posNew;
	}
	
	// Read elapsed time since the last position reading
	elapsedTime = TimeService::Instance()->secondsSince( timeStampOld );
	
	// Knowing that posOld, posNew and posDelta are 32bit signed numbers
	// (int's) there are no sign nor overflow problems.
	int posDelta = posNew - posOld;
	
	// Convert encoder ticks to real angle in radians and bound it to -pi.. pi
	double posDeltaReal = (double)posDelta * 2.0 * PI / (GEAR_RATIO * PULSES_PER_REVOLUTION);
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
	encoderData[ 1 ] = -posAcc;
	encoderData[ 2 ] = sin( -posAcc );
	encoderData[ 3 ] = cos( -posAcc );
	encoderData[ 4 ] = -omegaNew;
	encoderData[ 5 ] = -omegaNew; 
	encoderData[ 6 ] = -omegaNew / (2.0 * PI) * 60.0;
	
	// Output data to the port
	portEncoderData.write( encoderData );
	
	// Prepare for the next sampling time.
	posOld = posNew;
	timeStampOld = timeStampNew;
	omegaOld = omegaNew;
	
	// Output execution time of the component
	portExecTime.write(
		TimeService::Instance()->secondsSince( triggerTimeStamp )
	);
}

void  Encoder::stopHook()
{}

void  Encoder::errorHook()
{}

void  Encoder::cleanupHook()
{}

ORO_CREATE_COMPONENT( Encoder )
