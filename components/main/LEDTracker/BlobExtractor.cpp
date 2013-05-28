#include "math.h"

#include "BlobExtractor.hpp"
#include "MedianFinder.hpp"
#include "types.hpp"

using namespace std;
using namespace cv;

BlobExtractor::BlobExtractor(int w, int h)
{
	frame_w = w;
	frame_h = h;

	medianFinder_w_r = new MedianFinder(w);
	medianFinder_w_g = new MedianFinder(w);
	medianFinder_w_b = new MedianFinder(w);
	medianFinder_h_r = new MedianFinder(h);
	medianFinder_h_g = new MedianFinder(h);
	medianFinder_h_b = new MedianFinder(h);
	integrated_w_r = (int*) malloc(w*sizeof(int));
	integrated_w_g = (int*) malloc(w*sizeof(int));
	integrated_w_b = (int*) malloc(w*sizeof(int));
	integrated_h_r = (int*) malloc(h*sizeof(int));
	integrated_h_g = (int*) malloc(h*sizeof(int));
	integrated_h_b = (int*) malloc(h*sizeof(int));

#if ENABLE_RENDERING
	renderFrame = NULL;
#endif
}
BlobExtractor::~BlobExtractor()
{
	delete medianFinder_w_r;
	delete medianFinder_w_g;
	delete medianFinder_w_b;
	delete medianFinder_h_r;
	delete medianFinder_h_g;
	delete medianFinder_h_b;
	free(integrated_w_r);
	free(integrated_w_g);
	free(integrated_w_b);
	free(integrated_h_r);
	free(integrated_h_g);
	free(integrated_h_b);
}

bool BlobExtractor::compare_colors(uint8_t r1, 
		uint8_t g1,
		uint8_t b1,
		uint8_t r2,
		uint8_t g2,
		uint8_t b2)
{
	if(r1==0 && g1==0 && b1==0) return 0; 
	uint32_t i1 = r1*r1 + g1*g1 + b1*b1; // levels^2
	const uint32_t intensity_threshold = 32;
	if(i1<intensity_threshold*intensity_threshold) return 0;

	uint32_t i2 = r2*r2 + g2*g2 + b2*b2; // levels^2
	uint32_t dot = r1*r2 + g1*g2 + b1*b2; // levels^2

	// The compiler should be smart enough to reduce this at compile time:
	const float PI = 3.1415;
	const float color_angle_threshold = 20.0f * PI/180.0f; // Radians
	const float color_dot_product_threshold = cosf(color_angle_threshold); // unitless thresh on cosine of the color angle
	const float color_dot_product_threshold2 = color_dot_product_threshold
		        *color_dot_product_threshold;

	return (i1*i2*color_dot_product_threshold2 < dot*dot);
}

void BlobExtractor::find_leds(uint8_t * im)
{
	memset(integrated_w_r, 0, frame_w*sizeof(int));
	memset(integrated_w_g, 0, frame_w*sizeof(int));
	memset(integrated_w_b, 0, frame_w*sizeof(int));
	memset(integrated_h_r, 0, frame_h*sizeof(int));
	memset(integrated_h_g, 0, frame_h*sizeof(int));
	memset(integrated_h_b, 0, frame_h*sizeof(int));
	
	// Iterate over rows
	uint8_t * pp = im;
	for(int y=0; y<frame_h; y++)
	{
		// Iterate over columns
		for(int x=0; x<frame_w; x++)
		{
			uint8_t r1 = pp[0];	
			uint8_t g1 = pp[1];	
			uint8_t b1 = pp[2];	

			// Apply the color thresholds to our pixel
			bool is_r = compare_colors(r1,g1,b1,1,0,0);
			bool is_g = compare_colors(r1,g1,b1,0,1,0);
			bool is_b = compare_colors(r1,g1,b1,0,0,1);

			// Increment the number of pixels marked red in this row, column
			if ( is_r ) 
			{
				integrated_w_r[x] += 1;
				integrated_h_r[y] += 1;
			}
			if ( is_g ) 
			{
				integrated_w_g[x] += 1;
				integrated_h_g[y] += 1;
			}
			if ( is_b ) 
			{
				integrated_w_b[x] += 1;
				integrated_h_b[y] += 1;
			}

#if ENABLE_RENDERING
			renderFrame[y*frame_w*3 + x*3 + 0] = 255*is_r;
			renderFrame[y*frame_w*3 + x*3 + 1] = 255*is_g;
			renderFrame[y*frame_w*3 + x*3 + 2] = 255*is_b;
#endif

			pp += 3 * sizeof(uint8_t);
		}
	}

	markerLocations.rx = medianFinder_w_r->find_median(integrated_w_r);
	markerLocations.ry = medianFinder_h_r->find_median(integrated_h_r);
	markerLocations.gx = medianFinder_w_g->find_median(integrated_w_g);
	markerLocations.gy = medianFinder_h_g->find_median(integrated_h_g);
	markerLocations.bx = medianFinder_w_b->find_median(integrated_w_b);
	markerLocations.by = medianFinder_h_b->find_median(integrated_h_b);
	
}

