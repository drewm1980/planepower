#ifndef CAROUSEL_TYPES_H
#define CAROUSEL_TYPES_H

// All C and C++ code running on the carousel should
// use these types exclusively for indexing.
// We will keep this binary compatible with current state vector,
// until all components are upgraded.

#define carousel_float_t double

struct DiffStateVec
{
  carousel_float_t x; /* 0 */
  carousel_float_t y; /* 1 */
  carousel_float_t z; /* 2 */
  carousel_float_t dx; /* 3 */
  carousel_float_t dy; /* 4 */
  carousel_float_t dz; /* 5 */
  carousel_float_t e11; /* 6 */
  carousel_float_t e12; /* 7 */
  carousel_float_t e13; /* 8 */
  carousel_float_t e21; /* 9 */
  carousel_float_t e22; /* 10 */
  carousel_float_t e23; /* 11 */
  carousel_float_t e31; /* 12 */
  carousel_float_t e32; /* 13 */
  carousel_float_t e33; /* 14 */
  carousel_float_t wx; /* 15 */
  carousel_float_t wy; /* 16 */
  carousel_float_t wz; /* 17 */
  carousel_float_t delta; /* 18 */
  carousel_float_t ddelta; /* 19 */
  carousel_float_t ur; /* 20 */
  carousel_float_t up; /* 21 */
};

struct AlgVarVec
{
};

struct ControlVec
{
  carousel_float_t dddelta; /* 0 */
  carousel_float_t dur; /* 1 */
  carousel_float_t dup; /* 2 */
};

struct ParamVec
{
};

struct MeasurementsXVec {
  carousel_float_t uvC1M1_0;
  carousel_float_t uvC1M1_1;
  carousel_float_t uvC1M2_0;
  carousel_float_t uvC1M2_1;
  carousel_float_t uvC1M3_0;
  carousel_float_t uvC1M3_1;
  carousel_float_t uvC2M1_0;
  carousel_float_t uvC2M1_1;
  carousel_float_t uvC2M2_0;
  carousel_float_t uvC2M2_1;
  carousel_float_t uvC2M3_0;
  carousel_float_t uvC2M3_1;
  carousel_float_t wIMU_0;
  carousel_float_t wIMU_1;
  carousel_float_t wIMU_2;
  carousel_float_t aIMU_0;
  carousel_float_t aIMU_1;
  carousel_float_t aIMU_2;
  carousel_float_t delta;
  carousel_float_t ur;
  carousel_float_t up;
};

struct MeasurementsUVec {
  carousel_float_t dddelta;
  carousel_float_t dur;
  carousel_float_t dup;
};

#endif
