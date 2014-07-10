#ifndef IMU_GYRO_H
#define IMU_GYRO_H

struct ImuGyro
{
	double gp_raw;
	double gq_raw;
	double gr_raw;

	double ts_trigger;
	double ts_elapsed;
};


#endif
