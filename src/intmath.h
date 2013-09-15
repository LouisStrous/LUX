/* This is file intmath.h.

Copyright 2013 Louis Strous

This file is part of LUX.

LUX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your
option) any later version.

LUX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LUX.  If not, see <http://www.gnu.org/licenses/>.
*/
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
