#ifndef TIMING_H
#define TIMING_H

#include <time.h>

// Function that just returns the number of nanoseconds since the "highwind" epoch
__inline__ uint64_t gettime_highwind(void)
{
	time_t t;
	clock_gettime(CLOCK_MONOTONIC, &t);
}


/* return time passed since last call to tic on this timer */
real_t toc(timer* t)
{
	struct timespec temp;
    
	clock_gettime(CLOCK_MONOTONIC, &t->toc);	
    
	if ((t->toc.tv_nsec - t->tic.tv_nsec) < 0)
	{
		temp.tv_sec = t->toc.tv_sec - t->tic.tv_sec - 1;
		temp.tv_nsec = 1000000000+t->toc.tv_nsec - t->tic.tv_nsec;
	}
	else
	{
		temp.tv_sec = t->toc.tv_sec - t->tic.tv_sec;
		temp.tv_nsec = t->toc.tv_nsec - t->tic.tv_nsec;
	}
	
	return (real_t)temp.tv_sec + (real_t)temp.tv_nsec / 1e9;
}
