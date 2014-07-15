#ifndef __RAMPGENERATOR__
#define __RAMPGENERATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include <SiemensDriveState.h>
#include <SiemensDriveCommand.h>

class RampGenerator : public RTT::TaskContext
{
public:
	RampGenerator(std::string name);
	virtual ~RampGenerator(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();
protected:
	RTT::InputPort< SiemensDriveState > portDriveState;
//	RTT::InputPort< LineAngles > portLineAngles;
	RTT::OutputPort< SiemensDriveCommand > portDriveCommand;
	RTT::OutputPort< std::string> portInfo;
private:
	SiemensDriveState driveState;
	SiemensDriveCommand driveCommand;
//	LineAngles lineAngles;
//	ResampledMeasurements resampledMeasurements;
	double acceleration;
	double targetSpeed;
	
	double dt;
	double softlimit;
	double currentSetpoint;
	double currentSpeed;
	double nextSetpoint;
	double threshold;
	double stepheigth;
	int state;
	int retrys;
	std::string info;
};

#endif
