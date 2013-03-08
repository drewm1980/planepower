#ifndef CAROUSEL_TYPES_H
#define CAROUSEL_TYPES_H

// All C and C++ code running on the carousel should
// use these types exclusively for indexing.
// We will keep this binary compatible with current state vector,
// until all components are upgraded.

#define carousel_float_t double

struct StateVector
{
  carousel_float_t x;      // 0
  carousel_float_t y;      // 1
  carousel_float_t z;      // 2
  carousel_float_t dx;     // 3
  carousel_float_t dy;     // 4
  carousel_float_t dz;     // 5
  carousel_float_t e11;    // 6
  carousel_float_t e12;    // 7
  carousel_float_t e13;    // 8
  carousel_float_t e21;    // 8
  carousel_float_t e22;    // 10
  carousel_float_t e23;    // 11
  carousel_float_t e31;    // 12
  carousel_float_t e32;    // 13
  carousel_float_t e33;    // 14
  carousel_float_t wx;     // 15
  carousel_float_t wy;     // 16
  carousel_float_t wz;     // 17
  carousel_float_t delta;  // 18
  carousel_float_t ddelta; // 19
  carousel_float_t ur;     // 20
  carousel_float_t up;     // 21
};

// Kurt, please help define these
struct ControlVector
{
  carousel_float_t dddelta;
  carousel_float_t dur; // 20
  carousel_float_t dup; // 21
};

//struct StateVectorWithActuators
//{
//StateVector state;
//Actuators actuators;
//};

#endif
