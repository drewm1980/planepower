#include "voltageController.hpp"

#include <unistd.h>
#include <signal.h>

using namespace std;
using namespace RTT;
using namespace soem_ebox;

/// Maximum voltage
#define MAX_VOLTAGE 10.0
/// Minimum time to reach the maximum voltage
#define MIN_TIME_TO_MAX_VOLTAGE 25
/// SOEM chanel to which the motor controller is connected
#define MAIN_CAROUSEL_MOTOR_CHANEL 0
/// This guy enables increment to be a property, use with EXTREME care!!!
#define DEBUG 0

/// There can be only one!!!! - Nice try to make a singleton
VoltageController *theVoltageController;

void voltage_controller_signal_handler(int signum)
{
	cout << "Caught SIGSEGV!  Attempting carousel slowdown!" << endl;
	// We want to try to fix shit before giving control back to anything else. 
	theVoltageController->stopHook();
	signal(SIGSEGV, SIG_DFL);
}

VoltageController::VoltageController(std::string name)
	: TaskContext(name, PreOperational)
{
	//
	// Add ports
	//
	addPort("eboxAnalog", portEboxAnalog)
		.doc("Port to be connected with EBOX analog port");

	//
	// Configure ports
	//
	portEboxAnalog.setDataSample( eboxAnalog );
	portEboxAnalog.write( eboxAnalog  );

	//
	// Add operations
	//
	addOperation("setVoltage", &VoltageController::setVoltage, this, ClientThread)
		.doc("Set voltage in a safe way")
		.arg("channel", "The channel to write to (0 or 1)")
		.arg("voltage", "The voltage to put on [channel] (between -10 and 10)");

	//
	// Add properties
	//
#if DEBUG == 666
	addProperty("increment", voltage_increment)
		.doc("Only, and only for debugging purposes");
#endif // DEBUG == 666
	
	//
	// Configure fail-safe procedures
	//
	theVoltageController = this;
	signal(SIGSEGV, voltage_controller_signal_handler);
}

bool  VoltageController::configureHook()
{
	actual_voltage[ 0 ]  = actual_voltage[ 1 ] = 0.0;
	reference_voltage[ 0 ] = reference_voltage[ 1 ] = 0.0;
    
	return true;
}

bool VoltageController::startHook()
{
	if (getPeriod() < 1e-5)
	{
		log( Error ) << "The component must be periodic!" << endlog(); 
		return false;
	}

	voltage_increment = MAX_VOLTAGE / MIN_TIME_TO_MAX_VOLTAGE * (this->getPeriod());

	return true;
}

void VoltageController::updateHook()
{
	double inc = voltage_increment;

	for(int channel = 0; channel < 2; channel++)
	{
		if (channel == 0 && abs( actual_voltage[ channel ] ) > 5.1)
		{
			// If main motor, slow stop.
			inc *= 4;
		}

		if(abs(actual_voltage[ channel ] - reference_voltage[ channel ]) > inc)
		{
			if( actual_voltage[channel] < reference_voltage[channel] )
			{
				actual_voltage[channel] += inc;
			}
			else if( actual_voltage[channel] > reference_voltage[channel] )
			{
				actual_voltage[channel] -= inc;
			}
		}
		else
		{
			actual_voltage[channel] = reference_voltage[channel];
		}
		
		eboxAnalog.analog[ channel ] = actual_voltage[ channel ];
	}

	// Send data to the port
	portEboxAnalog.write( eboxAnalog );	
}

void VoltageController::stopHook()
{
	reference_voltage[ MAIN_CAROUSEL_MOTOR_CHANEL ] = 0.0;
	while(abs(actual_voltage[ MAIN_CAROUSEL_MOTOR_CHANEL ]
			  - reference_voltage[ MAIN_CAROUSEL_MOTOR_CHANEL ]) > 1e-5)
	{
		updateHook();
		usleep(this->getPeriod() * 1e6);
	}
}

void VoltageController::cleanupHook()
{}

void VoltageController::errorHook()
{}

bool VoltageController::setVoltage(int channel, double voltage)
{
	if (!(channel == 0 || channel == 1) )
		return false;

	if(voltage > 10.0 || channel < -10.0)
		return false;
	
	reference_voltage[ channel ] = voltage;
	
	return true;
}

ORO_CREATE_COMPONENT( VoltageController )
