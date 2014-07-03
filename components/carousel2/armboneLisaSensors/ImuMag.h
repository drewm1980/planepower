#ifndef IMU_MAG_H
#define IMU_MAG_H

struct ImuMag
{
	int mx_raw;
	int my_raw;
	int mz_raw;

	double ts_trigger;
	double ts_elapsed;
};


#endif
