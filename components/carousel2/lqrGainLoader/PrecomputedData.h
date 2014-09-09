#pragma once

// This makes explicit the binary format I'm using to pass
// data from octave into orocos

#include "State.h"
#include "ControllerParameterization.h"

// Our feedback matrix is just a single state-sized vector for now.
typedef struct State FeedbackMatrix;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
struct PrecomputedData
{
	// Remember to update compute_gains.m if you change this order!
	ControllerParameterization cp; // This is just stored for redundancy.
	State xss;
	State xss0;
	State xss1;
	FeedbackMatrix G; 
};
#pragma pack(pop)  /* pop current alignment from stack */

