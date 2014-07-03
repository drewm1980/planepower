#ifndef IMU_GYRO_H
#define IMU_GYRO_H

struct ImuGyro
{
	int gp_raw;
	int gq_raw;
	int gr_raw;

	double ts_trigger;
	double ts_elapsed;
};


#endif
