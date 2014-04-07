#ifndef __INPUT_SIGNAL_GENERATOR__
#define __INPUT_SIGNAL_GENERATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>

#include "mcuHandler/types/McuHandlerDataType.hpp"

/// Stupid component to generate some inputs for the ailerons
class InputSignalGenerator
	: public RTT::TaskContext
{
public:
	/// Ctor
	InputSignalGenerator(std::string name);
	/// Dtor
	virtual ~InputSignalGenerator()
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
	/// Output port containing voltages
	RTT::OutputPort< ControlSurfacesValues > portData;
	/// Output data holder
    ControlSurfacesValues data;

	/// Frequency of the output sine wave
	double fsine;
	/// Amplitude of the output sine wave
	double amplitude;

	/// A method to reset control surfaces to zero
	void reset();

private:
	double Ts;
	double angle;

	double aileron;
	double elevator;
};

#endif // __INPUT_SIGNAL_GENERATOR__
