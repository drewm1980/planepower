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

#define VISUAL 1
#if VISUAL
#include "Tracer.hpp"
#endif

using namespace std;
using std::ifstream;

StereoPair *p_t;
void cleanup(int s)
{
	COUT << "Caught SIGINT.  Cleaning up cameras before exiting." << ENDL;
	p_t->stopHook();
	p_t->cleanUpHook();
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

	while(1)
	{
		stereoPair.updateHook();
#if VISUAL
		tracer.update();
#endif
	}
	cleanup(0);
}

