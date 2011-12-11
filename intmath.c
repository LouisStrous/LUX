#include <stdlib.h>
#include "intmath.h"

/* 
   quotfloor => adiv
   divfloor => iaquot
   remfloor => iamod
   divlinefloor => alinequot
*/

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

#if OBSOLETE
/* determines the quotient and remainder of dividing the <numerator>
   by the <denominator>.  The remainder is always non-negative (unlike
   when the div() function is used directly).  The quotient is returned
   in <*quotient> if <quotient> is non-null, and likewise the remainder
   in <*remainder> if <remainder> is non-null.  LS 2011-07-20 */
void quotfloor(int numerator, int denominator, int *quotient, int *remainder)
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
  if (quotient)
    *quotient = d.quot;
  if (remainder)
    *remainder = d.rem;
}

/* returns the quotient (integer division) of the <numerator> and the
   <denominator>, in such a way that the remainder is non-negative and
   less than the magnitude of the <denominator>.
   LS 2011-07-20 */
int divfloor(int numerator, int denominator)
{
  int quotient;

  quotfloor(numerator, denominator, &quotient, NULL);
  return quotient;
}

int remfloor(int numerator, int denominator)
{
  int remainder;

  quotfloor(numerator, denominator, NULL, &remainder);
  return remainder;
}

/* returns the quotient of dividing a*x + b by c, using signed integer
   arithmetic only, in such a way that integer overflow is avoided if
   a*x + b does not fit in an integer but the end result does.  LS
   2011-07-25 */
int divlinefloor(int x, int a, int b, int c)
{
  div_t d1, d2;

  d1 = adiv(x, c);
  d2 = adiv(a, c);
  return iaquot(d2.rem*d1.rem + b, c) + d2.quot*d1.quot*c + d2.rem*d1.quot
    + d2.quot*d1.rem;
}

  int q, r, s, t;
  quotfloor(x, c, &q, &r);      /* x = q*c + r */
  quotfloor(a, c, &s, &t);      /* a = s*c + t */
  return divfloor(t*r + b, c) + s*q*c + t*q + s*r;
#endif

