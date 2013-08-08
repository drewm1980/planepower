#ifndef __LISA_BBONE_CLIENT_DATA_TYPE__
#define __LISA_BBONE_CLIENT_DATA_TYPE__

/// Custom data type for the MCU handler
typedef struct
{
	/// Gyro readings in [rad/s]
	float gyro_x, gyro_y, gyro_z;
	/// Acceleration readings in [m/s^2]
	float accl_x, accl_y, accl_z;
	/// Time-stamps: trigger [ticks] and elapsed time [s]
	double ts_trigger, ts_elapsed;
} LisaBboneClientDataType;

#endif // __LISA_BBONE_CLIENT_DATA_TYPE__
