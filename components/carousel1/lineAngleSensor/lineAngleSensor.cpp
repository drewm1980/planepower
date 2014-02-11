#include "lineAngleSensor.hpp"

#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <stdint.h>

typedef uint64_t TIME_TYPE;

using namespace std;
using namespace RTT;
using namespace RTT::os;
using namespace soem_ebox;

LineAngleSensor::LineAngleSensor( std::string name )
	: TaskContext(name, PreOperational)
{
	addEventPort("eboxOut", portEboxOut)
		.doc("Ebox port with measurements.");
	
	addPort("data", portData)
		.doc("Line angle sensor data. See types/LineAngleSensorDataType.hpp for meaning");

	addPort("voltageHorizontal",portVoltageHorizontal)
		.doc("Horizontal line angle sensor voltage, for calibration");
	addPort("voltageVertical",portVoltageVertical)
		.doc("Vertical line angle sensor voltage, for calibration");

	addProperty("angleHorGain", angleHorGain);
	addProperty("angleHorOffset", angleHorOffset);
	addProperty("angleVerGain", angleVerGain);
	addProperty("angleVerOffset", angleVerOffset);

	// Default values for gains and offsets
	angleVerOffset = -0.7; // Volts
	angleHorOffset = .69; // Volts
	angleVerGain = -0.029; // Radians per volt
	angleHorGain = -0.0428; // Radians per volt
	
	data.ts_trigger = TimeService::Instance()->getTicks();
	portData.setDataSample( data );
	portData.write( data );
}

bool LineAngleSensor::configureHook()
{
	Logger::In in( getName() );

	if (portEboxOut.connected() == false)
	{
		log( Error ) << "Measurements port is not connected" << endlog();
		return false;
	}

	return true;
}

bool LineAngleSensor::startHook()
{
	return true;
}

void LineAngleSensor::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	portEboxOut.read( eboxOut );

	data.angle_hor = (float)((eboxOut.analog[ 0 ] - angleHorOffset) * angleHorGain);
	data.angle_ver = (float)((eboxOut.analog[ 1 ] - angleVerOffset) * angleVerGain);

	data.ts_trigger = trigger;
	data.ts_elapsed = TimeService::Instance()->secondsSince( trigger );

	portData.write( data );

	portVoltageHorizontal.write(eboxOut.analog[0]);
	portVoltageVertical.write(eboxOut.analog[1]);
}

void LineAngleSensor::stopHook()
{}

void LineAngleSensor::errorHook()
{}

void LineAngleSensor::cleanupHook()
{}

//ORO_CREATE_COMPONENT( LineAngleSensor )
ORO_LIST_COMPONENT_TYPE( LineAngleSensor )
