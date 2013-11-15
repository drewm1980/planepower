#ifndef __PROPORTIONALCONTROLLER__
#define __PROPORTIONALCONTROLLER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

typedef uint64_t TIME_TYPE;

#include "encoder/types/EncoderDataType.hpp"
#include "lineAngleSensor/types/LineAngleSensorDataType.hpp"

class ProportionalController : public RTT::TaskContext
{
public:
	ProportionalController(std::string name);
	virtual ~ProportionalController(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	//
	// Input Ports
	//
	RTT::InputPort< EncoderDataType > portEncoderData;
	EncoderDataType encData;

	RTT::InputPort< LineAngleSensorDataType > portLASData;
	LineAngleSensorDataType lasData;
	
	//
	// Output Ports
	//
	RTT::OutputPort< std::vector< double > > portControls;
	std::vector< double > controls;
	RTT::OutputPort< TIME_TYPE > portTriggerOut;

	//
	// Properties
	//
	// Feedback gains.  Unitless, since angle -> angle
	double azimuthElevator;
	double elevationElevator;
	double azimuthAileron;
	double elevationAileron;

};

#endif
