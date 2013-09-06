#ifndef INTMATH_H
#define INTMATH_H

#include <stdlib.h>
#include "action.h"

typedef struct {
  int32_t quot;
  int32_t rem;
} Div_t;

Div_t adiv(Int numerator, Int denominator);
Int iaquot(Int numerator, Int denominator);
Int iamod(Int numerator, Int denominator);
Div_t alinediv(Int numerator, Int factor, Int addend, Int denominator);
Int alinequot(Int numerator, Int factor, Int addend, Int denominator);
Int alinemod(Int numerator, Int factor, Int addend, Int denominator);

#endif
