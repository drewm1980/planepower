#include <string>
#include <iostream>
#include <stdint.h>
#include <math.h>
#include <signal.h>

#include "MultiViewer.hpp"
#include "cout.hpp"
#include "gamma.hpp"

#include "debayer.hpp"

using namespace std;
using namespace cv;

void MultiViewer::init(CameraArray *ca, BlobExtractor **bes)
{
	cameraArray = ca;
	frame_h = cameraArray->frame_h;
	frame_w = cameraArray->frame_w;
	camera_count = cameraArray->camera_count;
	blobExtractors = bes;

	concatenated_h = frame_h*camera_count;
	concatenated_w = frame_w;
	concatenated = Mat::zeros(concatenated_h, concatenated_w, CV_8UC(3));
#if ENABLE_RENDERING
	concatenated_masks = Mat::zeros(concatenated_h, concatenated_w, CV_8UC(3));
#endif
	resized_h = concatenated_h/render_scale_factor;
	resized_w = concatenated_w/render_scale_factor;
	resized = Mat::zeros(resized_h, resized_w, CV_8UC(3));
#if ENABLE_RENDERING
	resized_masks = Mat::zeros(resized_h, resized_w, CV_8UC(3));
#endif
	rotated_h = resized_w;
	rotated_w = resized_h;
	transposed = Mat::zeros(rotated_h, rotated_w, CV_8UC(3));
#if ENABLE_RENDERING
	transposed_masks = Mat::zeros(rotated_h, rotated_w, CV_8UC(3));
#endif
	rotated = Mat::zeros(rotated_h, rotated_w, CV_8UC(3));
#if ENABLE_RENDERING
	rotated_masks = Mat::zeros(rotated_h, rotated_w, CV_8UC(3));
#endif

	init_gamma_compress_lookup_table();

	namedWindow("MultiViewer",CV_WINDOW_NORMAL|CV_WINDOW_FREERATIO|CV_GUI_NORMAL);
	if (blobExtractors != NULL)
	{
#if ENABLE_RENDERING
		for(int i=0; i<camera_count; i++)
		{
			renderFrames[i] = concatenated_masks.ptr<uchar>(0) + i*frame_w*frame_h*3;
		}
		namedWindow("Masks",CV_WINDOW_NORMAL|CV_WINDOW_FREERATIO|CV_GUI_NORMAL);
#endif
	}
}
MultiViewer::MultiViewer(CameraArray *ca, BlobExtractor **bes)
{
	init(ca,bes);
}
MultiViewer::MultiViewer(CameraArray *ca)
{
	init(ca,NULL);
}

MultiViewer::~MultiViewer()
{
	destroyWindow("MultiViewer");
#if ENABLE_RENDERING
	destroyWindow("Masks");
#endif

}

void MultiViewer::update(void)
{
	// Concatenate the images vertically.
	// This is another thing OpenCV doesn't have a good in-place function for.
	uint8_t * dest = concatenated.ptr<uchar>(0);
	int image_bytes = frame_h*frame_w*3;
	for(int i=0; i<camera_count; i++)
	{
		// Into the first frame, composite the original images
		uint8_t * src = cameraArray->current_frame_data[i];
		if(NEED_TO_DEBAYER)
		{
			debayer_frame(src, dest, frame_w, frame_h);
		} else {
			memcpy(dest,src,image_bytes);
		}
		dest += image_bytes;

		if (blobExtractors != NULL)
		{
			// Draw some circles for the extracted LED locations
		}
	}
	
	// Resize
	resize(concatenated,resized,Size(resized_w,resized_h));
#if ENABLE_RENDERING
	resize(concatenated_masks,resized_masks,Size(resized_w,resized_h));
#endif

	// Rotate LEFT 90 degrees efficiently.
	transpose(resized,transposed);
	flip(transposed,rotated,0);
#if ENABLE_RENDERING
	transpose(resized_masks,transposed_masks);
	flip(transposed_masks,rotated_masks,0);
#endif

	// Gamma Compress
	gamma_compress_in_place(rotated);

	// OpenCV has a funny BGR storage convention
	cvtColor(rotated,rotated,CV_RGB2BGR);
	imshow("MultiViewer",rotated);
#if ENABLE_RENDERING
	cvtColor(rotated_masks,rotated_masks,CV_RGB2BGR);
	imshow("Masks",rotated_masks);
#endif

	int k = waitKey(10); // This costs at ~least 1 ms.
	if(k=='q')
	{
		raise(SIGINT);
	}

#if ENABLE_RENDERING	
	// Need a clean slate since the blob extractors will run
	// while waiting for next updateHook.
	concatenated_masks.setTo(cv::Scalar::all(0));
#endif

}

