#ifndef __MULTI_VIEWER_HPP__
#define __MULTI_VIEWER_HPP__

#include "CameraArray.hpp"
#include "BlobExtractor.hpp"

#include <opencv2/opencv.hpp>

class MultiViewer
{
	protected:
	private:
		CameraArray *cameraArray;
		BlobExtractor *blobExtractors[CAMERA_COUNT];

		int frame_h;
		int frame_w;
		int camera_count;

		cv::Mat concatenated; // concatenated version of our frames
		cv::Mat debayered; // (concatenated and) debayered version of frame being processed.
	
	public:
		void init(CameraArray *ca, BlobExtractor **be1);
		MultiViewer(CameraArray *ca, BlobExtractor **be1);
		MultiViewer(CameraArray *ca);
		~MultiViewer();
		void update(void);
};

#endif

