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

#define VISUAL 0
#if VISUAL
#include "Tracer.hpp"
#endif

using namespace std;
using std::ifstream;

StereoPair *p_t;
uint8_t* imagebuffer;
void cleanup(int s)
{
	COUT << "Caught SIGINT.  Cleaning up cameras before exiting." << ENDL;
	p_t->stopHook();
	p_t->cleanUpHook();
	free(imagebuffer);
	signal(SIGINT,SIG_DFL);
}

int main(int argc, char **argv) 
{
	StereoPair stereoPair(false); // False for internal trigger
	p_t = &stereoPair;
	stereoPair.startHook();
	signal(SIGINT,cleanup);

#if VISUAL
	Tracer tracer(&stereoPair);	
#endif

	int frameCount = 10;
	imagebuffer = (uint8_t *) calloc(1600*1200*2*frameCount,sizeof(uint8_t));

	for(int frameIndex=0; frameIndex < )
	{
		stereoPair.updateHook();
		if()
#if VISUAL
		tracer.update();
#endif
	}
	cleanup(0);
}

