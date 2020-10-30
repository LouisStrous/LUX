/* This is file intmath.hh.

Copyright 2013-2014 Louis Strous

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
#include "action.hh"

/// A struct with fields for the quotient and remainder of a division of
/// `int32_t` values.  The standard std::div_t-like types are defined in terms
/// of standard integer types of which the width is not necessarily the same on
/// all systems.
struct Div_t {
  int32_t quot;                 //!< The quotient.
  int32_t rem;                  //!< The remainder.
};

Div_t adiv(int32_t numerator, int32_t denominator);
int32_t iaquot(int32_t numerator, int32_t denominator);
int32_t iamod(int32_t numerator, int32_t denominator);
Div_t alinediv(int32_t numerator, int32_t factor, int32_t addend, int32_t denominator);
int32_t alinequot(int32_t numerator, int32_t factor, int32_t addend, int32_t denominator);
int32_t alinemod(int32_t numerator, int32_t factor, int32_t addend, int32_t denominator);

#endif
