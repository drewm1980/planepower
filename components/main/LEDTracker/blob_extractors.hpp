#ifndef BLOB_EXTRACTORS_HPP
#define BLOB_EXTRACTORS_HPP

#include <string>
#include <iostream>
#include <stdint.h>
#include <math.h>

#include <opencv2/opencv.hpp>

// Main header for cvblobs library
//#include "BlobResult.h"
>>>>>>> origin/master:components/main/LEDTracker/src/blob_extractors.hpp

#include "types.hpp"

using namespace std;
using namespace cv;

uint8_t compare_colors(uint8_t r1, 
		uint8_t g1,
		uint8_t b1,
		uint8_t r2,
		uint8_t g2,
		uint8_t b2)
{
	if(r1==0 && g1==0 && b1==0) return 0; 
	uint32_t i1 = r1*r1 + g1*g1 + b1*b1; // levels^2
	const uint32_t intensity_threshold = 128;
	if(i1<intensity_threshold*intensity_threshold) return 0;

	uint32_t i2 = r2*r2 + g2*g2 + b2*b2; // levels^2
	uint32_t dot = r1*r2 + g1*g2 + b1*b2; // levels^2

	// The compiler should be smart enough to reduce this at compile time:
	const float PI = 3.1415;
	const float color_angle_threshold = 10.0f * PI/180.0f; // Radians
	const float color_dot_product_threshold = cosf(color_angle_threshold);
	const float color_dot_product_threshold2 = color_dot_product_threshold
		        *color_dot_product_threshold;

	return 255*(uint8_t)(i1*i2*color_dot_product_threshold2 < dot*dot);
}

// Given a histogram of values, find the median
template<int n>
int find_median(int *a)
{
	unsigned int cumsum[n] = 0;
	cumsum[0] = a[0];
	for(int i=1; i<n; i++)
	{
		cumsum[i] = cumsum[i-1] + a[i];
	}
	unsigned int sum = cumsum[n-1];
	unsigned int halfsum = sum/2;
	for(int i=0; i<n; i++)
	{
		if(cumsum[i]>=halfsum)
			return i;
	}
}

// Finds a single connected component in a "binary" uint8_t image.
// Return value is the number of connected components. (hopefully just 1)
// p is the center of one of the components.
// This implementation simply computes the median x and y coordinates of white pixels.
template<int w, int h>
void find_single_led_singlepass(uint8_t * im, Point2d &p)
{
	// Integrate our binary image in each direction
	int a[w] = 0;
	int b[h] = 0;
	for(int y=0; y<h; y++)
	{
		for(int x=0; x<w; x++)
		{
			uint8_t p = im[y*h + x];
			if(p>0)
			{
				a[x]++;
				b[y]++;
			}
		}
	}
	p.x = find_median<w>(a);
	p.y = find_median<h>(b);
}
template<int w, int h>
void find_single_led_singlepass(const Mat& m, Point2d &p)
{
	find_single_led_singlepass<w,h>(m.data, p);
}

<<<<<<< HEAD:components/main/LEDTracker/blob_extractors.hpp
=======
//Finds a single connected component in a "binary" uint8_t image.
//Return value is the number of connected components. (hopefully just 1)
//p is the center of one of the components.
template<int w, int h>
int find_single_led(const Mat& m, Point2d &p)
{
	// get blobs and filter them using its area
	CBlobResult blobs;

	IplImage iplm = m;

#if 0
	cout << "Displaying image input of find_single_led..." << endl;
	namedWindow("input");
	cvShowImage("input",&iplm);
	waitKey();	
	destroyWindow("input");
#endif

	// find blobs in image
	blobs = CBlobResult( &iplm, NULL, 0);

	//blobs.Filter( blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, param2 );

	int blobCount = blobs.GetNumBlobs();

	CBlob b;

	double mynan = nanl("b");
	Point2d nanpoint;
	nanpoint.x = mynan;
	nanpoint.y = mynan;
	if(blobCount == 0) 
	{
		p = nanpoint;
	} else {
		b = blobs.GetBlob(0);
#if 1
		// Use center of Bounding box as center
		CvRect r = b.GetBoundingBox();
		p.x = r.x + r.width/2;
		p.y = r.y + r.height/2;
#else
		// Use center of fitted ellipse as center
		CvBox2D box = b.GetEllipse();
		p = box.center;
#endif
	}
	
#if 0
	if(blobCount > 0)
	{
		cout << "Displaying blob detector result..." << endl;
		Mat foom = Mat::zeros(h, w, CV_8UC(3));
		IplImage foo = foom;
		b.FillBlob(&foo, Scalar(127,127,127));
		circle(foom, p, 3, Scalar(255, 255, 255), -1);
		namedWindow("foom");
		imshow("foom",foom);
		cvWaitKey(-1);
		destroyWindow("foom");
	}
#endif

	return blobCount;
}


>>>>>>> origin/master:components/main/LEDTracker/src/blob_extractors.hpp
// This function extracts a red, a green, and a blue blob from a bayer coded image.
// Coordinates are traditional image coordinates: u increases to the right along scaline,
// v increases as you go down rows in the image.
template<int w, int h>
void extract_blobs(const Mat & bayer, MarkerLocations &l)
{
	assert(bayer.data>0);
	Mat bgr = Mat::zeros(h, w, CV_8UC(3));
	cvtColor(bayer, bgr, CV_BayerBG2BGR);

	Mat r_mask = Mat::zeros(h, w, CV_8U);
	Mat g_mask = Mat::zeros(h, w, CV_8U);
	Mat b_mask = Mat::zeros(h, w, CV_8U);

	// Manual color thresholding.  
	for(int y=0; y<h; y++)
	{
		uchar* p_bgr = bgr.ptr<uchar>(y);
		uchar* p_r_mask = r_mask.ptr<uchar>(y);
		uchar* p_g_mask = g_mask.ptr<uchar>(y);
		uchar* p_b_mask = b_mask.ptr<uchar>(y);
		for( int x = 0; x<w; x+=1, p_bgr+=3 )
		{
			uint8_t b=p_bgr[0];
			uint8_t g=p_bgr[1];
			uint8_t r=p_bgr[2];

			p_b_mask[x] = compare_colors(b,g,r,1,0,0);
			p_g_mask[x] = compare_colors(b,g,r,0,1,0);
			p_r_mask[x] = compare_colors(b,g,r,0,0,1);
		}
	}

	find_single_led_singlepass<w,h>(r_mask, l.red);
	find_single_led_singlepass<w,h>(g_mask, l.green);
	find_single_led_singlepass<w,h>(b_mask, l.blue);

#if 0
	cout << "Displaying composite of thresholded images for debugging..." << endl;
	vector<Mat> mv;
	mv.push_back(b_mask);
	mv.push_back(g_mask);
	mv.push_back(r_mask);
	Mat temp;
	merge(mv, temp);
	namedWindow("temp");
	imshow("temp",temp);
#endif
};
template<int w, int h>
void extract_blobs(uint8_t *bayer, MarkerLocations &l)
{
	Mat mat_bayer(h, w, CV_8UC(1), bayer);
	extract_blobs<w,h>(mat_bayer, l);
}

#endif
