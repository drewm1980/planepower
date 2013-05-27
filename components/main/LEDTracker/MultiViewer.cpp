#include <string>
#include <iostream>
#include <stdint.h>
#include <math.h>
#include <signal.h>

#include "MultiViewer.hpp"
#include "cout.hpp"
#include "gamma.hpp"

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
	concatenated_masks = Mat::zeros(concatenated_h, concatenated_w, CV_8UC(3));
	resized_h = concatenated_h/render_scale_factor;
	resized_w = concatenated_w/render_scale_factor;
	resized = Mat::zeros(resized_h, resized_w, CV_8UC(3));
	resized_masks = Mat::zeros(resized_h, resized_w, CV_8UC(3));
	rotated_h = resized_w;
	rotated_w = resized_h;
	transposed = Mat::zeros(rotated_h, rotated_w, CV_8UC(3));
	transposed_masks = Mat::zeros(rotated_h, rotated_w, CV_8UC(3));
	rotated = Mat::zeros(rotated_h, rotated_w, CV_8UC(3));
	rotated_masks = Mat::zeros(rotated_h, rotated_w, CV_8UC(3));

	init_gamma_compress_lookup_table();

	namedWindow("MultiViewer",CV_WINDOW_NORMAL|CV_WINDOW_FREERATIO|CV_GUI_NORMAL);
	if (blobExtractors != NULL)
	{
		for(int i=0; i<camera_count; i++)
		{
			renderFrames[i] = concatenated_masks.ptr<uchar>(0) + i*frame_w*frame_h*3;
		}
		namedWindow("Masks",CV_WINDOW_NORMAL|CV_WINDOW_FREERATIO|CV_GUI_NORMAL);
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
	if (blobExtractors != NULL)
	{
		destroyWindow("Masks");
	}

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
		memcpy(dest,src,image_bytes);
		dest += image_bytes;

		if (blobExtractors != NULL)
		{
			// Draw some circles for the extracted LED locations
		}
	}
	
	// Resize
	resize(concatenated,resized,Size(resized_w,resized_h));
	resize(concatenated_masks,resized_masks,Size(resized_w,resized_h));

	// Rotate LEFT 90 degrees efficiently.
	transpose(resized,transposed);
	flip(transposed,rotated,0);
	transpose(resized_masks,transposed_masks);
	flip(transposed_masks,rotated_masks,0);

	// Gamma Compress
	gamma_compress_in_place(rotated);

	// OpenCV has a funny BGR storage convention
	cvtColor(rotated,rotated,CV_RGB2BGR);
	imshow("MultiViewer",rotated);
	cvtColor(rotated_masks,rotated_masks,CV_RGB2BGR);
	imshow("Masks",rotated_masks);

	int k = waitKey(10); // This costs at ~least 1 ms.
	if(k=='q')
	{
		raise(SIGINT);
	}

	if (blobExtractors != NULL)
	{
		// Need a clean slate since the blob extractors will run
		// while waiting for next updateHook.
		concatenated_masks.setTo(cv::Scalar::all(0));
	}
}

