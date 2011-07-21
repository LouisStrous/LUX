#include "calendar.h"
#include <stdlib.h>
#include <math.h>

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

int divfloor(int numerator, int denominator)
{
  int quotient;

  quotfloor(numerator, denominator, &quotient, NULL);
  return quotient;
}

/* Returns <*month> in the range 1..12. */
void JDtoCommonDate(double JD, int *year, int *month, double *day)
{
  if (JD >= (double) 2299160.5) {        /* Gregorian calendar */
    double j = JD - (double) 1721119.5;
    int y3 = floor(j);
    double d = j - y3;
    int x3 = divfloor(4*y3 + 3, 146097);
    int y2 = y3 - divfloor(146097*x3, 4);
    int x2 = divfloor(100*y2 + 99, 36525);
    int y1 = y2 - divfloor(36525*x2, 100);
    *year = 100*x3 + x2;
    *month = divfloor(5*y1 + 461, 153);
    *day = y1 - divfloor(153**month - 457, 5) + 1 + d;
    if (*month > 12) {
      *month -= 12;
      *year += 1;
    }
  } else {
    double jj = JD - (double) 1721117.5;
    int y2 = floor(jj);
    double d = jj - y2;
    int j = divfloor(4*y2 + 3, 1461);
    int y1 = y2 - divfloor(1461*j, 4);
    int m = divfloor(5*y1 + 461, 153);
    int dd = y1 - divfloor(153*m - 462, 5);
    if (m > 12) {
      m -= 12;
      j++;
    }
    *year = j;
    *month = m;
    *day = dd + d;
  }
}

/* <year>, <month>, <day> can be any numbers.  If <month> is outside
   of the range 1..12 then years of 12 months are added or subtracted
   as needed to bring it into that range.  <day> is added to the JD
   for the day preceding the first of the month denoted by <year> and
   <month>. */
double CommonDateToJD(int year, int month, double day)
{
  int j = divfloor(month - 1, 12);
  year += j;
  month -= 12*j;
  if (month < 3) {
    month += 12;
    year--;
  }
  int c = divfloor(year, 100);
  int x = year - 100*c;
  int J1 = divfloor(146097*c, 4);
  int J2 = divfloor(36525*x, 100);
  int J3 = divfloor(153*month - 457, 5);
  return 1721119.5 + J1 + J2 + J3 + day - 1;
}
