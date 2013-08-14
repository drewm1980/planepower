#ifndef __CAMERATRIGGER__
#define __CAMERATRIGGER__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>

typedef uint64_t TIME_TYPE;

/// Camera trigger component class
class CameraTrigger
	: public RTT::TaskContext
{
public:
	/// Ctor
	CameraTrigger(std::string name);
	/// Dtor
	virtual ~CameraTrigger()
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
	/// SOEM EBOX mathod to set digital port state
	RTT::OperationCaller< bool(unsigned int, bool) > setBit;
	/// Input trigger port
	RTT::InputPort< TIME_TYPE > _Trigger;
	/// Some debugging stuff
	RTT::OutputPort< TIME_TYPE > _TriggerTriggeredTime, _TriggerResetTime; 

private:
	void pull_trigger_high();
	void pull_trigger_low();
};

#endif
