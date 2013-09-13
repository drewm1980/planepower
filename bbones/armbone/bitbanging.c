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


unsigned int CS0 = 113;
unsigned int CS1 = 114;
unsigned int D0  = 111;
unsigned int CLK = 110;


int main(){
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

	uint16_t rsp_azimuth, rsp_elevation; 
	uint32_t rsp;

while(1){
	rsp_azimuth = 0;
	rsp_elevation = 0;
	rsp = 0;

	//BitBanging_Azimuth
	gpio_set_value(CS0, LOW);
	
	usleep(10);	
	int c = 0;
	unsigned int azimuth_bit = LOW;



	for (c; c < 16 ; c++){
		gpio_set_value(CLK, HIGH);

		int c0=0;
		for(c0;c0<1000;c0++);

		gpio_get_value(D0, &azimuth_bit);
 
		gpio_set_value(CLK, LOW);
	
		int c1=0;
		for(c1;c1<1000;c1++);
		

		rsp_azimuth = rsp_azimuth<<1;		
		rsp_azimuth = rsp_azimuth + azimuth_bit;

	}
	
	rsp_azimuth = rsp_azimuth - (0x21c2);

	
        usleep(10);
	gpio_set_value(CS0, HIGH);


	usleep(10);


	//BitBanging_Elevation

	gpio_set_value(CS1, LOW);
	
	usleep(10);
	int a = 0;
	unsigned int elevation_bit = LOW;	
	


	for (a; a < 16 ; a++){
		gpio_set_value(CLK, HIGH);

		int a0=0;
		for(a0;a0<1000;a0++);

		gpio_get_value(D0, &elevation_bit);
 
		gpio_set_value(CLK, LOW);
	
		int a1=0;
		for(a1;a1<1000;a1++);
		

		rsp_elevation = rsp_elevation<<1;		
		rsp_elevation = rsp_elevation + elevation_bit;			
	}

	rsp_elevation = rsp_elevation - (0xb9a1) + rsp_azimuth;



        usleep(10);
	gpio_set_value(CS1, HIGH);


	// Print the values to sanity check them
	/*int i;
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


	/*rsp = rsp + rsp_azimuth;
	rsp = rsp << 16;
	rsp = rsp + rsp_elevation;
	int k;
	for(k=sizeof(rsp)*8-1; k>=0; k--)
		{
		printf("%i", rsp>>k & (uint32_t)1);
		} 	printf("\n");*/
	
	int angle = 0;
	float angle_azimuth = ((float)rsp_azimuth)*(360/(pow(2,16)));
	float angle_elevation = ((float)rsp_elevation)*(360/(pow(2,16)));

	
	if(angle_azimuth>=0 && angle_azimuth<18)
		angle = 18;
	else if(angle_azimuth>=18 && angle_azimuth<36)	
		angle = 36;
	else if(angle_azimuth>=36 && angle_azimuth<54)	
		angle = 54;
	else if(angle_azimuth>=54 && angle_azimuth<72)	
		angle = 72;
	else if(angle_azimuth>=72 && angle_azimuth<90)	
		angle = 90;
	else if(angle_azimuth>=90 && angle_azimuth<108)	
		angle = 108;
	else if(angle_azimuth>=108 && angle_azimuth<126)	
		angle = 126;
	else if(angle_azimuth>=126 && angle_azimuth<144)	
		angle = 144;
	else if(angle_azimuth>=144 && angle_azimuth<162)	
		angle = 162;
	else if(angle_azimuth>=162 && angle_azimuth<180)	
		angle = 180;
	else if(angle_azimuth>=180 && angle_azimuth<198)	
		angle = 198;
	else if(angle_azimuth>=198 && angle_azimuth<216)	
		angle = 216;
	else if(angle_azimuth>=216 && angle_azimuth<234)	
		angle = 234;
	else if(angle_azimuth>=234 && angle_azimuth<252)	
		angle = 252;
	else if(angle_azimuth>=252 && angle_azimuth<270)	
		angle = 270;
	else if(angle_azimuth>=270 && angle_azimuth<288)	
		angle = 288;
	else if(angle_azimuth>=288 && angle_azimuth<306)	
		angle = 306;
	else if(angle_azimuth>=306 && angle_azimuth<324)	
		angle = 324;
	else if(angle_azimuth>=324 && angle_azimuth<342)	
		angle = 342;
	else if(angle_azimuth>=342 && angle_azimuth<360)	
		angle = 360;
	printf("  A:");	
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
