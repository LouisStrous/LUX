#ifndef REPLACEMENTS_HH_
#define REPLACEMENTS_HH_

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if !HAVE_SINCOS
void sincos(double x, double* sinx, double* cosx);
#endif

#endif
