#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bitbang_spi.h"

int main()
{
	printf("Intializing GPIO pins...\n");
	bitbang_init();

	float azimuth_radians,elevation_radians;

	while(1)
	{
		read_angle_sensors(&azimuth_radians, &elevation_radians);
#if 0
		// Print the values in radians
		printf("AZIMUTH: %f ELEVATION: %f\n",azimuth_radians, elevation_radians);
#endif
#if 1
		plot_two_angles(azimuth_radians, elevation_radians);
		usleep(300000);
#endif

		printf("\n");
		usleep(500);
	}
	bitbang_close();
}
