#include <stdlib.h>
#include "intmath.h"

#if INT_MAX >= INT32_MAX
#define div32 div
#define div32_t div_t
#else
#define div32 ldiv
#define div32_t ldiv_t
#endif

/* determines the quotient and remainder of dividing the <numerator>
   by the <denominator>, in such a way that the remainder is always
   non-negative and less than the magnitude of <denominator>.  This
   function is similar to div() but ensures that the remainder is
   nonnegative (unlike div(), which returns nonpositive remainders if
   the <numerator> or <denominator> are negative). */
Div_t adiv(Int numerator, Int denominator)
{
  div32_t d;

  d = div32(numerator, denominator);
  if (d.rem < 0) {
    if (denominator < 0)
      d.rem -= denominator;
    else
      d.rem += denominator;
    d.quot--;
  }
#if INT_MAX == INT32_MAX
  return d;
#else
  Div_t dd;
  dd.quot = d.quot;
  dd.rem = d.rem;
  return dd;
#endif
}

Int iaquot(Int numerator, Int denominator)
{
  Div_t d = adiv(numerator, denominator);
  return d.quot;
}

Int iamod(Int numerator, Int denominator)
{
  Div_t d = adiv(numerator, denominator);
  return d.rem;
}

/*
  y = ⌊(fx + t)/d⌋ (with all variables integer) has an intermediate
  result fx + t which can be much greater in magnitude than x, and so
  may be much too great to fit into a variable of the same type as x.

  We can assume that |t| < |d|; otherwise we can calculate y′ = ⌊(fx +
  t′)/d⌋ with y = y′ + ⌊t/d⌋ and t′ = t mod d.

  We can assume that |f| < |d|; otherwise we can calculate y″ = ⌊(f″x
  + t)/d⌋ with y = y″ + ⌊f/d⌋x and f″ = f mod d.

  A type-size-independent workaround (for |t|, |f| < |d|) is y =
  f⌊x/d⌋ + ⌊(f*(x mod d) + t)/d⌋, which has its intermediate result no
  greater than fd.
 */

Div_t alinediv(Int numerator, Int factor, Int addend, Int denominator)
{
  Div_t d1, d2, d;

  d1 = adiv(numerator, denominator);
  d2 = adiv(factor*d1.rem + addend, denominator);
  d.quot = factor*d1.quot + d2.quot;
  d.rem = d2.rem;
  return d;
}

Int alinequot(Int numerator, Int factor, Int addend, Int denominator)
{
  Div_t d = alinediv(numerator, factor, addend, denominator);
  return d.quot;
}

Int alinemod(Int numerator, Int factor, Int addend, Int denominator)
{
  Div_t d = alinediv(numerator, factor, addend, denominator);
  return d.rem;
}
