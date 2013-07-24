#ifndef DEBAYER_HPP
#define DEBAYER_HPP

#include <opencv2/opencv.hpp>

using namespace std;

static __inline__ __attribute__((always_inline)) void debayer_frame(uint8_t* src, uint8_t* dst, int frame_w, int frame_h)
{
using namespace cv;
	Mat srcmat(frame_h, frame_w, CV_8UC(1),src);
	Mat dstmat(frame_h, frame_w, CV_8UC(3),dst);
	cvtColor(srcmat, dstmat, CV_BayerBG2RGB);
}

#endif // guard
