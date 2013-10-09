#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "siemens_communication.hpp"

// Prompt for and return a speed from the user
float getSpeedFromUser()
{
	printf("Please enter the speed as a percentage of the nominal speed:\n");
	float speedpercent=0.0f;
	int numassigned = scanf("%f",&speedpercent);
	if (numassigned!=1) printf("Error reading string!\n");
	if (speedpercent>100.0) speedpercent=100.0;
	if (speedpercent<-100.0) speedpercent=-100.0;
	return speedpercent;
}

int main(int argc, char *argv[])
{
	Siemens siemens;
	bool cont=1;
	while(cont){
		double winchSpeed = getSpeedFromUser();
		double carouselSpeed = getSpeedFromUser();
		siemens.send_reference_speeds(winchSpeed,carouselSpeed);

		printf("Continue?\n");

		char c=EOF;
		while(c==EOF) c=getchar();
		switch (getchar())
		{
			case 'y':
			case 'Y':
			case ' ':
			case '\n':
				cont=1;
				break;
			case 'n':
			case 'q':
			case 'N':
				cont=0;
				break;
		}
	}
	
	return 0;	
}

