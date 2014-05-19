#include "stdint.h"

// API for reading from two aksim angle encoders to a beaglebone
// using a bitbanged spi implementation

// Does not depend on any networking or error logging stuff.

void bitbang_init(); // Needs to be called before  read_angle_sensors to initialize uc pins
// Returns already calibrated absolute angle values for the encoders
void read_angle_sensors(float* azimuth_radians, float* elevation_radians);
void bitbang_close();
void plot_two_angles(float az, float el);

/*----------  Below here are utility functions ------------ */

int bitbang_read(unsigned int cs_pin,
		unsigned int status_pin,
		uint16_t *rsp_raw);

float raw_to_radians(uint16_t raw);
void encoders_to_angles(uint16_t azimuth_raw, uint16_t elevation_raw,
			float *azimuth_radians, float *elevation_radians);

void print_uint16_as_binary(uint16_t raw);
