#include <stdlib.h>
#include "intmath.h"

/* determines the quotient and remainder of dividing the <numerator>
   by the <denominator>, in such a way that the remainder is always
   non-negative and less than the magnitude of <denominator>.  This
   function is similar to div() but ensures that the remainder is
   nonnegative (unlike div(), which returns nonpositive remainders if
   the <numerator> or <denominator> are negative). */
div_t adiv(int numerator, int denominator)
{
  div_t d;

  d = div(numerator, denominator);
  if (d.rem < 0) {
    if (denominator < 0)
      d.rem -= denominator;
    else
      d.rem += denominator;
    d.quot--;
  }
  return d;
}

int iaquot(int numerator, int denominator)
{
  div_t d = adiv(numerator, denominator);
  return d.quot;
}

int iamod(int numerator, int denominator)
{
  div_t d = adiv(numerator, denominator);
  return d.rem;
}

/*
  gy + t = qf + r (0 ≤ r < |q|)
  y = q₁f + r₁
   gq₁f + gr₁ + t = qf + r
  gr₁ + t = q₂f + r₂
   gq₁f + q₂f + r₂ = qf + r
  q = gq₁ + q₂
  r = r₂
 */

div_t alinediv(int numerator, int factor, int addend, int denominator)
{
  div_t d1, d2, d;

  d1 = adiv(numerator, denominator);
  d2 = adiv(factor*d1.rem + addend, denominator);
  d.quot = factor*d1.quot + d2.quot;
  d.rem = d2.rem;
  return d;
}

int alinequot(int numerator, int factor, int addend, int denominator)
{
  div_t d = alinediv(numerator, factor, addend, denominator);
  return d.quot;
}

int alinemod(int numerator, int factor, int addend, int denominator)
{
  div_t d = alinediv(numerator, factor, addend, denominator);
  return d.rem;
}
