#ifndef __MASTERTIMER__
#define __MASTERTIMER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

/// Time stamp type
typedef uint64_t TIME_TYPE;
/// Number of clocks other than master
#define CLOCK_COUNT 3

/// Master timer class
class MasterTimer : public RTT::TaskContext
{
public:
	/// Ctor
	MasterTimer(std::string name);
	/// Dtor
	virtual ~MasterTimer()
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
	/// Union that holds all sampling times, inetrfaced via properties
	union
	{
		double target_hz[ CLOCK_COUNT ];
		struct
		{
			double _imu_target_hz;
			double _camera_target_hz;
			double _controls_playback_target_hz;
		};
	};

	/// Clock dividers
	int dividers[ CLOCK_COUNT ];
	/// IMU trigger timestamp
	RTT::OutputPort<TIME_TYPE> _imuClock;
	/// Camera trigger timestamp
	RTT::OutputPort<TIME_TYPE> _cameraClock;
	/// Controls trigger timestamp
	RTT::OutputPort<TIME_TYPE> _controlsPlaybackClock;
	/// Master clock timestamp
	RTT::OutputPort<TIME_TYPE> _masterClock;
	/// Array of port pointers
	RTT::OutputPort<TIME_TYPE> *portPointers[ CLOCK_COUNT ];
	/// IMU/Camera clock's ratio
	RTT::OutputPort<int>	_imuCameraRatio;

private:
	uint64_t base_clock_index;
	TIME_TYPE myticks;
};

#endif // __MASTERTIMER__
