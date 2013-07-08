#ifndef __MCHUHANDLERDATATYPE__
#define __MCHUHANDLERDATATYPE__

/// Custom data type for the MCU handler
typedef struct
{
	/// Gyro readings in [rad/s]
	double gyro_x, gyro_y, gyro_z;
	/// Acceleration readings in [m/s^2]
	double accl_x, accl_y, accl_z;
	/// Time-stamps: trigger [ticks] and elapsed time [s]
	double ts_trigger, ts_elapsed;
} McuHandlerDataType;

#endif // __MCHUHANDLERDATATYPE__
