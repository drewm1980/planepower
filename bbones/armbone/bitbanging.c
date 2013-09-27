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

	//Set GPIO Pins Direction
	err = gpio_set_dir(CS0_PIN, OUTPUT_PIN);
	if(err!=0) printf("Trouble Setting a pin direction!\n");
	err = gpio_set_dir(CS1_PIN, OUTPUT_PIN);
	if(err!=0) printf("Trouble Setting a pin direction!\n");
	err = gpio_set_dir(DO_MISO_PIN, INPUT_PIN);
	if(err!=0) printf("Trouble Setting a pin direction!\n");
	err = gpio_set_dir(CLK_PIN, OUTPUT_PIN);
	if(err!=0) printf("Trouble Setting a pin direction!\n");

	//Set the Values
	err = gpio_set_value(CS0_PIN, HIGH);
	if(err!=0) printf("Trouble Setting a pin value!\n");
	err = gpio_set_value(CS1_PIN, HIGH);
	if(err!=0) printf("Trouble Setting a pin value!\n");
	err = gpio_set_value(CLK_PIN, LOW);
	if(err!=0) printf("Trouble Setting a pin value!\n");
}

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
	if (status)
	{
		printf("Warning! Line angle sensor reports bad status; probably misalignmened?!!!\n");
	}
	else
	{
		*rsp_raw = rsp;
	}
	return status;
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

void print_uint8_as_binary(uint8_t raw)
{
	for(int i=sizeof(raw)*8-1; i>=0; i--)
	{
		printf("%i", raw>>i & (uint16_t)1);
	}
}

int main()
{
	printf("Intializing GPIO pins...\n");
	bitbang_init();

	uint16_t azimuth_raw, elevation_raw; 
	float azimuth_radians,elevation_radians;

	while(1)
	{
		// Set to zero to make failure more obvious
		azimuth_raw = 0;
		elevation_raw = 0;

		//printf("Reading azimuth...\n");
		bitbang_read(CS0_PIN,CLK_PIN,DO_MISO_PIN,AZIMUTH_STATUS_PIN,
				&azimuth_raw);
		//printf("Reading elevation...\n");
		bitbang_read(CS1_PIN,CLK_PIN,DO_MISO_PIN,ELEVATION_STATUS_PIN,
				&elevation_raw);

		encoders_to_angles(azimuth_raw, elevation_raw,
				&azimuth_radians, &elevation_radians);

#if 1
		// Print the values to sanity check them
		printf("AZIMUTH:");
		printf("   %f",azimuth_radians);
		print_uint8_as_binary(azimuth_raw);
		printf("  ***  ELEVATION:");
		printf("   %f",elevation_radians);
		print_uint8_as_binary(elevation_raw);
		printf("\n");
#endif

#if 0
		uint32_t rsp;
		rsp=0;
		rsp = rsp + azimuth_raw;
		rsp = rsp << 16;
		rsp = rsp + elevation_raw;
		for(int k=sizeof(rsp)*8-1; k>=0; k--)
		{
			printf("%i", rsp>>k & (uint32_t)1);
		} 	printf("\n");
#endif
		printf("\n");
		usleep(500);
	}
}
