#ifndef __MULTI_VIEWER_HPP__
#define __MULTI_VIEWER_HPP__

#include "CameraArray.hpp"
#include "BlobExtractor.hpp"

#ifndef ENABLE_RENDERING
#define ENABLE_RENDERING 0
#endif

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
		
		// For compositing the original images,
		// and potentially the exatracted markers too.
		cv::Mat concatenated; // Concatenated vertically 
		cv::Mat resized; // Scaled down if necessary
		cv::Mat transposed; // Transpose as first step to rotating
		cv::Mat rotated; // Rotated 90 Degrees

	public:
		
#if ENABLE_RENDERING
		// Similar to above, but for compositing the masks.
		cv::Mat concatenated_masks; // Concatenated vertically 
		cv::Mat resized_masks; // Scaled down if necessary
		cv::Mat transposed_masks; // Transpose as first step to rotating
		cv::Mat rotated_masks; // Rotated 90 Degrees
		uint8_t *renderFrames[CAMERA_COUNT];
#endif

		void init(CameraArray *ca, BlobExtractor **bes);
		MultiViewer(CameraArray *ca, BlobExtractor **bes);
		MultiViewer(CameraArray *ca);
		~MultiViewer();
		void update(void);
};

#endif

