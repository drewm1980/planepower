#include <string>
#include <iostream>
#include <stdint.h>
#include <vector>
using namespace std;

// This is just an example of the ways in which the types
// in carousel_types.h can be used

#include "carousel_types.h"

int main(int argc, char **argv) 
{
	Vector x;
	Rotation e;
	StateVector X1,X2;
	e.e11 = 1.0;
	X2.e = e;
	X1.e = X2.e;
	X2 = X1;
	vector<StateVector> horizon(10);
	horizon[0] = X1;
	horizon[3] = X2;
	
}
