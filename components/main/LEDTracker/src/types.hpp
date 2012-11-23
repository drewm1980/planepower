#ifndef TYPES_HPP
#define TYPES_HPP

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

struct cameraSpecs
{
	int w; // pixels
	int h; // pixels
	int markerWidth; // pixels.  Reasonable guess needed for benchmark
};

#endif
