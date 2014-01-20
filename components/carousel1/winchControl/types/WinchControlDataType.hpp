#ifndef __WINCH_CONTROL_DATA_TYPE__
#define __WINCH_CONTROL_DATA_TYPE__

#include <stdint.h>

/// Custom data type for the encoder component
typedef struct
{
	/// Tether length [m]
	float length;
	/// Tether speed [m/s]
  	float speed;
  	/// Time-stamps: trigger [ticks] and elapsed [sec]
	double ts_trigger, ts_elapsed;
	/// A few debug things
	int32_t dbg_theta;
	int32_t dbg_omega;
  	int32_t dbg_current;
} WinchControlDataType;

#endif // __WINCH_CONTROL_DATA_TYPE__
