#ifndef __ICS_DATA_TYPE__
#define __ICS_DATA_TYPE__

/// Custom data type for the LED tracker
typedef struct
{
	/// Time-stamps: trigger [ticks], elapsed[sec]
	double ts_trigger, ts_elapsed;
} IndoorsCarouselSimulatorDataType;

#endif // __ICS_DATA_TYPE__
