#ifndef IMU_ACCEL_H
#define IMU_ACCEL_H

struct ImuAccel
{
	double ax;
	double ay;
	double az;

	double ts_trigger;
	double ts_elapsed;
};


#endif
