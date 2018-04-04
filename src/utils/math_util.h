#ifndef MATH_UTILS_H_
#define MATH_UTILS_H_

#include <math.h>
#include "common_def.h"

#define EPSILON        0.0001f

static inline float
linear_interpolate_float(float x0, float y0, float x1, float y1, float x)
{
  float a = (y1 - y0) / (x1 - x0);
  float b = -a*x0 + y0;
  float y = a * x + b;

  return y;
}

static inline float
relative_diff(float a, float b)
{
  float   c = fabs(a);
  float   d = fabs(b);

  d = fmax(c, d);

  return d == 0.0f ? 0.0f : fabs(a - b) / d;
}

static inline uint8_t
almost_equal(float a, float b)
{
  if(a == b)
  {
    return TRUE;
  }
  return fabs(a - b) <= EPSILON;
}

#endif /* MATH_UTILS_H_ */

