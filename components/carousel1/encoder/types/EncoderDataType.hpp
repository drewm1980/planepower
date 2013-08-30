#ifndef __ENCODERDATATYPE__
#define __ENCODERDATATYPE__

#include <stdint.h>

/// Custom data type for the encoder component
typedef struct
{
	/// Angle of the carousel [rad]
	float theta;
	/// Sine and cosine of the angle
	float sin_theta, cos_theta;
	/// Raw speed [rad/s]
	float omega;
	/// Filtered speed [rpm]
	float omega_filt_rpm;
	/// Time-stamps: trigger [ticks] and elapsed [sec]
	double ts_trigger, ts_elapsed;
	/// A few debug things
	float dbg_angle;
	int32_t dbg_raw_angle;
	float dbg_speed_voltage;
} EncoderDataType;

#endif // __ENCODERDATATYPE__
