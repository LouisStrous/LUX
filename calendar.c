#define _GNU_SOURCE             /* for %as format in sscanf */
/* HEADERS */
#include <malloc.h> /* for free malloc */
#include <math.h> /* for floor */
#include <stdio.h> /* for asprintf sscanf */
#include <string.h> /* for strcmp */
#include <time.h> /* for localtime time gmtime */
#include <stdlib.h>             /* for strtol */
/* END HEADERS */
#include <errno.h>
#include <ctype.h>
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
/** Translates a Chronological Julian Day Number to a calendar date in
    text format (numerical year, text month, numerical day).

    \param[in] CJDN the Chronological Julian Day Number to translate.

    \param[in] CJDNtoCal3 a pointer to a function that translates a
    Chronological Julian Day Number into a calendar date (year, month,
    day).

    \param monthnames a pointer to an array of month names in
    chronological order.  Element 0 must contain the name of the first
    month.  Must have at least as many elements as there are months!

    \return a pointer to the corresponding date in text format.  The
    text is stored in memory allocated by the routine.  Free the
    memory when you're done with it.
 */
static char *CJDNto3S(int CJDN,
                      void (*CJDNtoCal3)(int CJDN, int *year, int *month, int *day),
                      char const * const *monthnames)
{
  int year, month, day;
  char *date;

  CJDNtoCal3(CJDN, &year, &month, &day);
  if (asprintf(&date, "%d %s %d", year, monthnames[month - 1], day) < 0)
    return NULL;
  return date;
}
/*--------------------------------------------------------------------------*/
/** Translates a Chronological Julian Day Number to a calendar date in
    text format (numerical year, text month, numerical day).

    \param[in] CJDN a pointer to the Chronological Julian Day Number
    to translate.  Must not be NULL!

    \param[out] date a pointer to a string holding the corresponding
    calendar date in text format.  Must not be NULL!  The text is
    stored in memory allocated by the routine.  Free the memory when
    you're done with it.

    \param[in] CJDNtoCal3A a pointer to a function that translates a
    Chronological Julian Day Number into a calendar date (year, month,
    day).

    \param monthnames a pointer to an array of month names in
    chronological order.  Element 0 must contain the name of the first
    month.  Must have at least as many elements as there are months!

 */
static void CJDNto3SA(int const *CJDN, char **date,
                      void (*CJDNtoCal3A) (int const *, int *),
                      char const * const *monthnames)
{
  int datec[3];

  CJDNtoCal3A(CJDN, datec);
  if (asprintf(date, "%d %s %d", datec[0], monthnames[datec[1] - 1], datec[2])
      < 0)
    *date = NULL;
}
/*--------------------------------------------------------------------------*/
/** Translates a Chronological Julian Day to a calendar date in text
    format (numerical year, text month, numerical day).

    \param[in] CJD the Chronological Julian Day to translate.

    \param[in] CJDtoCal3 a pointer to a function that translates a
    Chronological Julian Day into a calendar date (year, month, day).

    \param monthnames a pointer to an array of month names in
    chronological order.  Element 0 must contain the name of the first
    month.  Must have at least as many elements as there are months!

    \return a pointer to the corresponding date in text format.  The
    text is stored in memory allocated by the routine.  Free the
    memory when you're done with it.
 */
static char *CJDto3S(double CJD,
                     void (*CJDtoCal3)(double, int *, int *, double *),
                     char const * const *monthnames)
{
  int year, month;
  double day;
  char *date;

  CJDtoCal3(CJD, &year, &month, &day);
  if (asprintf(&date, "%g %s %d", day, monthnames[month - 1], year) < 0)
    return NULL;
  return date;
}
/*--------------------------------------------------------------------------*/
/** Translates a Chronological Julian Day to a calendar date in text
    format (numerical year, text month, numerical day).

    \param[in] CJD a pointer to the Chronological Julian Day to
    translate.  Must not be NULL!

    \param[out] date a pointer to a string holding the corresponding
    calendar date in text format.  Must not be NULL!  The text is
    stored in memory allocated by the routine.  Free the memory when
    you're done with it.

    \param[in] CJDtoCal3A a pointer to a function that translates a
    Chronological Julian Day into a calendar date (year, month, day).

    \param monthnames a pointer to an array of month names in
    chronological order.  Element 0 must contain the name of the first
    month.  Must have at least as many elements as there are months!

 */
static void CJDto3SA(double const *CJD, char **date,
                     void (*CJDtoCal3A) (double const *, double *),
                     char const * const *monthnames)
{
  double datec[3];

  CJDtoCal3A(CJD, datec);
  if (asprintf(date, "%g %s %d", datec[2], monthnames[(int) datec[1] - 1],
               (int) datec[0]) < 0)
    *date = NULL;  
}
/*--------------------------------------------------------------------------*/
/** Compares two strings disregarding case distinctions and
    disregarding non-alphanumerical characters.

    \param[in] a the first string to compare

    \param[in] b the second string to compare

    \return a negative, zero, or positive value, depending on whether
    \p a compares less than, equal to, or greater than \p b.
 */
static int stralnumtouppercmp(char const * const a, char const * const b)
{
  char const *pa, *pb;

  pa = a;
  pb = b;
  while (*pa && *pb) {
    while (*pa && toupper(*pa) == toupper(*pb)) {
      pa++;
      pb++;
    }
    while (*pa && !isalnum(*pa))
      pa++;
    while (*pb && !isalnum(*pb))
      pb++;
    if (*pa != *pb)
      break;
  }
  return *pa - *pb;
}
/*--------------------------------------------------------------------------*/
/** Seeks a name in an array of unsorted names, disregarding case
   distinctions and disregarding non-alphanumerical characters.
   
   \param[in] name the name to seek
   \param[in] num_names the number of names in the array
   \param[in] names the array of names to search through

   \return the index into array \p names of the \p name if that name
   is found, and otherwise \p num_names.
 */
