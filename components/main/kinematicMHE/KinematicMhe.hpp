#ifndef __KINEMATIC_MHE__
#define __KINEMATIC_MHE__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Port.hpp>

typedef uint64_t TIME_TYPE;

// Here we import custom data type definitions
#include "mcuHandler/types/McuHandlerDataType.hpp"
#include "encoder/types/EncoderDataType.hpp"
#include "LEDTracker/types/LEDTrackerDataType.hpp"

#include "types/KinematicMheDataType.hpp"

/// KinematicMhe component class
class KinematicMhe
	: public RTT::TaskContext
{
public:
	/// Ctor
	KinematicMhe(std::string name);
	/// Dtor
	virtual ~KinematicMhe()
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
	/// Input trigger port
	RTT::InputPort< TIME_TYPE > portTrigger;
	/// MCU handler data inputs; buffered port
	RTT::InputPort< McuHandlerDataType > portMcuHandlerData;
	/// IMU data holder
	std::vector< McuHandlerDataType > imuData;
	/// Encoder data input
	RTT::InputPort< EncoderDataType > portEncoderData;
	/// Encoder data holder
	EncoderDataType encData;
	/// Camera data input
	RTT::InputPort< LEDTrackerDataType > portLEDTrackerData;
	/// Camera data holder
	LEDTrackerDataType camData;
	/// Output data for debug
	RTT::OutputPort< KinematicMheDataType > portDebugData;
	/// Debug data port holder
	KinematicMheDataType debugData;

private:

};

#endif // __KINEMATIC_MHE__
