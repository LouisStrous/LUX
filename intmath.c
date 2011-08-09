#include <stdlib.h>
#include "intmath.h"

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
   <denominator>, in such a way that the remainder is non-negative.
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
  int q, r, s, t;
  quotfloor(x, c, &q, &r);      /* x = q*c + r */
  quotfloor(a, c, &s, &t);      /* a = s*c + t */
  return divfloor(t*r + b, c) + s*q*c + t*q + s*r;
}
