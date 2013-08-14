#ifndef __POSEFROMMARKERSDATATYPE__
#define __POSEFROMMARKERSDATATYPE__

#include <vector>

/// Custom data type for the LED tracker
typedef struct
{
	/// Pose in w.r.t. the anchorpoint frame: x, y, z, e11, ..., e33
	std::vector< double > pose;

	/// Time-stamps: trigger [ticks], ts_entry [ticks], elapsed [sec]
	double ts_trigger, ts_entry, ts_elapsed;
} PoseFromMarkersDataType;

#endif // __POSEFROMMARKERSDATATYPE__
