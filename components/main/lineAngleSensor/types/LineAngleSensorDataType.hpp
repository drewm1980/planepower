#ifndef __LINEANGLESENSORDATATYPE__
#define __LINEANGLESENSORDATATYPE__

/// Custom data type for the encoder component
typedef struct
{
	/// Measured angles [rad]
	double angle1, angle2;
	/// Time-stamps: trigger [ticks] and elapsed [sec]
	double ts_trigger, ts_elapsed;
} LineAngleSensorDataType;

#endif // __LINEANGLESENSORDATATYPE__
