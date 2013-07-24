#include <string>
#include <iostream>
#include <stdint.h>

#include "BlobExtractor.hpp"

const int pixelstride=3;

void drawbox(uint8_t * frame_data, int h, int w, int frame_w, int colorIndex)
{
	for(int i=0; i<h; i++)
	{
		for(int j=0; j<w; j++)
		{
			frame_data[i*frame_w*pixelstride + j*pixelstride + colorIndex] = 255;
		}
	}

}

using namespace std;
int main(int argc, char **argv) 
{
	BlobExtractor *be;
	int frame_w = 800;
	int frame_h = 600;

	cout << "Instantating BlobExtractor" << endl;
	be = new BlobExtractor(frame_w,frame_h,false);
	uint8_t * frame_data = (uint8_t *) calloc(frame_w*frame_h*3,sizeof(uint8_t));

	cout << "Drawing some boxes..." << endl;
	drawbox(frame_data + 400*frame_w*pixelstride + 100*pixelstride, 10, 10, frame_w, 0);
	drawbox(frame_data + 400*frame_w*pixelstride + 200*pixelstride, 10, 10, frame_w, 1);
	drawbox(frame_data + 400*frame_w*pixelstride + 300*pixelstride, 10, 10, frame_w, 2);

	cout << "Running find_leds..." << endl;
	be->find_leds(frame_data);

	MarkerLocations ml = be->markerLocations;
	cout <<
	"ml.rx = " << ml.rx << endl <<
	"ml.ry = " << ml.ry << endl <<
	"ml.gx = " << ml.gx << endl <<
	"ml.gy = " << ml.gy << endl <<
	"ml.bx = " << ml.bx << endl <<
	"ml.by = " << ml.by << endl;

	if(
			abs(ml.rx-105)<2 &&
			abs(ml.ry-405)<2 &&
			abs(ml.gx-205)<2 &&
			abs(ml.gy-405)<2 &&
			abs(ml.bx-305)<2 &&
			abs(ml.by-405)<2
			)
	{
		cout << "Test passed!" << endl;
	}else{
		cout << "Test FAILED!!!!" << endl;
	}

	free(frame_data);
	delete be;
}
