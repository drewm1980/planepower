#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "SimpleGPIO.h"
#include <unistd.h>
#include <string.h>

#include "pins.h"
#define AZIMUTH_DEVICE_FILE "/dev/spidev1.0"
#define ELEVATION_DEVICE_FILE "/dev/spidev1.1"
#define USE_IOCTL_API 1

#define RESPONSE_TYPE uint16_t // 64 causes crash here.

int main()
{
	// Change status gpio pins to inputs
	// ... should be done in device tree overlay...
	gpio_export(AZIMUTH_STATUS_PIN);
	gpio_export(ELEVATION_STATUS_PIN);
	gpio_set_dir(AZIMUTH_STATUS_PIN, INPUT_PIN);
	gpio_set_dir(ELEVATION_STATUS_PIN, INPUT_PIN);

	// The raw values from the sensors.
	RESPONSE_TYPE rsp_azimuth;

	while(1)
	{
		// -----------------  READ AZIMUTH -----------------
		int fd_azimuth = open(AZIMUTH_DEVICE_FILE, O_RDWR);
		if (fd_azimuth < 0) {
			perror("Could not open azimuth device file!");
			abort();
		}

		rsp_azimuth = -1;

		unsigned int azimuth_status;
		gpio_get_value(AZIMUTH_STATUS_PIN, &azimuth_status);
		if (1){
		//if (azimuth_status == HIGH){
#if USE_IOCTL_API		
			// Configure the SPI port
			struct spi_ioc_transfer tr_azimuth[1];
			memset(&tr_azimuth[0], 0, sizeof(tr_azimuth));
			tr_azimuth[0].rx_buf = &rsp_azimuth;  // Throws warning
			tr_azimuth[0].tx_buf = 0;  // NULL pointer is OK
			// Below for bits per word "correct"
			//tr_azimuth[1].len = 0; // crash! Inappropriate ioctl for device
			tr_azimuth[0].len = 2; // No time varying data gets through
			//tr_azimuth[0].len = 2; // No time varying data gets through
			//tr_azimuth[0].len = 3; // No time varying data gets through
			//tr_azimuth[0].len = 4; // 2 bit shift per read
			//tr_azimuth[0].len = 5; // 1 bit shift per read
			//tr_azimuth[0].len = 6; // 1 bit shift per read
			//tr_azimuth[0].len = 7; // 1 bit shift per read
			//tr_azimuth[0].len = 8; // 1 bit shift per read
			//tr_azimuth[0].len = 9; // varying 0-1 bit shift per read
			//tr_azimuth[0].len = 10; // varying 0-1 bit shift per read
			//tr_azimuth[0].len = 11; // varying 0-1 bit shift per read
			//tr_azimuth[0].len = 12; // 2 bit shift per read
			//tr_azimuth[0].len = 13; // crash! bad file descriptor
			//tr_azimuth[0].len = 14; // crash! bad file descriptor
			//tr_azimuth[1].len = 15; // crash! bad file descriptor
			//tr_azimuth[0].len = 16; // crash! bad file descriptor
			//tr_azimuth[0].len = 8*sizeof(RESPONSE_TYPE); // in bits, causes crash!
			//tr_azimuth[0].len = sizeof(RESPONSE_TYPE); // in bytes
			tr_azimuth[0].delay_usecs =     100;
			tr_azimuth[0].speed_hz =        100000;
			//tr_azimuth[0].bits_per_word = sizeof(RESPONSE_TYPE)*8;
			// Below for len "correct"
			tr_azimuth[0].bits_per_word =8;
			//tr_azimuth[0].bits_per_word = 1; // crash!
			tr_azimuth[0].cs_change = 1;
			int ret_azimuth = ioctl(fd_azimuth, SPI_IOC_MESSAGE(1), &tr_azimuth);
			if (ret_azimuth < 0) perror("ioctl");
#else
			read(fd_azimuth, &rsp_azimuth, sizeof(RESPONSE_TYPE));
#endif
			
			// Print the values to sanity check them
			printf("0x%.8X ", rsp_azimuth);
			int i;
			for(i=sizeof(rsp_azimuth)*8-1; i>=0; i--)
			{
				printf("%i", rsp_azimuth>>i & (RESPONSE_TYPE)1);
			} printf("\n");

		}
		else{ printf("Warning, Low status pin!!!\n"); }
		int ret = close(fd_azimuth);
		if(ret<0) { perror("Could not close the device file!"); abort();}
		
		//uint16_t azimuth_swapped = __builtin_bswap16(rsp_azimuth[0]);
		//printf("rsp_azimuth = %x\n", rsp_azimuth[0]);
		//double azimuth = (double) azimuth_swapped * 360.0 / (1<<16); // degrees
		//double elevation = (double) (*rsp_elevation) *360.0 / (1<<16); // degrees
		//printf("azimuth = %f \n", azimuth_swapped);
		//printf("elevation = %f, azimuth = %f \n", elevation, azimuth);
		//usleep(500000);

	};

	gpio_unexport(AZIMUTH_STATUS_PIN);
	gpio_unexport(ELEVATION_STATUS_PIN);
}



