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

#define AZIMUTH_STATUS_PIN 115
#define ELEVATION_STATUS_PIN 116
#define AZIMUTH_DEVICE_FILE "/dev/spidev1.0"
#define ELEVATION_DEVICE_FILE "/dev/spidev1.1"

int main(int argc, const char *argv[])
{
	// Change status gpio pins to inputs
	gpio_export(AZIMUTH_STATUS_PIN);
	gpio_export(ELEVATION_STATUS_PIN);
	gpio_set_dir(AZIMUTH_STATUS_PIN, INPUT_PIN);
	gpio_set_dir(ELEVATION_STATUS_PIN, INPUT_PIN);

	// The raw values from the sensors.
	unsigned long foo;

	while(1)
	{
		// -----------------  READ AZIMUTH -----------------
		int fd_azimuth = open(AZIMUTH_DEVICE_FILE, O_RDWR);
		if (fd_azimuth < 0) {
			perror("Could not open azimuth device file!");
			abort();
		}

		foo = 0;

		unsigned int azimuth_status;
		gpio_get_value(AZIMUTH_STATUS_PIN, &azimuth_status);
		if (azimuth_status == HIGH){
			
			// Configure the SPI port
			struct spi_ioc_transfer tr_azimuth;
			tr_azimuth.rx_buf = &foo; 
			tr_azimuth.tx_buf = 0; 
			tr_azimuth.len = 1;
			tr_azimuth.delay_usecs =       100;
			tr_azimuth.speed_hz =        100000;
			tr_azimuth.bits_per_word = 16;
			tr_azimuth.cs_change = 1;

			// Performs the actual read!
			int ret_azimuth = ioctl(fd_azimuth, SPI_IOC_MESSAGE(1), &tr_azimuth);
			if (ret_azimuth < 0) perror("ioctl");
			
			// Print the values to sanity check them
			int i;
			for(i=15; i>=0; i--)
			{
				printf("%i", foo>>i & (unsigned long int)1);
			} printf("\n");
		}
		else{ printf("Warning, Low status pin!!!\n"); }
		//read(fd_azimuth, rsp_azimuth, 2);
		close(fd_azimuth);
		
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

