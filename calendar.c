#define _GNU_SOURCE             /* for %as format in sscanf */
/* HEADERS */
#include <malloc.h> /* for free malloc */
#include <math.h> /* for floor */
#include <stdio.h> /* for asprintf sscanf */
#include <string.h> /* for strcmp */
#include <time.h> /* for localtime time gmtime */
/* END HEADERS */
#include "calendar.h"
#include "intmath.h"

#define arraysize(x) sizeof((x))/sizeof(*(x))

/** Translates a Chronological Julian Day to a calendar date.

    \param CJD the Chronological Julian Day
    \param year (output) pointer to the corresponding year.  Must not be NULL!
    \param month (output) pointer to the corresponding month.  Must not be NULL!
    \param day (output) pointer to the corresponding day.  Must not be NULL!
    \param f a pointer to a function that calculates the calendar date
    from a Chronological Julian Day Number
 */
static void CJDtoCal(double CJD, int *year, int *month, double *day,
                     void (*f)(int, int *, int *, int *))
{
  /* separate the CJD into a CJDN and a day fraction */
  int CJDN = floor(CJD);
  double dayfraction = CJD - CJDN;

  int iday;
  f(CJDN, year, month, &iday);  /* calculate the integer calendar date */
  *day = iday + dayfraction;
}
/*--------------------------------------------------------------------------*/
/** Translates a calendar date into a Chronological Julian Day Number.

    \param year the year
    \param month the month
    \param day the day
    \param f a pointer to a function that calculates the Chronological
    Julian Day Number from a calendar date
    \return the Chronological Julian Date
 */
static double CaltoCJD(int year, int month, double day,
                       int (*f)(int, int, int))
{
  int d = floor(day);
  double part = day - d;
  int jd = f(year, month, d);
  return (double) jd + part;
}
/*--------------------------------------------------------------------------*/

