#ifndef __CONTROLINJECTOR__
#define __CONTROLINJECTOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

// We need the constants that convert the control surface angles from unitless to radians
#include <mcuHandler.hpp>

#include <stdint.h>

class ControlInjector : public RTT::TaskContext
{
public:
	ControlInjector(std::string name);
	virtual ~ControlInjector(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void applyControls();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

	/// Trigger the component to write the current control value
	RTT::InputPort< TIME_TYPE > portTrigger;
	/// Connect this to the mcuHandler
	RTT::OutputPort< ControlSurfacesValues > portControls;

	void setControlsUnitless(float right_aileron, float left_aileron, float elevator);
	
	// Holder for the control action to be send
	ControlSurfacesValues controls; // In radians!!!
};

#endif
