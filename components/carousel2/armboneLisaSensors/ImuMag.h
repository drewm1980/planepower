#ifndef IMU_MAG_H
#define IMU_MAG_H

struct ImuMag
{
	double mx_raw;
	double my_raw;
	double mz_raw;

	double ts_trigger;
	double ts_elapsed;
};


#endif
