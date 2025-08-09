/* This is file intmath.cc.

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
#include <stdlib.h>
#include "intmath.hh"

// adiv → fdivide
// Div_t → Div<int32_t>
// iaquot → fquotient
// iamod → fremainder

/// y = ⌊(f x + t)/d⌋ (with all variables integer) has an intermediate
/// result f x + t which can be much greater in magnitude than x, and
/// so may be much too great to fit into a variable of the same type
/// as x.
///
/// We can assume that |t| < |d|; otherwise we can calculate y′ = ⌊(f
/// x + t′)/d⌋ with y = y′ + ⌊t/d⌋ and t′ = t mod d.
///
/// We can assume that |f| < |d|; otherwise we can calculate y″ = ⌊(f″
/// x + t)/d⌋ with y = y″ + ⌊f/d⌋x and f″ = f mod d.
///
/// A type-size-independent workaround (for |t|, |f| < |d|) is y =
/// f⌊x/d⌋ + ⌊(f*(x mod d) + t)/d⌋, which has its intermediate result
/// no greater than fd.
Div<int32_t>
flinediv(int32_t numerator, int32_t factor, int32_t addend, int32_t denominator)
{
  Div<int32_t> d1, d2, d;

  d1 = fdivide(numerator, denominator);
  d2 = fdivide(factor*d1.rem + addend, denominator);
  d.quot = factor*d1.quot + d2.quot;
  d.rem = d2.rem;
  return d;
}

int32_t
flinequot(int32_t numerator, int32_t factor, int32_t addend,
          int32_t denominator)
{
  auto d = flinediv(numerator, factor, addend, denominator);
  return d.quot;
}

int32_t
flinemod(int32_t numerator, int32_t factor, int32_t addend, int32_t denominator)
{
  auto d = flinediv(numerator, factor, addend, denominator);
  return d.rem;
}
