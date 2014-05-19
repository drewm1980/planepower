#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>
#include "pins.h"
#include "SimpleGPIO.h"

// Try doing bitbanging without SimpleGPIO (in the tight loop)
// to save on device file opening overhead

#include "pins.h"
#include "encoder_calibration.h"

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
	int fd_CS0, fd_CS1, fd_MISO, fd_CLK;
	fd_CS0 = open(CS0_VALUE_FILE,O_RDWR);
	fd_CS1 = open(CS1_VALUE_FILE,O_RDWR);
	fd_MISO  = open(MISO_VALUE_FILE,O_RDWR);
	fd_CLK = open(CLK_VALUE_FILE,O_RDWR);
	if (fd_CS0 == -1){ printf("Couldn't open device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_CS1 == -1){ printf("Couldn't open device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_MISO  == -1){ printf("Couldn't open device file!!!\n"); exit(EXIT_FAILURE);}
	if (fd_CLK == -1){ printf("Couldn't open device file!!!\n"); exit(EXIT_FAILURE);}
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

int main()
{
	bitbang_init();
	while(1)
	{
#if 0
		err = gpio_set_value(CLK_PIN, HIGH);
		err = gpio_set_value(CLK_PIN, LOW);
#else
		write(fd_CLK,"1",2);
		write(fd_CLK,"0",2);
#endif
	}
}

