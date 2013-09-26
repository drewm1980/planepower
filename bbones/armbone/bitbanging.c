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

#include "pins.h"
#include "encoder_calibration.h"

void bitbang_init(){
	gpio_export(CS0_PIN);
	gpio_export(CS1_PIN);
	gpio_export(DO_MISO_PIN);
	gpio_export(CLK_PIN);

	//Set GPIO Pins Direction
	gpio_set_dir(CS0_PIN, OUTPUT_PIN);
	gpio_set_dir(CS1_PIN, OUTPUT_PIN);
	gpio_set_dir(DO_MISO_PIN, INPUT_PIN);
	gpio_set_dir(CLK_PIN, OUTPUT_PIN);

	//Set the Values
	gpio_set_value(CS0_PIN, HIGH);
	gpio_set_value(CS1_PIN, HIGH);
	gpio_set_value(CLK_PIN, LOW);
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
	gpio_get_value(status_pin, &status);
	unsigned int bit = LOW;

	for (int c=0; c < 16 ; c++)
	{
		gpio_set_value(clk_pin, HIGH);

		for(int c0=0; c0<1000; c0++);
		gpio_get_value(miso_pin, &bit);

		gpio_set_value(clk_pin, LOW);
		for(int c1=0; c1<1000; c1++);
		rsp = rsp<<1;		
		rsp = rsp + bit;
	}
	usleep(10);
	gpio_set_value(cs_pin, HIGH);
	usleep(10);
	if (!status)
	{
		printf("Warning! Status pin low!!!\n");
	}
	else
	{
		*rsp_raw = rsp;
	}
	return status;
}

