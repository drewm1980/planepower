#ifndef IMU_MAG_H
#define IMU_MAG_H

struct ImuMag
{
	double mx;
	double my;
	double mz;
	double angle;

	double ts_trigger;
	double ts_elapsed;
};


#endif
