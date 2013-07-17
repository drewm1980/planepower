#ifndef __INPUT_SIGNAL_GENERATOR__
#define __INPUT_SIGNAL_GENERATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>

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
	RTT::OutputPort< std::vector< double > > portData;
	/// Output data holder
	std::vector< double > data;

	/// Frequency of the output sine wave
	double fsine;

private:
	double Ts;
	double angle;
};

#endif // __INPUT_SIGNAL_GENERATOR__
