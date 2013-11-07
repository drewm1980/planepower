#ifndef __VOLTAGECONTROLLER__
#define __VOLTAGECONTROLLER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <soem_ebox/soem_ebox/EBOXAnalog.h>

class VoltageController : public RTT::TaskContext
{
public:
	VoltageController(std::string name);
	~VoltageController()
	{}

	virtual bool configureHook( );
	virtual bool startHook( );
	virtual void updateHook( );
	virtual void stopHook( );
	virtual void cleanupHook( );
	virtual void errorHook( );

protected:

	RTT::OutputPort< soem_ebox::EBOXAnalog > portEboxAnalog;
	soem_ebox::EBOXAnalog eboxAnalog;
  	bool setVoltage(int channel, double voltage);

private:
	double voltage_increment;
	double reference_voltage[ 2 ];
	double actual_voltage[ 2 ];
};

#endif // __VOLTAGECONTROLLER__
