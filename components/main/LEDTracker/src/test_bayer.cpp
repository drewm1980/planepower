#include <string>
#include <iostream>
#include <stdint.h>

#include "blob_extractors.hpp"


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
void to_bayer(Mat bgr, Mat bayer)
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


int main(int argc, char **argv) 
{
	const int frame_w = 320;
	const int frame_h = 240;

	// Draw red, green, and blue circles into a color image
	Mat bgr = Mat::zeros(frame_h, frame_w, CV_8UC(3));
	Point2d center(frame_w/2, frame_h/2);
	Point2d perturb(frame_h/3, 0);
	int radius = frame_h/30;
	Scalar blueColor(255, 0, 0); // Scalar initializer is in BGR order
	Scalar greenColor(0, 255, 0);
	Scalar redColor(0, 0, 255);
	circle(bgr, center-perturb, radius, blueColor, -1);
	circle(bgr, center        , radius, greenColor, -1);
	circle(bgr, center+perturb, radius, redColor, -1);
	namedWindow("bgr");
	imshow("bgr",bgr);

	// Convert to bayer pattern encoding
	Mat bayer = Mat::zeros(frame_h, frame_w, CV_8UC1);
	to_bayer(bgr,bayer);
	namedWindow("bayer");
	imshow("bayer",bayer);

	// Convert back to bgr as a sanity check
	Mat bgr2 = Mat::zeros(frame_h, frame_w, CV_8UC(3));
	cvtColor(bayer, bgr2, CV_BayerGR2BGR);
	namedWindow("bgr2");
	imshow("bgr2",bgr2);

	cvWaitKey(-1);

}
