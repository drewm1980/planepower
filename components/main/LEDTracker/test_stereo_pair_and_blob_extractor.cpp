#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>

#include "StereoPair.hpp"
#include "cout.hpp"
#include "BlobExtractor.hpp"

#define VERBOSE 1
#define VISUAL 1
#if VISUAL
#include "Tracer.hpp"
#endif

using namespace std;
using std::ifstream;

StereoPair *p_sp;
void cleanup(int s)
{
	COUT << "Caught SIGINT.  Cleaning up cameras before exiting." << ENDL;
	p_sp->stopHook();
	p_sp->cleanUpHook();
	signal(SIGINT,SIG_DFL);
}

int main(int argc, char **argv) 
{
	StereoPair sp(false); // false for internal trigger
	p_sp = &sp;

	BlobExtractor be1(sp.frame_w,sp.frame_h);
	BlobExtractor be2(sp.frame_w,sp.frame_h);

	sp.startHook();
	signal(SIGINT,cleanup);

#if VISUAL
	Tracer tracer(&sp, &be1, &be2);	
#endif

	while(1)
	{
		sp.updateHook();
		be1.extract_blobs(sp.right_frame_data);
		be2.extract_blobs(sp.left_frame_data);
#if VERBOSE
		cout 
			<< "         RED     GREEN   BLUE"  << endl
			<< "         x   y   x   y   x   y"  << endl
			<< "Frame 1: " 
			<< be1.markerLocations.red.x << ' '
			<< be1.markerLocations.red.y << ' '
			<< be1.markerLocations.green.x  << ' '
			<< be1.markerLocations.green.y << ' '
			<< be1.markerLocations.blue.x  << ' '
			<< be1.markerLocations.blue.y  << ' ' << endl
			<< "Frame 2: " 
			<< be2.markerLocations.red.x << ' '
			<< be2.markerLocations.red.y << ' '
			<< be2.markerLocations.green.x  << ' '
			<< be2.markerLocations.green.y << ' '
			<< be2.markerLocations.blue.x  << ' '
			<< be2.markerLocations.blue.y  << ' ' << endl
			<< endl;


#endif
#if VISUAL
		tracer.update();
#endif
	}
	cleanup(0);
}

