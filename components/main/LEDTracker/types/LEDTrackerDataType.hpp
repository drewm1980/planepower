#ifndef __LEDTRACKERDATATYPE__
#define __LEDTRACKERDATATYPE__

#include <vector>

/// Custom data type for the LED tracker
typedef struct
{
	/// Pixel positions of the markers
	std::vector< double > positions;
	/// Weights
	std::vector< double > weights;
	/// Pose
	std::vector< double > pose;

	/// Time-stamps: trigger [ticks], frame arrival [ticks], elapsed[sec]
	double ts_trigger, ts_frame, ts_elapsed;
} LEDTrackerDataType;

#endif // __LEDTRACKERDATATYPE__
