#include "stdint.h"

// API for reading from two aksim angle encoders to a beaglebone
// using a bitbanged spi implementation

// Does not depend on any networking or error logging stuff.

void bitbang_init(); // Needs to be called before  read_angle_sensors to initialize uc pins, open device files
// Returns already calibrated absolute angle values for the encoders
void read_angle_sensors(float* azimuth_radians, float* elevation_radians);
void bitbang_close();

// This is just for writing fun debugging code
void plot_two_angles(float az, float el);

