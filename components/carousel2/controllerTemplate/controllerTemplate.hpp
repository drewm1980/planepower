#ifndef __CONTROLLERTEMPLATE__
#define __CONTROLLERTEMPLATE__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "ResampledMeasurements.h"
#include "SiemensDriveCommand.h"
#include "Reference.h"
#include "PIDControllerGains.h"

class ControllerTemplate : public RTT::TaskContext
{
public:
	ControllerTemplate(std::string name);
	virtual ~ControllerTemplate(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	RTT::InputPort< ResampledMeasurements > portResampledMeasurements;
	RTT::InputPort< PIDControllerGains > portPIDControllerGains;
	RTT::InputPort< Reference > portReference;
	RTT::OutputPort< SiemensDriveCommand > portDriveCommand;
	RTT::OutputPort< PIDControllerGains > portGainsOut;

private:
	ResampledMeasurements resampledMeasurements;
	SiemensDriveCommand driveCommand;
	PIDControllerGains gains;
	Reference reference;
	double error;
	double ierror;
	double derror;
	double last_error;
	double el_ref;

	RTT::OperationCaller< double(double) > lookup_steady_state_speed;
	RTT::OperationCaller< double(double) > lookup_steady_state_elevation;

};

#endif
