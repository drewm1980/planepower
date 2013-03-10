#ifndef TYPES_HPP
#define TYPES_HPP

#include <opencv2/opencv.hpp>

struct cameraSpecs
{
	int w; // pixels
	int h; // pixels
	int markerWidth; // pixels.  Reasonable guess needed for benchmark
};

struct MarkerLocations
{
	cv::Point2d red;
	cv::Point2d green;
	cv::Point2d blue;
};


#endif
