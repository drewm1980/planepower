#ifndef __CONTROLLERTEMPLATE__
#define __CONTROLLERTEMPLATE__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "ResampledMeasurements.h"
#include "SiemensDriveCommand.h"

#include "ControllerGains.h"

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
	RTT::InputPort< ControllerGains > portControllerGains;
	RTT::InputPort< referenceElevation > portReference;
	RTT::OutputPort< SiemensDriveCommand > portDriveCommand;
	RTT::OutputPort< ControllerGains > portGainsOut;

private:
	ResampledMeasurements resampledMeasurements;
	SiemensDriveCommand driveCommand;
	ControllerGains gains;
	referenceElevation reference;
	double error;
	double ierror;
	double derror;
	double last_error;
};

#endif
