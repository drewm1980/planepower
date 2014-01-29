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
  	float dbg_theta; // [?]
  	float dbg_omega; // [rpm?]
  	float dbg_current; // [mA?]
} WinchControlDataType;

#endif // __WINCH_CONTROL_DATA_TYPE__
