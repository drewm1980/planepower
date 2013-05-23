#include <string>
#include <iostream>
#include <stdint.h>
#include <math.h>
#include <signal.h>

#include "Tracer.hpp"
#include "cout.hpp"

using namespace std;
using namespace cv;

#include "gamma.cpp"

void MultiViewer::init(CameraArray *ca, BlobExtractor **blobExtractors)
{
	cameraArray = ca;
	namedWindow("MultiViewer");
	frame_h = cameraArray->frame_h;
	frame_w = cameraArray->frame_w;
	camera_count = cameraArray->camera_count;

	const int render_scale_factor = 2;

	concatenated = Mat::zeros(frame_h*camera_count*render_scale_factor, frame_w, CV_8U3);
	debayered =    Mat::zeros(frame_h*camera_count*render_scale_factor, frame_w, CV_8U3);

	init_gamma_compress_lookup_table();
}
Tracer::Tracer(StereoPair *sp, BlobExtractors **be)
{
	init(sp,be);
}
Tracer::Tracer(StereoPair *sp)
{
	init(sp,NULL);
}


Tracer::~Tracer()
{
	destroyWindow("Tracer");
}

void Tracer::update(void)
{
	//scanline.setTo(cvScalar(0));
	//normline.setTo(cvScalar(0));
	scanline = cvScalar(0);
	normline = cvScalar(0);

	for(int frameidx=0; frameidx<2; frameidx++)
	//int frameidx=0;
	{
		Mat bayer(frame_h, frame_w, CV_8UC(1), 
				cameraArray->current_frame_data[frameidx]);
		assert(bayer.data>0);
		cvtColor(bayer, debayered, CV_BayerBG2BGR);

		// Gather a row containing the brightest pixel in each column
		for(int y=0; y<cameraArray->frame_h; y++)
		{
			uint8_t * db = debayered.ptr<uchar>(y);
			uint8_t * sl = scanline.ptr<uchar>(0);
			int32_t * nl = normline.ptr<int32_t>(0);
			for(int x=0, xx=0; x<cameraArray->frame_w; x+=1, xx+=3)
			{
				uint8_t b=db[xx+0];
				uint8_t g=db[xx+1];
				uint8_t r=db[xx+2];
				int32_t n = b*b+g*g+r+r;
				if(n>nl[x])
				{
					nl[x] = n;
					sl[xx+0] = db[xx+0];
					sl[xx+1] = db[xx+1];
					sl[xx+2] = db[xx+2];
				}
			}
		}
	}

	// Make high saturation more obvious by zebra striping saturated pixels.
	uint8_t * sl = scanline.ptr<uchar>(0);
	for(int x=0, xx=0; x<cameraArray->frame_w; x+=1, xx+=3)
	{
		if(sl[xx+0]==255 || sl[xx+1]==255 || sl[xx+2]==255)
		{
			if (x%2)
			{
				sl[xx+0] = 255;
				sl[xx+1] = 255;
				sl[xx+2] = 255;
			} else
			{
				sl[xx+0] = 0;
				sl[xx+1] = 0;
				sl[xx+2] = 0;
			}
		}
	}

	// Shift the image down a scanline.
	for(int i=renderframe_h-1; i>0; i--)
	{
		Mat r1 = renderframe.row(i-1);
		Mat r2 = renderframe.row(i);
		r1.copyTo(r2);
	}

	// Render one coordinate of the blob extractor results into the image.
	for(int i=0; i<2; i++)
	{
		BlobExtractor * be = blobExtractors[i];
		if(be!=NULL)
		{
			for(int j=0; j<3; j++)
			{
				cv::Point2d p = *(&(be->markerLocations.red) + j); // Pointer arithmetic!
				//cv::Point2d p = be->markerLocations.colors[j]; // This is what I ~wanted to do
				p.y = 0;
				if(!isnan(p.x))
				{
					circle(scanline, p, 3, cvScalar(255,255,255));
				}

			}
		}
	}

	// Copy the new pixels into the dislpayed image.
	Mat r = renderframe.row(0);
	scanline.copyTo(r);
		
	// Trim and zoom the image for better viewing
	Mat dst = renderframe;
#if 0
	Mat src = renderframe.colRange(300,600);
	//resize(src, dst, Size(), 4.0, 4.0, INTER_NEAREST);
	resize(src, dst, Size(), 4.0, 1.0, INTER_NEAREST);
#endif

	// Gamma compress the image before display.
	// The opencv devs had time to port the entire library to C++
	// but it still can't display linearly coded images properly.
#if 0
	for(int i=0; i<dst.rows; i++)
	{
		uint8_t * p_dst = dst.ptr<uchar>(i);
		for(int x=0, xx=0; x<dst.cols; x+=1, xx+=3)
		{
			for(int k=0; k<3; k++)
			{
				p_dst[xx+k] = gamma_compress_lookup_table[p_dst[xx+k]];
			}
		}
	}
#endif
	
	//cout << "Calling imshow!!!!" << endl;
	imshow("Tracer",dst);
	int k = waitKey(10); // This costs at ~least 1 ms.
	if(k=='q')
	{
		raise(SIGINT);
	}
}

