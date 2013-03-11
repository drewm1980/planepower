#ifndef __TRACER_HPP__
#define __TRACER_HPP__

#include "StereoPair.hpp"
#include "BlobExtractor.hpp"

#include "opencv2/opencv.hpp"

class Tracer
{
	protected:
	private:
		StereoPair *stereoPair;
		union{
			BlobExtractor *blobExtractors[2];
			struct {
				BlobExtractor *blobExtractor1;
				BlobExtractor *blobExtractor2;
			};
		};

		int frame_h;
		int frame_w;
		int renderframe_w;
		int renderframe_h;

		cv::Mat debayered; // debayered version of frame being processed.
		cv::Mat scanline;
		cv::Mat normline;
		cv::Mat renderframe;
	
	public:
		void init(StereoPair *sp, BlobExtractor *be1, BlobExtractor *be2);
		Tracer(StereoPair *sp, BlobExtractor *be1, BlobExtractor *be2);
		Tracer(StereoPair *sp);
		~Tracer();
		void update(void);
};

#endif

