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
	Rotation e;
	StateVector X1,X2;
	
	// You can address either by drilling down into the struct hierarchy:
	X1.position.x = 10;
	X1.velocity.x = 3;
	// Or, for the unioned members, directly:
	X1.x = 10;
	X1.dx = 3;

	// Exception: 
	X1.e.e11 = 1.0; // works, but
	//X1.e11 // doesn't work
	// We should work with e as a whole most of the time,
	// so that our will still work for e with different
	// rotation parameterization

	// You can copy POD structs directly!
	X2 = X1;
	
	// Nested POD structs can be set as well!
	X2.position = X1.position;
	X2.velocity = X1.velocity;
	X2.e = e;
	X1.e = X2.e;
	
	// How to wrap a C array with a POD struct
	double X3_arr[NX] = {0.0};
	X3_arr[0] = 3;
	StateVector *X3_poi; 
	X3_poi = (StateVector*) X3_arr;
	cout << X3_arr[0] << endl;
	cout << X3_poi->x << endl;

	// How to wrap an stl vector with a POD struct
	vector<double> X4_vec;
	X4_vec.resize(NX); X4_vec[0] = 4.0;
	StateVector *X4_poi; 
	X4_poi = (StateVector*) &X4_vec[0];
	cout << X4_vec[0] << endl;
	cout << X4_poi->x << endl;
	
	// You can still stick POD structs in STL vectors
	vector<StateVector> horizon(10);
	horizon[0] = X1;
	horizon[3] = X2;

	// Or in C arrays
	StateVector horizon_array[10];
	horizon[0] = X1;
	horizon[3] = X2;
	
}
