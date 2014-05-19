#include <stdio.h>
#include <unistd.h>
#include "realtime.h"

int main (int argc, char** argv) {
	sleep(10);
	printf("Enabeling RealTime Conditions...\n");
	enableRT();
	printPrio();
	sleep(10);	
	printf("Disabeling RealTime Conditions...\n");
	disableRT();
	printPrio();
	sleep(10);
}