/* THE GREGORIAN CALENDAR */
/*--------------------------------------------------------------------------*/
/** The Chronological Julian Day Number of the epoch of the Gregorian calendar */
#define GREGORIAN_EPOCH (1721120)
void CJDNtoGregorian(int CJDN, int *year, int *month, int *day)
{
  int y3 = CJDN - GREGORIAN_EPOCH;
  int x3 = divlinefloor(y3, 4, 3, 146097);
  int y2 = divlinefloor(x3, 146097, 0, 4);
  y2 = y3 - y2;
  int x2 = divfloor(y2*100 + 99, 36525);
  int y1 = y2 - divfloor(36525*x2, 100);
  *year = 100*x3 + x2;
  *month = divfloor(5*y1 + 461, 153);
  *day = y1 - divfloor(153**month - 457, 5) + 1; /* TODO: integrate +1 into -457 */
  if (*month > 12) {
    *month -= 12;
    *year += 1;
  }
}
/*--------------------------------------------------------------------------*/
void CJDNtoGregorianA(int const *CJDN, int *date)
{
  CJDNtoGregorian(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
static char const *Gregorian_Julian_monthnames[] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November",
  "December"
};
void CJDNto3SA(int const *CJDN, char **date,
               void (*CJDNtoCal3A) (int const *, int *),
               char const **monthnames)
{
  int datec[3];

  CJDNtoCal3A(CJDN, datec);
  if (asprintf(date, "%d %s %d", datec[2], monthnames[datec[1] - 1],
               datec[0]) < 0)
    *date = NULL;  
}
/*--------------------------------------------------------------------------*/
void CJDNtoGregorianSA(int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoGregorianA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoGregorian(double CJD, int *year, int *month, double *day)
{
  CJDtoCal(CJD, year, month, day, &CJDNtoGregorian);
}
/*--------------------------------------------------------------------------*/
void CJDtoGregorianA(double const *CJD, double *date)
{
  int year, month;
  CJDtoGregorian(*CJD, &year, &month, &date[2]);
  date[0] = (double) year;
  date[1] = (double) month;
}
/*--------------------------------------------------------------------------*/
void CJDto3SA(double const *CJD, char **date,
               void (*CJDtoCal3A) (double const *, double *),
               char const **monthnames)
{
  double datec[3];

  CJDtoCal3A(CJD, datec);
  if (asprintf(date, "%g %s %d", datec[2], monthnames[(int) datec[1] - 1],
               (int) datec[0]) < 0)
    *date = NULL;  
}
/*--------------------------------------------------------------------------*/
void CJDtoGregorianSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoGregorianA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
int GregoriantoCJDN(int year, int month, int day)
{
  if (month < 3) {
    month += 12;
    year--;
  }
  int c, x;
  quotfloor(year, 100, &c, &x);
  int J1 = divlinefloor(c, 146097, 0, 4);
  int J2 = divfloor(x*36525, 100);
  int J3 = divfloor(month*153 - 457, 5);
  return GREGORIAN_EPOCH - 1 + J1 + J2 + J3 + day;
}
/*--------------------------------------------------------------------------*/
void GregoriantoCJDNA(int const *date, int *CJDN)
{
  *CJDN = GregoriantoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
double GregoriantoCJD(int year, int month, double day)
{
  return CaltoCJD(year, month, day, GregoriantoCJDN);
}
/*--------------------------------------------------------------------------*/
void GregoriantoCJDA(double const *date, double *CJD)
{
  *CJD = GregoriantoCJD((int) date[0], (int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
void SA3toCJD(char * const *date, double *CJD,
              void (*Cal3AtoCJD) (double const *, double *),
              int nmonthnames, char const **monthnames)
{
  double datec[3];
  int i, n;
  char *monthname;

  n = sscanf(*date, "%lg %as %lg", &datec[2], &monthname, &datec[0]);
  if (n == 3) {
    for (i = 0; i < nmonthnames; i++) {
      if (!strcmp(monthname, monthnames[i]))
        break;
    }
    free(monthname);
    if (i == nmonthnames)         /* no match found */
      *CJD = 0;
    else {
      datec[1] = i + 1;
      Cal3AtoCJD(datec, CJD);
    }
  } else
    *CJD = 0;
}
/*--------------------------------------------------------------------------*/
void GregorianStoCJDA(char * const *date, double *CJD)
{
  SA3toCJD(date, CJD, GregoriantoCJDA, 
           arraysize(Gregorian_Julian_monthnames),
           Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/

/* THE JULIAN CALENDAR */
/*--------------------------------------------------------------------------*/
/** The Chronological Julian Day Number of the epoch of the Julian calendar */
#define JULIAN_EPOCH (1721118)
void CJDNtoJulian(int CJDN, int *year, int *month, int *day)
{
  int y2 = CJDN - JULIAN_EPOCH;
  int x2 = divlinefloor(y2, 4, 3, 1461);
  int y1 = divlinefloor(x2, 1461, 0, 4);
  y1 = y2 - y1;
  *year = x2;
  *month = divfloor(5*y1 + 461, 153);
  *day = y1 - divfloor(153**month - 457, 5) + 1;
  if (*month > 12) {
    *month -= 12;
    *year += 1;
  }
}
/*--------------------------------------------------------------------------*/
void CJDNtoJulianA(int const *CJDN, int *date)
{
  CJDNtoJulian(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
void CJDNtoJulianSA(int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoJulianA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoJulian(double CJD, int *year, int *month, double *day)
{
  CJDtoCal(CJD, year, month, day, CJDNtoJulian);
}
/*--------------------------------------------------------------------------*/
void CJDtoJulianA(double const *CJD, double *date)
{
  int year, month;
  CJDtoJulian(*CJD, &year, &month, &date[2]);
  date[0] = (double) year;
  date[1] = (double) month;
}
/*--------------------------------------------------------------------------*/
void CJDtoJulianSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoJulianA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
int JuliantoCJDN(int year, int month, int day)
{
  if (month < 3) {
    month += 12;
    year--;
  }
  int J1 = divlinefloor(year, 1461, 0, 4);
  int J2 = divfloor(month*153 - 457, 5);
  return J1 + J2 + day + JULIAN_EPOCH - 1;
}
/*--------------------------------------------------------------------------*/
void JuliantoCJDNA(int const *date, int *CJDN)
{
  *CJDN = JuliantoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
double JuliantoCJD(int year, int month, double day)
{
  return CaltoCJD(year, month, day, JuliantoCJDN);
}
/*--------------------------------------------------------------------------*/
void JuliantoCJDA(double const *date, double *CJD)
{
  *CJD = JuliantoCJD((int) date[0], (int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
void JulianStoCJDA(char * const *date, double *CJD)
{
  SA3toCJD(date, CJD, JuliantoCJDA,
           arraysize(Gregorian_Julian_monthnames),
           Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDNtoCommon(int CJDN, int *year, int *month, int *day)
{
  if (CJDN > 2299160)
    return CJDNtoGregorian(CJDN, year, month, day);
  else
    return CJDNtoJulian(CJDN, year, month, day);
}
/*--------------------------------------------------------------------------*/
void CJDNtoCommonA(int const *CJDN, int *date)
{
  CJDNtoCommon(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
void CJDNtoCommonSA(int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoCommonA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoCommon(double CJD, int *year, int *month, double *day)
{
  CJDtoCal(CJD, year, month, day, CJDNtoCommon);
}
/*--------------------------------------------------------------------------*/
void CJDtoCommonA(double const *CJD, double *date)
{
  int year, month;
  CJDtoCommon(*CJD, &year, &month, &date[2]);
  date[0] = (double) year;
  date[1] = (double) month;
}
/*--------------------------------------------------------------------------*/
void CJDtoCommonSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoCommonA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
int CommontoCJDN(int year, int month, int day)
{
  if (year > 1582
      || (year == 1582
          && (month > 10
              || (month == 10 && day > 4))))
    return GregoriantoCJDN(year, month, day);
  else
    return JuliantoCJDN(year, month, day);
}
/*--------------------------------------------------------------------------*/
void CommontoCJDNA(int const *date, int *CJDN)
{
  *CJDN = CommontoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
double CommontoCJD(int year, int month, double day)
{
  return CaltoCJD(year, month, day, CommontoCJDN);
}
/*--------------------------------------------------------------------------*/
void CommontoCJDA(double const *date, double *CJD)
{
  *CJD = CommontoCJD((int) date[0], (int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
void CommonStoCJDA(char * const *date, double *CJD)
{
  SA3toCJD(date, CJD, CommontoCJDA,
           arraysize(Gregorian_Julian_monthnames),
           Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
static int	hebrewMonthStarts[][13] = {
  /* deficient ordinary year: */
  /*   30, 29, 29,  29,  30,  29,  30,  29,  30,  29,  30,  29 */
  { 0, 30, 59, 88, 117, 147, 176, 206, 235, 265, 294, 324, 353 },
  /* regular ordinary year: */
  /*   30, 29, 30,  29,  30,  29,  30,  29,  30,  29,  30,  29 */
  { 0, 30, 59, 89, 118, 148, 177, 207, 236, 266, 295, 325, 354 },
  /* complete ordinary year: */
  /*   30, 30, 30,  29,  30,  29,  30,  29,  30,  29,  30,  29 */
  { 0, 30, 60, 90, 119, 149, 178, 208, 237, 267, 296, 326, 355 },
  /* deficient leap year: */
  /*   30, 29, 29,  29,  30,  59,  30,  29,  30,  29,  30,  29 */
  { 0, 30, 59, 88, 117, 147, 206, 236, 265, 295, 324, 354, 383 },
  /* regular leap year: */
  /*   30, 29, 30,  29,  30,  59,  30,  29,  30,  29,  30,  29 */
  { 0, 30, 59, 89, 118, 148, 207, 237, 266, 296, 325, 355, 384 },
  /* complete leap year: */
  /*   30, 30, 30,  29,  30,  59,  30,  29,  30,  29,  30,  29 */
  { 0, 30, 60, 90, 119, 149, 208, 238, 267, 297, 326, 356, 385 }
};
/** The Chronological Julian Day Number of the epoch of the Hebrew calendar */
#define HEBREW_EPOCH	(347997)
/*--------------------------------------------------------------------------*/
void CJDNtoHebrew(int CJDN, int *year, int *month, int *day)
{
  int	d, cjdn, cjdn0;
  int	approx_year, loy, yearType, m;

  d = CJDN - HEBREW_EPOCH;
  approx_year = floor(d*0.002737874608);
  cjdn = HebrewtoCJDN(approx_year, 1, 1);
  if (cjdn < CJDN)
    do {
      cjdn0 = cjdn;
      cjdn = HebrewtoCJDN(++approx_year, 1, 1);
    } while (cjdn < CJDN);
  else {
    cjdn0 = cjdn;
    do { 
      cjdn = cjdn0;
      cjdn0 = HebrewtoCJDN(--approx_year, 1, 1);
    } while (cjdn0 > CJDN);
  }
  loy = cjdn - cjdn0;		/* length of target year */
  yearType = (loy > 365)*3 + (loy % 30) - 23;
  d = CJDN - cjdn0;			/* day number in the year */
  m = d/35;
  while (m < 11 && hebrewMonthStarts[yearType][m + 1] < d)
    m++;
  *year = approx_year - 1;
  *month = m + 1;
  *day = d - hebrewMonthStarts[yearType][m] + 1;
}
/*--------------------------------------------------------------------------*/
void CJDNtoHebrewA(int const *CJDN, int *date)
{
  CJDNtoHebrew(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
static char const *Hebrew_monthnames[] = {
  "Tishri", "Heshvan", "Kislev", "Tevet", "Shevat", "Adar", "Nisan",
  "Iyar", "Sivan", "Tammuz", "Av", "Elul"
};
void CJDNtoHebrewSA(int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoHebrewA, Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoHebrew(double CJD, int *year, int *month, double *day)
{
  CJDtoCal(CJD, year, month, day, CJDNtoHebrew);
}
/*--------------------------------------------------------------------------*/
void CJDtoHebrewA(double const *CJD, double *date)
{
  int year, month;
  CJDtoHebrew(*CJD, &year, &month, &date[2]);
  date[0] = (double) year;
  date[1] = (double) month;
}
/*--------------------------------------------------------------------------*/
void CJDtoHebrewSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoHebrewA, Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
int HebrewtoCJDN(int year, int month, int day)
{
  int	n1, loy, yearType;
  
  n1 = tishri(year);
  loy = tishri(year + 1) - n1;

  yearType = (loy > 365)*3 + (loy % 30) - 23;

  return HEBREW_EPOCH + n1 + hebrewMonthStarts[yearType][month - 1] + day - 1;
}
/*--------------------------------------------------------------------------*/
void HebrewtoCJDNA(int const *date, int *CJDN)
{
  *CJDN = HebrewtoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
double HebrewtoCJD(int year, int month, double day)
{
  return CaltoCJD(year, month, day, HebrewtoCJDN);
}
/*--------------------------------------------------------------------------*/
void HebrewtoCJDA(double const *date, double *CJD)
{
  *CJD = HebrewtoCJD((int) date[0], (int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
void HebrewStoCJDA(char * const *date, double *CJD)
{
  SA3toCJD(date, CJD, HebrewtoCJDA,
           arraysize(Hebrew_monthnames),
           Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
/** The Chronological Julian Day Number of the epoch of the Islamic calendar */
#define ISLAMIC_EPOCH (1948440)
void CJDNtoIslamic(int CJDN, int *year, int *month, int *day)
{
  /* "Alfonsijnse tafels", s = 14, J₀ = 1948440 */
  int y2 = CJDN - ISLAMIC_EPOCH;
  *year = divlinefloor(y2, 30, 10646, 10631);
  int z2 = y2 - divlinefloor(*year, 10631, -10617, 30);
  *month = divfloor(11*z2 + 330, 325);
  *day = z2 - divfloor(325**month - 331, 11);
}
/*--------------------------------------------------------------------------*/
void CJDNtoIslamicA(int const *CJDN, int *date)
{
  CJDNtoIslamic(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
static char const *Islamic_monthnames[] = {
  "Muharram", "Safar", "Rabi`a I", "Rabi`a II", "Jumada I", "Jumada II",
  "Rajab", "Sha`ban", "Ramadan", "Shawwal", "Dhu al-Q`adah",
  "Dhu al-Hijjah"
};
void CJDNtoIslamicSA(int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoIslamicA, Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoIslamic(double CJD, int *year, int *month, double *day)
{
  CJDtoCal(CJD, year, month, day, CJDNtoIslamic);
}
/*--------------------------------------------------------------------------*/
void CJDtoIslamicA(double const *CJD, double *date)
{
  int year, month;
  CJDtoIslamic(*CJD, &year, &month, &date[2]);
  date[0] = (double) year;
  date[1] = (double) month;
}
/*--------------------------------------------------------------------------*/
void CJDtoIslamicSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoIslamicA, Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
int IslamictoCJDN(int year, int month, int day)
{
  return divlinefloor(year, 10631, -10617, 30)
    + divfloor(325*month - 320, 11) + day + ISLAMIC_EPOCH - 1;
}
/*--------------------------------------------------------------------------*/
void IslamictoCJDNA(int const *date, int *CJDN)
{
  *CJDN = IslamictoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
double IslamictoCJD(int year, int month, double day)
{
  return CaltoCJD(year, month, day, IslamictoCJDN);
}
/*--------------------------------------------------------------------------*/
void IslamictoCJDA(double const *date, double *CJD)
{
  *CJD = IslamictoCJD((int) date[0], (int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
void IslamicStoCJDA(char * const *date, double *CJD)
{
  SA3toCJD(date, CJD, IslamictoCJDA,
           arraysize(Islamic_monthnames),
           Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
/** The Chronological Julian Day Number of the epoch of the Egyptian calendar */
#define EGYPTIAN_EPOCH (1448273)
void CJDNtoEgyptian(int CJDN, int *year, int *month, int *day)
{
  /* era of Nabonassar */
  int y2 = CJDN - EGYPTIAN_EPOCH;
  int y1;
  quotfloor(y2, 365, year, &y1);
  quotfloor(y1, 30, month, day);
  *month += 1;
  *day += 1;
}
/*--------------------------------------------------------------------------*/
void CJDNtoEgyptianA(int const *CJDN, int *date)
{
  CJDNtoEgyptian(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
static char const *Egyptian_monthnames[] = {
  "Thoth", "Phaophi", "Athyr", "Choiak", "Tybi", "Mecheir", "Phamenoth",
  "Pharmuthi", "Pachon", "Payni", "Epiphi", "Mesore", "epagomenai"
};
void CJDNtoEgyptianSA(int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoEgyptianA, Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoEgyptian(double CJD, int *year, int *month, double *day)
{
  CJDtoCal(CJD, year, month, day, CJDNtoEgyptian);
}
/*--------------------------------------------------------------------------*/
void CJDtoEgyptianA(double const *CJD, double *date)
{
  int year, month;
  CJDtoEgyptian(*CJD, &year, &month, &date[2]);
  date[0] = (double) year;
  date[1] = (double) month;
}
/*--------------------------------------------------------------------------*/
void CJDtoEgyptianSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoEgyptianA, Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
int EgyptiantoCJDN(int year, int month, int day)
{
  return 365*year + 30*month + day + EGYPTIAN_EPOCH - 365 - 30 - 1;
}
/*--------------------------------------------------------------------------*/
void EgyptiantoCJDNA(int const *date, int *CJDN)
{
  *CJDN = EgyptiantoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
double EgyptiantoCJD(int year, int month, double day)
{
  return CaltoCJD(year, month, day, EgyptiantoCJDN);
}
/*--------------------------------------------------------------------------*/
void EgyptiantoCJDA(double const *date, double *CJD)
{
  *CJD = EgyptiantoCJD((int) date[0], (int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
void EgyptianStoCJDA(char * const *date, double *CJD)
{
  SA3toCJD(date, CJD, EgyptiantoCJDA,
           arraysize(Egyptian_monthnames),
           Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
double JDtoCJD(double JD)
{
  time_t t_JD, t_CJD;
  struct tm *bt;

  t_JD = (time_t) ((JD - (double) 2440587.5)*86400);
  bt = gmtime(&t_JD);
  t_CJD = t_JD + bt->tm_gmtoff;
  return t_CJD/(double) 86400.0 + 2440588;
}
/*--------------------------------------------------------------------------*/
void JDtoCJDA(double const *JD, double *CJD)
{
  *CJD = JDtoCJD(*JD);
}
/*--------------------------------------------------------------------------*/
double CJDtoJD(double CJD)
{
  time_t t_JD, t_CJD;
  struct tm *bt;

  t_CJD = (time_t) ((CJD - 2440588)*86400);
  bt = localtime(&t_CJD);
  t_JD = t_CJD - bt->tm_gmtoff;
  return t_JD/(double) 86400.0 + 2440587.5;
}
/*--------------------------------------------------------------------------*/
void CJDtoJDA(double const *CJD, double *JD)
{
  *JD = CJDtoJD(*CJD);
}
/*--------------------------------------------------------------------------*/
void CJDNtoCJDNA(int const *in, int *out)
{
  *out = *in;
}
/*--------------------------------------------------------------------------*/
void CJDtoCJDA(double const *in, double *out)
{
  *out = *in;
}
/*--------------------------------------------------------------------------*/
double CJDtoLunar(double CJD)
{
  double	k, CJD2;

  k = floor((CJDtoJD(CJD) - 2451550.09765)/29.530588853) + 83017;
  CJD2 = LunartoCJD(k);
  k += (CJD - CJD2)/29.530588853;
  return k;
}
/*--------------------------------------------------------------------------*/
void CJDtoLunarA(double const *CJD, double *lunar)
{
  *lunar = CJDtoLunar(*CJD);
}
/*--------------------------------------------------------------------------*/
double LunartoCJD(double lunar)
{
  double	T, k;

  k = lunar - 83017;
  T = k/1236.85;
  return JDtoCJD(2451550.09765) + 29.530588853*k
    + T*T*(1.337e-4 + T*(-1.5e-7 + 7.3e-10*T));
}
/*--------------------------------------------------------------------------*/
void LunartoCJDA(double const *lunar, double *CJD)
{
  *CJD = LunartoCJD(*lunar);
}
/*--------------------------------------------------------------------------*/
void CJDNtoMayan(int CJDN, int *trecena, int *venteina,
                 int *haab_day, int *haab_month,
                 int *year_trecena, int *year_venteina)
{
  *trecena = remfloor(CJDN + 5, 13) + 1;
  *venteina = remfloor(CJDN + 16, 20) + 1;
  int d = remfloor(CJDN + 65, 365);
  *haab_day = remfloor(d, 20);
  *haab_month = d/20 + 1;
  *year_trecena = remfloor(CJDN + 5 - d, 13) + 1;
  *year_venteina = remfloor(CJDN + 16 - d, 20) + 1;
}
/*--------------------------------------------------------------------------*/
void CJDNtoMayanA(int const *CJDN, int *date)
{
  CJDNtoMayan(*CJDN, &date[0], &date[1], &date[2], &date[3], &date[4],
              &date[5]);
}
/*--------------------------------------------------------------------------*/
/* TODO: MayantoCJDN */
/*--------------------------------------------------------------------------*/
static char const *Mayan_venteina_names[] = {
  "Imix'", "Ik'", "Ak'b'al", "K'an", "Chikchan", "Kimi", "Manik'", "Lamat",
  "Muluk", "Ok", "Chuwen", "Eb'", "B'en", "Ix", "Men", "Kib'", "Kab'an",
  "Etz'nab'", "Kawak", "Ajaw"
};

static char const *Mayan_haab_names[] = {
  "Pop", "Wo", "Sip", "Sotz'", "Sek", "Xul", "Yaxk'in", "Mol", "Ch'en",
  "Yax", "Sac", "Keh", "Mak", "K'ank'in", "Muwan", "Pax", "K'ayab'", "Kumk'u",
  "Wayeb'"
};

void CJDNtoMayanSA(int const *CJDN, char **date)
{
  int trecena, venteina, haab_day, haab_month, year_trecena, year_venteina;

  CJDNtoMayan(*CJDN, &trecena, &venteina, &haab_day, &haab_month,
              &year_trecena, &year_venteina);
  if (asprintf(date, "%d %s %d %s (%d %s)", trecena,
               Mayan_venteina_names[venteina - 1], haab_day,
               Mayan_haab_names[haab_month - 1], year_trecena,
               Mayan_venteina_names[year_venteina - 1]) < 0)
    *date = NULL;
}
/*--------------------------------------------------------------------------*/
#define LONGCOUNT_EPOCH (584283)
static int longcount_periods[] = { 20, 20, 18, 20 };
void CJDNtoLongCount(int CJDN, int *longcount)
{
  int d, i;

  d = CJDN - LONGCOUNT_EPOCH;
  for (i = sizeof(longcount_periods)/sizeof(*longcount_periods) - 1; i >= 0; i--)
    quotfloor(d, longcount_periods[i], &d, &longcount[i + 1]);
  longcount[0] = d;
}
/*--------------------------------------------------------------------------*/
void CJDNtoLongCountA(int const *CJDN, int *longcount)
{
  CJDNtoLongCount(*CJDN, longcount);
}
/*--------------------------------------------------------------------------*/
void CJDNtoLongCountSA(int const *CJDN, char **date)
{
  int longcount[5];

  CJDNtoLongCount(*CJDN, longcount);
  if (asprintf(date, "%d.%d.%d.%d.%d", longcount[0], longcount[1],
               longcount[2], longcount[3], longcount[4]) < 0)
    *date = NULL;
}
/*--------------------------------------------------------------------------*/
int LongCounttoCJDN(int const *longcount)
{
  int d, i;

  d = longcount[0];
  for (i = 0; i < sizeof(longcount_periods)/sizeof(*longcount_periods); i++)
    d = d*longcount_periods[i] + longcount[i + 1];
  return d + LONGCOUNT_EPOCH;
}
/*--------------------------------------------------------------------------*/
void LongCounttoCJDNA(int const *longcount, int *CJDN)
{
  *CJDN = LongCounttoCJDN(longcount);
}
/*--------------------------------------------------------------------------*/
