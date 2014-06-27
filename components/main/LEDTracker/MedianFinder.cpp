#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "MedianFinder.hpp"

MedianFinder::MedianFinder(int new_n)
{
	n = new_n;
	cumsum = (uint32_t*) malloc(n*sizeof(uint32_t));
}

MedianFinder::~MedianFinder()
{
	free(cumsum);
}

// Given a histogram of values, find the median
// Returns nan if the vector is zero.
float MedianFinder::find_median(uint32_t *a)
{
	memset(cumsum, 0, n*sizeof(uint32_t));
	cumsum[0] = a[0];
	for(int i=1; i<n; i++)
	{
		cumsum[i] = cumsum[i-1] + a[i];
	}
	unsigned int sum = cumsum[n-1];
	const int minBlobSize = 9; // pixels total
	if(sum < minBlobSize)
	{
		return nanf("z");
	}

	unsigned int halfsum = sum/2;
	for(int i=0; i<n; i++)
	{
		if(cumsum[i]>=halfsum)
			return i;
	}

	return nanf("z");
}
