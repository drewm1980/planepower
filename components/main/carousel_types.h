#ifndef CAROUSEL_TYPES_H
#define CAROUSEL_TYPES_H

// All C and C++ code running on the carousel should
// use these types exclusively for indexing.
// We will keep this binary compatible with current state vector,
// until all components are upgraded.

#define carousel_float_t double

#define NX (sizeof(StateVector)/sizeof(carousel_float_t))

struct Vector
{
	carousel_float_t x;
	carousel_float_t y;
	carousel_float_t z;
};

struct Rotation
{
	carousel_float_t e11;
	carousel_float_t e12;
	carousel_float_t e13;
	carousel_float_t e21;
	carousel_float_t e22;
	carousel_float_t e23;
	carousel_float_t e31;
	carousel_float_t e32;
	carousel_float_t e33;
};

struct StateVector
{
	union {
		struct {
			carousel_float_t x;	// 0
			carousel_float_t y;	// 1
			carousel_float_t z;	// 2
		};
		Vector position;
	};
	union {
		struct {
			carousel_float_t dx; // 3
			carousel_float_t dy; // 4
			carousel_float_t dz; // 5
		};
		Vector velocity;
	};
	Rotation e; // 6,7,8, 9,10,11, 12,13,14
	Vector w; // 15,16,17
	carousel_float_t delta; // 18
	carousel_float_t ddelta; // 19
};

// Kurt, please help define these
//struct Actuators
//{
	//carousel_float_t ur; // 20
	//carousel_float_t up; // 21
//}

//struct StateVectorWithActuators
//{
	//StateVector state;
	//Actuators actuators;
//};

#endif
