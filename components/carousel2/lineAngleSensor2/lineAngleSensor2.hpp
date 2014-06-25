#ifndef __LINEANGLESENSOR2__
#define __LINEANGLESENSOR2__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "LineAngles.h"

#include "udp_communication.h"
#include "data_decoding.h"

#define MAX_INPUT_STREAM_SIZE 255
#define UDP_SOCKET_TIMEOUT 1000000000


class LineAngleSensor2 : public RTT::TaskContext
{
public:
	LineAngleSensor2(std::string name);
	virtual ~LineAngleSensor2(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	RTT::OutputPort< LineAngles > portData;

private:
	ArmboneReveiver receiver;
	LineAngles lineAngles;
	bool keepRunning;
};

#endif
