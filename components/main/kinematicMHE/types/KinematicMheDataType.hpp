#ifndef __KINEMATIC_MHE_DATA_TYPE__
#define __KINEMATIC_MHE_DATA_TYPE__

typedef struct
{
	/// Number of data samples
	int num_imu_samples, num_enc_samples, num_cam_samples;
	/// Time-stamps: trigger [ticks], entry [ticks], elapsed[sec];
	double ts_trigger, ts_entry, ts_elapsed;
} KinematicMheDataType;

#endif // __KINEMATIC_MHE_DATA_TYPE__
