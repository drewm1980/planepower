#ifndef __POSEFROMMARKERS__
#define __POSEFROMMARKERS__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Port.hpp>

#include "LEDTracker/types/LEDTrackerDataType.hpp"
#include "types/PoseFromMarkersDataType.hpp"

#define NMARKERPOSITIONS 12
#define NPOSE 12

/// Component that calcuates aircraft pose based on marker positions
class PoseFromMarkers
	: public RTT::TaskContext
{
public:
	/// Ctor
	PoseFromMarkers(std::string name);
	/// Dtor
	virtual ~PoseFromMarkers()
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
	/// LED Tracker data
	RTT::InputPort< LEDTrackerDataType > portCamData;
	/// LED Tracker data holder
	LEDTrackerDataType camData;
	/// Pose data port
	RTT::OutputPort< PoseFromMarkersDataType > portData;
	/// Pose data holder
	PoseFromMarkersDataType data;

private:
	double* cInput[ 1 ];
	double* cOutput[ 3 ];

	double markers[ NMARKERPOSITIONS ];
	double pose[ NPOSE ];
	double cov[NPOSE * NPOSE];
	double invCov[NPOSE * NPOSE];
};

#endif // __POSEFROMMARKERS__
