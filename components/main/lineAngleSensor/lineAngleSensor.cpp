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
	addEventPort("EboxOut", portEboxOut)
		.doc("Ebox port with measurements.");
	
	addPort("data", portData)
		.doc("Line angle sensor data");

	addProperty("angle1Gain", angle1Gain);
	addProperty("angle1Offset", angle1Offset);
	addProperty("angle2Gain", angle2Gain);
	addProperty("angle2Offset", angle2Offset);

	// Default values for gains and offsets
	angle1Gain = angle2Gain = 1.0;
	angle1Offset = angle2Offset = 0.0;
	
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

	data.angle1 = (eboxOut.analog[ 0 ] - angle1Offset) * angle1Gain;
	data.angle2 = (eboxOut.analog[ 1 ] - angle2Offset) * angle2Gain;

	data.ts_trigger = trigger;
	data.ts_elapsed = TimeService::Instance()->secondsSince( trigger );

	portData.write( data );
}

void LineAngleSensor::stopHook()
{}

void LineAngleSensor::errorHook()
{}

void LineAngleSensor::cleanupHook()
{}

ORO_CREATE_COMPONENT( LineAngleSensor )
