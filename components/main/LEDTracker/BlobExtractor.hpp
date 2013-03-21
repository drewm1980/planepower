#ifndef __BLOB_EXTRACTOR_HPP__
#define __BLOB_EXTRACTOR_HPP__

#include <string>
#include <iostream>
#include <stdint.h>
#include <math.h>

#include <opencv2/opencv.hpp>

#include "types.hpp"
#include "MedianFinder.hpp"

// This class extracts colored blobs from a bayer coded image.
// May call malloc first time it's used, so re-use it.
class BlobExtractor
{
	protected:
	private:

		int* integrated_w;
		int* integrated_h;

		cv::Mat bgr;
		cv::Mat r_mask;
		cv::Mat g_mask;
		cv::Mat b_mask;

		uint8_t compare_colors(uint8_t r1, 
				uint8_t g1,
				uint8_t b1,
				uint8_t r2,
				uint8_t g2,
				uint8_t b2);

		MedianFinder *medianFinder_w, *medianFinder_h;
		
		// Find a single led in a single channel image
		void find_single_led_singlepass(uint8_t * im, cv::Point2d &p);
		void find_single_led_singlepass(const cv::Mat& m, cv::Point2d &p);

	public:

		BlobExtractor(int w, int h);
		~BlobExtractor();

		int frame_w;
		int frame_h;

		MarkerLocations markerLocations;

		// Extract a red, a green, and a blue blob from a bayer coded image.
		// Coordinates are traditional image coordinates: u increases to the right along scaline,
		// v increases as you go down rows in the image.
		void extract_blobs(const cv::Mat & bayer);
		void extract_blobs(uint8_t *bayer);
};

#endif
