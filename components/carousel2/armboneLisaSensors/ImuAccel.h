#ifndef IMU_ACCEL_H
#define IMU_ACCEL_H

struct ImuAccel
{
	int ax_raw;
	int ay_raw;
	int az_raw;

	double ts_trigger;
	double ts_elapsed;
};


#endif
