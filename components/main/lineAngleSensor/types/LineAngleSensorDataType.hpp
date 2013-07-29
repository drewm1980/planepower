#ifndef __LINEANGLESENSORDATATYPE__
#define __LINEANGLESENSORDATATYPE__

/// Custom data type for the encoder component
typedef struct
{
	/// Measured angles [rad]
	float angle_hor, angle_ver;
	/// Time-stamps: trigger [ticks] and elapsed [sec]
	double ts_trigger, ts_elapsed;
} LineAngleSensorDataType;

#endif // __LINEANGLESENSORDATATYPE__
