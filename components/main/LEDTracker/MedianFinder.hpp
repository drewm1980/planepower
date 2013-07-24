#ifndef __MEDIAN_FINDER_HPP__
#define __MEDIAN_FINDER_HPP__
#include <stdint.h>

class MedianFinder
{
	protected:
	private:
		int n;
		uint32_t* cumsum;

	public:
		MedianFinder(int new_n);
		~MedianFinder();
		// Given a histogram of values, find the median.  Returns nan if input vector is zero.
		float find_median(uint32_t *a);
};

#endif
