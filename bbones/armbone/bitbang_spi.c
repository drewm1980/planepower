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
#include <assert.h>

#include "SimpleGPIO.h"
#include "pins.h"
#include "encoder_calibration.h"
#include "bitbang_spi.h"

int fd_CS0, fd_CS1, fd_MISO, fd_CLK;

void bitbang_init()
{
	int err;
	err = gpio_export(CS0_PIN);
	if(err!=0) printf("Trouble Exporting a pin!\n");
	err = gpio_export(CS1_PIN);
	if(err!=0) printf("Trouble Exporting a pin!\n");
	err = gpio_export(MISO_PIN);
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
	err = gpio_set_dir(MISO_PIN, INPUT_PIN);
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

	// Open all of our device files ONCE
	fd_CS0 = open(CS0_VALUE_FILE,O_RDWR);
	fd_CS1 = open(CS1_VALUE_FILE,O_RDWR);
	fd_MISO  = open(MISO_VALUE_FILE,O_RDWR);
	fd_CLK = open(CLK_VALUE_FILE,O_RDWR);
	printf(CS0_VALUE_FILE "\n");
	if (fd_CS0 == -1){ printf("Couldn't open CS0 device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_CS1 == -1){ printf("Couldn't open CS1 device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_MISO  == -1){ printf("Couldn't open MISO device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_CLK == -1){ printf("Couldn't open CLK device file!!!\n"); exit(EXIT_FAILURE);}
}    
void bitbang_close()
{
	int err;
	err = gpio_unexport(CS0_PIN);
	if(err!=0) {printf("Trouble Unexporting a pin!\n"); exit(EXIT_FAILURE);};
	err = gpio_unexport(CS1_PIN);
	if(err!=0) {printf("Trouble Unexporting a pin!\n"); exit(EXIT_FAILURE);};
	err = gpio_unexport(MISO_PIN);
	if(err!=0) {printf("Trouble Unexporting a pin!\n"); exit(EXIT_FAILURE);};
	err = gpio_unexport(CLK_PIN);
	if(err!=0) {printf("Trouble Unexporting a pin!\n"); exit(EXIT_FAILURE);};
	err = gpio_unexport(AZIMUTH_STATUS_PIN);
	if(err!=0) {printf("Trouble Unexporting a pin!\n"); exit(EXIT_FAILURE);};
	err = gpio_unexport(ELEVATION_STATUS_PIN);
	if(err!=0) {printf("Trouble Unexporting a pin!\n"); exit(EXIT_FAILURE);};

	err = close(fd_CS0); 
	if (err==-1) {printf("Unable to close a device file!\n"); exit(EXIT_FAILURE);};
	err = close(fd_CS1); 
	if (err==-1) {printf("Unable to close a device file!\n"); exit(EXIT_FAILURE);};
	err = close(fd_MISO); 
	if (err==-1) {printf("Unable to close a device file!\n"); exit(EXIT_FAILURE);};
	err = close(fd_CLK); 
	if (err==-1) {printf("Unable to close a device file!\n"); exit(EXIT_FAILURE);};
}


// Read the values of an SPI attached encoder using bitbanging of GPIO pins.
// Inputs are the numbers of pins... not sure where they're documented...
// Output is the raw encoder value.
// Return value:  Zero if no error, 1 if encoder has bad status during read
int bitbang_read(unsigned int cs_pin,
		unsigned int status_pin,
		uint16_t *rsp_raw)
{
	gpio_set_value(cs_pin, LOW);
	uint16_t rsp = 0;

	usleep(10);	
	unsigned int status = 0;
	int err;
	err = gpio_get_value(status_pin, &status);
	if(err!=0) printf("Error reading status pin!\n");
	unsigned int bit = LOW;

	for (int c=0; c < 16 ; c++)
	{

#if 1
		// This implementation is re-opening the device files every clock edge:
		err = gpio_set_value(CLK_PIN, HIGH);
		if(err!=0) printf("Error setting clock pin value!\n");

		for(int c0=0; c0<1000; c0++);
		err = gpio_get_value(MISO_PIN, &bit);
		if(err!=0) printf("Error getting miso pin value!\n");

		err = gpio_set_value(CLK_PIN, LOW);
		assert(err==0);
		for(int c1=0; c1<1000; c1++);
#else
		// This implementation re-uses the already opened device files.
		writr(fd_CLK,"1",2);

		for(int c0=0; c0<1000; c0++);

		char value;
		read(fd_MISO,&value,1);
		bit = value == '1';

		write(fd_CLK,"0",2);

		for(int c1=0; c1<1000; c1++);
#endif

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
	bitbang_read(CS0_PIN,AZIMUTH_STATUS_PIN,
			&azimuth_raw);
	//printf("Reading elevation...\n");
	bitbang_read(CS1_PIN,ELEVATION_STATUS_PIN,
			&elevation_raw);

#if 0
	// Print the raw values in binary
	printf("AZ: ");
	print_uint16_as_binary(azimuth_raw);
	printf(" EL: ");
	print_uint16_as_binary(elevation_raw);
	printf("\n");
#endif

	encoders_to_angles(azimuth_raw, elevation_raw,
			azimuth_radians, elevation_radians);

}

#define PLOT_H 32
#define PLOT_W (PLOT_H*2)
char framebuffer[PLOT_H][PLOT_W+1];
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

