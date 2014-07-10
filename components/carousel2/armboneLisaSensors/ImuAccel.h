#ifndef IMU_ACCEL_H
#define IMU_ACCEL_H

struct ImuAccel
{
	double ax_raw;
	double ay_raw;
	double az_raw;

	double ts_trigger;
	double ts_elapsed;
};


#endif
