#ifndef TYPES_HPP
#define TYPES_HPP

#include "opencv2/opencv.hpp"

struct cameraSpecs
{
	int w; // pixels
	int h; // pixels
	int markerWidth; // pixels.  Reasonable guess needed for benchmark
};

struct MarkerLocations
{
	// WARNING: The following union doesn't work due to some C++ template b.s.
	//			As a work around, some code that loops acceses green and blue
	//			by indexing off of red.  So don't insert any new members between them!
	//			Technically, the layout of structures in memory is also compiler
	//			dependent, but in practice it's always sequential.
	//union{
		//cv::Point2d colors[3];
		//struct {
			cv::Point2d red;
			cv::Point2d green;
			cv::Point2d blue;
		//};
	//};
};

#endif