float raw_to_radians(uint16_t raw)
{
	const float rescale = 360.0 / ((float) (0x1<<16)));
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
	bitbang_init();

	uint16_t aximuth_raw, elevation_raw; 

	while(1)
	{
		// Set to zero to make failure more obvious
		aximuth_raw = 0;
		elevation_raw = 0;

		bitbang_read(CS0_PIN,CLK_PIN,DO_MISO_PIN,AZIMUTH_STATUS_PIN,
				&aximuth_raw);
		bitbang_read(CS1_PIN,CLK_PIN,DO_MISO_PIN,ELEVATION_STATUS_PIN,
				&elevation_raw);

		encoders_to_angles(azimuth_raw, elevation_raw,
				&azimuth_radians, &elevation_radians);

#if 1
		// Print the values to sanity check them
		printf("AZIMUTH:");
		printf("   %f",azimuth_radians);
		print_uint8_as_binary(aximuth_raw);
		printf("  ***  ELEVATION:");
		printf("   %f",angle_elevation);
		print_uint8_as_binary(elevation_raw);
		printf("\n");
#endif

#if 0
		uint32_t rsp;
		rsp=0;
		rsp = rsp + aximuth_raw;
		rsp = rsp << 16;
		rsp = rsp + elevation_raw;
		for(int k=sizeof(rsp)*8-1; k>=0; k--)
		{
			printf("%i", rsp>>k & (uint32_t)1);
		} 	printf("\n");
#endif

		int angle = 0;
		azimuth_radians = ((float)aximuth_raw)*(360/(pow(2,16)));
		angle_elevation = ((float)elevation_raw)*(360/(pow(2,16)));

		if(azimuth_radians>=0 && azimuth_radians<18)
			angle = 18;
		else if(azimuth_radians>=18 && azimuth_radians<36)	
			angle = 36;
		else if(azimuth_radians>=36 && azimuth_radians<54)	
			angle = 54;
		else if(azimuth_radians>=54 && azimuth_radians<72)	
			angle = 72;
		else if(azimuth_radians>=72 && azimuth_radians<90)	
			angle = 90;
		else if(azimuth_radians>=90 && azimuth_radians<108)	
			angle = 108;
		else if(azimuth_radians>=108 && azimuth_radians<126)	
			angle = 126;
		else if(azimuth_radians>=126 && azimuth_radians<144)	
			angle = 144;
		else if(azimuth_radians>=144 && azimuth_radians<162)	
			angle = 162;
		else if(azimuth_radians>=162 && azimuth_radians<180)	
			angle = 180;
		else if(azimuth_radians>=180 && azimuth_radians<198)	
			angle = 198;
		else if(azimuth_radians>=198 && azimuth_radians<216)	
			angle = 216;
		else if(azimuth_radians>=216 && azimuth_radians<234)	
			angle = 234;
		else if(azimuth_radians>=234 && azimuth_radians<252)	
			angle = 252;
		else if(azimuth_radians>=252 && azimuth_radians<270)	
			angle = 270;
		else if(azimuth_radians>=270 && azimuth_radians<288)	
			angle = 288;
		else if(azimuth_radians>=288 && azimuth_radians<306)	
			angle = 306;
		else if(azimuth_radians>=306 && azimuth_radians<324)	
			angle = 324;
		else if(azimuth_radians>=324 && azimuth_radians<342)	
			angle = 342;
		else if(azimuth_radians>=342 && azimuth_radians<360)	
			angle = 360;
		printf("  A:");	
		switch(angle)
		{
			case 18:
				printf("X-------------------");
				break;
			case 36:
				printf("-X------------------");
				break;
			case 54:
				printf("--X-----------------");
				break;
			case 72:
				printf("---X----------------");
				break;
			case 90:
				printf("----X---------------");
				break;
			case 108:
				printf("-----X--------------");
				break;
			case 126:
				printf("------X-------------");
				break;
			case 144:
				printf("-------X------------");
				break;
			case 162:
				printf("--------X-----------");
				break;
			case 180:
				printf("---------X----------");
				break;
			case 198:
				printf("----------X---------");
				break;
			case 216:
				printf("-----------X--------");
				break;
			case 234:
				printf("------------X-------");
				break;
			case 252:
				printf("-------------X------");
				break;
			case 270:
				printf("--------------X-----");
				break;
			case 288:
				printf("---------------X----");
				break;
			case 306:
				printf("----------------X---");
				break;
			case 324:
				printf("-----------------X--");
				break;
			case 342:
				printf("------------------X-");
				break;
			case 360:
				printf("-------------------X");
				break;

		}



		if(angle_elevation>=0 && angle_elevation<18)
			angle = 18;
		else if(angle_elevation>=18 && angle_elevation<36)	
			angle = 36;
		else if(angle_elevation>=36 && angle_elevation<54)	
			angle = 54;
		else if(angle_elevation>=54 && angle_elevation<72)	
			angle = 72;
		else if(angle_elevation>=72 && angle_elevation<90)	
			angle = 90;
		else if(angle_elevation>=90 && angle_elevation<108)	
			angle = 108;
		else if(angle_elevation>=108 && angle_elevation<126)	
			angle = 126;
		else if(angle_elevation>=126 && angle_elevation<144)	
			angle = 144;
		else if(angle_elevation>=144 && angle_elevation<162)	
			angle = 162;
		else if(angle_elevation>=162 && angle_elevation<180)	
			angle = 180;
		else if(angle_elevation>=180 && angle_elevation<198)	
			angle = 198;
		else if(angle_elevation>=198 && angle_elevation<216)	
			angle = 216;
		else if(angle_elevation>=216 && angle_elevation<234)	
			angle = 234;
		else if(angle_elevation>=234 && angle_elevation<252)	
			angle = 252;
		else if(angle_elevation>=252 && angle_elevation<270)	
			angle = 270;
		else if(angle_elevation>=270 && angle_elevation<288)	
			angle = 288;
		else if(angle_elevation>=288 && angle_elevation<306)	
			angle = 306;
		else if(angle_elevation>=306 && angle_elevation<324)	
			angle = 324;
		else if(angle_elevation>=324 && angle_elevation<342)	
			angle = 342;
		else if(angle_elevation>=342 && angle_elevation<360)	
			angle = 360;
		printf("  E:");
		switch(angle){
			case 18:
				printf("X-------------------");
				break;
			case 36:
				printf("-X------------------");
				break;
			case 54:
				printf("--X-----------------");
				break;
			case 72:
				printf("---X----------------");
				break;
			case 90:
				printf("----X---------------");
				break;
			case 108:
				printf("-----X--------------");
				break;
			case 126:
				printf("------X-------------");
				break;
			case 144:
				printf("-------X------------");
				break;
			case 162:
				printf("--------X-----------");
				break;
			case 180:
				printf("---------X----------");
				break;
			case 198:
				printf("----------X---------");
				break;
			case 216:
				printf("-----------X--------");
				break;
			case 234:
				printf("------------X-------");
				break;
			case 252:
				printf("-------------X------");
				break;
			case 270:
				printf("--------------X-----");
				break;
			case 288:
				printf("---------------X----");
				break;
			case 306:
				printf("----------------X---");
				break;
			case 324:
				printf("-----------------X--");
				break;
			case 342:
				printf("------------------X-");
				break;
			case 360:
				printf("-------------------X");
				break;

		}
		printf("\n");
		usleep(500);
	}
}
