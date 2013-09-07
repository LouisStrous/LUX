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
#include "action.h"
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
static void CJDtoCal(Double CJD, Int *year, Int *month, Double *day,
                     void (*f)(Int, Int *, Int *, Int *))
{
  /* separate the CJD into a CJDN and a day fraction */
  Int CJDN = floor(CJD);
  Double dayfraction = CJD - CJDN;

  Int iday;
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
static Double CaltoCJD(Int year, Int month, Double day,
                       Int (*f)(Int, Int, Int))
{
  Int d = floor(day);
  Double part = day - d;
  Int jd = f(year, month, d);
  return (Double) jd + part;
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
static char *CJDNto3S(Int CJDN,
                      void (*CJDNtoCal3)(Int CJDN, Int *year, Int *month, Int *day),
                      char const * const *monthnames)
{
  Int year, month, day;
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
static void CJDNto3SA(Int const *CJDN, char **date,
                      void (*CJDNtoCal3A) (Int const *, Int *),
                      char const * const *monthnames)
{
  Int datec[3];

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
static char *CJDto3S(Double CJD,
                     void (*CJDtoCal3)(Double, Int *, Int *, Double *),
                     char const * const *monthnames)
{
  Int year, month;
  Double day;
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
static void CJDto3SA(Double const *CJD, char **date,
                     void (*CJDtoCal3A) (Double const *, Double *),
                     char const * const *monthnames)
{
  Double datec[3];

  CJDtoCal3A(CJD, datec);
  if (asprintf(date, "%g %s %d", datec[2], monthnames[(Int) datec[1] - 1],
               (Int) datec[0]) < 0)
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
static Int stralnumtouppercmp(char const * const a, char const * const b)
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
static size_t find_name(char * const name, Int num_names,
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
static Int S3toCJDN(char const * date,
                    Int (*Cal3toCJDN) (Int, Int, Int),
                    size_t nmonthnames, char const * const *monthnames)
{
  Int year, month, day;
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
static void S3toCJDNA(char * const * date, Int *CJDN,
                      void (*Cal3toCJDNA) (Int const *, Int *),
                      size_t nmonthnames, char const * const *monthnames)
{
  Int datec[3];
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
static Double S3toCJD(char const * date,
                      Double (*Cal3toCJD) (Int, Int, Double),
                      size_t nmonthnames, char const * const *monthnames)
{
  Int year, month;
  Double day;
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
static void S3toCJDA(char * const *date, Double *CJD,
                     void (*Cal3AtoCJD) (Double const *, Double *),
                     size_t nmonthnames, char const * const *monthnames)
{
  Double datec[3];
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
Double CJD_now(void)
{
  time_t t;
  Double cjd;
  struct tm *bd;

  t = time(NULL);
  bd = localtime(&t);
  cjd = (Double) (t + bd->tm_gmtoff)/86400.0 + 2440588.0;
  return cjd;
}
/*--------------------------------------------------------------------------*/
Int CJDN_now(void)
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
void CJDNtoGregorian(Int CJDN, Int *year, Int *month, Int *day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  Int y3 = CJDN - GREGORIAN_EPOCH;
  Int x3 = alinequot(y3, 4, 3, 146097);
  Int y2 = alinequot(x3, 146097, 0, 4);
  y2 = y3 - y2;
  Int x2 = iaquot(y2*100 + 99, 36525);
  Int y1 = y2 - iaquot(36525*x2, 100);
  Int x1 = iaquot(5*y1 + 2, 153);
  Int c0 = iaquot(x1 + 2, 12);
  *year = 100*x3 + x2 + c0;
  *month = x1 - 12*c0 + 3;
  *day = y1 - iaquot(153*x1 - 3, 5);
}
/*--------------------------------------------------------------------------*/
void CJDNtoGregorianA(Int const *CJDN, Int *date)
{
  CJDNtoGregorian(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoGregorianS(Int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoGregorian, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDNtoGregorianSA(Int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoGregorianA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoGregorian(Double CJD, Int *year, Int *month, Double *day)
{
  CJDtoCal(CJD, year, month, day, &CJDNtoGregorian);
}
/*--------------------------------------------------------------------------*/
void CJDtoGregorianA(Double const *CJD, Double *date)
{
  Int year, month;
  CJDtoGregorian(*CJD, &year, &month, &date[2]);
  date[0] = (Double) year;
  date[1] = (Double) month;
}
/*--------------------------------------------------------------------------*/
char *CJDtoGregorianS(Double CJD)
{
  return CJDto3S(CJD, CJDtoGregorian, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoGregorianSA(Double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoGregorianA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
Int GregoriantoCJDN(Int year, Int month, Int day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  Int x1;
  Int c0 = iaquot(month - 3, 12);
  Int x4 = year + c0;
  Div_t d = adiv(x4, 100);
  x1 = month - 12*c0 - 3;
  Int J1 = alinequot(d.quot, 146097, 0, 4);
  Int J2 = iaquot(d.rem*36525, 100);
  Int J3 = iaquot(x1*153 + 2, 5);
  return GREGORIAN_EPOCH - 1 + J1 + J2 + J3 + day;
}
/*--------------------------------------------------------------------------*/
void GregoriantoCJDNA(Int const *date, Int *CJDN)
{
  *CJDN = GregoriantoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Int GregorianStoCJDN(char const * date)
{
  return S3toCJDN(date, GregoriantoCJDN,
                  arraysize(Gregorian_Julian_monthnames),
                  Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void GregorianStoCJDNA(char * const * date, Int *CJDN)
{
  S3toCJDNA(date, CJDN, GregoriantoCJDNA,
            arraysize(Gregorian_Julian_monthnames),
            Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
Double GregoriantoCJD(Int year, Int month, Double day)
{
  return CaltoCJD(year, month, day, GregoriantoCJDN);
}
/*--------------------------------------------------------------------------*/
void GregoriantoCJDA(Double const *date, Double *CJD)
{
  *CJD = GregoriantoCJD((Int) date[0], (Int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Double GregorianStoCJD(char const * date)
{
  return S3toCJD(date, GregoriantoCJD,
                 arraysize(Gregorian_Julian_monthnames),
                 Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void GregorianStoCJDA(char * const * date, Double *CJD)
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
void CJDNtoJulian(Int CJDN, Int *year, Int *month, Int *day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  Int y2 = CJDN - JULIAN_EPOCH;
  Int x2 = alinequot(y2, 4, 3, 1461);
  Int z2 = y2 - alinequot(x2, 1461, 0, 4);
  Int x1 = iaquot(5*z2 + 2, 153);
  Int c0 = iaquot(x1 + 2, 12);
  *year = x2 + c0;
  *month = x1 - 12*c0 + 3;
  *day = z2 - iaquot(153*x1 - 3, 5);
}
/*--------------------------------------------------------------------------*/
void CJDNtoJulianA(Int const *CJDN, Int *date)
{
  CJDNtoJulian(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoJulianS(Int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoJulian, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDNtoJulianSA(Int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoJulianA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoJulian(Double CJD, Int *year, Int *month, Double *day)
{
  CJDtoCal(CJD, year, month, day, CJDNtoJulian);
}
/*--------------------------------------------------------------------------*/
void CJDtoJulianA(Double const *CJD, Double *date)
{
  Int year, month;
  CJDtoJulian(*CJD, &year, &month, &date[2]);
  date[0] = (Double) year;
  date[1] = (Double) month;
}
/*--------------------------------------------------------------------------*/
char *CJDtoJulianS(Double CJD)
{
  return CJDto3S(CJD, CJDtoJulian, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoJulianSA(Double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoJulianA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
Int JuliantoCJDN(Int year, Int month, Int day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  Int c0 = iaquot(month - 3, 12);
  Int J1 = alinequot(year + c0, 1461, 0, 4);
  Int J2 = iaquot(month*153 - 1836*c0 - 457, 5);
  return J1 + J2 + day + JULIAN_EPOCH - 1;
}
/*--------------------------------------------------------------------------*/
void JuliantoCJDNA(Int const *date, Int *CJDN)
{
  *CJDN = JuliantoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Int JulianStoCJDN(char const *date)
{
  return S3toCJDN(date, JuliantoCJDN,
                  arraysize(Gregorian_Julian_monthnames),
                  Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void JulianStoCJDNA(char * const *date, Int *CJDN)
{
  S3toCJDNA(date, CJDN, JuliantoCJDNA,
            arraysize(Gregorian_Julian_monthnames),
            Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
Double JuliantoCJD(Int year, Int month, Double day)
{
  return CaltoCJD(year, month, day, JuliantoCJDN);
}
/*--------------------------------------------------------------------------*/
void JuliantoCJDA(Double const *date, Double *CJD)
{
  *CJD = JuliantoCJD((Int) date[0], (Int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Double JulianStoCJD(char const *date)
{
  return S3toCJD(date, JuliantoCJD,
                 arraysize(Gregorian_Julian_monthnames),
                 Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void JulianStoCJDA(char * const *date, Double *CJD)
{
  S3toCJDA(date, CJD, JuliantoCJDA,
           arraysize(Gregorian_Julian_monthnames),
           Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/

/* COMMON CALENDAR */

/*--------------------------------------------------------------------------*/
void CJDNtoCommon(Int CJDN, Int *year, Int *month, Int *day)
{
  if (CJDN > 2299160)
    return CJDNtoGregorian(CJDN, year, month, day);
  else
    return CJDNtoJulian(CJDN, year, month, day);
}
/*--------------------------------------------------------------------------*/
void CJDNtoCommonA(Int const *CJDN, Int *date)
{
  CJDNtoCommon(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoCommonS(Int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoCommon, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDNtoCommonSA(Int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoCommonA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoCommon(Double CJD, Int *year, Int *month, Double *day)
{
  CJDtoCal(CJD, year, month, day, CJDNtoCommon);
}
/*--------------------------------------------------------------------------*/
void CJDtoCommonA(Double const *CJD, Double *date)
{
  Int year, month;
  CJDtoCommon(*CJD, &year, &month, &date[2]);
  date[0] = (Double) year;
  date[1] = (Double) month;
}
/*--------------------------------------------------------------------------*/
char *CJDtoCommonS(Double CJD)
{
  return CJDto3S(CJD, CJDtoCommon, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoCommonSA(Double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoCommonA, Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
Int CommontoCJDN(Int year, Int month, Int day)
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
void CommontoCJDNA(Int const *date, Int *CJDN)
{
  *CJDN = CommontoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Int CommonStoCJDN(char const *date)
{
  return S3toCJDN(date, CommontoCJDN,
                  arraysize(Gregorian_Julian_monthnames),
                  Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CommonStoCJDNA(char * const *date, Int *CJDN)
{
  S3toCJDNA(date, CJDN, CommontoCJDNA,
            arraysize(Gregorian_Julian_monthnames),
            Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
Double CommontoCJD(Int year, Int month, Double day)
{
  return CaltoCJD(year, month, day, CommontoCJDN);
}
/*--------------------------------------------------------------------------*/
void CommontoCJDA(Double const *date, Double *CJD)
{
  *CJD = CommontoCJD((Int) date[0], (Int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Double CommonStoCJD(char const *date)
{
  return S3toCJD(date, CommontoCJD,
                 arraysize(Gregorian_Julian_monthnames),
                 Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CommonStoCJDA(char * const *date, Double *CJD)
{
  S3toCJDA(date, CJD, CommontoCJDA,
           arraysize(Gregorian_Julian_monthnames),
           Gregorian_Julian_monthnames);
}
/*--------------------------------------------------------------------------*/

/* HEBREW CALENDAR */

/*--------------------------------------------------------------------------*/
/** The Chronological Julian Day Number of the epoch of the Hebrew calendar */
#define HEBREW_EPOCH	(347998)

static char const * const Hebrew_monthnames[] = {
  "Tishri", "Heshvan", "Kislev", "Tevet", "Shevat", "Adar", "Nisan",
  "Iyar", "Sivan", "Tammuz", "Av", "Elul"
};
/*--------------------------------------------------------------------------*/
/* The Hebrew calendar year begins with day 1 of month 7 (= New Year)
   and runs through the end of the next month 6.  The calculation year
   begins with day 1 of month 1 preceding New Year, and runs until but
   excluding the next day 1 of month 1.  The calculation year, month,
   day begin at 0. */
/*--------------------------------------------------------------------------*/
void CJDNtoHebrew(Int CJDN, Int *year, Int *month, int *day)
{
  Int Hebrew_cycm2rd(Int x1, Int x3);

  /* running day number since day 1 of month 1 preceding New Year of
     year 1 */
  /* -2147483647 ≤ CJDN ≤ 2147483647 */
  Int y4 = CJDN - HEBREW_EPOCH + 177;
  /* -2147135826 ≤ CJDN ≤ 2147483647
     -2147483647 ≤ y₄ ≤ 2147135826 */

  /* calculate the provisional running calculation month number,
     disregarding the 2nd, 3rd and 4th delays.  The formula is
     ⌊(25920y₄ + 13835)/765433⌋, but the intermediate value 25920y₄ +
     13835 overflows already if |y₄| > INT32_MAX/25920 = 82850 days =
     226 years, so we must use a roundabout calculation to get a
     bigger range.  See
     http://http://aa.quae.nl/en/reken/juliaansedag.html */
  Div_t d1 = adiv(y4, 1447);
  Int y1 = 49*d1.quot + (23*d1.quot + 25920*d1.rem + 13835)/765433;
  /* -2147135826 ≤ CJDN ≤ 2147483647
     -2147483647 ≤ y₄ ≤ 2147135826
     -72720640 ≤ y₁ ≤ 72708859 */

  /* The provisional running calculation month number might be correct
     or one too small or one too great. */

  Int x1 = iaquot(19*y1 + 17, 235); /* provisional calculation year number */
  /* -5879541 ≤ x₁ ≤ 5878588 */
  Int x3 = y1 - iaquot(235*x1 + 1, 19); /* provisional calculation
					   month number */
  /* calculate the running day number of the beginning of calculation
     month x3 of calculation year x1 */
  Int c4 = Hebrew_cycm2rd(x1, x3);
  /* -5879539 ≤ x₁ ≤ 5878588 */

  Int z4 = y4 - c4;		/* provisional calculation day */
  if (z4 < 0 || z4 > 28) {	/* the month number may be wrong */
    if (z4 < 0) {	 /* the provisional month was one too great */
      --y1;
      x1 = iaquot(19*y1 + 17, 235);
      x3 = y1 - iaquot(235*x1 + 1, 19);
      c4 = Hebrew_cycm2rd(x1, x3);
      z4 = y4 - c4;
    } else {			/* the provisional month may be one too small */
      Int y1b = y1 + 1;
      Int x1b = iaquot(19*y1b + 17, 235);
      Int x3b = y1b - iaquot(235*x1b + 1, 19);
      Int c4b = Hebrew_cycm2rd(x1b, x3b);
      Int z4b = y4 - c4b;
      if (z4b == 0) {		/* provisional month y1 was indeed one too small */
	x1 = x1b;
	x3 = x3b;
	z4 = z4b;
      }	/* else y1 was already correct after all */
    }
  } /* else y1 was already correct */
  /* translate from calculation year/month/day to calendar
     year/month/day */
  Int c0 = (12 - x3)/7;
  *year = x1 + 1 - c0;
  *month = x3 + 1;
  *day = z4 + 1;
}
/*--------------------------------------------------------------------------*/
void CJDNtoHebrewA(Int const *CJDN, Int *date)
{
  CJDNtoHebrew(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoHebrewS(Int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoHebrew, Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDNtoHebrewSA(Int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoHebrewA, Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoHebrew(Double CJD, Int *year, Int *month, Double *day)
{
  CJDtoCal(CJD, year, month, day, CJDNtoHebrew);
}
/*--------------------------------------------------------------------------*/
void CJDtoHebrewA(Double const *CJD, Double *date)
{
  Int year, month;
  CJDtoHebrew(*CJD, &year, &month, &date[2]);
  date[0] = (Double) year;
  date[1] = (Double) month;
}
/*--------------------------------------------------------------------------*/
char *CJDtoHebrewS(Double CJD)
{
  return CJDto3S(CJD, CJDtoHebrew, Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoHebrewSA(Double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoHebrewA, Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
Int tishri2(Int x1)
/* returns the number of days between New Year (= the first day of
   month Tishri) of calculation year 0 and New Year of calculation
   year x₁, including the first two delays.  To avoid overflow in the
   final result, we must have -5879540 ≤ x₁ ≤ 5879541 -- but this is
   not checked.  LS 24dec2012 */
{
  /* calculate the running month number of calculation month 0 of
     calculation year x₁; i.e., the number of months between
     calculation month 0 of calculation year 0 and calculation month 0
     of calculation year x₁ */
  /* -2147483647 ≤ x₁ ≤ 2147483647 */
  Int c1 = iaquot(235*x1 + 1, 19);
  /* -173626337 ≤ c₁ ≤ 173626337
     -2147483642 ≤ x₁ ≤ 2147483641 */

  /* calculate the number of days between 1 Tishri A.M. 1 and the
     first day of running month c₁, taking into account the first
     delay.  The formula is ⌊(765433c₁ + 12084)/25920⌋ = 29c₁ +
     ⌊(13753c₁ + 12084)/25920⌋, but the intermediate value 765433c₁ +
     12084 overflows already if |c₁| > INT32_MAX/765433 ≈ 2805 months
     ≈ 226 years, and intermediate value 13753c₁ + 12084 overflows
     already if |c₁| > INT32_MAX/13753 ≈ 156146 months ≈ 12624 years,
     which are much less than the repeat period of the calendar, which
     is 8527680 months = 689472 years.  We use a roundabout
     calculation to get a bigger range.  See
     http://http://aa.quae.nl/en/reken/juliaansedag.html */
  Div_t d = adiv(c1, 1095);
  Int nu1 = 32336*d.quot + iaquot(15*d.quot + 765433*d.rem + 12084, 25920);
  /* -2147483321 ≤ υ₁ ≤ 2147483646
     -72720627 ≤ c₁ ≤ 72720638
     -5879540 ≤ x₁ ≤ 5879541 */

  /* calculate the number of days between 1 Tishri A.M. 1 and the
     first day of running month c₁, taking into account the first
     and second delays. */
  Int nu2 = nu1 + iaquot(6*iamod(nu1, 7), 7)%2;
  /* -2147483321 ≤ υ₂ ≤ 2147483647
     -2147483321 ≤ υ₁ ≤ 2147483646
     -72720627 ≤ c₁ ≤ 72720638
     -5879540 ≤ x₁ ≤ 5879541 */
  return nu2;
}
/*--------------------------------------------------------------------------*/
Int Hebrew_cycm2rd(Int x1, Int x3)
/* returns the running day number of the first day of Hebrew
   calculation year <x1>, calculation month <x3>, relative to the
   first day of calculation year 0, calculation month 0.  To avoid
   overflow, we must have -5879539 ≤ x₁ ≤ 5879539 -- but this is not
   checked. */
{
  /* calculate the running day number (since New Year of calculation
     year 0) of New Year of calculation year x₁ - 1, taking into
     account only the first two delays */
  Int nu2m = tishri2(x1 - 1);
  /* -5879539 ≤ x₁ ≤ 5879541 */
  Int nu2 = tishri2(x1);	/* likewise for year x₁ */
  /* -5879539 ≤ x₁ ≤ 5879541
     -2147482937 ≤ υ₂ ≤ 2147482937 */
  Int nu2p = tishri2(x1 + 1);	/* likewise for year x₁ + 1 */
  /* -5879539 ≤ x₁ ≤ 5879540 */
  Int nu2p2 = tishri2(x1 + 2);	/* likewise for year x₁ + 2 */
  /* -5879539 ≤ x₁ ≤ 5879539 */

  /* calculate the length of year x₁ - 1, taking into account only the
     first two delays */
  Int L2m = nu2 - nu2m;
  Int L2 = nu2p - nu2;		/* likewise for year x₁ */
  Int L2p = nu2p2 - nu2p;	/* likewise for year x₁ + 1 */

  /* calculate the effect of the third delay on the length of year x₁ */
  Int v3 = (L2 == 356)? 2: 0;
  Int v3p = (L2p == 356)? 2: 0;	/* likewise for year x₁ + 1 */

  /* calculate the effect of the fourth delay on the length of year x₁ */
  Int v4 = (L2m == 382);
  Int v4p = (L2 == 382);	/* likewise for year x₁ + 1 */

  /* calculate the running day number of New Year of calculation year x₁ */
  Int c2 = nu2 + v3 + v4;
  Int c2p = nu2p + v3p + v4p;	/* likewise for year x₁ + 1 */
  Int L = c2p - c2;		/* the length of calculation year x₁ */

  /* calculate the running day number within the calendar year of the
     first day of calculation month x₃, assuming a regular year, and
     assuming 0 ≤ x₃ ≤ 12 */
  Int c3 = (384*x3 + 7)/13;
  /* 0 ≤ c₃ ≤ 355 */

  /* the lengths of calendar months 8 and 9 depend on the length of
     the year */
  if (x3 > 7)
    c3 += ((L + 7)/2)%15;	/* adjustment for length of calendar month 8 */
  if (x3 > 8)
    c3 -= ((385 - L)/2)%15;	/* adjustment for length of calendar month 9 */
  /* 0 ≤ c₃ ≤ 356 */

  return c2 + c3;
  /* -2147482937 ≤ result ≤ 2147483293 */
}
/*--------------------------------------------------------------------------*/
Int HebrewtoCJDN(int year, int month, int day)
/* assumed: 1 ≤ month ≤ 13; no overflow occurs for years between
   -5879540 and 5878588, inclusive */
{
  /* The calendar year runs from calendar day 1 of calendar month 7 (=
     New Year) through the last calendar day of calendar month 6.  The
     calculation year runs from calendar day 1 of calendar month 1
     through the last day of the last calendar month (12 or 13).  New
     Year of a certain calendar year is the first day of the
     calculation year with the same number.  The calculation year,
     month, day start counting at 0.

     Translate (calendar) year, month, day into calculation year,
     calculation month, calculation day. */
  Int c0 = (13 - month)/7;	/* assumption: 1 ≤ month ≤ 13  */
  Int x1 = year - 1 + c0;	/* calculation year */
  Int x3 = month - 1;		/* calculation month */
  Int z4 = day - 1;		/* calculation day */
  /* Calculate the running day number of the first day of calculation
     year x₁, calculation month x₃, relative to calendar day 1 of
     calendar month 1 preceding New Year (calendar day 1 of calendar
     day 7) of year A.M. 1 */
  Int rd = Hebrew_cycm2rd(x1, x3);
  return rd + HEBREW_EPOCH - 177 + z4; /* CJDN */
}
/*--------------------------------------------------------------------------*/
void HebrewtoCJDNA(Int const *date, Int *CJDN)
{
  *CJDN = HebrewtoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Int HebrewStoCJDN(char const *date)
{
  return S3toCJDN(date, HebrewtoCJDN,
                  arraysize(Hebrew_monthnames),
                  Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
void HebrewStoCJDNA(char * const * date, Int *CJDN)
{
  S3toCJDNA(date, CJDN, HebrewtoCJDNA,
            arraysize(Hebrew_monthnames),
            Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
Double HebrewtoCJD(Int year, Int month, Double day)
{
  return CaltoCJD(year, month, day, HebrewtoCJDN);
}
/*--------------------------------------------------------------------------*/
void HebrewtoCJDA(Double const *date, Double *CJD)
{
  *CJD = HebrewtoCJD((Int) date[0], (Int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Double HebrewStoCJD(char const *date)
{
  return S3toCJD(date, HebrewtoCJD,
                 arraysize(Hebrew_monthnames),
                 Hebrew_monthnames);
}
/*--------------------------------------------------------------------------*/
void HebrewStoCJDA(char * const *date, Double *CJD)
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
void CJDNtoIslamic(Int CJDN, Int *year, Int *month, Int *day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  /* "Alfonsine tables", s = 14, J₀ = 1948440 */
  Int y2 = CJDN - ISLAMIC_EPOCH;
  *year = alinequot(y2, 30, 10646, 10631);
  Int z2 = y2 - alinequot(*year, 10631, -10617, 30);
  *month = iaquot(11*z2 + 330, 325);
  *day = z2 - iaquot(325**month - 331, 11);
}
/*--------------------------------------------------------------------------*/
void CJDNtoIslamicA(Int const *CJDN, Int *date)
{
  CJDNtoIslamic(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoIslamicS(Int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoIslamic, Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDNtoIslamicSA(Int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoIslamicA, Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoIslamic(Double CJD, Int *year, Int *month, Double *day)
{
  CJDtoCal(CJD, year, month, day, CJDNtoIslamic);
}
/*--------------------------------------------------------------------------*/
void CJDtoIslamicA(Double const *CJD, Double *date)
{
  Int year, month;
  CJDtoIslamic(*CJD, &year, &month, &date[2]);
  date[0] = (Double) year;
  date[1] = (Double) month;
}
/*--------------------------------------------------------------------------*/
char *CJDtoIslamicS(Double CJD)
{
  return CJDto3S(CJD, CJDtoIslamic, Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoIslamicSA(Double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoIslamicA, Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
Int IslamictoCJDN(Int year, Int month, Int day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  return alinequot(year, 10631, -10617, 30)
    + iaquot(325*month - 320, 11) + day + ISLAMIC_EPOCH - 1;
}
/*--------------------------------------------------------------------------*/
void IslamictoCJDNA(Int const *date, Int *CJDN)
{
  *CJDN = IslamictoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Int IslamicStoCJDN(char const *date)
{
  return S3toCJDN(date, IslamictoCJDN,
                  arraysize(Islamic_monthnames),
                  Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
void IslamicStoCJDNA(char * const *date, Int *CJDN)
{
  S3toCJDNA(date, CJDN, IslamictoCJDNA,
            arraysize(Islamic_monthnames),
            Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
Double IslamictoCJD(Int year, Int month, Double day)
{
  return CaltoCJD(year, month, day, IslamictoCJDN);
}
/*--------------------------------------------------------------------------*/
void IslamictoCJDA(Double const *date, Double *CJD)
{
  *CJD = IslamictoCJD((Int) date[0], (Int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Double IslamicStoCJD(char const *date)
{
  return S3toCJD(date, IslamictoCJD,
                 arraysize(Islamic_monthnames),
                 Islamic_monthnames);
}
/*--------------------------------------------------------------------------*/
void IslamicStoCJDA(char * const *date, Double *CJD)
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
void CJDNtoEgyptian(Int CJDN, Int *year, Int *month, Int *day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  /* era of Nabonassar */
  Int y2 = CJDN - EGYPTIAN_EPOCH;
  Div_t d = adiv(y2, 365);
  *year = d.quot + 1;
  d = adiv(d.rem, 30);
  *month = d.quot + 1;
  *day = d.rem + 1;
}
/*--------------------------------------------------------------------------*/
void CJDNtoEgyptianA(Int const *CJDN, Int *date)
{
  CJDNtoEgyptian(*CJDN, &date[0], &date[1], &date[2]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoEgyptianS(Int CJDN)
{
  return CJDNto3S(CJDN, CJDNtoEgyptian, Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDNtoEgyptianSA(Int const *CJDN, char **date)
{
  CJDNto3SA(CJDN, date, CJDNtoEgyptianA, Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoEgyptian(Double CJD, Int *year, Int *month, Double *day)
{
  CJDtoCal(CJD, year, month, day, CJDNtoEgyptian);
}
/*--------------------------------------------------------------------------*/
void CJDtoEgyptianA(Double const *CJD, Double *date)
{
  Int year, month;
  CJDtoEgyptian(*CJD, &year, &month, &date[2]);
  date[0] = (Double) year;
  date[1] = (Double) month;
}
/*--------------------------------------------------------------------------*/
char *CJDtoEgyptianS(Double CJD)
{
  return CJDto3S(CJD, CJDtoEgyptian, Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
void CJDtoEgyptianSA(Double const *CJD, char **date)
{
  CJDto3SA(CJD, date, CJDtoEgyptianA, Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
Int EgyptiantoCJDN(Int year, Int month, Int day)
{
  /* This algorithm by Louis Strous
     (http://aa.quae.nl/en/reken/juliaansedag.html) */
  return 365*year + 30*month + day + EGYPTIAN_EPOCH - 365 - 30 - 1;
}
/*--------------------------------------------------------------------------*/
void EgyptiantoCJDNA(Int const *date, Int *CJDN)
{
  *CJDN = EgyptiantoCJDN(date[0], date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Int EgyptianStoCJDN(char const *date)
{
  return S3toCJDN(date, EgyptiantoCJDN,
                  arraysize(Egyptian_monthnames),
                  Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
void EgyptianStoCJDNA(char * const *date, Int *CJDN)
{
  S3toCJDNA(date, CJDN, EgyptiantoCJDNA,
            arraysize(Egyptian_monthnames),
            Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
Double EgyptiantoCJD(Int year, Int month, Double day)
{
  return CaltoCJD(year, month, day, EgyptiantoCJDN);
}
/*--------------------------------------------------------------------------*/
void EgyptiantoCJDA(Double const *date, Double *CJD)
{
  *CJD = EgyptiantoCJD((Int) date[0], (Int) date[1], date[2]);
}
/*--------------------------------------------------------------------------*/
Double EgyptianStoCJD(char const *date)
{
  return S3toCJD(date, EgyptiantoCJD,
                 arraysize(Egyptian_monthnames),
                 Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/
void EgyptianStoCJDA(char * const *date, Double *CJD)
{
  S3toCJDA(date, CJD, EgyptiantoCJDA,
           arraysize(Egyptian_monthnames),
           Egyptian_monthnames);
}
/*--------------------------------------------------------------------------*/

/* JULIAN DAY */

/*--------------------------------------------------------------------------*/
Double JDtoCJD(Double JD)
{
  return JD + 0.5;
}
/*--------------------------------------------------------------------------*/
void JDtoCJDA(Double const *JD, Double *CJD)
{
  *CJD = JDtoCJD(*JD);
}
/*--------------------------------------------------------------------------*/
Double CJDtoJD(Double CJD)
{
  return CJD - 0.5;
}
/*--------------------------------------------------------------------------*/
void CJDtoJDA(Double const *CJD, Double *JD)
{
  *JD = CJDtoJD(*CJD);
}
/*--------------------------------------------------------------------------*/
void CJDNtoCJDNA(Int const *in, Int *out)
{
  *out = *in;
}
/*--------------------------------------------------------------------------*/
void CJDtoCJDA(Double const *in, Double *out)
{
  *out = *in;
}
/*--------------------------------------------------------------------------*/

/* LUNAR CALENDAR */

/*--------------------------------------------------------------------------*/
Double CJDtoLunar(Double CJD)
{
  Double	k, CJD2;

  k = floor((CJDtoJD(CJD) - 2451550.09765)/29.530588853) + 83017;
  CJD2 = LunartoCJD(k);
  k += (CJD - CJD2)/29.530588853;
  return k;
}
/*--------------------------------------------------------------------------*/
void CJDtoLunarA(Double const *CJD, Double *lunar)
{
  *lunar = CJDtoLunar(*CJD);
}
/*--------------------------------------------------------------------------*/
Double LunartoCJD(Double lunar)
{
  Double	T, k;

  k = lunar - 83017;
  T = k/1236.85;
  return JDtoCJD(2451550.09765) + 29.530588853*k
    + T*T*(1.337e-4 + T*(-1.5e-7 + 7.3e-10*T));
}
/*--------------------------------------------------------------------------*/
void LunartoCJDA(Double const *lunar, Double *CJD)
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
void CJDNtoMayan(Int CJDN, Int *trecena, Int *venteina,
                 Int *haab_day, Int *haab_month,
                 Int *year_trecena, Int *year_venteina)
{
  *trecena = iamod(CJDN + 5, 13) + 1;
  *venteina = iamod(CJDN + 16, 20) + 1;
  Div_t d = adiv(CJDN + 65, 365);
  *haab_day = iamod(d.rem, 20);
  *haab_month = d.rem/20 + 1;
  *year_trecena = iamod(d.quot + 5, 13) + 1;
  *year_venteina = iamod(5*d.quot + 11, 20) + 1;
}
/*--------------------------------------------------------------------------*/
void CJDNtoMayanA(Int const *CJDN, Int *date)
{
  CJDNtoMayan(*CJDN, &date[0], &date[1], &date[2], &date[3], &date[4],
              &date[5]);
}
/*--------------------------------------------------------------------------*/
char *CJDNtoMayanS(Int CJDN)
{
  Int trecena, venteina, haab_day, haab_month, year_trecena, year_venteina;
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
void CJDNtoMayanSA(Int const *CJDN, char **date)
{
  Int trecena, venteina, haab_day, haab_month, year_trecena, year_venteina;

  CJDNtoMayan(*CJDN, &trecena, &venteina, &haab_day, &haab_month,
              &year_trecena, &year_venteina);
  if (asprintf(date, "%d %s %d %s (%d %s)", trecena,
               Mayan_venteina_names[venteina - 1], haab_day,
               Mayan_haab_names[haab_month - 1], year_trecena,
               Mayan_venteina_names[year_venteina - 1]) < 0)
    *date = NULL;
}
/*--------------------------------------------------------------------------*/
void CJDtoMayan(Double CJD, Int *trecena, Int *venteina,
                Double *haab_day, Int *haab_month,
                Int *year_trecena, Int *year_venteina)
{
  Int ihaab_day, CJDN;
  Double frac;

  CJDN = (Int) floor(CJD);
  frac = CJD - CJDN;
  CJDNtoMayan(CJDN, trecena, venteina, &ihaab_day, haab_month, year_trecena,
              year_venteina);
  *haab_day = ihaab_day + frac;
}
/*--------------------------------------------------------------------------*/
void CJDtoMayanA(Double const *CJD, Double *date)
{
  Int CJDN, datec[6];
  Double frac;

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
char *CJDtoMayanS(Double CJD)
{
  Int trecena, venteina, haab_month, year_trecena, year_venteina;
  Double haab_day;
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
void CJDtoMayanSA(Double const *CJD, char **date)
{
  *date = CJDtoMayanS(*CJD);
}
/*--------------------------------------------------------------------------*/
Int MayantoCJDN(Int trecena, Int venteina, Int haab_day, Int haab_month,
                Int year_trecena, Int year_venteina, Int CJDN_upper)
{
  Int do_tzolkin = (trecena && venteina);
  Int do_haab = (haab_month != 0);
  Int do_year = (year_trecena && year_venteina);
  Int haab, tzolkin, year, haab_year;

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
      Int tzolkin_haab = iamod(365*tzolkin - 364*haab, 18980);
      return CJDN_upper - iamod(CJDN_upper - tzolkin_haab, 18980);
    }
  case 4:                       /* year */
    return CJDN_upper - iamod(CJDN_upper - haab_year, 18980);
  case 5:                       /* tzolkin & year */
    {
      Int CJDN_tzolkin = CJDN_upper - iamod(CJDN_upper - tzolkin, 260);
      Int CJDN_haab_year = CJDN_upper - iamod(CJDN_upper - haab_year, 18980);
      /* now CJDN_tzolkin is the last matching tzolkin on/before
         CJDN_upper, and CJDN_haab_year is the last matching beginning of
         the haab year on/before CJDN_upper, but CJDN_tzolkin may
         still be before CJDN_haab_year */
      return CJDN_tzolkin + (18980-260)*iaquot(CJDN_tzolkin - CJDN_haab_year, 260);
    }
  case 6:                       /* haab & year */
    {
      Int CJDN_haab = CJDN_upper - iamod(CJDN_upper - haab, 365);
      Int CJDN_haab_year = CJDN_upper - iamod(CJDN_upper - haab_year, 18980);
      return CJDN_haab + (18980-365)*iaquot(CJDN_haab - CJDN_haab_year, 365);
    }
  case 7:                       /* tzolkin & haab & year */
    {
      Int CJDN_tzolkin = CJDN_upper - iamod(CJDN_upper - tzolkin, 260);
      Int CJDN_haab_year = CJDN_upper - iamod(CJDN_upper - haab_year, 18980);
      Int CJDN = CJDN_tzolkin + (18980-260)*iaquot(CJDN_tzolkin - CJDN_haab_year, 260);
      Int CJDN_haab = CJDN_upper - iamod(CJDN_upper - haab, 365);
      return CJDN == CJDN_haab? CJDN: 0;
    }
    break;
  }
  return 0;
}
/*--------------------------------------------------------------------------*/
void MayantoCJDNA(Int const *date, Int *CJDN)
{
  *CJDN = MayantoCJDN(date[0], date[1], date[2], date[3], date[4], date[5],
                      date[6]);
}
/*--------------------------------------------------------------------------*/
static void MayanSto(char const *date, Int *CJDN, Double *CJD)
{
  Int n, nc, datepos, inumber;
  Double number, haab_day;
  char *name;
  size_t i;
  Int trecena, venteina = -1, haab_month = -1,
    trecena_year, venteina_year = -1;
  enum state {
    BUSY, OK, BAD
  } state = BUSY;
  Int CJDN_upper;
  
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
Int MayanStoCJDN(char const *date)
{
  Int CJDN;

  MayanSto(date, &CJDN, NULL);
  return CJDN;
}
/*--------------------------------------------------------------------------*/
void MayanStoCJDNA(char * const *date, Int *CJDN)
{
  MayanSto(*date, CJDN, NULL);
}
/*--------------------------------------------------------------------------*/
Double MayantoCJD(Int trecena, Int venteina, Double haab_day, Int haab_month,
                  Int year_trecena, Int year_venteina, Int CJDN_upper)
{
  Int ihaab_day = floor(haab_day);
  Double frac = haab_day - ihaab_day;
  Int CJDN = MayantoCJDN(trecena, venteina, ihaab_day, haab_month,
                         year_trecena, year_venteina, CJDN_upper);
  return CJDN + frac;
}
/*--------------------------------------------------------------------------*/
void MayantoCJDA(Double const *date, Double *CJD)
{
  *CJD = MayantoCJD(floor(date[0]), floor(date[1]), date[2], floor(date[3]),
                    floor(date[4]), floor(date[5]), floor(date[6]));
}
/*--------------------------------------------------------------------------*/
Double MayanStoCJD(char const *date)
{
  Double CJD;
  
  MayanSto(date, NULL, &CJD);
  return CJD;
}
/*--------------------------------------------------------------------------*/
void MayanStoCJDA(char * const *date, Double *CJD)
{
  MayanSto(*date, NULL, CJD);
}
/*--------------------------------------------------------------------------*/

/* MAYAN LONG COUNT CALENDAR */

#define LONGCOUNT_EPOCH (584283)
static Int longcount_periods[] = { 20, 20, 18, 20 };

void CJDNtoLongCount(Int CJDN, Int *baktun, Int *katun, Int *tun, Int *uinal,
                     Int *kin)
{
  Int d;

  d = CJDN - LONGCOUNT_EPOCH;
  Div_t x = adiv(d, longcount_periods[3]);
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
void CJDNtoLongCountA(Int const *CJDN, Int *longcount)
{
  Int i;
  Div_t x;

  x.quot = *CJDN;
  for (i = arraysize(longcount_periods) - 1; i >= 0; i--) {
    x = adiv(x.quot, longcount_periods[i]);
    longcount[i + 1] = x.rem;
  }
  longcount[0] = x.quot;
}
/*--------------------------------------------------------------------------*/
char *CJDNtoLongCountS(Int CJDN)
{
  Int baktun, katun, tun, uinal, kin;
  char *date;

  CJDNtoLongCount(CJDN, &baktun, &katun, &tun, &uinal, &kin);
  if (asprintf(&date, "%d.%d.%d.%d.%d", baktun, katun, tun, uinal, kin) < 0)
    date = NULL;
  return date;
}
/*--------------------------------------------------------------------------*/
void CJDNtoLongCountSA(Int const *CJDN, char **date)
{
  Int baktun, katun, tun, uinal, kin;

  CJDNtoLongCount(*CJDN, &baktun, &katun, &tun, &uinal, &kin);
  if (asprintf(date, "%d.%d.%d.%d.%d", baktun, katun, tun, uinal, kin) < 0)
    *date = NULL;
}
/*--------------------------------------------------------------------------*/
void CJDtoLongCount(Double CJD, Int *baktun, Int *katun, Int *tun, Int *uinal,
                    Double *kin)
{
  Int d;
  Double frac;
  Div_t x;

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
void CJDtoLongCountA(Double const *CJD, Double *longcount)
{
  Int i;
  Int d = floor(*CJD);
  Double frac = *CJD - d;
  for (i = arraysize(longcount_periods) - 1; i >= 0; i--) {
    Div_t x = adiv(d, longcount_periods[i]);
    d = x.quot;
    longcount[i + 1] = x.rem;
  }
  longcount[0] = d + frac;
}
/*--------------------------------------------------------------------------*/
char *CJDtoLongCountS(Double CJD)
{
  return CJDNtoLongCountS(floor(CJD));
}
/*--------------------------------------------------------------------------*/
void CJDtoLongCountSA(Double const *CJD, char **date)
{
  Int CJDN = floor(*CJD);
  CJDNtoLongCountSA(&CJDN, date);
}
/*--------------------------------------------------------------------------*/
Int LongCounttoCJDN(Int baktun, Int katun, Int tun, Int uinal, Int kin)
{
  Int d;

  d = baktun;
  d = longcount_periods[0]*d + katun;
  d = longcount_periods[1]*d + tun;
  d = longcount_periods[2]*d + uinal;
  d = longcount_periods[3]*d + kin;
  return d + LONGCOUNT_EPOCH;
}
/*--------------------------------------------------------------------------*/
void LongCounttoCJDNA(Int const *longcount, Int *CJDN)
{
  *CJDN = LongCounttoCJDN(longcount[0], longcount[1], longcount[2],
                          longcount[3], longcount[4]);
}
/*--------------------------------------------------------------------------*/
Int LongCountStoCJDN(char const *date)
{
  Int datec[5] = { 0, 0, 0, 0, 0 }, i, value;
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
void LongCountStoCJDNA(char * const *date, Int *CJDN)
{
  *CJDN = LongCountStoCJDN(*date);
}
/*--------------------------------------------------------------------------*/
Double LongCounttoCJD(Int baktun, Int katun, Int tun, Int uinal, Double kin)
{
  Int ikin = floor(kin);
  Double frac = kin - ikin;
  return LongCounttoCJDN(baktun, katun, tun, uinal, ikin) + frac;
}
/*--------------------------------------------------------------------------*/
void LongCounttoCJDA(Double const *date, Double *CJD)
{
  *CJD = LongCounttoCJD(floor(date[0]), floor(date[1]), floor(date[2]),
                        floor(date[3]), date[4]);
}
/*--------------------------------------------------------------------------*/
Double LongCountStoCJD(char const *date)
{
  return LongCountStoCJDN(date);
}
/*--------------------------------------------------------------------------*/
void LongCountStoCJDA(char * const * date, Double *CJD)
{
  Int CJDN;

  LongCountStoCJDNA(date, &CJDN);
  *CJD = (Double) CJDN;
}
/*--------------------------------------------------------------------------*/