static size_t find_name(char * const name, int num_names,
                        char const * const *names)
{
  size_t i;

  for (i = 0; i < num_names; i++) {
    if (!stralnumtouppercmp(name, names[i]))
      break;
  }
  return i;
}
/*--------------------------------------------------------------------------*/
/** Translates a calendar date in text to a Chronological Julian Day
    Number.

    \param[in] date the date string to parse.  Must not be NULL!

    \param[in] Cal3toCJDN a pointer to a calendar-specific function
    that translates a numerical calendar date into a Chronological
    Julian Day Number.
    
    \param[in] nmonthnames the number of month names in \p monthnames

    \param[in] monthnames an array of month names in increasing order
    of month number.  Element 0 corresponds to the first month.  There
    must be at least \p nmonthnames month names!
    
    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
static int S3toCJDN(char const * date,
                    int (*Cal3toCJDN) (int, int, int),
                    size_t nmonthnames, char const * const *monthnames)
{
  int year, month, day;
  size_t i, n;
  char *monthname;

  n = sscanf(date, "%d %as %d", &day, &monthname, &year);
  if (n == 3) {
    i = find_name(monthname, arraysize(monthnames), monthnames);
    free(monthname);
    if (i < nmonthnames) {      /* found a match */
      month = i + 1;
      return Cal3toCJDN(year, month, day);
    }
  }
  return 0;                     /* found no match */
}
/*--------------------------------------------------------------------------*/
/** Translates a calendar date in text to a Chronological Julian Day
    Number.

    \param[in] date the date string to parse.  Must not be NULL!

    \param[in] Cal3toCJDN a pointer to a calendar-specific function
    that translates a numerical calendar date into a Chronological
    Julian Day Number.
    
    \param[in] nmonthnames the number of month names in \p monthnames

    \param[in] monthnames an array of month names in increasing order
    of month number.  Element 0 corresponds to the first month.  There
    must be at least \p nmonthnames month names!
    
    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
static void S3toCJDNA(char * const * date, int *CJDN,
                      void (*Cal3toCJDNA) (int const *, int *),
                      size_t nmonthnames, char const * const *monthnames)
{
  int datec[3];
  size_t i, n;
  char *monthname;

  n = sscanf(*date, "%d %as %d", &datec[2], &monthname, &datec[0]);
  if (n == 3) {
    i = find_name(monthname, arraysize(monthnames), monthnames);
    free(monthname);
    if (i < nmonthnames) {      /* found a match */
      datec[1] = i + 1;
      Cal3toCJDNA(datec, CJDN);
    }
  }
  *CJDN = 0;                     /* found no match */
}
/*--------------------------------------------------------------------------*/
/** Translates a calendar date in text to a Chronological Julian Day.

    \param[in] date the date string to parse.  Must not be NULL!

    \param[in] Cal3toCJD a pointer to a calendar-specific function
    that translates a numerical calendar date into a Chronological
    Julian Day.
    
    \param[in] nmonthnames the number of month names in \p monthnames

    \param[in] monthnames an array of month names in increasing order
    of month number.  Element 0 corresponds to the first month.  There
    must be at least \p nmonthnames month names!
    
    \return the corresponding Chronological Julian Day Number, or 0 if
    no legal date was recognized in the input.
 */
static double S3toCJD(char const * date,
                      double (*Cal3toCJD) (int, int, double),
                      size_t nmonthnames, char const * const *monthnames)
{
  int year, month;
  double day;
  size_t i, n;
  char *monthname;

  n = sscanf(date, "%lg %as %d", &day, &monthname, &year);
  if (n == 3) {
    i = find_name(monthname, arraysize(monthnames), monthnames);
    free(monthname);
    if (i < nmonthnames) {      /* found a match */
      month = i + 1;
      return Cal3toCJD(year, month, day);
    }
  }
  return 0;                     /* found no match */
}
/*--------------------------------------------------------------------------*/
/** Translates a calendar date in text to a Chronological Julian Day.
    
    \param[in] date a pointer to the date string to parse.  The
    pointer must not be NULL, and the date string itself must not be
    NULL!

    \param[out] CJD a pointer to the corresponding Chronological
    Julian Day.  Must not be NULL!  If no legal date is recognized in
    the input, then Chronological Julian Day 0 is returned.
    
    \param[in] Cal3AtoCJDN a pointer to a calendar-specific function
    that translates a numerical calendar date into a Chronological
    Julian Day Number.
    
    \param[in] nmonthnames the number of month names in \p monthnames

    \param[in] monthnames an array of month names in increasing order
    of month number.  Element 0 corresponds to the first month.  There
    must be at least \p nmonthnames month names!
    
 */
