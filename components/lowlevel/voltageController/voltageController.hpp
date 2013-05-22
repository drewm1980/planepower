#ifndef __VOLTAGECONTROLLER__
#define __VOLTAGECONTROLLER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

// Include the EBOX encoder class
#include <soem_ebox/soem_ebox/EBOXAnalog.h>

/// VoltageController class
class VoltageController : public RTT::TaskContext
{
public:
	/// Ctor
	VoltageController(std::string name);
    /// Dtor
	~VoltageController()
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

	/// EBOX analog port
	RTT::OutputPort< soem_ebox::EBOXAnalog  > portEboxAnalog;
	/// EBOX analog port data
	soem_ebox::EBOXAnalog eboxAnalog;
	/// "Public" method for setting the voltage
  	bool setVoltage(int channel, double voltage);

private:
	double					voltage_increment;
	double					reference_voltage[ 2 ];
	double					actual_voltage[ 2 ];        
};

#endif // __VOLTAGECONTROLLER__
