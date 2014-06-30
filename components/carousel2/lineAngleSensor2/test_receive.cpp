#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <math.h>

#include "armbone_communication.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	ArmboneReceiver s;
	LineAngles ds;

	cout << "Waiting for at least one packet..." << endl;
	s.read(&ds);

	cout << "Measuring how fast we are receiving packets from the Armbone..." << endl;
	timespec t1, t2;
	const float trials = 1000.0;
	clock_gettime(CLOCK_REALTIME, &t1); 
	for(int i=0; i<trials; i++) { s.read(&ds); }
	clock_gettime(CLOCK_REALTIME, &t2); 
	time_t dsec = t2.tv_sec-t1.tv_sec;
	long dnsec = t2.tv_nsec-t1.tv_nsec;
	double dt = (dsec + dnsec*1.0e-9)*1000.0; // milliseconds
	dt/=trials;
	cout << "Received " << trials << " packets at dt= " << dt << " ms per sample" << endl;

        double dt_average = dt;
        cout << "Measuring jitter in data from Armbone..." << endl;
        double worst_case = 0.0;
        double average_jitter = 0.0;
        for(int i=0; i<trials; i++) {
                clock_gettime(CLOCK_REALTIME, &t1);
                s.read(&ds);
                clock_gettime(CLOCK_REALTIME, &t2);
                dsec = (int64_t)t2.tv_sec - (int64_t)t1.tv_sec;
                dnsec = (int64_t)t2.tv_nsec- (int64_t)t1.tv_nsec;
                dt = (dsec + dnsec*1.0e-9)*1000.0; // milliseconds 
                double err = fabs(dt - dt_average);
                if( err > worst_case) worst_case = err;
                average_jitter+=err;
        }
        average_jitter/=trials;

        cout << "After " << trials << " packets, worst case jitter was " << worst_case << " ms " << endl;
        cout << "Average jitter was " << average_jitter << " ms" << endl;

	while(1)
	{
		printf("\nWaiting for data...\n");
		fflush(stdout);
		s.read(&ds);
		printf("Azimuth: %f units\n",ds.azimuth);
		printf("Elevation: %f units\n",ds.elevation);
	}
}

