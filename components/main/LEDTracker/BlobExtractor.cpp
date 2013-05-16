#include "math.h"

#include "BlobExtractor.hpp"
#include "MedianFinder.hpp"
#include "types.hpp"

using namespace std;
using namespace cv;

BlobExtractor::BlobExtractor(int w, int h)
{
	frame_w = w;
	frame_h = h;
	medianFinder_w = new MedianFinder(w);
	medianFinder_h = new MedianFinder(h);

	bgr = Mat::zeros(frame_h, frame_w, CV_8UC(3));
	r_mask = Mat::zeros(frame_h, frame_w, CV_8U);
	g_mask = Mat::zeros(frame_h, frame_w, CV_8U);
	b_mask = Mat::zeros(frame_h, frame_w, CV_8U);

	integrated_w = (int*) malloc(w*sizeof(int));
	integrated_h = (int*) malloc(h*sizeof(int));
}
BlobExtractor::~BlobExtractor()
{
	delete medianFinder_w;
	delete medianFinder_h;
	free(integrated_w);
	free(integrated_h);
}

uint8_t BlobExtractor::compare_colors(uint8_t r1, 
		uint8_t g1,
		uint8_t b1,
		uint8_t r2,
		uint8_t g2,
		uint8_t b2)
{
	if(r1==0 && g1==0 && b1==0) return 0; 
	uint32_t i1 = r1*r1 + g1*g1 + b1*b1; // levels^2
	const uint32_t intensity_threshold = 32;
	if(i1<intensity_threshold*intensity_threshold) return 0;

	uint32_t i2 = r2*r2 + g2*g2 + b2*b2; // levels^2
	uint32_t dot = r1*r2 + g1*g2 + b1*b2; // levels^2

	// The compiler should be smart enough to reduce this at compile time:
	const float PI = 3.1415;
	const float color_angle_threshold = 20.0f * PI/180.0f; // Radians
	const float color_dot_product_threshold = cosf(color_angle_threshold); // unitless thresh on cosine of the color angle
	const float color_dot_product_threshold2 = color_dot_product_threshold
		        *color_dot_product_threshold;

	return 255*(uint8_t)(i1*i2*color_dot_product_threshold2 < dot*dot);
}


// Finds a single connected component in a "binary" uint8_t image.
// Return value is the number of connected components. (hopefully just 1)
// p is the center of one of the components.
// This implementation simply computes the median x and y coordinates of white pixels.
void BlobExtractor::find_single_led_singlepass(uint8_t * im, Point2d &p)
{
	// Integrate our binary image in each direction
	memset(integrated_w, 0, frame_w*sizeof(int));
	memset(integrated_h, 0, frame_h*sizeof(int));
	for(int y=0; y<frame_h; y++)
	{
		for(int x=0; x<frame_w; x++)
		{
			uint8_t p = im[y*frame_w + x];
			if(p>0)
			{
				integrated_w[x]++;
				integrated_h[y]++;
			}
		}
	}

	p.x = medianFinder_w->find_median(integrated_w);
	p.y = medianFinder_h->find_median(integrated_h);
	
	// DIRTY HACK
	// Once in thousands of samples I get a measurement at (0,0).  No clue why.
	if(p.x==0 || p.y==0)
	{
		p.x = nanf("z");
		p.y = nanf("z");
	}
}
void BlobExtractor::find_single_led_singlepass(const Mat& m, Point2d &p)
{
	find_single_led_singlepass(m.data, p);
}

// This function extracts a red, a green, and a blue blob from a bayer coded image.
// Coordinates are traditional image coordinates: u increases to the right along scaline,
// v increases as you go down rows in the image.
void BlobExtractor::extract_blobs(const Mat & bayer)
{
	assert(bayer.data>0);
	cvtColor(bayer, bgr, CV_BayerBG2BGR);

	// Manual color thresholding.  
	for(int y=0; y<frame_h; y++)
	{
		uchar* p_bgr = bgr.ptr<uchar>(y);
		uchar* p_r_mask = r_mask.ptr<uchar>(y);
		uchar* p_g_mask = g_mask.ptr<uchar>(y);
		uchar* p_b_mask = b_mask.ptr<uchar>(y);
		for( int x = 0; x<frame_w; x+=1, p_bgr+=3 )
		{
			uint8_t b=p_bgr[0];
			uint8_t g=p_bgr[1];
			uint8_t r=p_bgr[2];

			p_b_mask[x] = compare_colors(b,g,r,1,0,0);
			p_g_mask[x] = compare_colors(b,g,r,0,1,0);
			p_r_mask[x] = compare_colors(b,g,r,0,0,1);
		}
	}
	//imwrite("r_mask.bmp",r_mask);
	//imwrite("g_mask.bmp",g_mask);
	//imwrite("b_mask.bmp",b_mask);

	find_single_led_singlepass(r_mask, markerLocations.red);
	find_single_led_singlepass(g_mask, markerLocations.green);
	find_single_led_singlepass(b_mask, markerLocations.blue);

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
void BlobExtractor::extract_blobs(uint8_t *bayer)
{
	Mat mat_bayer(frame_h, frame_w, CV_8UC(1), bayer);
	extract_blobs(mat_bayer);
}

