#include "voltageController.hpp"
#include <ocl/Component.hpp>
#include <unistd.h>
#include <signal.h>

ORO_CREATE_COMPONENT( OCL::VoltageController)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

// Maximum voltage
#define MAX_VOLTAGE 10.0
// Minimum time to reach the maximum voltage
#define MIN_TIME_TO_MAX_VOLTAGE 25

#define MAIN_CAROUSEL_MOTOR_CHANEL 0

VoltageController *theVoltageController; // There can be only one!!!!

void voltage_controller_signal_handler(int signum)
{
	cout << "Caught SIGSEGV!  Attempting carousel slowdown!" << endl;
	theVoltageController->stopHook(); // We want to try to fix shit before giving control back to anything else. 
	signal(SIGSEGV,SIG_DFL);
}

namespace OCL
{
     VoltageController::VoltageController(std::string name)
         : TaskContext(name)
     {
		addOperation("setVoltage",&VoltageController::setVoltage,this).doc("Set voltage in a safe way").arg("channel","The channel to write to (0 or 1)").arg("voltage","The voltage to put on [channel] (between -10 and 10)");
		theVoltageController = this;
		signal(SIGSEGV,voltage_controller_signal_handler);
    }

    VoltageController::~VoltageController()
    {
    }

    bool  VoltageController::configureHook()
    {
		assert(this->hasPeer("soemMaster"));
		assert(this->getPeer("soemMaster")->provides()->hasService("Slave_1001"));
		assert(this->getPeer("soemMaster")->provides()->getService("Slave_1001")->provides()->hasOperation("writeAnalog"));
		writeAnalog = getPeer("soemMaster")->provides()->getService("Slave_1001")->provides()->getOperation("writeAnalog");

		actual_voltage[0] = 0.0;
		actual_voltage[1] = 0.0;
		reference_voltage[0] = 0.0;
		reference_voltage[1] = 0.0;
        
        return true;
	}

	bool  VoltageController::startHook()
	{
		return true;
	}

    void  VoltageController::updateHook()
    {
		for(int channel = 0; channel < 2; channel++){
			if(channel==0)
			{
				// If main motor, slow stop.
				voltage_increment = MAX_VOLTAGE/MIN_TIME_TO_MAX_VOLTAGE*(this->getPeriod());
				if(abs(actual_voltage[channel])>5.1)
				{
					voltage_increment *= 4;
				}
			}

			if(abs(actual_voltage[channel]-reference_voltage[channel]) > voltage_increment )
			{
				if( actual_voltage[channel] < reference_voltage[channel] )
				{
					actual_voltage[channel] += voltage_increment;
				}
				if( actual_voltage[channel] > reference_voltage[channel] )
				{
					actual_voltage[channel] -= voltage_increment;
				}
			}
			else
			{
				actual_voltage[channel] = reference_voltage[channel];
			}

			writeAnalog(channel,actual_voltage[channel]);
		}
	}

    void  VoltageController::stopHook()
	{
		reference_voltage[MAIN_CAROUSEL_MOTOR_CHANEL] = 0.0;
		while(abs(actual_voltage[MAIN_CAROUSEL_MOTOR_CHANEL]-reference_voltage[MAIN_CAROUSEL_MOTOR_CHANEL]) > 1e-5)
		{
			updateHook();
			usleep(this->getPeriod()*1e6);
		}
    }

    void  VoltageController::cleanUpHook()
    {
    }

    bool VoltageController::setVoltage(int channel, double voltage){
	if( !(channel == 0 || channel == 1) ){
		cout << "The first argument, channel, should be 0 or 1. You specified channel " << channel << ", which does not exist"<< endl;
		return false;
	}
	if(voltage > 10 || channel < -10){
		cout << "The second argument, voltage, should be between -10 and 10" << endl;
		return false;
	}
	
	reference_voltage[channel] = voltage;
	return true;
    }


}//namespace

