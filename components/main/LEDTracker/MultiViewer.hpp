#ifndef __MULTI_VIEWER_HPP__
#define __MULTI_VIEWER_HPP__

#include "CameraArray.hpp"
#include "BlobExtractor.hpp"

#include <opencv2/opencv.hpp>

const int render_scale_factor = 1;

class MultiViewer
{
	protected:
	private:
		CameraArray *cameraArray;
		BlobExtractor **blobExtractors;

		int frame_h;
		int frame_w;
		int camera_count;

		int concatenated_h;
		int concatenated_w;
		int resized_h;
		int resized_w;
		int rotated_h;
		int rotated_w;
		cv::Mat concatenated; // Concatenated vertically 
		cv::Mat resized; // Scaled down if necessary
		cv::Mat transposed; // Transpose as first step to rotating
		cv::Mat rotated; // Rotated 90 Degrees
	
	public:
		void init(CameraArray *ca, BlobExtractor **be1);
		MultiViewer(CameraArray *ca, BlobExtractor **be1);
		MultiViewer(CameraArray *ca);
		~MultiViewer();
		void update(void);
};

#endif

