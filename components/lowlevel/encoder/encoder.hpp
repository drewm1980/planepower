#ifndef __ENCODER__
#define __ENCODER__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Port.hpp>
#include <rtt/Operation.hpp>
#include <rtt/OperationCaller.hpp>

#include <stdint.h>
typedef uint64_t TIME_TYPE;

/// Encoder class
class Encoder : public RTT::TaskContext
{
public:
	/// Ctor
	Encoder(std::string name);
	/// Dtor
	virtual ~Encoder()
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
	/// Input trigger port.
	RTT::InputPort< TIME_TYPE > portTrigger;
	/// Trigger time stamp
	TIME_TYPE triggerTimeStamp;
	/// Output port for encoder data:
	/// [timestamp, delta, sin_delta, cos_delta, omega, omega_filtered, omega_rpm].
	RTT::OutputPort< std::vector< double > > portEncoderData;
	/// Vector that holds measurements.
	std::vector< double > encoderData;
	/// Port which holds the execution time.
	RTT::OutputPort< double > portExecTime;
	
	/// Operation caller for SOEM "readEncoder" function.
	RTT::OperationCaller<int(unsigned int) > readEncoder;
	
	/// Encoder port, SOEM related.
	unsigned encoderPort;

private:
	TIME_TYPE timeStampOld, timeStampNew;
	double elapsedTime;
	int32_t posOld, posNew, posDelta;
	double posDeltaReal, posAcc;
	double omegaOld, omegaNew;
};

#endif // __ENCODER__
