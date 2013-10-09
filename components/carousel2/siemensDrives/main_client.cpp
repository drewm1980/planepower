#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h> 
#include <ctype.h>

#include "siemens_communication.hpp"

// Prompt for and return a speed from the user
float getSpeedFromUser()
{
	printf("Please enter the speed as a percentage of the nominal speed:\n");
	float speedpercent;
	scanf("%f",&speedpercent);
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

		//printf("Continue?\n");
		//char cc = getchar();
//		if(cc!='y' && cc!='Y') cont=0;
	}
	
	return 0;	
}

