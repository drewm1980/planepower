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

int fd_CS0, fd_CS1, fd_MISO, fd_CLK, fd_AZ_STATUS, fd_EL_STATUS;

static float raw_to_radians(uint16_t raw);
static void encoders_to_angles(uint16_t azimuth_raw, uint16_t elevation_raw,
			float *azimuth_radians, float *elevation_radians);

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
	fd_AZ_STATUS = open(AZ_STATUS_VALUE_FILE,O_RDONLY);
	fd_EL_STATUS = open(EL_STATUS_VALUE_FILE,O_RDONLY);

	printf(CS0_VALUE_FILE "\n");
	if (fd_CS0 == -1){ printf("Couldn't open CS0 device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_CS1 == -1){ printf("Couldn't open CS1 device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_MISO  == -1){ printf("Couldn't open MISO device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_CLK == -1){ printf("Couldn't open CLK device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_AZ_STATUS == -1){ printf("Couldn't open AZIMUTH_STATUS device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_EL_STATUS == -1){ printf("Couldn't open ELEVATION_STATUS device file!!!\n"); exit(EXIT_FAILURE);}
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
	err = close(fd_AZ_STATUS); 
	if (err==-1) {printf("Unable to close a device file!\n"); exit(EXIT_FAILURE);};
	err = close(fd_EL_STATUS); 
	if (err==-1) {printf("Unable to close a device file!\n"); exit(EXIT_FAILURE);};
}


// Read the values of an SPI attached encoder using bitbanging of GPIO pins.
// Inputs are the numbers of pins... not sure where they're documented...
// Output is the raw encoder value.
// Return value:  Zero if no error, 1 if encoder has bad status during read
/*
*/
static inline int bitbang_read(unsigned int cs_pin,
		uint16_t *rsp_raw)
{
	_Bool cs0 = cs_pin == CS0_PIN;
	// Depending on the CSx_PIN write to file that it belongs
	if (cs0) {
		write(fd_CS0, "0", 2);	
	} else {
		write(fd_CS1, "0", 2);	
	}

	uint16_t rsp = 0;

	unsigned int status = 0;
	// int err;

	//err = gpio_get_value(status_pin, &status);
	char status_char;
	// If the Function is called with the CS0_PIN(Read Azimuth) 
	// we have to read the AZIMUTH_STATUS_PIN
	if (cs0) {
		pread(fd_AZ_STATUS, &status_char, 1, 0);
	} else {
		pread(fd_EL_STATUS, &status_char, 1, 0);
	}
	// Check the actual status
	if (status_char == '1') status = 1;

	// if(err!=0) printf("Error reading status pin!\n");
	unsigned int bit = LOW;

	for (int c=0; c < 16 ; c++)
	{
#if 0
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
		write(fd_CLK,"1",2);
		char value;
		pread(fd_MISO,&value,1,0);  // pread to reset the file pointer to the beginning before reading
		bit = value == '1';
		write(fd_CLK,"0",2);
#endif
		rsp = rsp<<1;		
		rsp = rsp + bit;
	}
	if (cs0) {
		write(fd_CS0, "1", 2);
	} else {
		write(fd_CS1, "1", 2);
	}
	// err = gpio_set_value(cs_pin, HIGH);
	// assert(err==0);
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

static inline float raw_to_radians(uint16_t raw)
{
	const unsigned int counts = 0x1 << 16;
	const float rescale = 2.0*3.1415 / counts;
	return ((float)raw) * rescale;
}

static inline void encoders_to_angles(uint16_t azimuth_raw, uint16_t elevation_raw,
			float *azimuth_radians, float *elevation_radians)
{
	// Correct for the offsets and add the angles
	azimuth_raw = azimuth_raw - AZIMUTH_ZERO_POINT;
	elevation_raw = (elevation_raw - ELEVATION_ZERO_POINT) + azimuth_raw;

	// Convert to angles in radians
	*azimuth_radians = raw_to_radians(azimuth_raw);
	*elevation_radians = raw_to_radians(elevation_raw);
}

void read_angle_sensors(float* azimuth_radians, float* elevation_radians)
{
	uint16_t azimuth_raw=0.0, elevation_raw=0.0; 

	//printf("Reading azimuth...\n");
	bitbang_read(CS0_PIN, &azimuth_raw);
	//printf("Reading elevation...\n");
	bitbang_read(CS1_PIN, &elevation_raw);

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

