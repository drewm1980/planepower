#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <iostream>

#include "siemens_communication.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	SiemensCommunicator s;
	SiemensDriveState ds;

	cout << "Measuring how fast we are receiving packets from the PLC" << endl;
	timespec t1, t2;
	clock_gettime(CLOCK_REALTIME, &t1); // Works on Linux
	for(int i=0; i<1000; i++) { s.read(&ds); }
	clock_gettime(CLOCK_REALTIME, &t2); // Works on Linux
	time_t dsec = t2.tv_sec-t1.tv_sec;
	long dnsec = t2.tv_nsec-t1.tv_nsec;
	
	double dt;
	//if(dnsec>0) dt = dsec + dnsec*1e-9

	while(1)
	{
		printf("\nWaiting for data...\n");
		fflush(stdout);
		s.read(&ds);
		printf("winchSpeedSmoothed: %f\n",ds.winchSpeedSmoothed);
		//printf("winchPosition: %f\n",ds.winchEncoderPosition);
		//printf("winchSpeedSetpoint: %f\n",ds.winchSpeedSetpoint);
	}
}

