#ifndef __BLOB_EXTRACTOR_HPP__
#define __BLOB_EXTRACTOR_HPP__

#include <string>
#include <iostream>
#include <stdint.h>
#include <math.h>

#include <opencv2/opencv.hpp>

#include "types.hpp"
#include "MedianFinder.hpp"

#define LED_COUNT 3

#ifndef ENABLE_RENDERING
#define ENABLE_RENDERING 0
#endif

// This class extracts colored blobs from RGB8 codede images.
// Calls malloc a bunch of times in initialization, so re-use it.
class BlobExtractor
{
	protected:
	private:

		// These are integrated versions of the binary masks for
		// pixels that passed the color thresholds.
		// In otherwords, they are histograms of image coordinates
		// of pixels that pass the color threshold checks.
		uint32_t* integrated_w_r;
		uint32_t* integrated_w_g;
		uint32_t* integrated_w_b;

		uint32_t* integrated_h_r;
		uint32_t* integrated_h_g;
		uint32_t* integrated_h_b;

		bool source_is_bayer_coded;
		uint8_t* debayered_frame_rgb;

		bool compare_colors(uint8_t r1, 
				uint8_t g1,
				uint8_t b1,
				uint8_t r2,
				uint8_t g2,
				uint8_t b2);

		MedianFinder *medianFinder_w_r;
		MedianFinder *medianFinder_w_g;
		MedianFinder *medianFinder_w_b;

		MedianFinder *medianFinder_h_r;
		MedianFinder *medianFinder_h_g;
		MedianFinder *medianFinder_h_b;
		
	public:

		BlobExtractor(int w, int h, bool source_is_bayer_coded);
		~BlobExtractor();

		int frame_w;
		int frame_h;
		
		// Find the 3 LED's in an rgb image.
		// The results go in the markerLocations member variable.
		void find_leds(uint8_t * im);

		// Coordinates are traditional image coordinates: u increases to the right along scaline,
		// v increases as you go down rows in the image.
		MarkerLocations markerLocations;

#if ENABLE_RENDERING
		uint8_t * renderFrame;
#endif

};

#endif
