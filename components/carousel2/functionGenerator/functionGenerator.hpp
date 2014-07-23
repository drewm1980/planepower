#ifndef __CONTROLLERTEMPLATE__
#define __CONTROLLERTEMPLATE__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>
#include "Reference.h"
#include "SiemensDriveCommand.h"

typedef uint64_t TIME_TYPE;

class FunctionGenerator : public RTT::TaskContext
{
public:
	FunctionGenerator(std::string name);
	virtual ~FunctionGenerator(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	RTT::OutputPort< SiemensDriveCommand > portDriveCommand;
	RTT::OutputPort< Reference > portReference;
	RTT::InputPort< double > portTriggerIn;

private:
	SiemensDriveCommand driveCommand;
	Reference reference;
	int type; // 0 -> Sin  1 -> Square
	int whichDrive; // 0 -> winch 1 -> carousel
	double amplitude; // Rad/s (inherrited from SiemensDriveCommand)
	double offset;  // Rad/s   (inherrited from SiemensDriveCommand)
	double phase; // Radians
	double frequency; // Hz
	TIME_TYPE startTime; // nsec.  Time is relative to this.
};

#endif
