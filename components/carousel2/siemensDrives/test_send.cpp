#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "siemens_communication.hpp"

// Prompt for and return a speed from the user
float getRefSpeedFromUser()
{
	printf("please enter the speed as a percentage:\n");
	float speedpercent=0.0f;
	int numassigned = scanf("%f",&speedpercent);
	if (numassigned!=1) printf("Error reading string!\n");
	if (speedpercent>100.0) speedpercent=100.0;
	if (speedpercent<-100.0) speedpercent=-100.0;
	return speedpercent;
}

float getCalSpeedFromUser()
{
	printf("please enter the speed:\n");
	float speed=0.0f;
	int numassigned = scanf("%f",&speed);
	if (numassigned!=1) printf("Error reading string!\n");
	return speed;
}
int main(int argc, char *argv[])
{
	SiemensSender siemens;
	bool cont=1;
	while(cont){
		/*printf("For the winch, ");
		double winchSpeed = getRefSpeedFromUser(); // in % 
		printf("For the carousel, ");
		double carouselSpeed = getRefSpeedFromUser(); // in %
		printf("Sending the reference speeds to the PLC!\n");
		siemens.send_carousel_reference_speed(carouselSpeed);
		*/printf("For the winch in m/s, ");
		double winchSpeed = getCalSpeedFromUser(); // m/s
		printf("For the carousel in rad/s, ");
		double carouselSpeed = getCalSpeedFromUser(); // rad/s
		printf("Sending the calibrated speeds to the PLC!\n");
		siemens.send_calibrated_speeds(winchSpeed,carouselSpeed);
	}
	
	return 0;	
}