static void S3toCJDA(char * const *date, double *CJD,
                     void (*Cal3AtoCJD) (double const *, double *),
                     size_t nmonthnames, char const * const *monthnames)
{
  double datec[3];
  size_t i, n;
  char *monthname;

  n = sscanf(*date, "%lg %as %lg", &datec[2], &monthname, &datec[0]);
  if (n == 3) {
    i = find_name(monthname, arraysize(monthnames), monthnames);
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
double CJD_now(void)
{
  time_t t;
  double cjd;
  struct tm *bd;

  t = time(NULL);
  bd = localtime(&t);
  cjd = (double) (t + bd->tm_gmtoff)/86400.0 + 2440588.0;
  return cjd;
}
/*--------------------------------------------------------------------------*/
int CJDN_now(void)
{
  return floor(CJD_now());
}
/*--------------------------------------------------------------------------*/

/** Month names for the Julian and Gregorian calendars */
static char const * const Gregorian_Julian_monthnames[] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November",
  "December"
};
/*--------------------------------------------------------------------------*/

/* THE GREGORIAN CALENDAR */

/*--------------------------------------------------------------------------*/
/** The Chronological Julian Day Number of the epoch of the Gregorian calendar */
#define GREGORIAN_EPOCH (1721120)
void CJDNtoGregorian(int CJDN, int *year, int *month, int *day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  int y3 = CJDN - GREGORIAN_EPOCH;
  int x3 = alinequot(y3, 4, 3, 146097);
  int y2 = alinequot(x3, 146097, 0, 4);
  y2 = y3 - y2;
  int x2 = iaquot(y2*100 + 99, 36525);
  int y1 = y2 - iaquot(36525*x2, 100);
  int x1 = iaquot(5*y1 + 2, 153);
  int c0 = iaquot(x1 + 2, 12);
  *year = 100*x3 + x2 + c0;
  *month = x1 - 12*c0 + 3;
  *day = y1 - iaquot(153*x1 - 3, 5);
}
/*--------------------------------------------------------------------------*/
void CJDNtoGregorianA(int const *CJDN, int *date)
{
  CJDNtoGregorian(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoGregorianS(int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoGregorian, Gregorian_Julian_monthnames);
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
char *CJDtoGregorianS(double CJD)
{
  return CJDto3S(CJD, CJDtoGregorian, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoGregorianSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoGregorianA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
int GregoriantoCJDN(int year, int month, int day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  int x1;
  int c0 = iaquot(month - 3, 12);
  int x4 = year + c0;
  div_t d = adiv(x4, 100);
  x1 = month - 12*c0 - 3;
  int J1 = alinequot(d.quot, 146097, 0, 4);
  int J2 = iaquot(d.rem*36525, 100);
  int J3 = iaquot(x1*153 + 2, 5);
  return GREGORIAN_EPOCH - 1 + J1 + J2 + J3 + day;
}
/*--------------------------------------------------------------------------*/
void GregoriantoCJDNA(int const *date, int *CJDN)
{
  *CJDN = GregoriantoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
int GregorianStoCJDN(char const * date)
{
  return S3toCJDN(date, GregoriantoCJDN,
                  arraysize(Gregorian_Julian_monthnames),
                  Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void GregorianStoCJDNA(char * const * date, int *CJDN)
{
  S3toCJDNA(date, CJDN, GregoriantoCJDNA,
            arraysize(Gregorian_Julian_monthnames),
            Gregorian_Julian_monthnames);
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
double GregorianStoCJD(char const * date)
{
  return S3toCJD(date, GregoriantoCJD,
                 arraysize(Gregorian_Julian_monthnames),
                 Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void GregorianStoCJDA(char * const * date, double *CJD)
{
  S3toCJDA(date, CJD, GregoriantoCJDA,
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
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  int y2 = CJDN - JULIAN_EPOCH;
  int x2 = alinequot(y2, 4, 3, 1461);
  int z2 = y2 - alinequot(x2, 1461, 0, 4);
  int x1 = iaquot(5*z2 + 2, 153);
  int c0 = iaquot(x1 + 2, 12);
  *year = x2 + c0;
  *month = x1 - 12*c0 + 3;
  *day = z2 - iaquot(153*x1 - 3, 5);
}
/*--------------------------------------------------------------------------*/
void CJDNtoJulianA(int const *CJDN, int *date)
{
  CJDNtoJulian(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoJulianS(int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoJulian, Gregorian_Julian_monthnames);
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
char *CJDtoJulianS(double CJD)
{
  return CJDto3S(CJD, CJDtoJulian, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoJulianSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoJulianA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
int JuliantoCJDN(int year, int month, int day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  int c0 = iaquot(month - 3, 12);
  int J1 = alinequot(year + c0, 1461, 0, 4);
  int J2 = iaquot(month*153 - 1836*c0 - 457, 5);
  return J1 + J2 + day + JULIAN_EPOCH - 1;
}
/*--------------------------------------------------------------------------*/
void JuliantoCJDNA(int const *date, int *CJDN)
{
  *CJDN = JuliantoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
int JulianStoCJDN(char const *date)
{
  return S3toCJDN(date, JuliantoCJDN,
                  arraysize(Gregorian_Julian_monthnames),
                  Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void JulianStoCJDNA(char * const *date, int *CJDN)
{
  S3toCJDNA(date, CJDN, JuliantoCJDNA,
            arraysize(Gregorian_Julian_monthnames),
            Gregorian_Julian_monthnames);
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
double JulianStoCJD(char const *date)
{
  return S3toCJD(date, JuliantoCJD,
                 arraysize(Gregorian_Julian_monthnames),
                 Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void JulianStoCJDA(char * const *date, double *CJD)
{
  S3toCJDA(date, CJD, JuliantoCJDA,
           arraysize(Gregorian_Julian_monthnames),
           Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/

/* COMMON CALENDAR */

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
char *CJDNtoCommonS(int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoCommon, Gregorian_Julian_monthnames);
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
char *CJDtoCommonS(double CJD)
{
  return CJDto3S(CJD, CJDtoCommon, Gregorian_Julian_monthnames);
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
int CommonStoCJDN(char const *date)
{
  return S3toCJDN(date, CommontoCJDN,
                  arraysize(Gregorian_Julian_monthnames),
                  Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CommonStoCJDNA(char * const *date, int *CJDN)
{
  S3toCJDNA(date, CJDN, CommontoCJDNA,
            arraysize(Gregorian_Julian_monthnames),
            Gregorian_Julian_monthnames);
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
double CommonStoCJD(char const *date)
{
  return S3toCJD(date, CommontoCJD,
                 arraysize(Gregorian_Julian_monthnames),
                 Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CommonStoCJDA(char * const *date, double *CJD)
{
  S3toCJDA(date, CJD, CommontoCJDA,
           arraysize(Gregorian_Julian_monthnames),
           Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/

/* HEBREW CALENDAR */

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

static char const * const Hebrew_monthnames[] = {
  "Tishri", "Heshvan", "Kislev", "Tevet", "Shevat", "Adar", "Nisan",
  "Iyar", "Sivan", "Tammuz", "Av", "Elul"
};
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
char *CJDNtoHebrewS(int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoHebrew, Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
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
char *CJDtoHebrewS(double CJD)
{
  return CJDto3S(CJD, CJDtoHebrew, Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoHebrewSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoHebrewA, Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
static int tishri(int year)
/* returns the number of days between Tishri 1, A.M. 1 and Tishri 1, */
/* A.M. <year>.  LS 2oct98 */
{
  static int	nLeap[] = {
    0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6
  };
  static int	isLeap[] = {
    0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1
  };
  int	n, leap, halakim, hours, dow, weeks, j;

  year--;
  n = iaquot(year,19);		/* number of cycles of Meton */
  year -= n*19;			/* year within current cycle */
  leap = nLeap[year];		/* number of leap days in current cycle */

  halakim = 595*n + 876*year - 287*leap + 204;
  j = iaquot(halakim,1080);
  hours = j + 16*n + 8*year + 13*leap + 5;
  halakim -= 1080*j;
  j = iaquot(hours,24);
  dow = j + 6939*n + 354*year + 29*leap + 1;
  hours -= 24*j;
  weeks = iaquot(dow,7);
  dow -= 7*weeks;
  if (dow == 0 || dow == 3 || dow == 5)	/* dehiyyot 1 */
    dow++;
  else if (hours >= 18) {	/* dehiyyot 2 */
    dow++;
    if (dow == 7) {
      weeks++;
      dow = 0;
    }
    if (dow == 0 || dow == 3 || dow == 5) /* dehiyyot 1 */
      dow++;
  } else if (!isLeap[year] && dow == 2 && 1080*hours + halakim >= 9924)
    dow += 2;
  else if ((year == 0 || isLeap[year - 1])
	   && dow == 1 && 1080*hours + halakim >= 16789)
    dow++;

  return weeks*7 + dow;
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
int HebrewStoCJDN(char const *date)
{
    return S3toCJDN(date, HebrewtoCJDN,
                  arraysize(Hebrew_monthnames),
                  Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
void HebrewStoCJDNA(char * const * date, int *CJDN)
{
  S3toCJDNA(date, CJDN, HebrewtoCJDNA,
            arraysize(Hebrew_monthnames),
            Hebrew_monthnames);
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
double HebrewStoCJD(char const *date)
{
  return S3toCJD(date, HebrewtoCJD,
                 arraysize(Hebrew_monthnames),
                 Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
void HebrewStoCJDA(char * const *date, double *CJD)
{
  S3toCJDA(date, CJD, HebrewtoCJDA,
           arraysize(Hebrew_monthnames),
           Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/

/* ISLAMIC CALENDAR */

/** The Chronological Julian Day Number of the epoch of the Islamic calendar */
#define ISLAMIC_EPOCH (1948440)

static char const *Islamic_monthnames[] = {
  "Muharram", "Safar", "Rabi`a I", "Rabi`a II", "Jumada I", "Jumada II",
  "Rajab", "Sha`ban", "Ramadan", "Shawwal", "Dhu al-Q`adah",
  "Dhu al-Hijjah"
};

/*--------------------------------------------------------------------------*/
void CJDNtoIslamic(int CJDN, int *year, int *month, int *day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  /* "Alfonsine tables", s = 14, J₀ = 1948440 */
  int y2 = CJDN - ISLAMIC_EPOCH;
  *year = alinequot(y2, 30, 10646, 10631);
  int z2 = y2 - alinequot(*year, 10631, -10617, 30);
  *month = iaquot(11*z2 + 330, 325);
  *day = z2 - iaquot(325**month - 331, 11);
}
/*--------------------------------------------------------------------------*/
void CJDNtoIslamicA(int const *CJDN, int *date)
{
  CJDNtoIslamic(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoIslamicS(int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoIslamic, Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
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
char *CJDtoIslamicS(double CJD)
{
  return CJDto3S(CJD, CJDtoIslamic, Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoIslamicSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoIslamicA, Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
int IslamictoCJDN(int year, int month, int day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  return alinequot(year, 10631, -10617, 30)
    + iaquot(325*month - 320, 11) + day + ISLAMIC_EPOCH - 1;
}
/*--------------------------------------------------------------------------*/
void IslamictoCJDNA(int const *date, int *CJDN)
{
  *CJDN = IslamictoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
int IslamicStoCJDN(char const *date)
{
  return S3toCJDN(date, IslamictoCJDN,
                  arraysize(Islamic_monthnames),
                  Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
void IslamicStoCJDNA(char * const *date, int *CJDN)
{
  S3toCJDNA(date, CJDN, IslamictoCJDNA,
            arraysize(Islamic_monthnames),
            Islamic_monthnames);
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
double IslamicStoCJD(char const *date)
{
  return S3toCJD(date, IslamictoCJD,
                 arraysize(Islamic_monthnames),
                 Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
void IslamicStoCJDA(char * const *date, double *CJD)
{
  S3toCJDA(date, CJD, IslamictoCJDA,
           arraysize(Islamic_monthnames),
           Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/

/* EGYPTIAN CALENDAR */

/** The Chronological Julian Day Number of the epoch of the Egyptian calendar */
#define EGYPTIAN_EPOCH (1448273)

static char const *Egyptian_monthnames[] = {
  "Thoth", "Phaophi", "Athyr", "Choiak", "Tybi", "Mecheir", "Phamenoth",
  "Pharmuthi", "Pachon", "Payni", "Epiphi", "Mesore", "epagomenai"
};

/*--------------------------------------------------------------------------*/
void CJDNtoEgyptian(int CJDN, int *year, int *month, int *day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  /* era of Nabonassar */
  int y2 = CJDN - EGYPTIAN_EPOCH;
  div_t d = adiv(y2, 365);
  *year = d.quot;
  d = adiv(d.rem, 30);
  *month = d.quot + 1;
  *day = d.rem + 1;
}
/*--------------------------------------------------------------------------*/
void CJDNtoEgyptianA(int const *CJDN, int *date)
{
  CJDNtoEgyptian(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoEgyptianS(int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoEgyptian, Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
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
char *CJDtoEgyptianS(double CJD)
{
  return CJDto3S(CJD, CJDtoEgyptian, Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoEgyptianSA(double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoEgyptianA, Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
int EgyptiantoCJDN(int year, int month, int day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  return 365*year + 30*month + day + EGYPTIAN_EPOCH - 365 - 30 - 1;
}
/*--------------------------------------------------------------------------*/
void EgyptiantoCJDNA(int const *date, int *CJDN)
{
  *CJDN = EgyptiantoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
int EgyptianStoCJDN(char const *date)
{
  return S3toCJDN(date, EgyptiantoCJDN,
                  arraysize(Egyptian_monthnames),
                  Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
void EgyptianStoCJDNA(char * const *date, int *CJDN)
{
  S3toCJDNA(date, CJDN, EgyptiantoCJDNA,
            arraysize(Egyptian_monthnames),
            Egyptian_monthnames);
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
double EgyptianStoCJD(char const *date)
{
  return S3toCJD(date, EgyptiantoCJD,
                 arraysize(Egyptian_monthnames),
                 Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
void EgyptianStoCJDA(char * const *date, double *CJD)
{
  S3toCJDA(date, CJD, EgyptiantoCJDA,
           arraysize(Egyptian_monthnames),
           Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/

/* JULIAN DAY */

/*--------------------------------------------------------------------------*/
double JDtoCJD(double JD)
{
  return JD + 0.5;
}
/*--------------------------------------------------------------------------*/
void JDtoCJDA(double const *JD, double *CJD)
{
  *CJD = JDtoCJD(*JD);
}
/*--------------------------------------------------------------------------*/
double CJDtoJD(double CJD)
{
  return CJD - 0.5;
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

/* LUNAR CALENDAR */

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

/* MAYAN TZOLKIN/HAAB CALENDAR */

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

#define find_venteina_name(s) find_name(s, arraysize(Mayan_venteina_names), Mayan_venteina_names)
#define find_haab_name(s) find_name(s, arraysize(Mayan_haab_names), Mayan_haab_names)

/*--------------------------------------------------------------------------*/
void CJDNtoMayan(int CJDN, int *trecena, int *venteina,
                 int *haab_day, int *haab_month,
                 int *year_trecena, int *year_venteina)
{
  *trecena = iamod(CJDN + 5, 13) + 1;
  *venteina = iamod(CJDN + 16, 20) + 1;
  div_t d = adiv(CJDN + 65, 365);
  *haab_day = iamod(d.rem, 20);
  *haab_month = d.rem/20 + 1;
  *year_trecena = iamod(d.quot + 5, 13) + 1;
  *year_venteina = iamod(5*d.quot + 11, 20) + 1;
}
/*--------------------------------------------------------------------------*/
void CJDNtoMayanA(int const *CJDN, int *date)
{
  CJDNtoMayan(*CJDN, &date[0], &date[1], &date[2], &date[3], &date[4],
              &date[5]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoMayanS(int CJDN)
{
  int trecena, venteina, haab_day, haab_month, year_trecena, year_venteina;
  char *date;

  CJDNtoMayan(CJDN, &trecena, &venteina, &haab_day, &haab_month,
              &year_trecena, &year_venteina);
  if (asprintf(&date, "%d %s %d %s (%d %s)", trecena,
               Mayan_venteina_names[venteina - 1], haab_day,
               Mayan_haab_names[haab_month - 1], year_trecena,
               Mayan_venteina_names[year_venteina - 1]) < 0)
    date = NULL;
  return date;
}
/*--------------------------------------------------------------------------*/
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
void CJDtoMayan(double CJD, int *trecena, int *venteina,
                double *haab_day, int *haab_month,
                int *year_trecena, int *year_venteina)
{
  int ihaab_day, CJDN;
  double frac;

  CJDN = (int) floor(CJD);
  frac = CJD - CJDN;
  CJDNtoMayan(CJDN, trecena, venteina, &ihaab_day, haab_month, year_trecena,
              year_venteina);
  *haab_day = ihaab_day + frac;
}
/*--------------------------------------------------------------------------*/
void CJDtoMayanA(double const *CJD, double *date)
{
  int CJDN, datec[6];
  double frac;

  CJDN = floor(*CJD);
  frac = *CJD - CJDN;
  CJDNtoMayanA(&CJDN, datec);
  date[0] = datec[0];
  date[1] = datec[1];
  date[2] = datec[2] + frac;
  date[3] = datec[3];
  date[4] = datec[4];
  date[5] = datec[5];
}
/*--------------------------------------------------------------------------*/
char *CJDtoMayanS(double CJD)
{
  int trecena, venteina, haab_month, year_trecena, year_venteina;
  double haab_day;
  char *date;

  CJDtoMayan(CJD, &trecena, &venteina, &haab_day, &haab_month, &year_trecena,
             &year_venteina);
  if (asprintf(&date, "%d %s %g %s (%d %s)", trecena,
               Mayan_venteina_names[venteina - 1], haab_day,
               Mayan_haab_names[haab_month - 1], year_trecena,
               Mayan_venteina_names[year_venteina - 1]) < 0)
    date = NULL;
  return date;
}
/*--------------------------------------------------------------------------*/
void CJDtoMayanSA(double const *CJD, char **date)
{
  *date = CJDtoMayanS(*CJD);
}
/*--------------------------------------------------------------------------*/
int MayantoCJDN(int trecena, int venteina, int haab_day, int haab_month,
                int year_trecena, int year_venteina, int CJDN_upper)
{
  int do_tzolkin = (trecena && venteina);
  int do_haab = (haab_month != 0);
  int do_year = (year_trecena && year_venteina);
  int haab, tzolkin, year, haab_year;

  if (do_tzolkin) {
    trecena = iamod(trecena - 6, 13);
    venteina = iamod(venteina - 17, 20);
    tzolkin = iamod(40*trecena - 39*venteina, 260);
  }
  if (do_haab)
    haab = iamod((haab_month - 4)*20 + haab_day - 5, 365);
  if (do_year) {
    if ((year_venteina - 12) % 5)
      return 0;                 /* impossible year venteina */
    year_trecena = iamod(year_trecena - 6, 13);
    year_venteina = iamod(iaquot(year_venteina - 12, 5), 4);
    year = iamod(40*year_trecena + 13*year_venteina, 52);
    haab_year = 365*year - 65;
    /* CJDN = haab_year + 18980*year + haab */
    /* CJDN_firstofyear = haab_year (mod 18980) */
    /* CJDN_lastofyear = haab_year + 364 (mod 18980) */
  }
  switch (do_tzolkin + (do_haab << 1) + (do_year << 2)) {
  case 0:
    return 0;
  case 1:                       /* tzolkin */
    return CJDN_upper - iamod(CJDN_upper - tzolkin, 260);
  case 2:                       /* haab */
    return CJDN_upper - iamod(CJDN_upper - haab, 365);
  case 3:                       /* tzolkin & haab */
    {
      int tzolkin_haab = iamod(365*tzolkin - 364*haab, 18980);
      return CJDN_upper - iamod(CJDN_upper - tzolkin_haab, 18980);
    }
  case 4:                       /* year */
    return CJDN_upper - iamod(CJDN_upper - haab_year, 18980);
  case 5:                       /* tzolkin & year */
    {
      int CJDN_tzolkin = CJDN_upper - iamod(CJDN_upper - tzolkin, 260);
      int CJDN_haab_year = CJDN_upper - iamod(CJDN_upper - haab_year, 18980);
      /* now CJDN_tzolkin is the last matching tzolkin on/before
         CJDN_upper, and CJDN_haab_year is the last matching beginning of
         the haab year on/before CJDN_upper, but CJDN_tzolkin may
         still be before CJDN_haab_year */
      return CJDN_tzolkin + (18980-260)*iaquot(CJDN_tzolkin - CJDN_haab_year, 260);
    }
  case 6:                       /* haab & year */
    {
      int CJDN_haab = CJDN_upper - iamod(CJDN_upper - haab, 365);
      int CJDN_haab_year = CJDN_upper - iamod(CJDN_upper - haab_year, 18980);
      return CJDN_haab + (18980-365)*iaquot(CJDN_haab - CJDN_haab_year, 365);
    }
  case 7:                       /* tzolkin & haab & year */
    {
      int CJDN_tzolkin = CJDN_upper - iamod(CJDN_upper - tzolkin, 260);
      int CJDN_haab_year = CJDN_upper - iamod(CJDN_upper - haab_year, 18980);
      int CJDN = CJDN_tzolkin + (18980-260)*iaquot(CJDN_tzolkin - CJDN_haab_year, 260);
      int CJDN_haab = CJDN_upper - iamod(CJDN_upper - haab, 365);
      return CJDN == CJDN_haab? CJDN: 0;
    }
    break;
  }
  return 0;
}
/*--------------------------------------------------------------------------*/
void MayantoCJDNA(int const *date, int *CJDN)
{
  *CJDN = MayantoCJDN(date[0], date[1], date[2], date[3], date[4], date[5],
                      date[6]);
}
/*--------------------------------------------------------------------------*/
static void MayanSto(char const *date, int *CJDN, double *CJD)
{
  int n, nc, datepos, inumber;
  double number, haab_day;
  char *name;
  size_t i;
  int trecena, venteina = -1, haab_month = -1,
    trecena_year, venteina_year = -1;
  enum state {
    BUSY, OK, BAD
  } state = BUSY;
  int CJDN_upper;
  
  name = NULL;
  datepos = 0;
  n = sscanf(date, "%lg %as%n", &number, &name, &nc);
  if (n == 2) {
    if ((i = find_venteina_name(name)) 
        < arraysize(Mayan_venteina_names)) { /* a venteina name */
      trecena = floor(number);
      venteina = i + 1;
    } else if ((i = find_haab_name(name))
               < arraysize(Mayan_haab_names)) { /* a haab name */
      haab_day = number;
      haab_month = i + 1;
    } else                      /* unrecognized Mayan date */
      state = BAD;
  } else                        /* couldn't read anything date-like */
    state = BAD;
  free(name);
  if (state == BUSY) {
    name = NULL;
    datepos += nc;
    n = sscanf(date + datepos, "%lg %as%n", &number, &name, &nc);
    if (n == 2) {
      if ((i = find_venteina_name(name)) 
          < arraysize(Mayan_venteina_names)) { /* a venteina name */
        if (venteina >= 0) {      /* already have "day" tzolkin */
          trecena_year = floor(number);  /* must be "year" tzolkin */
          venteina_year = i + 1;
        } else {                  /* "day" tzolkin */
          trecena = floor(number);
          venteina = i + 1;
        }
      } else if (haab_month < 0   /* no haab found yet */
                 && (i = find_haab_name(name))
                 < arraysize(Mayan_haab_names)) { /* a haab name */
        haab_day = number;
        haab_month = i + 1;
      } else                    /* unrecognized Mayan date */
        state = BAD;
    } else
      state = OK;
    free(name);
  }
  if (state == BUSY) {
    name = NULL;
    datepos += nc;
    n = sscanf(date + datepos, "%d %as%n", &inumber, &name, &nc);
    if (n == 2) {
      if (venteina_year < 0     /* no "year" tzolkin found yet */
          && (i = find_venteina_name(name)) 
          < arraysize(Mayan_venteina_names)) { /* a venteina name */
        trecena_year = inumber;
        venteina_year = i + 1;
      } else if (haab_month < 0 /* no haab found yet */
                 && (i = find_haab_name(name))
                 < arraysize(Mayan_haab_names)) { /* a haab name */
        haab_day = inumber;
        haab_month = i + 1;
      } else                    /* unrecognized Mayan date */
        state = BAD;
    } else
      state = OK;
    free(name);
  }
  if (state == BUSY) {
    datepos += nc;
    n = sscanf(date + datepos, "%d", &inumber);
    if (n == 1)
      CJDN_upper = inumber;
    else
      CJDN_upper = CJDN_now();
  } else if (state == OK)
    CJDN_upper = CJDN_now();

  if (state == BAD) {
    if (CJDN)
      *CJDN = 0;
    if (CJD)
      *CJD = 0.0;
    return;
  }
  if (venteina < 0)
    venteina = 0;
  if (haab_month < 0)
    haab_month = 0;
  if (venteina_year < 0)
    venteina_year = 0;

  if (CJDN)
    *CJDN = MayantoCJDN(trecena, venteina, floor(haab_day), haab_month,
                        trecena_year, venteina_year, CJDN_upper);
  if (CJD)
    *CJD = MayantoCJD(trecena, venteina, haab_day, haab_month,
                      trecena_year, venteina_year, CJDN_upper);
}
/*--------------------------------------------------------------------------*/
int MayanStoCJDN(char const *date)
{
  int CJDN;

  MayanSto(date, &CJDN, NULL);
  return CJDN;
}
/*--------------------------------------------------------------------------*/
void MayanStoCJDNA(char * const *date, int *CJDN)
{
  MayanSto(*date, CJDN, NULL);
}
/*--------------------------------------------------------------------------*/
double MayantoCJD(int trecena, int venteina, double haab_day, int haab_month,
                  int year_trecena, int year_venteina, int CJDN_upper)
{
  int ihaab_day = floor(haab_day);
  double frac = haab_day - ihaab_day;
  int CJDN = MayantoCJDN(trecena, venteina, ihaab_day, haab_month,
                         year_trecena, year_venteina, CJDN_upper);
  return CJDN + frac;
}
/*--------------------------------------------------------------------------*/
void MayantoCJDA(double const *date, double *CJD)
{
  *CJD = MayantoCJD(floor(date[0]), floor(date[1]), date[2], floor(date[3]),
                    floor(date[4]), floor(date[5]), floor(date[6]));
}
/*--------------------------------------------------------------------------*/
double MayanStoCJD(char const *date)
{
  double CJD;
  
  MayanSto(date, NULL, &CJD);
  return CJD;
}
/*--------------------------------------------------------------------------*/
void MayanStoCJDA(char * const *date, double *CJD)
{
  MayanSto(*date, NULL, CJD);
}
/*--------------------------------------------------------------------------*/

/* MAYAN LONG COUNT CALENDAR */

#define LONGCOUNT_EPOCH (584283)
static int longcount_periods[] = { 20, 20, 18, 20 };

void CJDNtoLongCount(int CJDN, int *baktun, int *katun, int *tun, int *uinal,
                     int *kin)
{
  int d;

  d = CJDN - LONGCOUNT_EPOCH;
  div_t x;
  x = adiv(d, longcount_periods[3]);
  *kin = x.rem;
  x = adiv(x.quot, longcount_periods[2]);
  *uinal = x.rem;
  x = adiv(x.quot, longcount_periods[1]);
  *tun = x.rem;
  x = adiv(x.quot, longcount_periods[0]);
  *baktun = x.quot;
  *katun = x.rem;
}
/*--------------------------------------------------------------------------*/
void CJDNtoLongCountA(int const *CJDN, int *longcount)
{
  int i;
  div_t x;

  x.quot = *CJDN;
  for (i = arraysize(longcount_periods) - 1; i >= 0; i--) {
    x = adiv(x.quot, longcount_periods[i]);
    longcount[i + 1] = x.rem;
  }
  longcount[0] = x.quot;
}
/*--------------------------------------------------------------------------*/
char *CJDNtoLongCountS(int CJDN)
{
  int baktun, katun, tun, uinal, kin;
  char *date;

  CJDNtoLongCount(CJDN, &baktun, &katun, &tun, &uinal, &kin);
  if (asprintf(&date, "%d.%d.%d.%d.%d", baktun, katun, tun, uinal, kin) < 0)
    date = NULL;
  return date;
}
/*--------------------------------------------------------------------------*/
void CJDNtoLongCountSA(int const *CJDN, char **date)
{
  int baktun, katun, tun, uinal, kin;

  CJDNtoLongCount(*CJDN, &baktun, &katun, &tun, &uinal, &kin);
  if (asprintf(date, "%d.%d.%d.%d.%d", baktun, katun, tun, uinal, kin) < 0)
    *date = NULL;
}
/*--------------------------------------------------------------------------*/
void CJDtoLongCount(double CJD, int *baktun, int *katun, int *tun, int *uinal,
                    double *kin)
{
  int d;
  double frac;
  div_t x;

  d = floor(CJD);
  frac = CJD - d;
  d -= LONGCOUNT_EPOCH;
  x = adiv(d, longcount_periods[3]);
  *kin = x.rem + frac;
  x = adiv(x.quot, longcount_periods[2]);
  *uinal = x.rem;
  x = adiv(d, longcount_periods[1]);
  *tun = x.rem;
  x = adiv(d, longcount_periods[0]);
  *baktun = x.quot;
  *katun = x.rem;
}
/*--------------------------------------------------------------------------*/
void CJDtoLongCountA(double const *CJD, double *longcount)
{
  int i;
  int d = floor(*CJD);
  double frac = *CJD - d;
  for (i = arraysize(longcount_periods) - 1; i >= 0; i--) {
    div_t x = adiv(d, longcount_periods[i]);
    d = x.quot;
    longcount[i + 1] = x.rem;
  }
  longcount[0] = d + frac;
}
/*--------------------------------------------------------------------------*/
char *CJDtoLongCountS(double CJD)
{
  return CJDNtoLongCountS(floor(CJD));
}
/*--------------------------------------------------------------------------*/
void CJDtoLongCountSA(double const *CJD, char **date)
{
  int CJDN = floor(*CJD);
  CJDNtoLongCountSA(&CJDN, date);
}
/*--------------------------------------------------------------------------*/
int LongCounttoCJDN(int baktun, int katun, int tun, int uinal, int kin)
{
  int d;

  d = baktun;
  d = longcount_periods[0]*d + katun;
  d = longcount_periods[1]*d + tun;
  d = longcount_periods[2]*d + uinal;
  d = longcount_periods[3]*d + kin;
  return d + LONGCOUNT_EPOCH;
}
/*--------------------------------------------------------------------------*/
void LongCounttoCJDNA(int const *longcount, int *CJDN)
{
  *CJDN = LongCounttoCJDN(longcount[0], longcount[1], longcount[2],
                          longcount[3], longcount[4]);
}
/*--------------------------------------------------------------------------*/
int LongCountStoCJDN(char const *date)
{
  int datec[5] = { 0, 0, 0, 0, 0 }, i, value;
  char *p;

  p = (char *) date;
  errno = 0;
  for (i = 4; i >= 0; i--) {
    value = strtol(p, &p, 10);
    if (errno)
      break;
    datec[i] = value;
    if (*p && !isdigit(*p))
      p++;
  }
  if (errno)                    /* some error encountered: no legal date */
    return 0;
  return LongCounttoCJDN(datec[0], datec[1], datec[2], datec[3], datec[4]);
}
/*--------------------------------------------------------------------------*/
void LongCountStoCJDNA(char * const *date, int *CJDN)
{
  *CJDN = LongCountStoCJDN(*date);
}
/*--------------------------------------------------------------------------*/
double LongCounttoCJD(int baktun, int katun, int tun, int uinal, double kin)
{
  int ikin = floor(kin);
  double frac = kin - ikin;
  return LongCounttoCJDN(baktun, katun, tun, uinal, ikin) + frac;
}
/*--------------------------------------------------------------------------*/
void LongCounttoCJDA(double const *date, double *CJD)
{
  *CJD = LongCounttoCJD(floor(date[0]), floor(date[1]), floor(date[2]),
                        floor(date[3]), date[4]);
}
/*--------------------------------------------------------------------------*/
double LongCountStoCJD(char const *date)
{
  return LongCountStoCJDN(date);
}
/*--------------------------------------------------------------------------*/
void LongCountStoCJDA(char * const * date, double *CJD)
{
  int CJDN;

  LongCountStoCJDNA(date, &CJDN);
  *CJD = (double) CJDN;
}
/*--------------------------------------------------------------------------*/
