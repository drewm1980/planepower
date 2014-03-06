#include "InputSignalGenerator.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <cmath>

using namespace std;
using namespace RTT;
using namespace RTT::os;

InputSignalGenerator::InputSignalGenerator(string name)
	: TaskContext(name, PreOperational)
{
	//
	// Add ports
	//
	addPort("data", portData)
		.doc("Input data for the airplane...");

	data.reset();
	portData.setDataSample( data );
	portData.write( data );

	addProperty("fsine", fsine)
		.doc("Frequency of the sine wave");
	fsine = 0.0;

	addProperty("amplitude", amplitude)
		.doc("Amplitude of the sine wave");
	amplitude = 0.2;
}

bool InputSignalGenerator::configureHook()
{
	Logger::In in( getName() );

	Ts = getPeriod();
	if (abs( Ts ) < 1e-10)
	{
		log( Error ) << "This component must be periodic" << endlog();
		return false;
	}

	if (fsine < 0.0)
	{
		log( Error ) << "Nope, frequency has to be a positive number" << endlog();
		return false;
	}
	
	return true;
}

bool InputSignalGenerator::startHook()
{
	angle = 0.0;

	return true;
}

void InputSignalGenerator::updateHook()
{
	angle += 2 * M_PI * Ts * fsine;

	if (angle > M_PI)
		angle -= 2.0 * M_PI;
	else if (angle < -M_PI)
		angle += 2.0 * M_PI;

	double sin_angle = sin( angle );

	if (abs(fsine) > 1e-4)
	{
		data.ua1 = amplitude * sin_angle;
		data.ua2 = amplitude * sin_angle;
		data.ue = 0.0;
	}
	else
		data.reset();

	portData.write( data );
}

void InputSignalGenerator::stopHook()
{
	data.reset();
	portData.write( data );
}

void InputSignalGenerator::errorHook()
{}

void InputSignalGenerator::cleanupHook()
{}

ORO_CREATE_COMPONENT( InputSignalGenerator )
