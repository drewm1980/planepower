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
	CameraArray cameraArray(false); // False for internal trigger
	p_t = &cameraArray;
	cameraArray.startHook();
	signal(SIGINT,cleanup);

#if VISUAL
	MultiViewer mv(&cameraArray);	
#endif

	while(1)
	{
		cameraArray.updateHook();
#if VISUAL
		mv.update();
#endif
	}
	cleanup(0);
}

