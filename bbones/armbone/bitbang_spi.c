#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "SimpleGPIO.h"
#include "assert.h"

#include "pins.h"
#include "encoder_calibration.h"

void bitbang_init()
{
	int err;
	err = gpio_export(CS0_PIN);
	if(err!=0) printf("Trouble Exporting a pin!\n");
	err = gpio_export(CS1_PIN);
	if(err!=0) printf("Trouble Exporting a pin!\n");
	err = gpio_export(DO_MISO_PIN);
	if(err!=0) printf("Trouble Exporting a pin!\n");
	err = gpio_export(CLK_PIN);
	if(err!=0) printf("Trouble Exporting a pin!\n");
	err = gpio_export(AZIMUTH_STATUS_PIN);
	if(err!=0) printf("Trouble Exporting a pin!\n");
	err = gpio_export(ELEVATION_STATUS_PIN);
	if(err!=0) printf("Trouble Exporting a pin!\n");

	//Set GPIO Pins Direction
	err = gpio_set_dir(CS0_PIN, OUTPUT_PIN);
	if(err!=0) printf("Trouble Setting a pin direction!\n");
	err = gpio_set_dir(CS1_PIN, OUTPUT_PIN);
	if(err!=0) printf("Trouble Setting a pin direction!\n");
	err = gpio_set_dir(DO_MISO_PIN, INPUT_PIN);
	if(err!=0) printf("Trouble Setting a pin direction!\n");
	err = gpio_set_dir(CLK_PIN, OUTPUT_PIN);
	if(err!=0) printf("Trouble Setting a pin direction!\n");
	err = gpio_set_dir(AZIMUTH_STATUS_PIN, INPUT_PIN);
	if(err!=0) printf("Trouble Setting a pin direction!\n");
	err = gpio_set_dir(ELEVATION_STATUS_PIN, INPUT_PIN);
	if(err!=0) printf("Trouble Setting a pin direction!\n");

	//Set the Values
	err = gpio_set_value(CS0_PIN, HIGH);
	if(err!=0) printf("Trouble Setting a pin value!\n");
	err = gpio_set_value(CS1_PIN, HIGH);
	if(err!=0) printf("Trouble Setting a pin value!\n");
	err = gpio_set_value(CLK_PIN, LOW);
	if(err!=0) printf("Trouble Setting a pin value!\n");
}

// Read the values of an SPI attached encoder using bitbanging of GPIO pins.
// Inputs are the numbers of pins... not sure where they're documented...
// Output is the raw encoder value.
// Return value:  Zero if no error, 1 if encoder has bad status during read
int bitbang_read(unsigned int cs_pin,
		unsigned int clk_pin,
		unsigned int miso_pin,
		unsigned int status_pin,
		uint16_t *rsp_raw)
{
	gpio_set_value(cs_pin, LOW);
	uint16_t rsp = 0;

	usleep(10);	
	int status = 0;
	int err;
	err = gpio_get_value(status_pin, &status);
	if(err!=0) printf("Error reading status pin!\n");
	unsigned int bit = LOW;

	for (int c=0; c < 16 ; c++)
	{
		err = gpio_set_value(clk_pin, HIGH);
		if(err!=0) printf("Error setting clock pin value!\n");

		for(int c0=0; c0<1000; c0++);
		err = gpio_get_value(miso_pin, &bit);
		if(err!=0) printf("Error getting miso pin value!\n");

		err = gpio_set_value(clk_pin, LOW);
		assert(err==0);
		for(int c1=0; c1<1000; c1++);
		rsp = rsp<<1;		
		rsp = rsp + bit;
	}
	usleep(10);
	err = gpio_set_value(cs_pin, HIGH);
	assert(err==0);
	usleep(10);
	if (status==0)
	{
		printf("Warning! Line angle sensor reports bad status; probably misalignmened?!!!\n");
	}
	else
	{
		*rsp_raw = rsp;
	}
	return !status;
}

float raw_to_radians(uint16_t raw)
{
	const unsigned int counts = 0x1 << 16;
	const float rescale = 2.0*3.1415 / counts;
	return ((float)raw) * rescale;
}

void encoders_to_angles(uint16_t azimuth_raw, uint16_t elevation_raw,
			float *azimuth_radians, float *elevation_radians)
{
	// Correct for the offsets and add the angles
	azimuth_raw = azimuth_raw - AZIMUTH_ZERO_POINT;
	elevation_raw = (elevation_raw - ELEVATION_ZERO_POINT) + azimuth_raw;

	// Convert to angles in radians
	*azimuth_radians = raw_to_radians(azimuth_raw);
	*elevation_radians = raw_to_radians(elevation_raw);
}

void print_uint16_as_binary(uint16_t raw)
{
	for(int i=sizeof(raw)*8-1; i>=0; i--)
	{
		printf("%i", raw>>i & (uint16_t)1);
	}
}

void read_angle_sensors(float* azimuth_radians, float* elevation_radians)
{
	uint16_t azimuth_raw=0.0, elevation_raw=0.0; 

	//printf("Reading azimuth...\n");
	bitbang_read(CS0_PIN,CLK_PIN,DO_MISO_PIN,AZIMUTH_STATUS_PIN,
			&azimuth_raw);
	//printf("Reading elevation...\n");
	bitbang_read(CS1_PIN,CLK_PIN,DO_MISO_PIN,ELEVATION_STATUS_PIN,
			&elevation_raw);

	encoders_to_angles(azimuth_raw, elevation_raw,
			&azimuth_radians, &elevation_radians);

#if 0
	// Print the raw values in binary
	printf("AZ: ");
	print_uint16_as_binary(azimuth_raw);
	printf(" EL: ");
	print_uint16_as_binary(elevation_raw);
	printf("\n");
#endif

}

#define PLOT_H 32
#define PLOT_W (PLOT_H*2)
unsigned char framebuffer[PLOT_H][PLOT_W+1];
void plot_two_angles(float az, float el)
{
	memset(framebuffer, '_', sizeof(framebuffer));
	for(int i=0; i<PLOT_H; i++) framebuffer[i][PLOT_W] = '\n';
	framebuffer[PLOT_H][PLOT_W] = 0; // Null terminator
	int az_index = az/(2*3.1415)*PLOT_W + PLOT_W/2;
	az_index %= PLOT_W;
	int el_index = el/(2*3.1415)*PLOT_H + PLOT_H/2;
	el_index %= PLOT_H;
	framebuffer[el_index][az_index] = '+';
	puts(framebuffer[0]);
	putchar('\n');
	putchar('\n');
}

