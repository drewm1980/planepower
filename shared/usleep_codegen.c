#include <time.h>
#include <stdint.h>
#include <stdio.h>

#define sleep_loop(x) {for(volatile int i=x; i>0; i--);}

#define TEST_LOOPS 100000

/*
typedef struct timespec {
               time_t tv_sec;         //seconds 
               long   tv_nsec;       // nanoseconds 
           }; Timespec;
*/

// Codegen a macro that does accurate busy sleeps in units of microseconds
void generate()
{
	struct timespec t1,t2;
	clock_gettime(CLOCK_REALTIME,&t1);
	sleep_loop(TEST_LOOPS);
	clock_gettime(CLOCK_REALTIME,&t2);
	int64_t dt1 = (t2.tv_sec - t1.tv_sec)*1e9; // ns
	int64_t dt2 = (t2.tv_nsec - t1.tv_nsec); // ns
	int64_t dt = (dt1 + dt2) / 1000; // us
	int64_t loops = TEST_LOOPS/dt; // loops / us
	printf("// Time (us) for %i loops: %ld\n",TEST_LOOPS,dt);
	printf("// Loops per microsecond: %ld\n\n",loops);
	printf("#define usleep_busy(x) {for(int i=%ld; i>0; i--);}\n",loops);
}

int main() {generate();};
