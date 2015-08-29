/* Provides a fallback implementation of the sincos function, in case
   the current system doesn't provide one. */

#include <math.h>

double sincos(double x, double* sinx, double* cosx)
{
  *sinx = sin(x);
  *cosx = cos(x);
}
