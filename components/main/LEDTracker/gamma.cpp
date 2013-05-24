#include <opencv2/opencv.hpp>

#include "gamma.hpp"

uint8_t gamma_compress_lookup_table[255];

float gamma_compress(float p, int input_image_scale){
	float a = 0.055;
	float out = p;
	if (input_image_scale != 1){
		out = out / input_image_scale;
	}
	if(out <= 0.0031308){
		out = out * 12.92;
	}else{
		out = powf(out, 1/2.4)*(1+a)-a;
	}
	if (input_image_scale != 1){
		out = out * input_image_scale;
	}
	return out;
}

void init_gamma_compress_lookup_table()
{
	for(int i=0; i<256; i++)
	{
		gamma_compress_lookup_table[i] = gamma_compress(i,255);
	}
}

// Gamma Compress
// The opencv devs had time to port the entire library to C++
// but it still can't display linearly coded images properly.
void gamma_compress_in_place(cv::Mat &im)
{
	for(int i=0; i<im.rows; i++)
	{
		uint8_t * p_im = im.ptr<uchar>(i);
		for(int x=0, xx=0; x<im.cols; x+=1, xx+=3)
		{
			for(int k=0; k<3; k++)
			{
				p_im[xx+k] = gamma_compress_lookup_table[p_im[xx+k]];
			}
		}
	}
}
	
