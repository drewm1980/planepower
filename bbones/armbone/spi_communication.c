#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <unistd.h>


#include "SimpleGPIO.h"
#include "spi_communication.h"

#include "bitbang_spi.h"

const unsigned int CS0 = 113;
const unsigned int CS1 = 114;
const unsigned int D0  = 111;
const unsigned int CLK = 110;
const unsigned int status0 = 115;
const unsigned int status1 = 116;

SPI_errCode spi_open()
{
	//Access GPIO Pins
	gpio_export(CS0);
	gpio_export(CS1);
	gpio_export(D0);
	gpio_export(CLK);

	//Set GPIO Pins Direction
	gpio_set_dir(CS0, OUTPUT_PIN);
	gpio_set_dir(CS1, OUTPUT_PIN);
	gpio_set_dir(D0, INPUT_PIN);
	gpio_set_dir(CLK, OUTPUT_PIN);

	//Set the Values
	gpio_set_value(CS0, HIGH);
	gpio_set_value(CS1, HIGH);
	gpio_set_value(CLK, LOW);	

	return SPI_ERR_NONE;
}

SPI_errCode spi_read(uint8_t data_sensors[])
{
	uint32_t rsp;
	uint16_t rsp_azimuth, rsp_elevation; 	
	rsp_azimuth = 0;
	rsp_elevation = 0;
	rsp = 0;

	unsigned int azimuth_bit = LOW;
	unsigned int elevation_bit = LOW;

	//***************************BitBanging Azimuth***************************//

	gpio_set_value(CS0, LOW);	
	usleep(10);

	// int c = 0;
	for (int c = 0; c < 16 ; c++){
		//CLK GOES HIGH
		gpio_set_value(CLK, HIGH);
		for(int c0=0;c0<1000;c0++); // Busy sleep

		gpio_get_value(D0, &azimuth_bit);	//GET 1 BIT

		//CLK GOES LOW 
		gpio_set_value(CLK, LOW);
		for(int c1=0;c1<1000;c1++); // Busy sleep

		//PUSH 1 BIT INTO AN ARRAY
		rsp_azimuth = rsp_azimuth<<1;		
		rsp_azimuth = rsp_azimuth + azimuth_bit;

	}

	rsp_azimuth = rsp_azimuth - (0x21c2);		//DEFINING THE ZERO FOR AZIMUTH     


	usleep(10);
	gpio_set_value(CS0, HIGH);


	usleep(10);					//DELAY BETWEEN READING OF AZIMUTH AND ELEVATION


	//***************************BitBanging Elevation***************************//

	gpio_set_value(CS1, LOW);	
	usleep(10);

	// int a = 0;	
	for (int a = 0; a < 16 ; a++){
		//CLK GOES HIGH		
		gpio_set_value(CLK, HIGH);
		// int a0=0;
		for(int a0 = 0;a0<1000;a0++);

		gpio_get_value(D0, &elevation_bit);	//GET 1 BIT

		//CLK GOES LOW 
		gpio_set_value(CLK, LOW);	
		// int a1=0;
		for(int a1 = 0;a1<1000;a1++);

		//PUSH 1 BIT INTO AN ARRAY
		rsp_elevation = rsp_elevation<<1;		
		rsp_elevation = rsp_elevation + elevation_bit;			
	}

	rsp_elevation = rsp_elevation - (0xb9a1) + rsp_azimuth;	//DEFINING THE ZERO FOR ELEVATION AND MAKE IT NEUTRAL TO AZIMUTH MOVEMENT

	usleep(10);
	gpio_set_value(CS1, HIGH);


	/*// Print the values to sanity check them
	  int i;
	  printf("AZIMUTH:");
	  for(i=sizeof(rsp_azimuth)*8-1; i>=0; i--)
	  {
	  printf("%i", rsp_azimuth>>i & (uint16_t)1);
	  }
	  float angle_azimuth = ((float)rsp_azimuth)*(360/(pow(2,16)));
	  printf("   %f",angle_azimuth);

	  int j;
	  printf("  ***  ELEVATION:");
	  for(j=sizeof(rsp_elevation)*8-1; j>=0; j--)
	  {
	  printf("%i", rsp_elevation>>j & (uint16_t)1);
	  } 
	  float angle_elevation = ((float)rsp_elevation)*(360/(pow(2,16)));
	  printf("   %f",angle_elevation);
	  printf("\n");*/


	//PUT AZIMUTH AND ELEVATION DATA IN ONE ARRAY******(AZIMUTH.ELEVATION)
	//printf("AZ: %i\n", rsp_azimuth);
	//printf("EL: %i\n", rsp_elevation);
	rsp = rsp + rsp_azimuth;
	rsp = rsp << 16;
	rsp = rsp + rsp_elevation;

	/*data_sensors[0] = rsp;
	  data_sensors[1] = rsp>>8;
	  data_sensors[2] = rsp>>16;
	  data_sensors[3] = rsp>>24;*/

	memcpy(data_sensors, &rsp , sizeof(uint32_t));


	/*int k;
	  for(k=sizeof(rsp)*8-1; k>=0; k--)
	  {
	  printf("%i", rsp>>k & (uint32_t)1);
	  } 	printf("\n");*/


	return SPI_ERR_NONE;
}

SPI_errCode spi_close()
{

	gpio_unexport(CS0);
	gpio_unexport(CS1);
	gpio_unexport(D0);
	gpio_unexport(CLK);

	return SPI_ERR_NONE;
}

void SPI_err_handler(SPI_errCode err,void (*write_error_ptr)(char *,char *,int))
{
	//write error to local log
	switch( err ) {
		case SPI_ERR_NONE:
                        // Write something in err_pointer because tup complains
                        write_error_ptr("","", -1);
			break;
                        /*
	        case  SPI_ERR_UNDEFINED:
		        write_error_ptr(SOURCEFILE,"undefined spi error",err);
			break;
                case  SPI_ERR_OPEN_DEV:
	                write_error_ptr(SOURCEFILE,"failed to open spi port",err);
			break;
                        */
		default: break;
	}
}



