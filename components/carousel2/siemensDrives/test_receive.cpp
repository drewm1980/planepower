#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include <time.h>

#include "siemens_communication.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	SiemensReceiver s;
	SiemensDriveState ds;

	cout << "Waiting for at least one packet..." << endl;
	s.read(&ds);

	cout << "Measuring how fast we are receiving packets from the PLC..." << endl;
	timespec t1, t2;
	const float trials = 100.0;
	clock_gettime(CLOCK_REALTIME, &t1); 
	for(int i=0; i<trials; i++) { s.read(&ds); }
	clock_gettime(CLOCK_REALTIME, &t2); 
	time_t dsec = t2.tv_sec-t1.tv_sec;
	long dnsec = t2.tv_nsec-t1.tv_nsec;
	double dt = (dsec + dnsec*1.0e-9)/trials; // seconds per sample
	cout << "Received " << trials << " packets at dt= " << dt/trials << " s per sample" << endl;

	while(0)
	{
		printf("\nWaiting for data...\n");
		fflush(stdout);
		s.read(&ds);
		printf("winchSpeedSmoothed: %f\n",ds.winchSpeedSmoothed);
		//printf("winchPosition: %f\n",ds.winchEncoderPosition);
		//printf("winchSpeedSetpoint: %f\n",ds.winchSpeedSetpoint);
	}
}

