#include <string>
#include <iostream>
#include <stdint.h>

#include "blob_extractors.hpp"
#include "types.hpp"

using namespace std;
using namespace cv;

// Encode a BGR three chanel image with Bayer pattern of our sensor.
//
// Based on:
//		http://html.alldatasheet.com/html-pdf/47468/SONY/ICX274/80/1/ICX274.html
// 
// our sensor has the Bayer pattern:
// GB
// RG 
//
// Based on:
//		http://opencv.willowgarage.com/documentation/cpp/imgproc_miscellaneous_image_transformations.html#cvtColor
//	this corresponds to "BayerGR" coding
//
void to_bayer(const Mat &bgr, Mat &bayer)
{
	assert(bayer.rows == bgr.rows);
	assert(bayer.cols == bgr.cols);
	
	vector<Mat> planes;
	split(bgr, planes);
	Mat blueplane = planes[0];
	Mat greenplane = planes[1];
	Mat redplane = planes[2];

	for( int y = 0; y < bgr.rows; y+=2 )
	{
		uchar* bayer_ptr1 = bayer.ptr<uchar>(y);
		uchar* bayer_ptr2 = bayer.ptr<uchar>(y+1);
		for( int x = 0; x < bgr.cols; x+=2 )
		{
			bayer_ptr1[x] = saturate_cast<uchar>(greenplane.at<uchar>(y,x));
			bayer_ptr1[x+1] = saturate_cast<uchar>(blueplane.at<uchar>(y,x+1));
			bayer_ptr2[x] = saturate_cast<uchar>(redplane.at<uchar>(y+1,x));
			bayer_ptr2[x+1] = saturate_cast<uchar>(greenplane.at<uchar>(y+1,x+1));
		}
	}
}

void simulate_image_formation(const MarkerLocations &l, const cameraSpecs &s, Mat &bayer)
{
	// Draw red, green, and blue circles into a color image
	Mat bgr = Mat::zeros(s.h, s.w, CV_8UC(3));
	int radius = s.markerWidth/2;
	Scalar blueColor(255, 0, 0); // Scalar initializer is in BGR order
	Scalar greenColor(0, 255, 0);
	Scalar redColor(0, 0, 255);
	circle(bgr, l.blue, radius, blueColor, -1);
	circle(bgr, l.red, radius, redColor, -1);
	circle(bgr, l.green, radius, greenColor, -1);

	// Convert to bayer pattern encoding
	bayer = Mat::zeros(s.h, s.w, CV_8UC1);
	to_bayer(bgr,bayer);
#if 1
	cout << "Displaying initial test image..." << endl;
	namedWindow("bgr");
	imshow("bgr",bgr);
	waitKey();	
	destroyWindow("bgr");
#endif
}

int main(int argc, char **argv) 
{
	const cameraSpecs s = {320, 240, 20}; const int h=240, w=320;
	//const cameraSpecs s = {1600, 1200, 8}; const int h=1200, w=1600;
	//const int h = s.h;
	//const int w = s.w;

	Point2d center(s.w/2, s.h/2);
	Point2d perturb(s.h/3, 0);
	MarkerLocations l;
	l.red = center - perturb;
	l.green = center;
	l.blue = center + perturb;

	Mat bayer;
	simulate_image_formation(l, s, bayer);
#if 0
	cout << "Displaying Bayer filtered image..." << endl;
	namedWindow("bayer");
	imshow("bayer",bayer);
	waitKey();	
	destroyWindow("bayer");
#endif

#if 0
	Mat bgr2 = Mat::zeros(s.h, s.w, CV_8UC(3));
	cvtColor(bayer, bgr2, CV_BayerGR2BGR);
	cout << "Displaying Debayered image as a sanity check..." << endl;
	namedWindow("bgr2");
	imshow("bgr2",bgr2);
	waitKey();	
	destroyWindow("bgr2");
#endif

	MarkerLocations l_estimated;
	//for(int i=0; i<100; i++)
	//{
		extract_blobs<w,h>(bayer, l_estimated);	
	//}

	//cvWaitKey(-1);

}
