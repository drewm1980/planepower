#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>

#include "CameraArray.hpp"
#include "cout.hpp"
#include "BlobExtractor.hpp"

#define VERBOSE 1
#define VISUAL 1
#if VISUAL
#include "MultiViewer.hpp"
#endif

using namespace std;
using std::ifstream;

CameraArray *p_t;
void cleanup(int s)
{
	COUT << "Caught SIGINT.  Cleaning up cameras before exiting." << ENDL;
	p_t->stopHook();
	p_t->cleanUpHook();
	signal(SIGINT,SIG_DFL);
}

int main(int argc, char **argv) 
{
	CameraArray cameraArray(false); // false for internal trigger
	p_t = &cameraArray;

	BlobExtractor *blobExtractors[CAMERA_COUNT];

	int frame_w = cameraArray.frame_w;
	int frame_h = cameraArray.frame_h;
	for(int i=0; i<CAMERA_COUNT; i++)
	{
		blobExtractors[i] = new BlobExtractor(frame_w, frame_h, NEED_TO_DEBAYER);
	}

	cameraArray.startHook();
	signal(SIGINT,cleanup);

#if VISUAL
	MultiViewer mv(&cameraArray, blobExtractors);	
	// Register the MultiViewer's rendering locations with the Blob Extractors
	for(int i=0; i<CAMERA_COUNT; i++)
	{
		blobExtractors[i]->renderFrame = mv.renderFrames[i];
	}
#endif

	while(1)
	{
		cameraArray.updateHook();
		for(int i=0; i<CAMERA_COUNT; i++)
		{
			blobExtractors[i] -> find_leds(cameraArray.current_frame_data[i]);
		}
#if VERBOSE
		cout 
			<< "         RED     GREEN   BLUE"  << endl
			<< "         x   y   x   y   x   y"  << endl;
		for(int i=0; i<CAMERA_COUNT; i++)
		{
			cout << "Frame " << i << ": "
			<< blobExtractors[i]->markerLocations.rx << ' '
			<< blobExtractors[i]->markerLocations.ry << ' '
			<< blobExtractors[i]->markerLocations.gx  << ' '
			<< blobExtractors[i]->markerLocations.gy << ' '
			<< blobExtractors[i]->markerLocations.bx  << ' '
			<< blobExtractors[i]->markerLocations.by  << ' ' << endl;
		}

#endif
#if VISUAL
		mv.update();
#endif

		cameraArray.sync_camera_parameters();
	}
	cleanup(0);
}

