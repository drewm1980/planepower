#ifndef __LINE_ANGLE_SENSOR__
#define __LINE_ANGLE_SENSOR__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Port.hpp>

#include <soem_ebox/soem_ebox/EBOXOut.h>
#include "types/LineAngleSensorDataType.hpp"

/// The line angle sensor component
class LineAngleSensor
	: public RTT::TaskContext
{
public:
	/// Ctor
	LineAngleSensor(std::string name);
	/// Dtor
	virtual ~LineAngleSensor()
	{}
	
	/// Configuration hook.
	virtual bool configureHook( );
	/// Start hook.
	virtual bool startHook( );
	/// Update hook.
	virtual void updateHook( );
	/// Stop hook.
	virtual void stopHook( );
	/// Cleanup hook.
	virtual void cleanupHook( );
	/// Error hook.
	virtual void errorHook( );

protected:
	/// EBOX output port
	RTT::InputPort< soem_ebox::EBOXOut > portEboxOut;
	/// EBOX output port data holder
	soem_ebox::EBOXOut eboxOut;
	/// Sensor output data port
	RTT::OutputPort< LineAngleSensorDataType > portData;
	/// Sensor data holder
	LineAngleSensorDataType data;
		
	/// Horizontal angle offset
	double angleHorOffset;
	/// Horizontal angle gain
	double angleHorGain;
	/// Vertical angle offset
	double angleVerOffset;
	/// Vartical angle gain
	double angleVerGain;
};

#endif /// __LINE_ANGLE_SENSOR__
