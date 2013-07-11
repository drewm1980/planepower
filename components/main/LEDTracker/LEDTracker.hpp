#ifndef __LEDTRACKER__
#define __LEDTRACKER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>
#include <rtt/Attribute.hpp>

#include "CameraArray.hpp"
#include "BlobExtractor.hpp"

#include "types/LEDTrackerDataType.hpp"

typedef uint64_t TIME_TYPE;

/// This class captures images from a single firewire camera,
/// and finds the location of three LED markers in the image.
/// The emphasis is on speed, rather than robustness.
class LEDTracker
	: public RTT::TaskContext
{
public:
	/// Ctor
	LEDTracker(std::string name);
	/// Dtor
	virtual ~LEDTracker();
	
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
	/// Input trigger signal.
	RTT::InputPort< TIME_TYPE > _triggerTimeStampIn;
	/// LED tracker output data.
	RTT::OutputPort< LEDTrackerDataType > portData;
	/// Data holder
	LEDTrackerDataType data;
	
	/// Use external triggering or not.
	bool _useExternalTrigger;
	/// Frame width and heigth.
	int frame_w, frame_h;
	/// Standard deviation of marker positions
	double sigma_marker;

private:
	CameraArray *cameraArray;
	BlobExtractor *blobExtractors[ CAMERA_COUNT ];

	// These are staging areas
	std::vector< double > markerPositions;
};

#endif // __LEDTRACKER__
