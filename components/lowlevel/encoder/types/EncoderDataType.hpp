#ifndef __ENCODERDATATYPE__
#define __ENCODERDATATYPE__

/// Custom data type for the encoder component
typedef struct
{
	/// Angle of the carousel [rad]
	double theta;
	/// Sine and cosine of the angle
	double sin_theta, cos_theta;
	/// Raw speed [rad/s]
	double omega;
	/// Filtered speed [rpm]
	double omega_filt_rpm;
	/// Time-stamps: trigger [ticks] and elapsed [sec]
	double ts_trigger, ts_elapsed;
} EncoderDataType;

#endif // __ENCODERDATATYPE__
