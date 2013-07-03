#ifndef __LEDTRACKERDATATYPE__
#define __LEDTRACKERDATATYPE__

/// Custom data type for the LED tracker
typedef struct
{
	/// Pixel positions of the markers
	double positions[ 12 ];
	/// Weights
	double weights[ 12 ];
	/// Time-stamps: trigger [ticks], frame arrival [ticks], elapsed[sec]
	double ts_trigger, ts_frame, ts_elapsed;
} LEDTrackerDataType;

#endif // __LEDTRACKERDATATYPE__
