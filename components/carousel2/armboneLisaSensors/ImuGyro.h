#ifndef IMU_GYRO_H
#define IMU_GYRO_H

struct ImuGyro
{
	double gp;
	double gq;
	double gr;

	double ts_trigger;
	double ts_elapsed;
};


#endif
