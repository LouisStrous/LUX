#include "calendar.h"

/* Returns <*month> in the range 1..12. */
void JDtoCommonDate(double JD, int *year, int *month, double *day)
{
  double j = JD - (double) 1721119.5;
  int y3 = intfloor(j);
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
