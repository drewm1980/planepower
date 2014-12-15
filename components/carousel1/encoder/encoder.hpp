#ifndef __ENCODER__
#define __ENCODER__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>
typedef uint64_t TIME_TYPE;

// Include SOEM EBOX encoder headers
#include <soem_ebox/soem_ebox/EBOXOut.h>
#include <soem_ebox/soem_ebox/EBOXAnalog.h>

// Include custom "output" data type header
#include "types/EncoderDataType.hpp"

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
	/// EBOX encoder port.
	RTT::InputPort< soem_ebox::EBOXOut > portEboxOut;
	/// EBOX encoder data.
	soem_ebox::EBOXOut eboxOut;
	
	/// EBOX analog port
	RTT::InputPort< soem_ebox::EBOXAnalog > portEboxAnalog;
	/// EBOX analog port data
	soem_ebox::EBOXAnalog eboxAnalog;
	
	/// Output port for encoder data.
	RTT::OutputPort< EncoderDataType > portEncoderData;
	/// Vector that holds measurements.
	EncoderDataType encoderData; 
		
	/// Encoder port, SOEM related.
	unsigned encoderPort;
	/// Sampling time of the component
	double Ts;

private:
	TIME_TYPE timeStampOld, timeStampNew, tickStart;
	double elapsedTime;
	int32_t posOld, posNew, posDelta;
	double posDeltaReal, posAcc, posRaw;
	double omegaNew, omegaFiltNew, omegaFiltOld, omegaPosAcc;
	unsigned omegaCnt, omegaPeriod;
	bool analogConn;
};

#endif // __ENCODER__
