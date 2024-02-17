/* This is file astron.cc.

Copyright 2013-2020 Louis Strous

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

/** \file
  LUX routines for calculating various astronomical
   ephemerides and for transforming dates between various calendars.

   Includes formulas from "Astronomical Algorithms" by Jean Meeus,
   Willmann-Bell, Inc. (1991) ("JM"), from "Explanatory Supplement to
   the Astronomical Almanac", edited by P. K. Seidelmann, University
   Science Books (1992) ("ES").

   Note on time systems:

   - UTC = "Universal Coordinated Time": basis of everyday time; local
         official time is commonly equal to UTC plus a fixed amount,
         usually an integer number of hours.  UTC differs from TAI by
         an integral number of seconds.  UTC days may differ in length
         from 86400 seconds because of leap seconds that are inserted
         irregularly and infrequently to keep time in step with the
         Earth's rotation.

   - UT1 = "Universal Time 1": time sequence linked to the Earth's
         rotation by (slight) variation of the length of its second.
         UTC leap seconds are inserted such that UT1 never differs
         more than 0.9 seconds from UTC.

   - TAI = "International Atomic Time": a time sequence without
         regards to the Earth's rotation, without leap seconds.

   - TDT = "Terrestrial Dynamical Time": a time sequence introduced as
         independent variable in calculations of planetary motion.  It
         is currently defined equal to TAI + 32.184 seconds.

   - GMT = "Greenwich Mean Time": historically, the official time of
         Great Britain.  Currently used as a synonym of UTC.

   UTC leap seconds are introduced such that the sequence of UTC
   second markers is 23:59:59 23:59:60 00:00:00.

   The return format of time() in <time.h> is left unspecified in the
   2nd edition of Kernighan & Richie's C manual, but SVr4, SVID,
   POSIX, X/OPEN, and BSD 4.3 define the return value to be equal to
   the number of seconds since 00:00:00 GMT on 1 January 1970.
   However, UTC (taken as a synonym of GMT) is rendered irregular by
   the occasional leap seconds, whose introduction is dependent on the
   unpredictable irregularities in the rotation of the Earth.  These
   irregular adjustments cannot conveniently be part of any standard.
   Still, it is UTC that local times, including the time system on
   most computers, are based on -- also because a reliable source of
   TAI is not generally available to the average computer.  It
   therefore seems to me that the return value of time() is in fact
   not equal to the number of elapsed seconds since 0 UTC on 1 January
   1970, but rather equal to the number of elapsed *non-leap* seconds
   since 0 UT on 1 January 1970.
*/

#include <algorithm>
#include <limits>

#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"

#include "AstronomicalConstants.hh"
#include "Ellipsoid.hh"
#include "Rotate3d.hh"
#include "action.hh"
#include "astrodat2.hh"
#include "astrodat3.hh"
#include "astron.hh"
#include "calendar.hh"
#include "replacements.h"       // if sincos is missing
#include "vsop.hh"

#define extractbits(value, base, bits) (((value) >> (base)) & ((1 << (bits)) - 1))

#define S_ECLIPTICAL    (1)
#define S_EQUATORIAL    (2)
#define S_HORIZONTAL    (3)
#define S_ELONGATION    (4)
#define S_COORDS        (7)
#define S_XYZ           (1<<3) // 8
#define S_LIGHTTIME     (1<<4) // 16
#define S_DATE          (1<<5) // 32
#define S_TDT           (1<<6) // 64
#define S_ABERRATION    (1<<8) // 256
#define S_NUTATION      (1<<9) // 512
#define S_QELEMENTS     (1<<10) // 1024
#define S_FK5           (1<<11) // 2048
#define S_TRUNCATEVSOP  (1<<12) // 4096
#define S_CONJSPREAD    (1<<13) // 8192
#define S_PLANETOCENTRIC (1<<14) // 16384
#define S_KEEPDIMS      (1<<15) // 32768
#define S_VOCAL         (1<<16) // 65536
#define S_VSOP87A       (0)
#define S_VSOP87C       (1<<17) // 131072
#define S_VSOP          (S_VSOP87A|S_VSOP87C)
#define S_BARE          (1<<18) // 262144
#define AUtoJD          ((149.597870691e9/299792458)/86400)
#define NOBJECTS        9

#define EARTH           3

const double AU_m = 149597870700.;

int32_t         findint(int32_t, int32_t *, int32_t);
void    UTC_to_TAI(double *), TAI_to_UTC(double *);
void    CJDLT_to_TAI(double *), TAI_to_CJDLT(double *);

char const* GregorianMonths[] = {
  "January", "February", "March", "April", "May", "June", "July",
  "August", "September", "October", "November", "December"
};

char const* IslamicMonths[] = {
  "Muharram", "Safar", "Rabi`a I", "Rabi`a II", "Jumada I", "Jumada II",
  "Rajab", "Sha`ban", "Ramadan", "Shawwal", "Dhu al-Q`adah",
  "Dhu al-Hijjah"
};

char const* tikalVenteina[] = {
  "Imix'", "Ik'", "Ak'b'al", "K'an", "Chikchan", "Kimi", "Manik'", "Lamat",
  "Muluk", "Ok", "Chuwen", "Eb'", "B'en", "Ix", "Men", "Kib'", "Kab'an",
  "Etz'nab'", "Kawak", "Ajaw"
};

char const* tikalMonth[] = {
  "Pop", "Wo", "Sip", "Sotz'", "Sek", "Xul", "Yaxk'in", "Mol", "Ch'en",
  "Yax", "Sac", "Keh", "Mak", "K'ank'in", "Muwan", "Pax", "K'ayab'", "Kumk'u",
  "Wayeb'"
};

char const* HebrewMonths[] = {
  "Tishri", "Heshvan", "Kislev", "Tevet", "Shevat", "Adar", "Nisan",
  "Iyar", "Sivan", "Tammuz", "Av", "Elul"
};

char const* EgyptianMonths[] = {
  "Thoth", "Phaophi", "Athyr", "Choiak", "Tybi", "Mecheir", "Phamenoth",
  "Pharmuthi", "Pachon", "Payni", "Epiphi", "Mesore", "epagomenai"
};

int32_t         nonae[] = {
  5, 5, 7, 5, 7, 5, 7, 5, 5, 7, 5, 5
}, idus[] = {
  13, 13, 15, 13, 15, 13, 15, 13, 13, 15, 13, 13
};
char const* latinMonthsI[] = {
  "Ianuariis", "Februariis", "Martiis", "Aprilibus", "Maiis", "Iuniis",
  "Iuliis", "Augustis", "Septembribus", "Octobribus", "Novembribus",
  "Decembribus"
}, *latinMonthsII[] = {
  "Ianuarias", "Februarias", "Martias", "Apriles", "Maias", "Iunias",
  "Iulias", "Augustas", "Septembres", "Octobres", "Novembres", "Decembres"
};

char const** calendars[] = {
  GregorianMonths, IslamicMonths, HebrewMonths, EgyptianMonths
};
int32_t calendarType[] = {
  CAL_COMMON, CAL_ISLAMIC, CAL_HEBREW, CAL_EGYPTIAN
};

// 2nd-degree polynomial evaluation
#define pol2(a0,a1,a2,t) ((a0) + (t)*((a1) + (t)*(a2)))

// 3rd-degree polynomial evaluation
#define pol3(a0,a1,a2,a3,t) ((a0) + (t)*((a1) + (t)*((a2) + (t)*(a3))))

// 4th-degree polynomial evaluation
#define pol4(a0,a1,a2,a3,a4,t) (a0 + t*(a1 + t*(a2 + t*(a3 + t*a4))))

double JDE(double, int32_t);
// time in centuries since epoch 1900.0
#define TC1900(JD) ((JD - 2415020.0)/36525)
// time in centuries since epoch 2000.0
#define TC2000(JD) ((JD - 2451545.0)/36525)
// time in millennia since epoch 2000.0
#define TM2000(JD) ((JD - 2451545.0)/365250)

// 2nd-degree polynomial evaluation (modulo)
#define mpol2(a0,a1,a2,t,n) (fmod(a0 + t*(a1 + t*a2),n))

// 3rd-degree polynomial evaluation (modulo)
#define mpol3(a0,a1,a2,a3,t,n) (fmod(a0 + t*(a1 + t*(a2 + t*a3)),n))

// 4th-degree polynomial evaluation (modulo)
#define mpol4(a0,a1,a2,a3,a4,t,n) (fmod(a0 + t*(a1 + t*(a2 + t*(a3 + t*a4))),n))

#define J2000   (2451545.0)
#define B1950   (2433282.4235)

#define ecos    0.9174369451    // cos(ecliptic2000)
#define esin    0.3978812030    // sin(ecliptic2000)

static int32_t  *extraIDs;

struct orbitParams {
  double        JDE;
  double        q;
  double        e;
  double        v_factor;
  double        xfac;
  double        yfac;
  double        zfac;
  double        xangle;
  double        yangle;
  double        zangle;
  double        M;
  double        n;
};

static struct extraInfo {
  int32_t               nterms;
  double        equinox;
  double        absmag;
  char          *comment;
  struct orbitParams *orbits;
} *extraOrbits = NULL;

static int32_t  nExtraObj = 0;

static double   extraElements[9];
static char     haveExtraElements;

extern int32_t getAstronError;
extern int32_t fullVSOP;

void LBRtoXYZ(double *pos, double *pos2);
void XYZtoLBR(double *, double *);
#if HAVE_LIBGSL
void XYZ_eclipticPrecession(double *pos, double equinox1, double equinox2);
#endif

int32_t idiv(int32_t x, int32_t y)
     // returns the largest integer n such that x >= y*n
{
  return (int32_t) floor(((float) x)/y);
}

void printXYZtoLBR(double *xyz);
void printLBRtoXYZ(double *lbr);
void printHBRtoXYZ(double *lbr);
void showraddms(char const* prefix, double x);
void showradhms(char const * prefix, double x);

#define TAI_to_TT(jd)   (*(jd) += 32.184/86400)
#define TT_to_TAI(jd)   (*(jd) -= 32.184/86400)

// Calendars: (all dates in the Gregorian calendar (from 15 October 1582)
//            or the Julian proleptic calendar (before 15 October 1582),
//            unless indicated otherwise).  The year preceding Year 1
//            is called Year 0, and preceding that is Year -1 (astronomical
//            reckoning).
// Gregorian: the liturgical calendar of the Christian faith, and the civil
//            calendar of many countries in the world.  It was installed as
//            liturgical calendar by Pope Gregory XIII on 15 October 1582
//            (following 4 October 1582 on the Julian calendar).  The start
//            of the calendar was fixed by Dionysius Exiguus in the 6th
//            century.
// Islamic (civil): the religious calendar of Islam is based on the first
//            sighting of the lunar crescent after New Moon.  For civil use,
//            various tabulated calenders have been used.  The one used here
//            is the "civil" one most often used by historians, in
//            which A.H. 1 Muharram 1 corresponds to 16 July 622.  If the
//            astronomical epoch is preferred, then 1 Muharram 1 corresponds
//            to 15 July 622.
// Julian Proleptic: a calendar which differs slightly from the Gregorian
//            in the occurrence of leap years.  Devised by Julius Caesar
//            in or just after -45.  Used by historians with extension
//            into the past (proleptic).
//--------------------------------------------------------------------------
double meanTropicalYear(double JD)
     // the mean length of the tropical year, in days, as a function of
     // Julian Date JD(TDT) [ES12.11-1,Laskar1986].
     // The time from one vernal equinox to the next may vary from this
     // mean by several minutes.
{
  double        T;

  T = (JD - 2451545.0)/36525;
  return pol3(365.2421896698, -6.15359e-6, -7.29e-10, 2.64e-10, T);
}
//--------------------------------------------------------------------------
double meanSynodicMonth(double JD)
     // the mean length of the synodic month, in days, as a function of
     // Julian Date JD(TDT) [ES12.11-2,Chapront-Touz\'e&Chapront1988].
     // Any particular phase cycle may vary from the mean by up to
     // seven hours.
{
  double        T;

  T = (JD - 2451545.0)/36525;
  return pol2(29.5305888531, 2.1621e-7, -3.64e-10, T);
}
//--------------------------------------------------------------------------
int32_t EasterDate(int32_t year, int32_t *month, int32_t *day)
     // the date of Easter in year y (Julian until 1582, Gregorian after
     // 1582) is returned in *month and *day. [ES12.22,Oudin1940; JM4]
{
  int32_t       c, n, k, i, j, l;

  if (year < 1)
    puts("The current algorithm may not work for non-positive years!");
  if (year < 1582) {
    c = year - 4*(year/4);
    n = year - 7*(year/7);
    k = year - 19*(year/19);
    i = 19*k + 15;
    i = i - 30*(i/30);
    n = 2*c + 4*n - i + 34;
    n = n - 7*(n/7);
    n = i + n + 114;
    *month = n/31;
    *day = n - 31* *month;
  } else {
    c = year/100;
    n = year - 19*(year/19);
    k = (c - 17)/25;
    i = c - c/4 - (c - k)/3 + 19*n + 15;
    i = i - 30*(i/30);
    i = i - (i/28)*(1 - (i/28)*(29/(i + 1))*((21 - n)/11));
    j = year + year/4 + i + 2 - c + c/4;
    j = j - 7*(j/7);
    l = i - j;
    *month = 3 + (l + 40)/44;
    *day = l + 28 - 31*(*month/4);
  }
  return 1;
}
//--------------------------------------------------------------------------
double lunarToJD(double k)
{
  double        T;

  k -= 83017;
  T = k/1236.85;
  return 2451550.09765 + 29.530588853*k
    + T*T*(1.337e-4 + T*(-1.5e-7 + 7.3e-10*T));
}
//--------------------------------------------------------------------------
double JDtoLunar(double JD)
{
  double        k, JD2;

  k = floor((JD - 2451550.09765)/29.530588853) + 83017;
  JD2 = lunarToJD(k);
  k += (JD - JD2)/29.530588853;
  return k;
}
//--------------------------------------------------------------------------
int32_t DatetoCJDN(int32_t year, int32_t month, int32_t day, int32_t calendar)
/* Calculated the Chronological Julian Day Number at the given
   calendar date.  "calendar" indicates the calendar that year, month,
   and date are in.  possible values for calendar: CAL_GREGORIAN
   (Gregorian, started 15 October 1582); CAL_ISLAMIC (civil calendar);
   CAL_JULIAN (Julian proleptic calendar). */
{
  static int32_t (*f[])(int32_t, int32_t, int32_t) =
    { NULL, CommontoCJDN, GregoriantoCJDN, IslamictoCJDN,
      JuliantoCJDN, HebrewtoCJDN, EgyptiantoCJDN };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f)
      || !f[calendar])
    return 0;

  return f[calendar](year, month, day);
}
//--------------------------------------------------------------------------
double DatetoCJD(int32_t year, int32_t month, double day, int32_t calendar)
/* Calculates the Chronological Julian Date at the given date.
   "calendar" indicates the calendar that year, month, and date are
   in.  possible values for calendar: CAL_GREGORIAN (Gregorian,
   started 15 October 1582); CAL_ISLAMIC (civil calendar); CAL_JULIAN
   (Julian proleptic calendar).  The Chronological Julian Date is
   returned in the same time base as its arguments (TAI, UTC, TT, or
   something else altogether) */
{
  static double (*f[])(int32_t, int32_t, double) =
    { NULL, CommontoCJD, GregoriantoCJD, IslamictoCJD,
      JuliantoCJD, HebrewtoCJD, EgyptiantoCJD };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f)
      || !f[calendar])
    return 0.0;

  return f[calendar](year, month, day);
}
//--------------------------------------------------------------------------
void CJDNtoDate(int32_t CJDN, int32_t *year, int32_t *month, int32_t *day, int32_t calendar)
     // returns the date corresponding to Julian Date JD.
     // possible values for calendar (see JulianDate): CAL_GREGORIAN,
     // CAL_ISLAMIC, CAL_JULIAN, CAL_HEBREW, CAL_EGYPTIAN, CAL_COMMON
{
  static void (*f[])(int32_t, int32_t *, int32_t *, int32_t *) =
    { CJDNtoCommon, CJDNtoGregorian, CJDNtoIslamic,
      CJDNtoJulian, CJDNtoHebrew, CJDNtoEgyptian };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f))
    *year = *month = *day = 0;
  else
    f[calendar](CJDN, year, month, day);
}
//--------------------------------------------------------------------------
void CJDtoDate(double CJD, int32_t *year, int32_t *month, double *day, int32_t calendar)
     /* returns the date corresponding to Chronological Julian Day
        CJD.  possible values for calendar (see JulianDate):
        CAL_GREGORIAN, CAL_ISLAMIC, CAL_JULIAN, CAL_HEBREW,
        CAL_EGYPTIAN, CAL_COMMON */
{
  static void (*f[])(double, int32_t *, int32_t *, double *) =
    { CJDtoCommon, CJDtoGregorian, CJDtoIslamic,
      CJDtoJulian, CJDtoHebrew, CJDtoEgyptian };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f)
      || !f[calendar])
    *year = *month = *day = 0;
  else
    f[calendar](CJD, year, month, day);
}
//--------------------------------------------------------------------------
void findTextDate(char *text, int32_t *year, int32_t *month, double *day, int32_t *cal,
                  int32_t order)
// interpret the <text> as a date string in the calendar indicated by
// <*cal> (e.g., CAL_HEBREW).  If <*cal> is CAL_DEFAULT, then the
// used calendar is deduced from (the start of) the month name.  <order>
// indicates the order of the year and day numbers, it must be either
// CAL_YMD or CAL_DMY. The
// deduced calendar is returned in <*cal>, and the year, month number,
// and day are returned in <*year>, <*month>, and <*day>, respectively.
{
  int32_t       i, j, nnum = 0, ntext = 0, c;
  char  *daystring = NULL, *monthstring = NULL, *yearstring = NULL, *text0;

  // first we seek the starts of the day, month, and year.  We assume that
  // the day and year are numerical, and the month is in text form.
  // If <order> == CAL_DMY, then we assume that the first number is the day
  // and the second number is the year; if <order> == CAL_YMD, then
  // the first number is the
  // year and the second number is the day.  The month string may come
  // anywhere before, between, or after the numbers.
  text0 = text;
  while (isspace((int32_t) *text))      // skip whitespace
    text++;
  do {
    if (*text == '-' || *text == '+' || isdigit((int32_t) *text)) {     // a number
      switch (nnum) {
        case 0:                         // the first number
          switch (order) {
            case CAL_DMY:
              daystring = text;         // take it for the day
              break;
            case CAL_YMD:
              yearstring = text; // take it for the year
              break;
          }
          break;
        case 1:                         // the second number
          switch (order) {
            case CAL_DMY:
              yearstring = text; // take it for the year
              break;
            case CAL_YMD:       // take it for the day
              daystring = text;
              break;
          }
          break;
      }
      nnum++;
      while (*text && !isspace((int32_t) *text)) // find next whitespace
        text++;
      while (*text && isspace((int32_t) *text)) // skip it
        text++;
    } else {                    // a non-number
      switch (ntext) {
        case 0:                         // take it for the month name
          monthstring = text;
          break;
      }
      ntext++;
      text++;
      do {
        while (*text && !isdigit((int32_t) *text) && *text != '-' && *text != '+')
          text++;
        if (!isspace((int32_t) text[-1]))
          while (*text && !isspace((int32_t) *text))
            text++;
      } while (*text
               && (!(isdigit((int32_t) *text) || *text == '+' || *text == '-')
                   || !isspace((int32_t) text[-1])));
    }
  } while (*text);

  if (yearstring) {             // have a year number
    *year = atof(yearstring);
    if (daystring)
      *day = atof(daystring);
    else
      *day = 1;
  } else {
    if (daystring) {
      *year = atof(daystring);
      *day = 1;
    } else {
      *day = 1;
      *year = 1;
    }
  }
  if (monthstring) {
    // we must temporarily terminate the month string, or else any following
    // numbers (day, year) may be interpreted as part of the month name
    if (daystring > monthstring) {
      // the day number follows the month name; we seek the whitespace
      // after the last character of the month name
      text = daystring;
      while (text > text0 && isspace((int32_t) text[-1]))
        text--;
    } else if (yearstring > monthstring) {
      text = yearstring;
      while (text > text0 && isspace((int32_t) text[-1]))
        text--;
    }
    c = *text;
    *text = '\0';
    if (*cal == CAL_DEFAULT) {
      for (j = 0; j < 4; j++) {
        for (i = 0; i < 12; i++)
          if (!strncasecmp(monthstring, calendars[j][i], 3)) {
            *cal = calendarType[j];
            break;
          }
        if (*cal != CAL_DEFAULT)
          break;
      }
    }
    switch (*cal) {
      case CAL_COMMON: case CAL_GREGORIAN: case CAL_JULIAN:
        for (i = 0; i < 12; i++)
          if (!strncasecmp(monthstring, GregorianMonths[i], 3)) {
            *month = i + 1;
            break;
          }
        if (i == 12) {          // didn't find it
          printf("WARNING - couldn't find \"%s\" in the Common calendar.\n",
                 text0);
          *month = 1;
        }
        break;
      case CAL_EGYPTIAN:
        for (i = 0; i < 12; i++)
          if (!strncasecmp(monthstring, EgyptianMonths[i], 4)) {
            *month = i + 1;
            break;
          }
        if (i == 12) {          // didn't find it
          printf("WARNING - couldn't find \"%s\" in the Egyptian calendar.\n",
                 text0);
          *month = 1;
        }
        break;
      case CAL_ISLAMIC:
        for (i = 0; i < 12; i++)
          if (!strncasecmp(monthstring, IslamicMonths[i], 9)) {
            *month = i + 1;
            break;
          }
        break;
        if (i == 12) {          // didn't find it
          printf("WARNING - couldn't find \"%s\" in the Islamic calendar.\n",
                 text0);
          *month = 1;
        }
      case CAL_HEBREW:
        for (i = 0; i < 12; i++)
          if (!strncasecmp(monthstring, HebrewMonths[i], 2)) {
            *month = i + 1;
            break;
          }
        if (i == 12) {          // didn't find it
          printf("WARNING - couldn't find \"%s\" in the Hebrew calendar.\n",
                 text0);
          *month = 1;
        }
        break;
      default:
        if (*cal == CAL_DEFAULT)
          printf("WARNING - don't know what calendar \"%s\" belongs to.\n",
                 text0);
        else
          printf("No text parsing for calendar %1d\n", *cal);
        *year = 1;
        *month = 1;
        *day = 1.0;
        return;
    }
    // now we must restore the original string
    if (daystring > monthstring || yearstring > monthstring)
      *text = c;
  } else
    *month = 1;
}
//--------------------------------------------------------------------------
void roman_numeral(int32_t number)
/* Stores at curScrat the number in Roman numeral form if it is positive and
   less than 4,000, otherwise in regular digital form. */
{
  char  *p;
  int32_t       i;

  if (number <= 0 || number >= 4000) {
    sprintf(curScrat, "%1d", number);
    return;
  }
  p = curScrat;
  if (number >= 1000) {
    i = number/1000;
    number -= 1000*i;
    while (i--)
      *p++ = 'M';
  }
  if (number >= 900) {
    *p++ = 'C';
    *p++ = 'M';
    number -= 900;
  }
  if (number >= 500) {
    *p++ = 'D';
    number -= 500;
  }
  if (number >= 400) {
    *p++ = 'C';
    *p++ = 'D';
    number -= 400;
  }
  if (number >= 100) {
    i = number/100;
    number -= 100*i;
    while (i--)
      *p++ = 'C';
  }
  if (number >= 90) {
    *p++ = 'X';
    *p++ = 'C';
    number -= 90;
  }
  if (number >= 50) {
    *p++ = 'L';
    number -= 50;
  }
  if (number >= 40) {
    *p++ = 'X';
    *p++ = 'L';
    number -= 40;
  }
  if (number >= 10) {
    i = number/10;
    number -= 10*i;
    while (i--)
      *p++ = 'X';
  }
  if (number == 9) {
    *p++ = 'I';
    *p++ = 'X';
    number -= 9;
  }
  if (number >= 5) {
    *p++ = 'V';
    number -= 5;
  }
  if (number == 4) {
    *p++ = 'I';
    *p++ = 'V';
    number -= 4;
  }
  if (number > 0) {
    while (number--)
      *p++ = 'I';
  }
  *p++ = '\0';
}
//--------------------------------------------------------------------------
int32_t construct_output_dims(int32_t *input_dims, int32_t num_input_dims,
                          int32_t inputs_per_entry,
                          int32_t *output_dims, int32_t output_dims_size,
                          int32_t outputs_per_entry)
/* constructs a list of output dimensions for cases where the number of
   inputs per entry and the number of outputs per entry may differ.
   All dimensions for which there is no room in the output are combined
   with the last dimension of the output for which there is room.
   input_dims = pointer to list of dimensions of the input
   num_input_dims = number of dimensions in the input
   inputs_per_entry = number of adjacent input values that define one entry
   output_dims = pointer to preallocated list of dimensions for the output
   output_dims_size = number of dimensions for which there is room
      in output_dims
   outputs_per_entry = number of adjacent output values produced for each entry
   RETURNS the number of dimensions stored in the output, or a negative
   value if an error occurred.
   */
{
  int32_t num_output_dims = 0;

  if (!input_dims || num_input_dims < 1 || inputs_per_entry < 1
      || !output_dims || output_dims_size < 1 || outputs_per_entry < 1)
    // illegal values
    return -1;
  if (input_dims[0] % inputs_per_entry != 0)
    // first input dimension doesn't fit a whole number of entries
    return -2;

  if (outputs_per_entry > 1) {
    // put the output values for each entry into the first dimension
    *output_dims++ = outputs_per_entry;
    output_dims_size--;
    num_output_dims++;
  }
  if (inputs_per_entry > 1) {
    // don't need these in the output
    input_dims++;
    num_input_dims--;
  }
  while (num_input_dims && output_dims_size) {
    *output_dims++ = *input_dims++;
    num_input_dims--;
    output_dims_size--;
    num_output_dims++;
  }
  while (num_input_dims) {
    /* couldn't fit all input dimensions into the available room for
       output dimensions: combine the remaining dimensions with the
       last output dimension */
    output_dims[-1] *= *input_dims++;
    num_input_dims--;
  }
  return num_output_dims;
}
//--------------------------------------------------------------------------
int32_t gcd(int32_t a, int32_t b)
{
  int32_t c;

  if (!a || !b)
    return 1;
  if (a < 0)
    a = -a;
  if (b < 0)
    b = -b;
  if (b > a) {
    c = b; b = a; a = c;
  }
  // now a >= b > 0
  c = a % b;
  while (c) {
    a = b; b = c;
  }
  return b;
}
//--------------------------------------------------------------------------
int32_t lux_calendar(ArgumentCount narg, Symbol ps[])
{
  int32_t result, input_elem_per_date, output_elem_per_date, iq;
  int32_t *dims = NULL, ndim = 0;
  LoopInfo tgtinfo, srcinfo;
  Pointer src, tgt;
  // enum Calendar_order fromorder, toorder;
  enum Calendar fromcalendar, tocalendar;
  enum Calendar_timescale fromtime, totime;
  Symboltype inputtype, internaltype, outputtype;
  enum Calendar_outputtype outputkind;
  static struct {
    int32_t to_elements_per_date;   // translating to calendar date
    int32_t from_elements_per_date; // translating from calendar date
    void (*CJDNtoCal)(int32_t const *CJDN, int32_t *date);
    void (*CJDtoCal)(double const *CJD, double *date);
    void (*CaltoCJDN)(int32_t const *date, int32_t *CJDN);
    void (*CaltoCJD)(double const *date, double *CJD);
    void (*CJDNtoCalS)(int32_t const *CJDN, char **date);
    void (*CJDtoCalS)(double const *CJD, char **date);
    void (*CalStoCJDN)(char * const *date, int32_t *CJDN);
    void (*CalStoCJD)(char * const *date, double *CJD);
  } cal_data[] = {
    { 0, 0, NULL,             NULL,            NULL,             NULL,            NULL,              NULL,             NULL,              NULL },
    { 3, 3, CJDNtoCommonA,    CJDtoCommonA,    CommontoCJDNA,    CommontoCJDA,    CJDNtoCommonSA,    CJDtoCommonSA,    CommonStoCJDNA,    CommonStoCJDA },
    { 3, 3, CJDNtoGregorianA, CJDtoGregorianA, GregoriantoCJDNA, GregoriantoCJDA, CJDNtoGregorianSA, CJDtoGregorianSA, GregorianStoCJDNA, GregorianStoCJDA },
    { 3, 3, CJDNtoIslamicA,   CJDtoIslamicA,   IslamictoCJDNA,   IslamictoCJDA,   CJDNtoIslamicSA,   CJDtoIslamicSA,   IslamicStoCJDNA,   IslamicStoCJDA },
    { 3, 3, CJDNtoJulianA,    CJDtoJulianA,    JuliantoCJDNA,    JuliantoCJDA,    CJDNtoJulianSA,    CJDtoJulianSA,    JulianStoCJDNA,    JulianStoCJDA },
    { 3, 3, CJDNtoHebrewA,    CJDtoHebrewA,    HebrewtoCJDNA,    HebrewtoCJDA,    CJDNtoHebrewSA,    CJDtoHebrewSA,    HebrewStoCJDNA,    HebrewStoCJDA },
    { 3, 3, CJDNtoEgyptianA,  CJDtoEgyptianA,  EgyptiantoCJDNA,  EgyptiantoCJDA,  CJDNtoEgyptianSA,  CJDtoEgyptianSA,  EgyptianStoCJDNA,  EgyptianStoCJDA },
    { 1, 1, NULL,             CJDtoJDA,        NULL,             JDtoCJDA,        NULL,              NULL,             NULL,              NULL },
    { 1, 1, CJDNtoCJDNA,      CJDtoCJDA,       CJDNtoCJDNA,      CJDtoCJDA,       NULL,              NULL,             NULL,              NULL },
    { 1, 1, NULL,             CJDtoLunarA,     NULL,             LunartoCJDA,     NULL,              NULL,             NULL,              NULL },
    { 6, 7, CJDNtoMayanA,     CJDtoMayanA,     MayantoCJDNA,     MayantoCJDA,     CJDNtoMayanSA,     CJDtoMayanSA,     MayanStoCJDNA,     MayanStoCJDA },
    { 5, 5, CJDNtoLongCountA, CJDtoLongCountA, LongCounttoCJDNA, LongCounttoCJDA, CJDNtoLongCountSA, CJDtoLongCountSA, LongCountStoCJDNA, LongCountStoCJDA },
  };

  // initialize to zero
  src.v = NULL;
  tgt.v = NULL;

  fromcalendar = (Calendar) extractbits(internalMode, CAL_CALENDAR_BASE,
                                        CAL_CALENDAR_BITS);
  tocalendar = (Calendar) extractbits(internalMode,
                                      CAL_CALENDAR_BASE + CAL_CALENDAR_BITS,
                                      CAL_CALENDAR_BITS);
  // fromcalendar, tocalendar:
  //  0 : based on input
  //  1 : common
  //  2 : gregorian
  //  3 : islamic
  //  4 : julian
  //  5 : hebrew
  //  6 : egyptian
  //  7 : jd
  //  8 : cjd
  //  9 : lunar
  // 10 : mayan
  // 11 : longcount
  // 12 : latin

  outputkind = (Calendar_outputtype) extractbits(internalMode,
                                                 CAL_OUTPUT_BASE,
                                                 CAL_OUTPUT_BITS);
  // outputkind:
  //  0 : numeric
  //  1 : integer
  //  2 : double
  //  3 : text

  fromtime = (Calendar_timescale) extractbits(internalMode, CAL_TIME_BASE,
                                              CAL_TIME_BITS);
  totime = (Calendar_timescale) extractbits(internalMode,
                                            CAL_TIME_BASE + CAL_TIME_BITS,
                                            CAL_TIME_BITS);
  // fromtime, totime:
  // 0 : UTC
  // 1 : TAI
  // 2 : TT
  // 3 : LT

  if (narg == 1) {
    numerical_or_string(ps[0], &dims, &ndim, NULL, NULL);
  }

  /* If no specific "from" calendar is specified, then assume the Common
     calendar if the input is a string or string array, or if there are 3 input
     symbols or if there is 1 input symbol and it has 3 elements in its first
     dimension.  Otherwise, assume CJD. */
  if (fromcalendar == CAL_DEFAULT) {
    if (symbolIsString(ps[0])
        || narg == 3 || (narg == 1 && dims[0] == 3))
      fromcalendar = CAL_COMMON;
    else
      fromcalendar = CAL_CJD;
  }

  void (*CaltoCJDN)(int32_t const *date, int32_t *CJDN)
    = cal_data[fromcalendar].CaltoCJDN;
  void (*CaltoCJD)(double const *date, double *CJDN)
    = cal_data[fromcalendar].CaltoCJD;
  /*
    void (*CalStoCJDN)(char * const *date, int32_t *CJDN)
    = cal_data[fromcalendar].CalStoCJDN;
  */
  void (*CalStoCJD)(char * const *date, double *CJD)
    = cal_data[fromcalendar].CalStoCJD;

  if (narg == 1) {
    iq = ps[0];
    {
      Symboltype type;

      /* if the input type is integer, then promote to LONG.  If the input type
         is floating point, then promote to DOUBLE. */
      type = symbol_type(iq);
      if (isIntegerType(type)) {
        iq = lux_long(1, &iq);
        inputtype = LUX_INT32;
      } else if (isFloatType(type)) {
        iq = lux_double(1, &iq);
        inputtype = LUX_DOUBLE;
      } else                        // must be text type
        inputtype = LUX_STRING_ARRAY;
    }

    // number of input elements expected per calendar date
    input_elem_per_date = cal_data[fromcalendar].from_elements_per_date;
    if (isStringType(inputtype))
      input_elem_per_date = 1; // strings have all elements of a date in a
                               // single string

    if (dims[0] % input_elem_per_date)
      return luxerror("Incompatible first dimension: expected a multiple "
                      "of %d but found %d", ps[0], input_elem_per_date,
                      dims[0]);

    /* If no specific "to" calendar is specified, then assume the Common
       calendar if the (effective) "from" calendar is a one-element-per-date
       calendar, and otherwise CJD */
    if (tocalendar == CAL_DEFAULT) {
      if (input_elem_per_date == 1)
        tocalendar = CAL_COMMON;
      else
        tocalendar = CAL_CJD;
    }

    if (fromcalendar == tocalendar
        && fromtime == totime)    // no conversion
      return *ps;

    void (*CJDNtoCal)(int32_t const *CJDN, int32_t *date)
      = cal_data[tocalendar].CJDNtoCal;
    void (*CJDtoCal)(double const *CJD, double *date)
      = cal_data[tocalendar].CJDtoCal;
    void (*CJDNtoCalS)(int32_t const *CJDN, char **date)
      = cal_data[tocalendar].CJDNtoCalS;
    void (*CJDtoCalS)(double const *CJD, char **date)
      = cal_data[tocalendar].CJDtoCalS;

    assert(CaltoCJDN || CaltoCJD);
    assert(CJDNtoCal || CJDtoCal);

    // number of output elements per calendar date
    output_elem_per_date = cal_data[tocalendar].to_elements_per_date;

    /* if the (promoted) input type is LONG, then the internal type is LONG if
       CaltoCJDN is not NULL, or DOUBLE if CaltoCJDN is NULL.  If CaltoCJDN is
       NULL and the number of input elements per date is not equal to 1, then
       work with a DOUBLE copy of the input symbol. */
    if (inputtype == LUX_INT32) {
      if (CaltoCJDN && fromtime == totime)
        internaltype = LUX_INT32;
      else {
        internaltype = LUX_DOUBLE;
        if (input_elem_per_date != 1) {
          iq = lux_double(1, &iq);
          inputtype = LUX_DOUBLE;
        }
      }
    }

    /* if the (promoted) input type is DOUBLE, then the internal type is DOUBLE
       if CaltoCJD is not NULL, or LONG if CaltoCJD is NULL.  If CaltoCJD is
       NULL and the number of input elements per date is not equal to 1, then
       work with a LONG ("floor") copy of the input symbol. */

    else if (inputtype == LUX_DOUBLE) {
      if (CaltoCJD || fromtime != totime)
        internaltype = LUX_DOUBLE;
      else {
        internaltype = LUX_INT32;
        if (input_elem_per_date != 1) {
          int32_t lux_floor(int32_t, int32_t *);
          iq = lux_floor(1, &iq);
          inputtype = LUX_INT32;
        }
      }
    }

    /* if the input type is a string type, then the internal type depends on the
       output kind. */
    else {
      if (outputkind == CAL_DOUBLE)
        internaltype = LUX_DOUBLE;
      else
        internaltype = LUX_INT32;
      input_elem_per_date = 1; // all input elements in a single text value
    }

    /* for numeric output, the output type is DOUBLE if CJDNtoCal is NULL (i.e.,
       a conversion routine to LONG is not available), or LONG if CJDtoCal is
       NULL (i.e., a conversion routine to DOUBLE is not available), or else is
       equal to the internal type.  For non-numerical (i.e., text) output, then
       output type indicates text.
    */
    if (outputkind == CAL_TEXT) {
      outputtype = LUX_STRING_ARRAY;
      output_elem_per_date = 1; // all date components in a single text value
    } else if (outputkind == CAL_LONG || internaltype == LUX_INT32)
      outputtype = CJDNtoCal? LUX_INT32: LUX_DOUBLE;
    else if (outputkind == CAL_DOUBLE || internaltype == LUX_DOUBLE)
      outputtype = CJDtoCal? LUX_DOUBLE: LUX_INT32;
    else                          // should not happen
      outputtype = internaltype;

    {
      int32_t more[1], less[1], nMore, nLess;

      // does the output need more elements than the input?
      nMore = nLess = 0;
      if (output_elem_per_date != input_elem_per_date) { // yes, different
        if (output_elem_per_date > 1) { // output needs more elements
          nMore = 1;                    // prefix one dimension
          more[0] = output_elem_per_date;
        }
        if (input_elem_per_date > 1) { // output needs fewer elements
          nLess = 1;
          less[0] = input_elem_per_date; // reduce 1st dimension
        }
      }

      if (standardLoopX(iq, LUX_ZERO,
                        SL_AXISCOORD
                        | SL_EACHROW,
                        &srcinfo, &src,
                        nMore, more,
                        nLess, less,
                        outputtype,
                        SL_AXISCOORD
                        | SL_EACHROW,
                        &result,
                        &tgtinfo, &tgt) < 0)
        return LUX_ERROR;
    }

    if (srcinfo.rdims[0] != input_elem_per_date) {
      /* we have a multiple of the expected number of input elements per date,
         but the first dimension must be exactly equal to the expected number
         else the standard loop ignores the excess elements.  We shift the
         excess to the second dimension. */
      int32_t dims[MAX_DIMS];

      assert(srcinfo.dims[0] % input_elem_per_date == 0);
      memcpy(dims, &srcinfo.dims[0], srcinfo.ndim*sizeof(*dims));
      if (srcinfo.ndim == 1) {    // there is only one dimension
        dims[1] = 1;              // add a 2nd dimension
        srcinfo.ndim = 2;
      }
      int32_t d = srcinfo.dims[0]/input_elem_per_date;
      dims[1] *= d;
      dims[0] /= d;
      srcinfo.setupDimensionLoop(srcinfo.ndim, dims, srcinfo.type,
                                 srcinfo.naxes, srcinfo.axes, srcinfo.data,
                                 srcinfo.mode);
    }

    /* complain if the desired type of translation is not available.  We don't
       need to check for numerical types, because we demand that at least one of
       the translations (to LONG/DOUBLE) is available, and are prepared to
       handle the case where only one is available. */
    if (inputtype == LUX_STRING_ARRAY && !CalStoCJD)
      return luxerror("Translating from STRING is not supported for"
                      " this calendar", ps[0]);

    /* complain if the desired type of translation is not available.  We
       don't need to check for numerical types, because we demand that
       at least one of the translations (from LONG/DOUBLE) is available,
       and are prepared to handle the case where only one is
       available. */
    if (outputtype == LUX_STRING_ARRAY)
      switch (internaltype) {
      case LUX_INT32:
        if (!CJDNtoCalS)
          return luxerror("Translating CJDN to STRING is not supported"
                          " for this calendar", ps[0]);
        break;
      case LUX_DOUBLE:
        if (!CJDtoCalS)
          return luxerror("Translating CJD to STRING is not supported"
                          " for this calendar", ps[0]);
        break;
      default:
        break;
      }

    // now loop over all dates to translate
    do {
      Scalar timestamp, temp;

      // translate input to CJD or CJDN
      switch (internaltype) {
      case LUX_INT32:              // translate to CJDN
        switch (inputtype) {
        case LUX_INT32:
          CaltoCJDN(src.i32, &timestamp.i32);
          src.i32 += input_elem_per_date;
          break;
        case LUX_DOUBLE: // only cases with one element per date reach here
          assert(input_elem_per_date == 1);
          temp.i32 = (int32_t) floor(*src.d); // translate from DOUBLE to LONG
          CaltoCJDN(&temp.i32, &timestamp.i32); // use LONG translation
          src.d += input_elem_per_date;
          break;
        default:
          break;
        }
        break;
      case LUX_DOUBLE:            // translate to CJD
        switch (inputtype) {
        case LUX_INT32: // only cases with one element per date reach here
          assert(input_elem_per_date == 1);
          temp.d = (double) *src.i32; // translate from LONG to DOUBLE
          CaltoCJD(&temp.d, &timestamp.d); // use DOUBLE translation
          src.i32 += input_elem_per_date;
          break;
        case LUX_DOUBLE:
          CaltoCJD(src.d, &timestamp.d);
          src.d += input_elem_per_date;
          break;
        case LUX_STRING_ARRAY: case LUX_LSTRING:
          CalStoCJD(src.sp, &timestamp.d);
          src.sp += input_elem_per_date;
          break;
        default:
          return cerror(ILL_TYPE, ps[0]);
        }
        break;
      default:
        return cerror(ILL_TYPE, ps[0]);
      }

      if (fromtime != totime) {
        switch (fromtime) {
        case CAL_UTC:
          UTC_to_TAI(&timestamp.d);
          break;
        case CAL_TAI:
          break;
        case CAL_TT:
          TT_to_TAI(&timestamp.d);
          break;
        case CAL_LT:
          CJDLT_to_TAI(&timestamp.d);
          break;
        }

        switch (totime) {
        case CAL_UTC:
          TAI_to_UTC(&timestamp.d);
          break;
        case CAL_TAI:
          break;
        case CAL_TT:
          TAI_to_TT(&timestamp.d);
          break;
        case CAL_LT:
          TAI_to_CJDLT(&timestamp.d);
          break;
        }
      }

      // translate CJD or CJDN to output
      switch (internaltype) {
      case LUX_INT32:
        switch (outputtype) {
        case LUX_INT32:
          CJDNtoCal(&timestamp.i32, tgt.i32);
          tgt.i32 += output_elem_per_date;
          break;
        case LUX_DOUBLE: // only cases with one element per date reach here
          assert(output_elem_per_date == 1);
          temp.d = (double) timestamp.i32; // translate from LONG to DOUBLE
          CJDtoCal(&temp.d, tgt.d);      // use DOUBLE translation
          tgt.d += output_elem_per_date;
          break;
        case LUX_STRING_ARRAY:
          CJDNtoCalS(&timestamp.i32, tgt.sp);
          tgt.sp += output_elem_per_date;
          break;
        default:
          break;
        }
        break;
      case LUX_DOUBLE:
        switch (outputtype) {
        case LUX_INT32:
          temp.i32 = (int32_t) floor(timestamp.d); // translate from DOUBLE to
                                                   // LONG
          CJDNtoCal(&temp.i32, tgt.i32);         // use LONG translation
          tgt.i32 += output_elem_per_date;
          break;
        case LUX_DOUBLE:
          CJDtoCal(&timestamp.d, tgt.d);
          tgt.d += output_elem_per_date;
          break;
        case LUX_STRING_ARRAY:
          CJDtoCalS(&timestamp.d, tgt.sp);
          tgt.sp += output_elem_per_date;
          break;
        default:
          break;
        }
        break;
      default:
        break;
      }
    } while (tgtinfo.advanceLoop(&tgt.ui8),
             srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
    if (!tgtinfo.loopIsAtStart())
      return luxerror("Source loop is finished but target loop is not!", ps[0]);
  } else if (narg == 3) {
    if (!symbolIsNumerical(ps[0]))
      return cerror(ONLY_A_S, ps[0]);
    if (!symbolIsNumerical(ps[1]))
      return cerror(ONLY_A_S, ps[1]);
    if (!symbolIsNumerical(ps[2]))
      return cerror(ONLY_A_S, ps[2]);
    input_elem_per_date = cal_data[fromcalendar].from_elements_per_date;
    if (input_elem_per_date != 3)
      return luxerror("Input calendar does not have 3 elements per date", 0);
    int32_t *dims_year;
    int32_t *dims_month;
    int32_t *dims_day;
    int32_t ndim_year;
    int32_t ndim_month;
    int32_t ndim_day;
    int32_t n_year;
    int32_t n_month;
    int32_t n_day;
    Pointer year;
    Pointer month;
    Pointer day;
    numerical(lux_long(1, &ps[0]), &dims_year, &ndim_year, &n_year, &year);
    numerical(lux_long(1, &ps[1]), &dims_month, &ndim_month, &n_month, &month);
    numerical(lux_long(1, &ps[2]), &dims_day, &ndim_day, &n_day, &day);
    int32_t n = std::max({n_year, n_month, n_day});
    int32_t *dims;
    int32_t ndim;
    if (n == n_year) {
      dims = dims_year;
      ndim = ndim_year;
    } else if (n == n_month) {
      dims = dims_month;
      ndim = ndim_month;
    } else if (n == n_day) {
      dims = dims_day;
      ndim = ndim_day;
    } else {
      dims = &n;
      ndim = 1;
    }
    Pointer cjdn;
    if (ndim == 1 && dims[0] == 1) {
      result = scalar_scratch(LUX_INT32);
      cjdn.i32 = &scalar_value(result).i32;
    } else {
      result = array_scratch(LUX_INT32, ndim, dims);
      cjdn.i32 = static_cast<int32_t*>(array_data(result));
    }
    int32_t ix_year = 0;
    int32_t ix_month = 0;
    int32_t ix_day = 0;
    int32_t date[3];
    while (n--) {
      date[0] = year.i32[ix_year];
      if (++ix_year == n_year)
        ix_year = 0;
      date[1] = month.i32[ix_month];
      if (++ix_month == n_month)
        ix_month = 0;
      date[2] = day.i32[ix_day];
      if (++ix_day == n_day)
        ix_day = 0;
      CaltoCJDN(date, cjdn.i32);
      ++cjdn.i32;
    }
  } else {
    return cerror(WRNG_N_ARG, 0);
  }

  return result;
}
//--------------------------------------------------------------------------
int32_t lux_calendar_OLD(ArgumentCount narg, Symbol ps[])
     // general calendar conversion routine
     // syntax: DATE2 = CALENDAR(DATE1, /FROMCALENDAR, /TOCALENDAR)
     // "/FROMCALENDAR":  /FROMCOMMON /FROMGREGORIAN /FROMISLAMIC
     /*                   /FROMJULIAN /FROMJD /FROMCJD
                          /FROMUTC /FROMTAI /FROMTT */
     // "/TOCALENDAR": /TOCOMMON /TOGREGORIAN /TOISLAMIC /TOJULIAN /TOJD
     //                /TOCJD /TOMAYAN /TOLONGCOUNT /TOUTC /TOTAI /TOTT
{
  int32_t       n, *dims, ndim, nRepeat, i, iq, cal, newDims[MAX_DIMS],
    num_newDims, year, month, nd, d, t, v, m, sec, min, hour,
    fromcalendar, tocalendar, fromtime, totime, output, fromorder, toorder,
    iday;
  Symboltype type, outtype;
  char  isFree = 0, *line;
  char const** monthNames;
  Pointer       data, JD;
  double        day;

  fromcalendar = extractbits(internalMode, CAL_CALENDAR_BASE,
                             CAL_CALENDAR_BITS);
  tocalendar = extractbits(internalMode, CAL_CALENDAR_BASE + CAL_CALENDAR_BITS,
                           CAL_CALENDAR_BITS);
  output = extractbits(internalMode, CAL_OUTPUT_BASE, CAL_OUTPUT_BITS);
  fromorder = extractbits(internalMode, CAL_ORDER_BASE, CAL_ORDER_BITS);
  toorder = extractbits(internalMode, CAL_ORDER_BASE + CAL_ORDER_BITS,
                        CAL_ORDER_BITS);
  fromtime = extractbits(internalMode, CAL_TIME_BASE, CAL_TIME_BITS);
  totime = extractbits(internalMode, CAL_TIME_BASE + CAL_TIME_BITS,
                       CAL_TIME_BITS);

  if (fromcalendar == tocalendar && fromcalendar != CAL_DEFAULT)
    return *ps;                         // no conversion
  iq = *ps;
  switch (symbol_class(iq)) {
  case LUX_SCALAR:
    n = 1;
    nRepeat = 1;
    ndim = 1;
    dims = &n;
    type = scalar_type(iq);
    if (type < LUX_INT32) {
      iq = lux_long(1, ps);
      type = LUX_INT32;
    } else if (type == LUX_FLOAT) {
      iq = lux_double(1, ps);
      type = LUX_DOUBLE;
    }
    data.ui8 = &scalar_value(iq).ui8;
    break;
  case LUX_ARRAY:
    nRepeat = array_size(iq);
    ndim = array_num_dims(iq);
    dims = array_dims(iq);
    if (symbolIsStringArray(iq) || dims[0] != 3)
      n = 1;
    else {
      n = 3;
      nRepeat /= 3;
    }
    type = array_type(iq);
    if (type < LUX_INT32) {
      iq = lux_long(1, ps);
      type = LUX_INT32;
    } else if (type == LUX_FLOAT) {
      iq = lux_double(1, ps);
      type = LUX_DOUBLE;
    }
    data.i32 = (int32_t *) array_data(iq);
    break;
  case LUX_STRING:
    nRepeat = 1;
    n = 1;
    ndim = 1;
    dims = &n;
    type = LUX_STRING_ARRAY;
    data.sp = &string_value(iq);
    break;
  default:
    return cerror(ILL_CLASS, *ps);
  }

  switch (fromcalendar) {       // source calendar
  case CAL_DEFAULT:
    /* if the first dimension has 3 elements or if it is a string
       array, then we assume it is in the common calendar; otherwise
       we assume it is measured in Julian Days */
    if (type != LUX_STRING_ARRAY) {
      if (n == 3)
        fromcalendar = cal = CAL_COMMON;
      else if (type == LUX_DOUBLE)
        fromcalendar = cal = CAL_JD;
      else
        fromcalendar = cal = CAL_CJD;
    } else
      cal = CAL_DEFAULT;
    break;
  case CAL_JD: case CAL_CJD: case CAL_LUNAR: // /FROMJD, /FROMCJD, /FROMLUNAR
    nRepeat *= n;
    n = 1;
    // fall-through
  default:
    cal = fromcalendar;
    break;
  }

  outtype = type;               // default
  if (fromcalendar == CAL_LUNAR || tocalendar == CAL_LUNAR
      || fromcalendar == CAL_JD || tocalendar == CAL_JD
      || type == LUX_STRING_ARRAY)
    outtype = LUX_DOUBLE;

  /* if outtype == LUX_INT32, then JD.i32 contains CJDN values
     if outtype == LUX_DOUBLE, then JD.d contains JD values
     CJDN = floor(JD + 0.5) */

  // temporary space for JDs
  switch (outtype) {
  case LUX_INT32:
    JD.i32 = (int32_t *) malloc(nRepeat*sizeof(int32_t));
    if (!JD.i32)
      return cerror(ALLOC_ERR, *ps);
    break;
  case LUX_DOUBLE: case LUX_TEMP_STRING:
    JD.d = (double *) malloc(nRepeat*sizeof(double));
    if (!JD.d)
      return cerror(ALLOC_ERR, *ps);
    break;
  default:
    return cerror(ILL_CLASS, ps[0]);
  }

  if (type == LUX_STRING_ARRAY) {
    assert(outtype == LUX_DOUBLE);
    switch (cal) {
    case CAL_COMMON: case CAL_DEFAULT: case CAL_GREGORIAN: case CAL_JULIAN:
      for (i = 0; i < nRepeat; i++) {
        findTextDate(*data.sp, &year, &month, &day, &cal, fromorder);
        JD.d[i] = CommontoCJD(year, month, day);
        data.sp++;
      }
      break;
    case CAL_LONGCOUNT:
      for (i = 0; i < nRepeat; i++) {
        char *p, *q;
        int32_t level = 0;
        long stride = 1;
        long d = 0, this_value = 0;

        p = data.sp[i];
        q = strchr(p,'\0');     // end of string
        while (q > p) {
          while (q > p && !isdigit(q[-1]))
            q--;
          if (q > p) {
            while (q > p && isdigit(q[-1]))
              q--;
            this_value = atol(q);
            d += this_value*stride;
            stride *= (level == 1? 18: 20);
            level++;
          }
        }
        JD.d[i] = 584282.5 + d;
      }
      break;
    default:
      return luxerror("Cannot parse text-based dates in this calendar", *ps);
    }
    cal = CAL_JD;               // signify that translation to JD is complete
  } else switch (cal) {                 // from calendar
    case CAL_COMMON: case CAL_GREGORIAN: case CAL_ISLAMIC: case CAL_JULIAN:
    case CAL_HEBREW: case CAL_EGYPTIAN:
      if (n != 3)
        return
          luxerror("Need 3 numbers (year, month, day) per calendar date!", *ps);
      break;
    case CAL_JD:                // from Julian Day
      assert(outtype == LUX_DOUBLE);
      switch (type) {
      case LUX_INT32:
        for (i = 0; i < nRepeat; i++)
          JD.d[i] = data.i32[i];
        break;
      case LUX_DOUBLE:
        memcpy(JD.d, data.d, nRepeat*sizeof(*JD.d));
        break;
      }
      break;
    case CAL_CJD:               // from Chronological Julian Day
      switch (outtype) {
      case LUX_INT32:
        switch (type) {
        case LUX_INT32:
          memcpy(JD.i32, data.i32, nRepeat*sizeof(*JD.i32));
          break;
        case LUX_DOUBLE:
          // from JD to CJDN
          for (i = 0; i < nRepeat; i++)
            JD.i32[i] = floor(JDtoCJD(data.d[i]));
          break;
        }
        break;
      case LUX_DOUBLE:
        switch (type) {
        case LUX_INT32:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = CJDtoJD((double) data.i32[i]);
          break;
        case LUX_DOUBLE:
          memcpy(JD.d, data.d, nRepeat*sizeof(*JD.d));
          break;
        }
      }
      break;
    case CAL_LUNAR:             // from lunar calendar
      assert(outtype == LUX_DOUBLE);
      switch (type) {
      case LUX_INT32:
        for (i = 0; i < nRepeat; i++)
          JD.d[i] = lunarToJD((double) data.i32[i]);
        break;
      case LUX_DOUBLE:
        for (i = 0; i < nRepeat; i++)
          JD.d[i] = lunarToJD(data.d[i]);
        break;
      }
      cal = CAL_JD;
      break;
    default:
      switch (type) {
      case LUX_INT32:
        free(JD.i32);
        break;
      case LUX_DOUBLE:
        free(JD.d);
        break;
      }
      return luxerror("Illegal source calendar specification (%1d)", 0, cal);
    }

  // go from FROM_CALENDAR to Julian Date
  if (cal != CAL_JD)            // not done yet, calculate Julian date
    switch (outtype) {
    case LUX_INT32:              // convert to CJDN
      switch (type) {
      case LUX_INT32:
        switch (fromorder) {
        case CAL_YMD:
          for (i = 0; i < nRepeat; i++)
            JD.i32[i] = DatetoCJDN(data.i32[3*i], data.i32[3*i + 1],
                                 data.i32[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.i32[i] = DatetoCJDN(data.i32[3*i + 2], data.i32[3*i + 1],
                                 data.i32[3*i], cal);
          break;
        }
        break;
      case LUX_DOUBLE:
        switch (fromorder) {
        case CAL_YMD:
          for (i = 0; i < nRepeat; i++)
            JD.i32[i] = DatetoCJDN((int32_t) data.d[3*i], (int32_t) data.d[3*i + 1],
                                 (int32_t) data.d[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.i32[i] = DatetoCJDN((int32_t) data.d[3*i + 2], (int32_t) data.d[3*i + 1],
                                 (int32_t) data.d[3*i], cal);
          break;
        }
        break;
      }
      break;
    case LUX_DOUBLE:            // convert to JD
      switch (type) {
      case LUX_INT32:
        switch (fromorder) {
        case CAL_YMD:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD(data.i32[3*i], data.i32[3*i + 1],
                                (double) data.i32[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD(data.i32[3*i + 2], data.i32[3*i + 1],
                                (double) data.i32[3*i], cal);
          break;
        }
        break;
      case LUX_DOUBLE:
        switch (fromorder) {
        case CAL_YMD:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD((int32_t) data.d[3*i], (int32_t) data.d[3*i + 1],
                                data.d[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD((int32_t) data.d[3*i + 2], (int32_t) data.d[3*i + 1],
                                data.d[3*i], cal);
          break;
        }
        break;
      }
      break;
    }

  switch (outtype) {
  case LUX_DOUBLE:
    // now do any required time base translation
    if (fromtime != totime) {   // we need some conversion
      // we go through TAI, which is a uniform time base
      switch (fromtime) {
      case CAL_UTC:             // /FROMUTC
        for (i = 0; i < nRepeat; i++)
          UTC_to_TAI(&JD.d[i]);
        break;
      case CAL_TT:              // /FROMTT
        for (i = 0; i < nRepeat; i++)
          TT_to_TAI(&JD.d[i]);
        break;
      }
      // and now to the target base
      switch (totime) {
      case CAL_UTC:             // /TOUTC
        for (i = 0; i < nRepeat; i++)
          TAI_to_UTC(&JD.d[i]);
        break;
      case CAL_TT:              // /TOTT
        for (i = 0; i < nRepeat; i++)
          TAI_to_TT(&JD.d[i]);
        break;
      }
      break;
    case LUX_INT32:                // no time translation
      break;
    }
  }

  // n = number of input values per input calendar date

  // now from JD or CJDN to the required target calendar
  if (tocalendar == CAL_DEFAULT)// default
    tocalendar = (fromcalendar == CAL_JD)? CAL_COMMON: CAL_JD;
  switch (tocalendar) {
  case CAL_COMMON:              // to common
    if (output != CAL_NUMERIC) { // text output
      if (nRepeat > 1) {
        num_newDims = construct_output_dims(dims, ndim, n,
                                            newDims, MAX_DIMS, 1);
        iq = array_scratch(LUX_TEMP_STRING, num_newDims, newDims);
        data.sp = (char **) array_data(iq);
      } else {
        iq = string_scratch(-1);
        data.sp = (char **) &string_value(iq);
      }
      ALLOCATE(line, 80, char);
    } else {                    // numeric output
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 3);
      iq = array_scratch(outtype, num_newDims, newDims);
      data.d = (double *) array_data(iq);
    }
    for (i = 0; i < nRepeat; i++) {
      if (outtype == LUX_DOUBLE)
        CJDtoDate(JD.d[i], &year, &month, &day, CAL_COMMON);
      else
        CJDNtoDate(JD.i32[i], &year, &month, &iday, CAL_COMMON);
      switch (output) {
      case 999 /* CAL_ISOTEXT */:
        if (outtype == LUX_DOUBLE) {
          sec = (day - (int32_t) day)*86400;
          min = sec/60;
          sec = sec % 60;
          hour = min/60;
          min = min % 60;
          sprintf(line,"%4d-%02d-%02dT%02d:%02d:%02d", year, month,
                  (int32_t) day, hour, min, sec);
        } else
          sprintf(line,"%4d-%02d-%02d", year, month, iday);
        *data.sp++ = strsave(line);
        break;
      case CAL_TEXT:
        switch (toorder) {
        case CAL_YMD:
          if (outtype == LUX_DOUBLE)
            sprintf(line, "%1d %s %1d", year,
                    GregorianMonths[month - 1], (int32_t) day);
          else
            sprintf(line, "%1d %s %1d", year,
                    GregorianMonths[month - 1], iday);
          break;
        case CAL_DMY:
          if (outtype == LUX_DOUBLE)
            sprintf(line, "%1d %s %1d", (int32_t) day,
                    GregorianMonths[month - 1], year);
          else
            sprintf(line, "%1d %s %1d", iday,
                    GregorianMonths[month - 1], year);
          break;
        }
        *data.sp++ = strsave(line);
        break;
      case CAL_NUMERIC:
        switch (toorder) {
        case CAL_YMD:
          if (outtype == LUX_DOUBLE) {
            *data.d++ = year;
            *data.d++ = month;
            *data.d++ = day;
          } else {
            *data.i32++ = year;
            *data.i32++ = month;
            *data.i32++ = iday;
          }
          break;
        case CAL_DMY:
          if (outtype == LUX_DOUBLE) {
            *data.d++ = day;
            *data.d++ = month;
            *data.d++ = year;
          } else {
            *data.i32++ = iday;
            *data.i32++ = month;
            *data.i32++ = year;
          }
          break;
        }
        break;
      }
    }
    if (!isFree)
      free(JD.i32);
    if (output != CAL_NUMERIC)
      free(line);
    if (symbol_class(iq) == LUX_STRING)
      symbol_memory(iq) = strlen(string_value(iq)) + 1;
    return iq;
  case CAL_GREGORIAN: case CAL_JULIAN: // to Gregorian
    monthNames = GregorianMonths;
    break;
  case CAL_ISLAMIC:             // to Islamic
    monthNames = IslamicMonths;
    break;
  case CAL_EGYPTIAN:
    monthNames = EgyptianMonths;
    break;
  case CAL_LATIN:
    /* the "Latin" calendar is equal to the /TOTEXT version of the
       Common calendar but using the Roman way of writing year numbers,
       designating days within the month, and writing month names. */
    if (nRepeat > 1) {
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(LUX_TEMP_STRING, num_newDims, newDims);
      data.sp = (char **) array_data(iq);
    } else {
      iq = string_scratch(-1);
      data.sp = (char **) &string_value(iq);
    }
    for (i = 0; i < nRepeat; i++) {
      char      *p = curScrat, type = 0;
      int32_t   l;

      if (outtype == LUX_DOUBLE) {
        CJDtoDate(JD.d[i], &year, &month, &day, CAL_COMMON);
        iday = (int32_t) day;
      } else
        CJDNtoDate(JD.i32[i], &year, &month, &iday, CAL_COMMON);
      /* first we determine the designation of the day; days are counted
         backwards from the next Nonae, Idus, or Kalendae */
      if (iday == 1) {
        strcpy(curScrat, "Kalendis ");
        curScrat += strlen(curScrat);
      } else if (iday < nonae[month - 1]) {
        l = nonae[month - 1] - iday + 1;
        if (l == 2)
          strcpy(curScrat, "Pridie");
        else {
          strcpy(curScrat, "Ante Diem ");
          curScrat += strlen(curScrat);
          roman_numeral(l);
        }
        curScrat += strlen(curScrat);
        *curScrat++ = ' ';
        strcpy(curScrat, "Nonas ");
        curScrat += strlen(curScrat);
        type = 1;
      } else if (iday == nonae[month - 1]) {
        strcpy(curScrat, "Nonis ");
        curScrat += strlen(curScrat);
      } else if (iday < idus[month - 1]) {
        l = idus[month - 1] - iday + 1;
        if (l == 2)
          strcpy(curScrat, "Pridie");
        else {
          strcpy(curScrat, "Ante Diem ");
          curScrat += strlen(curScrat);
          roman_numeral(l);
        }
        curScrat += strlen(curScrat);
        *curScrat++ = ' ';
        strcpy(curScrat, "Idus ");
        curScrat += strlen(curScrat);
        type = 1;
      } else if (iday == idus[month - 1]) {
        strcpy(curScrat, "Idibus ");
        curScrat += strlen(curScrat);
      } else {
        if (month == 12)
          l = 33 - iday;
        else
          l = DatetoCJDN(year, month + 1, 1, CAL_COMMON)
            - DatetoCJDN(year, month, 1, CAL_COMMON) + 2 - iday;
        if (l == 2)
          strcpy(curScrat, "Pridie");
        else {
          strcpy(curScrat, "Ante Diem ");
          curScrat += strlen(curScrat);
          roman_numeral(l);
        }
        curScrat += strlen(curScrat);
        *curScrat++ = ' ';
        strcpy(curScrat, "Kalendas ");
        curScrat += strlen(curScrat);
        if (++month == 13) {
          month -= 12;
          year++;
        }
        type = 1;
      }
      strcpy(curScrat,
             type? latinMonthsII[month - 1]: latinMonthsI[month - 1]);
      curScrat += strlen(curScrat);
      strcpy(curScrat, " Anno ");
      curScrat += strlen(curScrat);
      roman_numeral(year);
      *data.sp++ = strsave(p);
      curScrat = p;
    }
    if (!isFree)
      free(JD.i32);
    if (symbol_class(iq) == LUX_STRING)
      symbol_memory(iq) = strlen(data.sp[-1]) + 1;
    return iq;
    break;
  case CAL_JD:          // /TOJD
    assert(outtype == LUX_DOUBLE);
    if (nRepeat == 1) {         // need scalar
      iq = scalar_scratch(outtype);
      data.d = &scalar_value(iq).d;
    } else {                    // need array
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(outtype, num_newDims, newDims);
      data.d = (double *) array_data(iq);
    }
    memcpy(data.d, JD.d, nRepeat*sizeof(double));
    if (!isFree)
      free(JD.i32);
    return iq;
  case CAL_CJD:                 // /TOCJD
    assert(outtype == LUX_INT32);
    if (nRepeat == 1) {         // need scalar
      iq = scalar_scratch(outtype);
      data.i32 = &scalar_value(iq).i32;
    } else {                    // need array
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(outtype, num_newDims, newDims);
      data.i32 = (int32_t *) array_data(iq);
    }
    memcpy(data.i32, JD.i32, nRepeat*sizeof(*JD.i32));
    if (!isFree)
      free(JD.i32);
    return iq;
  case CAL_LUNAR:
    assert(outtype == LUX_DOUBLE);
    if (nRepeat == 1) {         // need scalar
      iq = scalar_scratch(LUX_DOUBLE);
      data.d = &scalar_value(iq).d;
    } else {                    // need array
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(LUX_DOUBLE, num_newDims, newDims);
      data.d = (double *) array_data(iq);
    }
    for (i = 0; i < nRepeat; i++)
      *data.d++ = JDtoLunar(JD.d[i]);
    if (!isFree)
      free(JD.d);
    return iq;
  case CAL_MAYAN:               // /TOMAYAN
    if (nRepeat > 1) {
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(LUX_TEMP_STRING, num_newDims, newDims);
      data.sp = (char **) array_data(iq);
    } else {
      iq = string_scratch(-1);
      data.sp = (char **) &string_value(iq);
    }
    ALLOCATE(line, 80, char);
    for (i = 0; i < nRepeat; i++) {
      if (outtype == LUX_DOUBLE)
        d = floor(JDtoCJD(JD.d[i]));
      else
        d = JD.i32[i];
      t = iamod(d + 5, 13) + 1;
      v = iamod(d + 16, 20);
      d = iamod(d + 65, 365);
      m = d/20;
      d = iamod(d, 20);
      sprintf(line, "%1d %s %1d %s", t, tikalVenteina[v], d, tikalMonth[m]);
      *data.sp++ = strsave(line);
    }
    if (!isFree)
      free(JD.i32);
    return iq;
  case CAL_LONGCOUNT:           // /TOLONGCOUNT
    if (nRepeat > 1) {
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(LUX_TEMP_STRING, num_newDims, newDims);
      data.sp = (char **) array_data(iq);
    } else {
      iq = string_scratch(-1);
      data.sp = (char **) &string_value(iq);
    }
    ALLOCATE(line, 80, char);
    for (i = 0; i < nRepeat; i++) {
      int32_t n;

      if (outtype == LUX_DOUBLE)
        d = floor(JDtoCJD(JD.d[i]) - 584283);
      else
        d = JD.i32[i] - 584283;
      n = idiv(d,20);
      t = d - n*20;
      d = idiv(n,18);
      v = n - d*18;
      n = idiv(d,20);
      m = d - n*20;
      d = idiv(n,20);
      nd = n - d*20;
      n = idiv(d,20);
      d -= n*20;
      sprintf(line, "%1d.%1d.%1d.%1d.%1d", d, nd, m, v, t);
      *data.sp++ = strsave(line);
    }
    if (!isFree)
      free(JD.i32);
    return iq;
  case CAL_HEBREW:
    monthNames = HebrewMonths;
    break;
  default:
    if (!isFree)
      free(JD.i32);
    return luxerror("Illegal target calendar specification (%1d)", 0, cal);
  }
  if (output != CAL_NUMERIC) {
    if (nRepeat > 1) {
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(LUX_TEMP_STRING, num_newDims, newDims);
      data.sp = (char **) array_data(iq);
    } else {
      iq = string_scratch(-1);
      data.sp = (char **) &string_value(iq);
    }
    ALLOCATE(line, 80, char);
  } else {
    num_newDims = construct_output_dims(dims, ndim, n,
                                        newDims, MAX_DIMS, 3);
    iq = array_scratch(outtype, num_newDims, newDims);
    data.d = (double *) array_data(iq);
  }
  for (i = 0; i < nRepeat; i++) {
    if (outtype == LUX_DOUBLE) {
      CJDtoDate(JD.d[i], &year, &month, &day, tocalendar);
      iday = (int32_t) day;
    } else
      CJDNtoDate(JD.i32[i], &year, &month, &iday, tocalendar);
    switch (output) {
    case 999 /* CAL_ISOTEXT */:
      if (outtype == LUX_DOUBLE) {
        sec = (day - (int32_t) day)*86400;
        min = sec/60;
        sec = sec % 60;
        hour = min/60;
        min = min % 60;
        sprintf(line,"%4d-%02d-%02dT%02d:%02d:%02d", year, month,
                iday, hour, min, sec);
      } else
        sprintf(line,"%4d-%02d-%02d", year, month, iday);
      *data.sp++ = strsave(line);
      break;
    case CAL_TEXT:
      switch (toorder) {
      case CAL_YMD:
        sprintf(line, "%1d %s %1d", year,
                monthNames[month - 1], iday);
        break;
      case CAL_DMY:
        sprintf(line, "%1d %s %1d", iday,
                monthNames[month - 1], year);
        break;
      }
      *data.sp++ = strsave(line);
      break;
    case CAL_NUMERIC:
      switch (outtype) {
      case LUX_DOUBLE:
        switch (toorder) {
        case CAL_YMD:
          *data.d++ = year;
          *data.d++ = month;
          *data.d++ = day;
          break;
        case CAL_DMY:
          *data.d++ = day;
          *data.d++ = month;
          *data.d++ = year;
          break;
        }
        break;
      case LUX_INT32:
        switch (toorder) {
        case CAL_YMD:
          *data.i32++ = year;
          *data.i32++ = month;
          *data.i32++ = iday;
          break;
        case CAL_DMY:
          *data.i32++ = iday;
          *data.i32++ = month;
          *data.i32++ = year;
          break;
        }
        break;
      }
      break;
    }
  }
  if (!isFree)
    free(JD.i32);
  if (output != CAL_NUMERIC)
    free(line);
  if (symbol_class(iq) == LUX_STRING)
    symbol_memory(iq) = strlen(string_value(iq)) + 1;
  return iq;
}
//--------------------------------------------------------------------------
int32_t lux_EasterDate(ArgumentCount narg, Symbol ps[])
     // returns dates of Easter in the common calendar (Gregorian-Julian).
     // syntax:  X = EASTERDATE(YEARS), where YEARS is an array.  X will
     // have three elements in its first dimension, in the order year,
     // month, day
{
  int32_t       *year, nYear, iq, *dims, nDim, newDims[MAX_DIMS], *ptr, month, day;

  switch (symbol_class(*ps))
  { case LUX_SCALAR:
      iq = lux_long(1, ps);     // make LUX_INT32
      year = &scalar_value(iq).i32;
      nYear = 1;  nDim = 0;
      break;
    case LUX_ARRAY:
      iq = lux_long(1, ps);
      year = (int32_t *) array_data(iq);
      nYear = array_size(iq);  dims = array_dims(iq);
      nDim = array_num_dims(iq);
      break;
    default:
      return cerror(ILL_CLASS, *ps); }
  // create result
  *newDims = 3;
  if (nDim) memcpy(newDims + 1, dims, nDim);
  iq = array_scratch(LUX_INT32, nDim + 1, newDims);
  ptr = (int32_t *) array_data(iq);
  while (nYear--)
  { if (EasterDate(*year, &month, &day) < 0)
      return -1;                // some error
    *ptr++ = *year;  *ptr++ = month;  *ptr++ = day; }
  return iq;
}
//--------------------------------------------------------------------------
double deltaT(double CJD)
/* returns the difference between TAI and UTC for a given CJD(UTC), in seconds.
   For UTC between 1961-01-01 and 2007-01-01. LS 24sep98 25dec06 */
{
  static double         JDs[] = {
    2437300.5, 2437512.5, 2437665.5, 2438334.5, 2438395.5, 2438486.5,
    2438639.5, 2438761.5, 2438820.5, 2438942.5, 2439004.5, 2439126.5,
    2439887.5, 2441317.5, 2441499.5, 2441683.5, 2442048.5, 2442413.5,
    2442778.5, 2443144.5, 2443509.5, 2443874.5, 2444239.5, 2444786.5,
    2445151.5, 2445516.5, 2446247.5, 2447161.5, 2447892.5, 2448257.5,
    2448804.5, 2449169.5, 2449534.5, 2450083.5, 2450630.5, 2451179.5,
    2453736.5,
  };
  static double         t0s[] = {
    2437300, 2437300, 2437665, 2437665, 2438761, 2438761,
    2438761, 2438761, 2438761, 2438761, 2438761, 2439126,
    2439126, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
  };
  static double         offsets[] = {
    1.4228180, 1.3728180, 1.8457580, 1.9458580, 3.2401300, 3.3401300,
    3.4401300, 3.5401300, 3.6401300, 3.7401300, 3.8401300, 4.3131700,
    4.2131700, 10.0, 11.0, 12.0, 13.0, 14.0,
    15.0, 16.0, 17.0, 18.0, 19.0, 20.0,
    21.0, 22.0, 23.0, 24.0, 25.0, 26.0,
    27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0,
  };
  static double         units[] = {
    0.001296, 0.001296, 0.0011232, 0.0011232, 0.001296, 0.001296,
    0.001296, 0.001296, 0.001296, 0.001296, 0.001296, 0.002592,
    0.002592, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
  };
  static int32_t        n = sizeof(JDs)/sizeof(double);
  double        T, dT;
  int32_t       lo, hi, mid;

  double JD = CJD - 0.5;
  if (JD < JDs[0]) {            // before 1961-01-01
    // do a rough estimate
    // The algorithm is based on observations between years -390
    // and +1986 [ES2.553].
    T = (JD - 2378495)/36525;   // centuries since 1800.0
    if (JD > 2314580.0)                 // year +1625
      dT = 5.156 + 13.3066*(T - 0.19)*(T - 0.19);
    else if (JD > 2067320)      // year +948
      dT = 25.5*T*T - 3.61*T - 29.3; // NOTE: I added the linear term and
    // offset to this formula to have this one and the previous one match
    // at the stated boundary of about 1625. LS 17dec95
    else
      dT = 1360 + 320*T + 44.3*T*T - 32.184;
    return dT;
  }
  mid = 0;
  lo = 0;
  hi = n - 1;
  while (lo <= hi) {
    mid = (lo + hi)/2;
    if (JD < JDs[mid])
      hi = mid - 1;
    else if (JD > JDs[mid])
      lo = mid + 1;
    else
      break;
  }

  lo = (hi < mid)? hi: mid;     // the last one from the table
  return offsets[lo] + units[lo]*(JD - t0s[lo]);
}
/* TIME SCALES:

     TAI      physically realized
      │
   (offset)   observed, nominally +32.184 seconds
      │
     TT       terrestrial time
      │
 (rate adjustment L_G) definition of TT
      │
     TCG      time scale for GCRS
      │
  (periodic terms)  iauDtdb is an implementation
      │
  (rate adjustment L_C) function of solar-system ephemeris
      │
     TCB      time scale for BCRS
      │
  (rate adjustment -L_B)
      │
     TDB      TCB scaled to track TT
      │
   (periodic terms) -iauDtdb is an implementation
      │
     TT       terrestrial time

  time_t is not a uniform timescale, because it does not count leap
  seconds.  time_t encodes the date as 86400 seconds times the number
  of calendar days since its epoch, plus 1 second for each second
  elapsed since the beginning of the calendar day. A leap second at
  the end of a calendar day gets the same time_t value as the first
  second of the following calendar day. */
//--------------------------------------------------------------------------
void UTC_to_TAI(double *CJD)
{
  *CJD += deltaT(*CJD)/86400;
}
//--------------------------------------------------------------------------
void TAI_to_UTC(double *CJD)
{
  /* deltaT(*JD) = ∆T = TAI - UTC for *JD interpreted as UTC

          UTC ∆T(UTC)      TAI
     23:59:42      17 23:59:59
     23:59:43      17 00:00:00
     23:59:59      17 00:00:16
     23:59:60      17 00:00:17
     00:00:00      18 00:00:18

     UTC_est = TAI - ∆T(TAI)

          TAI ∆T(TAI)  UTC_est ∆(UTC_est) TAI-∆(UTC_est)
     23:59:59      17 23:59:42         17 23:59:42
     00:00:00      18 23:59:42         17 23:59:43
     00:00:16      18 23:59:58         17 23:59:59
     00:00:17      18 23:59:59         17 00:00:00
     00:00:18      18 00:00:00         18 00:00:00

     TAI - ∆T(UTC_est) = UTC except for the leap second, as long as ∆T >= 0.
  */

  double UTC_est = *CJD - deltaT(*CJD)/86400;
  *CJD -= -deltaT(UTC_est)/86400;
}
//--------------------------------------------------------------------------
void CJDLT_to_TAI(double *CJD)
{
  time_t t_LT = (time_t) ((*CJD - 2440588)*86400);
  struct tm *bt = localtime(&t_LT);
  time_t t_UTC = t_LT - bt->tm_gmtoff;
  *CJD = t_UTC/(double) 86400.0 + 2440588;
  UTC_to_TAI(CJD);
}
//--------------------------------------------------------------------------
void TAI_to_CJDLT(double *CJD)
{
  TAI_to_UTC(CJD);
  time_t t_UTC = (time_t) ((*CJD - (double) 2440588)*86400);
  struct tm *bt = localtime(&t_UTC);
  time_t t_LT = t_UTC + bt->tm_gmtoff;
  *CJD = t_LT/(double) 86400.0 + 2440588;
}
//--------------------------------------------------------------------------
double JDE(double JD, int32_t direction)
     // corrects JD for changes in the rotation rate of the Earth.
     // (i.e., takes JD (UT) and returns an approximate JDE (TDT).
     // The algorithms are based on observations between years -390
     /* and +1986 [ES2.553].  if <direction> = +1, then goes from
      UT to TDT, if <direction> = -1, then goes from TDT to UT. */
{
  double        dT, T;

  T = (JD - 2378495)/36525;     // centuries since 1800.0
  if (JD > 2314580.0)           // year +1625
    dT = 5.156 + 13.3066*(T - 0.19)*(T - 0.19);
  else if (JD > 2067320)        // year +948
    dT = 25.5*T*T - 3.61*T - 29.3; // NOTE: I added the linear term and
  // offset to this formula to have this one and the previous one match
  // at the stated boundary of about 1625. LS 17dec95
  else
    dT = 1360 + 320*T + 44.3*T*T;
  return (direction < 0)? JD - dT/86400: JD + dT/86400;
}
//--------------------------------------------------------------------------
void rotate(double *pos, double angle_rad, int32_t axis)
// rotate X, Y, and Z (pos[0] through pos[2]) over the specified angle
// (in radians) along the specified axis (X = 0, etc.)
{
  switch (axis)
  {
  case 0:                       // around X axis
    Rotate3d::rotate_x(pos, angle_rad);
    break;
  case 1:                       // around Y axis
    Rotate3d::rotate_y(pos, angle_rad);
    break;
  case 2:                       // around Z axis
    Rotate3d::rotate_z(pos, angle_rad);
    break;
  }
}
//--------------------------------------------------------------------------
double  p_ce, p_se, p_pi, p_p;
void initEclipticPrecession(double JDE, double equinox)
     // initialize for precession from equinox <JDE> to equinox <equinox>
{
  double        t, eta, T;

  T = (JDE - J2000)/36525.0;
  t = (equinox - JDE)/36525.0;

  eta = t*(47.0029 + T*(-0.06603 + T*0.000598)
           + t*(-0.03302 + 0.000598*T + 0.000060*t))*DEG/3600;
  p_ce = cos(eta);
  p_se = sin(eta);
  p_pi = ((T*(3289.4789 + T*0.60622)
         + t*(-869.8089 - T*0.50491 + t*0.03536))/3600 + 174.876384)*DEG;
  p_p = t*(5029.0966 + T*(2.22226 - T*0.000042)
         + t*(1.11113 - T*0.000042 - t*0.000006))/3600*DEG;
}
//--------------------------------------------------------------------------
void eclipticPrecession(double *pos, double JDE, double equinox)
/* precess the ecliptical polar coordinates <pos> from the equinox of
   <JDE> (in JDE) to that of <equinox> (in JDE) */
{
  static double         oldJDE = DBL_MAX, oldEquinox = DBL_MAX;
  double        a, b, c, cb, sb, s;

  if (JDE != oldJDE || equinox != oldEquinox) {
    initEclipticPrecession(JDE, equinox);
    oldJDE = JDE;
    oldEquinox = equinox;
  }
  cb = cos(pos[1]);
  sb = sin(pos[1]);
  s = sin(p_pi - pos[0]);
  a = p_ce*cb*s - p_se*sb;
  b = cb*cos(p_pi - pos[0]);
  c = p_ce*sb + p_se*cb*s;
  pos[0] = p_p + p_pi - atan2(a,b);
  if (pos[0] < 0)
    pos[0] += TWOPI;
  else if (pos[0] >= TWOPI)
    pos[0] -= TWOPI;
  pos[1] = asin(c);
}
//--------------------------------------------------------------------------
void precessEquatorial(double *ra, double *dec, double JDfrom, double JDto)
/* precess the equatorial coordinates *ra (right ascension) and *dec
   (declination), both measured in radians, from the equinox of JDfrom
   to the equinox of JDto.  LS 2004may03 */
{
  static double zeta, z, theta, from = 0, to = 0;
  double A, B, C;

  if (from != JDfrom || to != JDto || !from || !to) {
    double T, t;

    T = (JDfrom - 2451545.0)/36525.0;
    t = (JDto - JDfrom)/36525.0;
    zeta = pol2(pol2(2306.2181, 1.39656, -0.000139, T),
                0.30188 - 0.000344*T, 0.017998, t)*t/3600*DEG;
    z = pol2(pol2(2306.2181, 1.39656, -0.000139, T),
             1.09468 + 0.000066*T, 0.018203, t)*t/3600*DEG;
    theta = pol2(pol2(2004.3109, -0.85330, -0.000217, T),
                 -0.42665 - 0.000217*T, -0.041833, t)*t/3600*DEG;
  }
  *ra += zeta;
  A = cos(*dec)*sin(*ra);
  B = cos(theta)*cos(*dec)*cos(*ra) - sin(theta)*sin(*dec);
  C = sin(theta)*cos(*dec)*cos(*ra) + cos(theta)*sin(*dec);
  *ra = atan2(A, B) + z;
  *dec = asin(C);
}
//--------------------------------------------------------------------------
int32_t lux_precess(ArgumentCount narg, Symbol ps[])
/* PRECESS(coords, JDfrom, JDto) precesses the equatorial coordinates
   <coords> from the equinox of <JDfrom> to <JDto>, both measured in
   Julian Days.  <coords>(0,*) is taken to contain right ascensions
   measured in degrees, and <coords>(1, *) declinations, measured in
   degrees.  LS 2004may03*/
{
  double JDfrom, JDto, alpha, delta;
  Pointer src, tgt;
  LoopInfo srcinfo, tgtinfo;
  int32_t n, result, done;
  Symboltype outtype;

  JDfrom = double_arg(ps[1]);
  JDto = double_arg(ps[2]);
  if (internalMode & 2) {
    JDfrom = 0.99997860193253*JDfrom + 0.0413818359375; // to Julian
    JDto = 0.99997860193253*JDto + 0.0413818359375; // to Julian
  }
  if (internalMode & 3) {
    JDfrom = (JDfrom - 2000.0)*365.25 + J2000;
    JDto = (JDto - 2000.0)*365.25 + J2000;
  }
  outtype = (symbol_type(ps[0]) > LUX_FLOAT)? LUX_DOUBLE: LUX_FLOAT;
  n = standardLoop(ps[0], LUX_ZERO, SL_AXISCOORD | SL_SRCUPGRADE,
                   outtype, &srcinfo, &src, &result, &tgtinfo, &tgt);
  if (n == LUX_ERROR)
    return n;
  if (srcinfo.ndim < 1) {
    zap(result);
    return cerror(NEED_ARR, ps[0]);
  }
  if (srcinfo.dims[0] < 2) {
    zap(result);
    return luxerror("Need at least 2 elements in the first dimension", ps[0]);
  }

  switch (symbol_type(ps[0])) {
  case LUX_INT8:
    do {
      alpha = *src.ui8*DEG;
      srcinfo.advanceLoop(&src.ui8);
      delta = *src.ui8*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      tgtinfo.advanceLoop(&tgt.ui8);
      *tgt.f = delta*RAD;
      do {
        done = srcinfo.advanceLoop(&src.ui8);
        tgtinfo.advanceLoop(&tgt.ui8);
        if (!done)
          *tgt.f = *src.ui8;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case LUX_INT16:
    do {
      alpha = *src.i16*DEG;
      srcinfo.advanceLoop(&src.ui8);
      delta = *src.i16*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      tgtinfo.advanceLoop(&tgt.ui8);
      *tgt.f = delta*RAD;
      do {
        done = srcinfo.advanceLoop(&src.ui8);
        tgtinfo.advanceLoop(&tgt.ui8);
        if (!done)
          *tgt.f = *src.i16;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case LUX_INT32:
    do {
      alpha = *src.i32*DEG;
      srcinfo.advanceLoop(&src.ui8);
      delta = *src.i32*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      tgtinfo.advanceLoop(&tgt.ui8);
      *tgt.f = delta*RAD;
      do {
        done = srcinfo.advanceLoop(&src.ui8);
        tgtinfo.advanceLoop(&tgt.ui8);
        if (!done)
          *tgt.f = *src.i32;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case LUX_FLOAT:
    do {
      alpha = *src.f*DEG;
      srcinfo.advanceLoop(&src.ui8);
      delta = *src.f*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      tgtinfo.advanceLoop(&tgt.ui8);
      *tgt.f = delta*RAD;
      do {
        done = srcinfo.advanceLoop(&src.ui8);
        tgtinfo.advanceLoop(&tgt.ui8);
        if (!done)
          *tgt.f = *src.f;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case LUX_DOUBLE:
    do {
      alpha = *src.d*DEG;
      srcinfo.advanceLoop(&src.ui8);
      delta = *src.d*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.d = alpha*RAD;
      tgtinfo.advanceLoop(&tgt.ui8);
      *tgt.d = delta*RAD;
      do {
        done = srcinfo.advanceLoop(&src.ui8);
        tgtinfo.advanceLoop(&tgt.ui8);
        if (!done)
          *tgt.d = *src.d;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  }
  return result;
}
//--------------------------------------------------------------------------
#include "constellations.hh"
int32_t constellation(double alpha, double delta)
/* returns the identity of the official constellation that the
   position is in that is indicated by right ascension <alpha> and
   declination <delta>, both measured in degrees relative to the
   equinox of B1875.0.  LS 2004may03
 */
{
  int32_t nb = sizeof(constellation_boundaries)/sizeof(struct constellation_struct);
  int32_t i;
  struct constellation_struct *cb = constellation_boundaries;

  if (delta < -90)
    delta = -90;
  else if (delta > 90)
    delta = +90;
  alpha = famod(alpha, 360.0);  // reduce to interval 0 - 360 degrees
  alpha /= 15;                  // from degrees to hours

  for (i = 0; i < nb && (delta < cb[i].delta || alpha >= cb[i].alpha2
                         || alpha < cb[i].alpha1); i++);
  return cb[i].constellation;
}
//--------------------------------------------------------------------------
#define B1875 (2405889.25855047) // from SOFA Epb2jd routine
int32_t lux_constellation(ArgumentCount narg, Symbol ps[])
/* CONSTELLATION(coords [, equinox, /JULIAN, /BESSELIAN]) returns the
   constellation of each set of <coords> relative to the given <equinox>.
   <coords>(0,*) are right ascensions measured in degrees.
   <coords>(1,*) are declinations measured in degrees.
   <equinox> is a Julian Date, if no keywords are specified.
   <equinox> is a Julian year (e.g., 2000.0 = J2000.0) if /JULIAN is specified.
   <equinox> is a Besselian year (e.g., 1875.0 = B1875.0) if /BESSELIAN
   is specified.  An integer number indicates each constellation.
   If <equinox> is not specified, then J2000.0 is assumed.
   LS 2004may03
 */
{
  int32_t n, result, done;
  double equinox, alpha, delta;
  LoopInfo srcinfo, tgtinfo;
  Pointer src, tgt;
  int32_t vocal;

  if (narg > 1) {
    equinox = double_arg(ps[1]);
    if (internalMode & 2)
      equinox = 0.99997860193253*equinox + 0.0413818359375; // to Julian
    if (internalMode & 3)
      equinox = (equinox - 2000.0)*365.25 + J2000;
  } else
    equinox = J2000;
  vocal = internalMode & 4;

  n = standardLoop(ps[0], LUX_ZERO, SL_AXISCOORD | SL_COMPRESS,
                   LUX_INT8, &srcinfo, &src, &result, &tgtinfo, &tgt);
  if (n == LUX_ERROR)
    return n;
  if (srcinfo.ndim < 1) {
    zap(result);
    return cerror(NEED_ARR, ps[0]);
  }
  if (srcinfo.dims[0] < 2) {
    zap(result);
    return luxerror("Need at least 2 elements in the first dimension", ps[0]);
  }

  switch (symbol_type(ps[0])) {
  case LUX_INT8:
    do {
      alpha = *src.ui8*DEG;
      srcinfo.advanceLoop(&src.ui8);
      delta = *src.ui8*DEG;
      do
        done = srcinfo.advanceLoop(&src.ui8);
      while (!done);
      if (vocal) {
        printf("CONSTELLATION (equinox %.10g)\n", equinox);
        showradhms(" alpha = ", alpha);
        showraddms(" delta = ", delta);
      }
      precessEquatorial(&alpha, &delta, equinox, B1875);
      if (vocal) {
        printf("CONSTELLATION (equinox B1875.0)\n");
        showradhms(" alpha = ", alpha);
        showraddms(" delta = ", delta);
      }
      *tgt.ui8 = constellation(alpha*RAD, delta*RAD);
      tgtinfo.advanceLoop(&tgt.ui8);
    } while (done < srcinfo.rndim);
    break;
  case LUX_INT16:
    do {
      alpha = *src.i16*DEG;
      srcinfo.advanceLoop(&src.ui8);
      delta = *src.i16*DEG;
      do
        done = srcinfo.advanceLoop(&src.ui8);
      while (!done);
      if (vocal) {
        printf("CONSTELLATION (equinox %.10g)\n", equinox);
        showradhms(" alpha = ", alpha);
        showraddms(" delta = ", delta);
      }
      precessEquatorial(&alpha, &delta, equinox, B1875);
      if (vocal) {
        printf("CONSTELLATION (equinox B1875.0)\n");
        showradhms(" alpha = ", alpha);
        showraddms(" delta = ", delta);
      }
      *tgt.ui8 = constellation(alpha*RAD, delta*RAD);
      tgtinfo.advanceLoop(&tgt.ui8);
    } while (done < srcinfo.rndim);
    break;
  case LUX_INT32:
    do {
      alpha = *src.i32*DEG;
      srcinfo.advanceLoop(&src.ui8);
      delta = *src.i32*DEG;
      do
        done = srcinfo.advanceLoop(&src.ui8);
      while (!done);
      if (vocal) {
        printf("CONSTELLATION (equinox %.10g)\n", equinox);
        showradhms(" alpha = ", alpha);
        showraddms(" delta = ", delta);
      }
      precessEquatorial(&alpha, &delta, equinox, B1875);
      if (vocal) {
        printf("CONSTELLATION (equinox B1875.0)\n");
        showradhms(" alpha = ", alpha);
        showraddms(" delta = ", delta);
      }
      *tgt.ui8 = constellation(alpha*RAD, delta*RAD);
      tgtinfo.advanceLoop(&tgt.ui8);
    } while (done < srcinfo.rndim);
    break;
  case LUX_FLOAT:
    do {
      alpha = *src.f*DEG;
      srcinfo.advanceLoop(&src.ui8);
      delta = *src.f*DEG;
      do
        done = srcinfo.advanceLoop(&src.ui8);
      while (!done);
      if (vocal) {
        printf("CONSTELLATION (equinox %.10g)\n", equinox);
        showradhms(" alpha = ", alpha);
        showraddms(" delta = ", delta);
      }
      precessEquatorial(&alpha, &delta, equinox, B1875);
      if (vocal) {
        printf("CONSTELLATION (equinox B1875.0)\n");
        showradhms(" alpha = ", alpha);
        showraddms(" delta = ", delta);
      }
      *tgt.ui8 = constellation(alpha*RAD, delta*RAD);
      tgtinfo.advanceLoop(&tgt.ui8);
    } while (done < srcinfo.rndim);
    break;
  case LUX_DOUBLE:
    do {
      alpha = *src.d*DEG;
      srcinfo.advanceLoop(&src.ui8);
      delta = *src.d*DEG;
      do
        done = srcinfo.advanceLoop(&src.ui8);
      while (!done);
      if (vocal) {
        printf("CONSTELLATION (equinox %.10g)\n", equinox);
        showradhms(" alpha = ", alpha);
        showraddms(" delta = ", delta);
      }
      precessEquatorial(&alpha, &delta, equinox, B1875);
      if (vocal) {
        printf("CONSTELLATION (equinox B1875.0)\n");
        showradhms(" alpha = ", alpha);
        showraddms(" delta = ", delta);
      }
      *tgt.ui8 = constellation(alpha*RAD, delta*RAD);
      tgtinfo.advanceLoop(&tgt.ui8);
    } while (done < srcinfo.rndim);
    break;
  }
  return result;
}
//--------------------------------------------------------------------------
int32_t lux_constellationname(ArgumentCount narg, Symbol ps[])
/* CONSTELLATIONNAME(<index>) returns the official abbreviation of the
   name of the constellation with the given <index> (may be an array),
   as returned by function CONSTELLATION.  LS 2004may03
 */
{
  int32_t result, n, nc;
  Pointer src;
  char **tgt;

  if (numerical(ps[0], NULL, NULL, &n, &src) == LUX_ERROR)
    return LUX_ERROR;
  if (n > 1) {
    result = array_clone(ps[0], LUX_STRING_ARRAY);
    tgt = (char**) array_data(result);
  } else {
    result = string_scratch(0);
    tgt = &string_value(result);
  }
  nc = sizeof(constellation_names)/sizeof(char *);
  switch (symbol_type(ps[0])) {
  case LUX_INT8:
    while (n--) {
      if (*src.ui8 >= nc)
        *tgt = strdup("***");
      else
        *tgt = strdup(constellation_names[*src.ui8]);
      tgt++;
      src.ui8++;
    }
    break;
  case LUX_INT16:
    while (n--) {
      if (*src.i16 < 0 || *src.i16 >= nc)
        *tgt = strdup("***");
      else
        *tgt = strdup(constellation_names[*src.i16]);
      tgt++;
      src.i16++;
    }
    break;
  case LUX_INT32:
    while (n--) {
      if (*src.i32 < 0 || *src.i32 >= nc)
        *tgt = strdup("***");
      else
        *tgt = strdup(constellation_names[*src.i32]);
      tgt++;
      src.i32++;
    }
    break;
  case LUX_FLOAT:
    while (n--) {
      if (*src.f < 0 || *src.f >= nc)
        *tgt = strdup("***");
      else
        *tgt = strdup(constellation_names[(int32_t) *src.f]);
      tgt++;
      src.f++;
    }
    break;
  case LUX_DOUBLE:
    while (n--) {
      if (*src.d < 0 || *src.d >= nc)
        *tgt = strdup("***");
      else
        *tgt = strdup(constellation_names[(int32_t) *src.d]);
      tgt++;
      src.d++;
    }
    break;
  }
  return result;
}
//--------------------------------------------------------------------------
float magnitude(double d, double r, double beta, int32_t objNum)
     // Estimates the visual magnitude from the square of the distance
     // to the object, the square of the distance of the object to the Sun,
     // and the phase angle in degrees
     // [ES7, 15]
{
  double        temp;
  int32_t       i;

  switch (objNum) {
    default:
      if (nExtraObj) {
        i = findint(objNum, extraIDs, nExtraObj);
        return extraOrbits[i].absmag + 5*log10(r*d);
      }
      return 0;
    case -1:                    // ELEMENTS
      if (haveExtraElements)
        return extraElements[8] + 5*log10(r*d);
      return 0;
    case 0:                     // Sun
      return -26.72 + 2.5*log10(d);
    case 1:                     // Mercury
      temp = fabs(beta)/100;
      return -0.42 + 5*log10(r*d) + temp*(3.80 + temp*(-2.73 + 2.00*temp));
    case 2:                     // Venus
      temp = fabs(beta)/100;
      return -4.40 + 5*log10(r*d) + temp*(0.09 + temp*(2.39 - 0.65*temp));
    case 3:                     // Earth
      // phase angle dependence derived from Redshift 2 planetarium program
      temp = fabs(beta)/100;
      return -3.86 + 5*log10(r*d) + temp*(0.509 + temp*1.494);
    case 4:                     // Mars
      return -1.52 + 5*log10(r*d) + 0.016*fabs(beta);
    case 5:                     // Jupiter
      return -9.40 + 5*log10(r*d) + 0.005*fabs(beta);
    case 6:                     // Saturn
      return -8.88 + 5*log10(r*d) + 0.044*fabs(beta);
    case 7:                     // Uranus
      return -7.19 + 5*log10(r*d) + 0.0028*fabs(beta);
    case 8:                     // Neptune
      // assume same beta-dependence as for Uranus
      return -6.87 + 5*log10(r*d) + 0.0028*fabs(beta);
    case 9:                     // Pluto
      return -1.01 + 5*log10(r*d) + 0.041*fabs(beta);
    case 10:                    // Moon [ES 9.342-2]
      temp = fabs(beta);
      temp *= temp;
      temp *= temp;
      return 0.21 + 5*log10(r*d) + 0.0217*fabs(beta) + 5.5e-9*temp;
  }
}
//--------------------------------------------------------------------------
char const *objectName(int32_t objNum)
{
  switch (objNum) {
    default:
      if (nExtraObj) {
        int32_t i = findint(objNum, extraIDs, nExtraObj);
        return extraOrbits[i].comment;
      }
      return "Unknown";
    case -1:                    // ELEMENTS
      return "ELEMENTS";
    case 0:                     // Sun
      return "Sun";
    case 1:                     // Mercury
      return "Mercury";
    case 2:                     // Venus
      return "Venus";
    case 3:                     // Earth
      return "Earth";
    case 4:                     // Mars
      return "Mars";
    case 5:                     // Jupiter
      return "Jupiter";
    case 6:                     // Saturn
      return "Saturn";
    case 7:                     // Uranus
      return "Uranus";
    case 8:                     // Neptune
      return "Neptune";
    case 9:                     // Pluto
      return "Pluto";
    case 10:                    // Moon [ES 9.342-2]
      return "Moon";
  }
}
//--------------------------------------------------------------------------
void nutation(double JDE, double *dPsi, double *cdPsi, double *sdPsi,
              double *dEps)
// calculates the nutation in longitude and/or obliquity
{
  double        d, m, mm, f, o, *amp, angle, T;
  int16_t       *mul;
  int32_t       i;

  T = (JDE - J2000)/36525;
  d = mpol3(297.85036, 445267.111480, -0.0019142, 1./189474, T, 360)*DEG;
  m = mpol3(357.52772, 35999.050340, -0.0001603, -1./300000, T, 360)*DEG;
  mm = mpol3(134.96298, 477198.867398, 0.0086972, 1./56250, T, 360)*DEG;
  f = mpol3(93.27191, 483202.017538, -0.0036825, 1./327270, T, 360)*DEG;
  o = mpol3(125.04452, -1934.136261, 0.0020708, 1./450000, T, 360)*DEG;
  if (dPsi)
    *dPsi = 0.0;
  if (dEps)
    *dEps = 0.0;
  mul = nutationMultiples;
  amp = nutationAmplitudes;
  if (dPsi && dEps) {
    for (i = 0; i < 63; i++) {
      angle = mul[0]*d + mul[1]*m + mul[2]*mm + mul[3]*f + mul[4]*o;
      *dPsi += (amp[0] + amp[1]*T)*4.848136811e-10*sin(angle);
      *dEps += (amp[2] + amp[3]*T)*4.848136811e-10*cos(angle);
      mul += 5;
      amp += 4;
    }
    *cdPsi = cos(*dPsi);
    *sdPsi = sin(*dPsi);
  } else if (dPsi) {
    for (i = 0; i < 63; i++) {
      angle = mul[0]*d + mul[1]*m + mul[2]*mm + mul[3]*f + mul[4]*o;
      *dPsi += (amp[0] + amp[1]*T)*4.848136811e-10*sin(angle);
      mul += 5;
      amp += 4;
    }
    *cdPsi = cos(*dPsi);
    *sdPsi = sin(*dPsi);
  } else if (dEps)
    for (i = 0; i < 63; i++) {
      angle = mul[0]*d + mul[1]*m + mul[2]*mm + mul[3]*f + mul[4]*o;
      *dEps += (amp[2] + amp[3]*T)*4.848136811e-10*cos(angle);
      mul += 5;
      amp += 4;
    }
}
//--------------------------------------------------------------------------
double obliquity(double JDE, double *dEps)
// returns the obliquity of the ecliptic: calculates nutation if
// dEps == NULL, otherwise adds *dEps.
{
  double        eps, T;

  T = (JDE - J2000)/3652500;    // 10,000 Julian years! since 2000.0
  eps = (((((((((T*2.45 + 5.79)*T + 27.87)*T + 7.12)*T - 39.05)*T - 249.67)*T
            - 51.38)*T + 1999.25)*T - 1.55)*T - 4680.93)*T*4.848136811e-6
              + 0.4090928042;
  if (!dEps)
    nutation(JDE, NULL, NULL, NULL, dEps);
  if (dEps)
    eps += *dEps;
  return eps;
}
//--------------------------------------------------------------------------
double standardSiderealTime(int32_t JD, double *dPsi, double ceps)
// returns the sidereal time at longitude zero at 0 UT of the day of which
// 12 UT corresponds to Julian Day number <JD>, in radians; mean if
// dPsi == NULL, apparent otherwise.
{
  double        c, T, jd;

  jd = floor(JD) + 0.5;
  T = (jd - J2000)/36525;       // Julian centuries
  c = pol3(0.2790572732639,100.00213903780093,1.0775926e-6,-7.176e-11,T);
  if (dPsi)
    c += *dPsi*ceps;
  c -= floor(c);
  c *= TWOPI;
  return c;
}
//--------------------------------------------------------------------------
double siderealTime(double JD, double *dPsi, double ceps)
// returns the sidereal time at longitude zero at the indicated JD (UT),
// in radians; mean if dPsi == NULL, apparent otherwise
{
  double        c, T, jd;

  jd = floor(JD) + 0.5;
  T = (jd - J2000)/36525;       // Julian centuries
  c = pol3(0.2790572732639,100.00213903780093,1.0775926e-6,-7.176e-11,T);
  c += (pol2(100.00213903780093,2*1.0775926e-6,3*-7.176e-11,T)/36525 + 1)*(JD - jd);
  if (dPsi)
    c += *dPsi*ceps;
  c -= floor(c);
  c *= TWOPI;
  return c;
}
//--------------------------------------------------------------------------
int32_t lux_siderealtime(ArgumentCount narg, Symbol ps[])
// SIDEREALTIME(<jd>) returns the mean sidereal time at the indicated
// julian dates, in hours
// LS 31mar2002
{
  double *jd, *out;
  int32_t n, iq, result;
  double dPsi, cdPsi, sdPsi, dEps, epsilon;

  switch (symbol_class(ps[0])) {
  case LUX_SCALAR:
    iq = lux_double(1, ps);
    jd = &scalar_value(iq).d;
    n = 1;
    result = scalar_scratch(LUX_DOUBLE);
    out = &scalar_value(result).d;
    break;
  case LUX_ARRAY:
    iq = lux_double(1, ps);
    jd = (double *) array_data(iq);
    n = array_size(iq);
    result = array_clone(iq, LUX_DOUBLE);
    out = (double *) array_data(result);
    break;
  default:
    return cerror(ILL_CLASS, ps[0]);
  }

  switch (internalMode & 3) {
  case 0:
    while (n--) {
      nutation(*jd, &dPsi, &cdPsi, &sdPsi, &dEps);
      epsilon = obliquity(*jd, &dEps);
      *out++ = siderealTime(*jd++, &dPsi, cos(epsilon))/TWOPI*24;
    }
    break;
  case 1:                       // /ATZEROTIME
    while (n--) {
      nutation(*jd, &dPsi, &cdPsi, &sdPsi, &dEps);
      epsilon = obliquity(*jd, &dEps);
      *out++ = standardSiderealTime(*jd++, &dPsi, cos(epsilon))/TWOPI*24;
    }
    break;
  case 2:                       // /MEAN
    while (n--)
      *out++ = siderealTime(*jd++, NULL, 0)/TWOPI*24;
    break;
  case 3:                       // /MEAN /ATZEROTIME
    while (n--)
      *out++ = standardSiderealTime(*jd++, NULL, 0)/TWOPI*24;
    break;
  }

  return result;
}
//--------------------------------------------------------------------------
void VSOPtoFK5(double T, double *pos)
/* transforms from VSOP polar coordinates to FK5 polar coordinates; T
   is in Julian centuries since J2000.0; pos[] contains polar
   coordinates (longitude, latitude, radius) in radians and AU,
   relative to the VSOP dynamical ecliptic and equinox of date */
{
  double        ll, cll, sll;

  ll = pos[0] - 0.024382*T*(1 + 0.000222*T);
  cll = cos(ll);
  sll = sin(ll);
  pos[0] += -4.3793e-7 + 1.8985e-7*(cll + sll)*tan(pos[1]);
  pos[1] += 1.8985e-7*(cll - sll);
}
//--------------------------------------------------------------------------
void XYZ_VSOPtoFK5(double T, double *pos)
/* transforms from VSOP cartesian coordinates (relative to the
   dynamical ecliptic and equinox of the date) to FK5 cartesian
   coordinates */
{
  double tpos[3];
  XYZtoLBR(pos, tpos);
  VSOPtoFK5(T, tpos);
  LBRtoXYZ(tpos, pos);
}
//--------------------------------------------------------------------------
#define EQUINOX_OF_DATE         DBL_MAX

int32_t readExtra(char const* file, char mode)
/* read orbital data for extra celestial bodies from file <file>.

 Data format:

 The first line of the file must contain the number of objects.

 For each object, the first line of the corresponding data segment
 consists of the object ID number (which should be distinct from those
 of the "standard" objects), followed by the number of data lines to
 follow, followed by the absolute magnitude, followed by an optional
 equinox for which the data are valid, and an optional indicator of
 the data format of the following data lines.  The equinox is
 specified either as a scalar, which indicates a JDE, or as a string
 consisting of a j or J (indicating Julian years), or a b or a B
 (indicating Besselian years), or a d or a D (indicating the equinox
 of the date).  If the equinox specification is absent, then J2000.0
 is assumed.  For Julian or Besselian years, a year must follow the
 initial letter.

 The data format indicator must be one of a, A, q, Q.  If the data
 format indicator is a or A or absent, then each data line for the
 current object must contain the following information: JDE a e i node
 peri M, where JDE is the Julian Date/TDT of the epoch, a is the
 semimajor axis, e is the eccentricity, i is the inclination, node is
 the longitude of the ascending node, peri is the argument of
 perihelion, and M is the mean anomaly at the epoch.  The angles are
 in degrees, the distances in AU.

 If the data format indicator is q or Q, then the data format is: JDE
 q e i node peri Tperi, where q is the perihelion distance and Tperi
 the JDE of the (last) perihelion transit of the object.

 The objects must be listed in ascending order of their ID numbers,
 and the data lines for each object in ascending order of JDE.  */
{
  FILE  *fp;
  char const* defaultFile = "$LUXDIR/orbits";
  char  orbitLine[256], *pp;
  int32_t       obj, id, rec, c, format, n, nmore, indx, nterm;
  double        jd, a, e, i, node, peri, m, equinox;
  double        sn, cn, si, ci, f, g, p, q, mag;

  if (!file)
    file = defaultFile;
  fp = fopen(expand_name(file, NULL), "r");
  if (!fp)
    return luxerror("Could not open orbital data file \"%s\".", 0, file);
  while ((c = getc(fp)) == '=') {       // comment lines
    ungetc(' ', fp);
    if (!fgets(orbitLine, 256, fp))
      return luxerror("Read error/unexpected data end in file %s", 0, file);
    if (mode & 1)       // /LIST
      printf(orbitLine);
  }
  ungetc(c, fp);
  fscanf(fp, "%d", &n);                 // number of objects in this file
  fgets(orbitLine, 256, fp);    // rest of 1st data line
  for (pp = orbitLine; isspace((uint8_t) *pp); pp++);
  if (*pp && mode & 1)
    printf(pp);

  if (extraOrbits && mode == 2) { // replace
    free(extraOrbits);
    free(extraIDs);
    nExtraObj = n;
    nmore = 0;
  } else {
    nmore = nExtraObj;
    nExtraObj += n;
  }

  if (mode == 2 || !extraOrbits) { // replace or new
    extraOrbits =
      (struct extraInfo *) malloc(nExtraObj*sizeof(struct extraInfo));
    extraIDs = (int32_t *) malloc(nExtraObj*sizeof(int32_t));
  } else {                      // merge
    extraOrbits =
      (struct extraInfo *) realloc(extraOrbits,
                                   nExtraObj*sizeof(struct extraInfo));
    extraIDs = (int32_t *) realloc(extraIDs, nExtraObj*sizeof(int32_t));
  }

  if (!extraOrbits || !extraIDs)
    return cerror(ALLOC_ERR, 0);

  for (obj = indx = 0; obj < n; obj++, indx++) {
    while ((c = getc(fp)) == '=') { // comment line
      ungetc(' ', fp);
      fgets(orbitLine, 256, fp);
      if (mode & 1)
        printf(orbitLine);
    }
    ungetc(c, fp);

    fscanf(fp, "%d %d %lf", &id, &nterm, &mag);
    while (nmore && extraIDs[indx] < id) {
      nmore--;
      indx++;
    }
    if (extraIDs[indx] == id) {         // object already exists: replace
      printf(
        "Warning - object %1d (%s) already exists.\nReplacing orbital data.\n",
        id,
        extraOrbits[indx].comment? extraOrbits[indx].comment: "no comment" );
      free(extraOrbits[indx].orbits);
      free(extraOrbits[indx].comment);
      nmore--;
    } else {                    // insert object
      memmove(extraIDs + indx + 1, extraIDs + indx, nmore*sizeof(int32_t));
      memmove(extraOrbits + indx + 1, extraOrbits + indx,
              nmore*sizeof(struct extraInfo));
    }

    extraIDs[indx] = id;
    extraOrbits[indx].nterms = nterm;
    extraOrbits[indx].absmag = mag;

                                // seek equinox specification
    format = 0;
    do
      c = getc(fp);
    while (c != EOF && c != '\n' && isspace(c));
    switch (c) {
      case 'j': case 'J':       // Julian
        extraOrbits[indx].equinox = 0;
        break;
      case 'b': case 'B':
        extraOrbits[indx].equinox = 1;
        break;
      case '\n':
        extraOrbits[indx].equinox = 2; // J2000
        break;
      case 'd': case 'D':
        extraOrbits[indx].equinox = EQUINOX_OF_DATE;
        break;
      case 'a': case 'A': case '=':
        format = 'A';
        extraOrbits[indx].equinox = 2; // J2000
        if (c == '=')
          ungetc(c, fp);
        break;
      case 'q': case 'Q':
        format = 'Q';
        extraOrbits[indx].equinox = 2; // J2000
        break;
      default:
        fclose(fp);
        return luxerror("Illegal equinox specification \"%c\" in orbital data file \"%s\".", 0, c, file);
    }
    if (extraOrbits[indx].equinox <= 1) { // read year
      fscanf(fp, "%lf", &equinox);
      /* Lieske 1979 A&A 73 282:
         JE = 2000.0 + (JED - 2451545.0)/365.25
         BE = 1900.0 + (JED - 2415020.31352)/365.242198781
         so
         JED = (BE - 1900.0)*365.242198781 + 2415020.31352
         JE = 2000.0 + ((BE - 1900.0)*365.242198781 + 2415020.31352 - 2451545.0)/365.25
         JE = 2000.0 + ((BE - 1900.0)*365.242198781 - 36524.68648)/365.25
         JE = 2000.0 + (BE - 1900)*(365.242198781/365.25) - 36524.68648/365.25
            = 0.04143966 + BE*0.99997864142642
       */
      if (extraOrbits[indx].equinox) // Besselian
        equinox = 0.99997860193253*equinox + 0.0413818359375; // to Julian
      equinox = (equinox - 2000.0)*365.25 + J2000;
      extraOrbits[indx].equinox = equinox;
    } else if (extraOrbits[indx].equinox == 2) // J2000
      extraOrbits[indx].equinox = J2000;

    if (!format) {              // not yet found format
      if (c == EOF)
        format = 'A';
      else {
        do
          c = getc(fp);
        while (c != EOF && c != '\n' && isspace(c));
        switch (c) {
          case 'a': case 'A': case '=':
            format = 'A';
            break;
          case 'q': case 'Q':
            format = 'Q';
            break;
          default:
            fclose(fp);
            return luxerror("Illegal data format specification \"%c\" in orbital data file \"%s\".", 0, c, file);
        }
      }
    }

    if (mode & 1)
      printf("%d ", extraIDs[indx]);
    if (c != '\n') {            // read comment; skip to end of line
      fgets(orbitLine, 256, fp);
      pp = orbitLine;
      while (*pp != '\n')
        pp++;
      *pp = '\0';               // remove final \n
      extraOrbits[indx].comment = strsave(orbitLine);
      if (mode & 1)
        puts(orbitLine);
    } else {
      extraOrbits[indx].comment = NULL;
      if (mode & 1)
        putchar('\n');
    }

    if (!(extraOrbits[indx].orbits = (struct orbitParams *)
          malloc(extraOrbits[indx].nterms*sizeof(struct orbitParams)))) {
      fclose(fp);
      return cerror(ALLOC_ERR, 0);
    }

    for (rec = 0; rec < extraOrbits[indx].nterms; rec++) {
      fscanf(fp, "%lf %lf %lf %lf %lf %lf %lf", &jd, &a, &e, &i, &node,
             &peri, &m);
      a = fabs(a);
      extraOrbits[indx].orbits[rec].JDE = jd;
      e = fabs(e);
      extraOrbits[indx].orbits[rec].e = e;
      extraOrbits[indx].orbits[rec].q = (format == 'A')? fabs(a*(1 - e)): a;
      extraOrbits[indx].orbits[rec].v_factor =
        (e == 1)? 0: sqrt(fabs((1 + e)/(1 - e)));
      i *= DEG;
      node *= DEG;
      peri *= DEG;
      if (e == 1) {     // parabolic orbit
        if (format == 'A') {
          fclose(fp);
          return luxerror("Parablic orbit has undefined semimajor axis", 0);
        }
        extraOrbits[indx].orbits[rec].n =
          a? 0.03649116245/(a*sqrt(a)): 0.0; // n rad/d
        extraOrbits[indx].orbits[rec].M =
          (jd - m)*extraOrbits[indx].orbits[rec].n;
      } else {
        if (format == 'Q') {
          a = fabs(a/(1 - e));  // semimajor axis from perifocus distance
          extraOrbits[indx].orbits[rec].n =
            a? 0.01720209895/(a*sqrt(a)): 0.0; // n rad/d
          extraOrbits[indx].orbits[rec].M =
            (jd - m)*extraOrbits[indx].orbits[rec].n;
        } else {
          extraOrbits[indx].orbits[rec].M = m*DEG;
          extraOrbits[indx].orbits[rec].n =
            a? 0.01720209895/(a*sqrt(a)): 0.0; // n rad/d
        }
        // if (e > 1) extraOrbits[indx].orbits[rec].n = -extraOrbits[indx].orbits[rec].n;
      }
      cn = cos(node);
      sn = sin(node);
      ci = cos(i);
      si = sin(i);
      f = cn;
      g = sn;
      p = -sn*ci;
      q = cn*ci;
      extraOrbits[indx].orbits[rec].xfac = sqrt(f*f + p*p); // "a"
      extraOrbits[indx].orbits[rec].xangle = atan2(f,p) + peri; // "A"
      extraOrbits[indx].orbits[rec].yfac = sqrt(g*g + q*q); // "b"
      extraOrbits[indx].orbits[rec].yangle = atan2(g,q) + peri; // "B"
      extraOrbits[indx].orbits[rec].zfac = si; // "c"
      extraOrbits[indx].orbits[rec].zangle = peri; // "C"
    } // end of for (rec,...)
  } // end of for (indx,...)
  if (ferror(fp))
    return luxerror("Read error in orbital data file \"%s\".", 0, file);
  fclose(fp);

  indx += nmore;
  if (indx < nExtraObj) {
    /* some IDs were already present, so some of the extra allocated
     memory was not used */
    extraOrbits =
      (struct extraInfo *) realloc(extraOrbits,
                                   indx*sizeof(struct extraInfo));
    extraIDs = (int32_t *) realloc(extraIDs, indx*sizeof(int32_t));
    nExtraObj = indx;
  }

  return 1;
}
//--------------------------------------------------------------------------
int32_t lux_readorbits(ArgumentCount narg, Symbol ps[])
/* READORBITS [,<file>,/LIST,/REPLACE] reads orbital information from <file>.
 Lists comments if /LIST is specified.  Replace preread data (if any)
 if /REPLACE is specified, else merges */
{
  char  *file;

  file = narg? string_arg(ps[0]): NULL;
  return readExtra(file, internalMode & 3);
}
//--------------------------------------------------------------------------
void showExtra(void)
// list the presently read auxilliary object
{
  int32_t       i;

  if (nExtraObj) {
    puts("Auxilliary orbits were read for:");
    for (i = 0; i < nExtraObj; i++)
      printf("%d %s\n", extraIDs[i],
             extraOrbits[i].comment? extraOrbits[i].comment: "(no comment)");
  } else
    puts("No auxilliary orbits have been read.");
}
//--------------------------------------------------------------------------
int32_t lux_showorbits(ArgumentCount narg, Symbol ps[])
{
  showExtra();
  return 1;
}
//--------------------------------------------------------------------------
void kepler(double m, double e, double v_factor, double *v, double *rf)
/* solves Kepler's equation for mean anomaly <m> (in radians) and
 eccentricity <e>.  Returns true anomaly <*v> and radius factor <*rf>.
 <v_factor> must be equal to sqrt(abs((1 + <e>)/(1 - <e>))).  The radius
 will be equal to the perihelion distance times <*rf>.  */
{
  double        E, de, p;

  if (fabs(e) < 1) {            // elliptical orbit
    m = fmod(m, TWOPI);
    E = m;
    do {
      p = 1 - e*cos(E);
      de = (m + e*sin(E) - E)/p;
      E += de;
    } while (fabs(de) > 1e3*DBL_EPSILON);
    *v = 2*atan(v_factor*tan(E/2)); // true anomaly
    *rf = p/(1 - e);
  } else if (fabs(e) > 1) {     // hyperbolic orbit
    E = m/e;
    do {
      p = E;
      E = asinh((m + E)/e);
    } while (E != p);
    *v = 2*atan(v_factor*tanh(E/2));
    *rf = (1 - e*cosh(E))/(1 - e);
  } else {                      // parabolic orbit
    E = m/2;
    e = cbrt(E + sqrt(E*E + 1));
    p = e - 1.0/e;
    *v = 2*atan(p);
    *rf = 1 + p*p;
  }
}
//--------------------------------------------------------------------------
/** \brief Solves Kepler's equation.

    \param[in] M is the mean (if (internalMode & 1) == 0) or perifocal
    anomaly (if (internalMode & 1) == 1), in radians.  Its value is
    unrestricted.

    \param[in] e is the eccentricity.  Its value is unrestricted.

    \return the true (if (internalMode & 6) == 0) or eccentric (if
    (internalMode & 6) == 2) anomaly, in radians, or the tangent of
    half of the true anomaly (if (internalMode & 6 == 4)).

    Kepler's equation is <tt>E = M + e*sin(E)</tt>, which we need to
    solve for \c E, given \c M and \c e.

    The solution is found through iteration (except when e == 1).
    Each iteration yields a correction to the previous one, until the
    relative precision cannot improve anymore.

    The perifocal anomaly is the mean anomaly in the circular orbit
    that touches the real orbit in its perifocus.

    Kepler's equation is usually stated in terms of the mean anomaly,
    which is based on the angular speed averaged over the orbital
    period, but there is no orbital period for parabolic or hyperbolic
    orbits.  For hyperbolic orbits, one can use the same formula for
    the mean anomaly that is also used for elliptical orbits, even
    though an object in a hyperbolic orbit never returns to a previous
    position in that orbit.  However, when the perifocal distance and
    time since perifocus are kept fixed, and the eccentricity
    approaches 1 from below or above, then the mean anomaly
    asymptotically goes to 0 for every non-zero perifocal distance, so
    the mean anomaly is not useful for parabolic orbits, and
    inconvenient for near-parabolic orbits.

    The perifocal anomaly is well-defined for elliptical, parabolic,
    and hyperbolic orbits, is continuous across eccentricity 1, and
    for near-parabolic orbits its value for a fixed time since
    perifocus hardly depends on the eccentricity.  This fits well with
    the observation that it is difficult to tell elliptical and
    hyperbolic near-parabolic orbits apart.

*/
double kepler_v(double M, double e)
{
  if (isnan(M) || isnan(e))
    return M;
  e = fabs(e);
  if (e == 0) {                 // circular orbit
    // then mean anomaly = perifocal anomaly = eccentric anomaly = true anomaly
    if ((internalMode & 6) == 4) // output is tau (tangent of half of
                                 // true anomaly)
      return tan(M/2);
    else                        // output is true or eccentric anomaly
      return fasmod(M, TWOPI);
  } else if (e == 1) {          // parabolic orbit
    // for parabolic orbits, mean anomaly and eccentric anomaly are
    // not defined.
    if ((internalMode & 1) == 0 // input is mean anomaly
        || (internalMode & 6) == 2) // output is eccentric anomaly
      return std::numeric_limits<double>::quiet_NaN(); // cannot compute
    else if (internalMode & 8)  // output is iterations count
      return 0;
    else {
      // if we get here then the input is perifocal anomaly and the
      // requested output is true anomaly or tau (= tangent of half of
      // the true anomaly)
      double W = sqrt(9./8.)*M;
      double u = cbrt(W + sqrt(W*W + 1));
      double tau = u - 1/u;
      if (internalMode & 4)     // output is tau
        return tau;
      else                      // output is true anomaly
        return 2*atan(tau);
    }
  }
  // if we get here then we're calculating for a non-parabolic orbit
  double delta = e - 1;
  double ad = fabs(delta);
  double srd = sqrt(ad);
  double srd3 = srd*ad;
  double isrd3 = 1/srd3;

  double Mq;                    // perifocal anomaly
  if (internalMode & 1) {       // input is perifocal anomaly
    Mq = M;
    M *= srd3;                  // from perifocal to mean anomaly
  }                             // otherwise the input is mean anomaly
  if (e < 1                     // elliptical orbit
      && fabs(M) > M_PI) {      // only modify if needed
    M = fasmod(M, TWOPI);       // move between -π and +π
    if (internalMode & 1)
      Mq = M*isrd3;
  }
  if ((internalMode & 1) == 0)
    Mq = M*isrd3;

  double E;
  {
    double W = sqrt(9./8.)*Mq/e;
    double u = cbrt(W + sqrt(W*W + 1/(e*e*e)));
    double T = u - 1/(e*u);
    // initial estimate based on small anomaly
    E = T*srd*sqrt(2);

    if (e > 1) {                  // hyperbolic orbit
      double Eh = asinh(M/e); // initial estimate based on large anomaly
      if (fabs(Eh) < 0.53*fabs(e*sinh(E) - E - M))
        E = Eh;
      // otherwise we use the initial estimate based on small anomaly
    }
  }

  int iterations_count = 0;
  {
    double dE, B;
    do {
      if (++iterations_count == 100) {
        E = acos(2);              // NaN
        break;
      }
      double s, c, d;
      if (e < 1) {                  // elliptic orbit
        s = e*sin(E);
        c = 1 - e*cos(E);
        d = M - E + s;
      } else {                      // hyperbolic orbit
        s = e*sinh(E);
        c = e*cosh(E) - 1;
        d = M + E - s;
      }
      B = fabs(2*std::numeric_limits<double>::epsilon()*E*c/s);
      dE = d/c;
      E += dE;
    } while (dE*dE >= B);
  }

  if (internalMode & 8)
    return iterations_count;

  {
    int kind = ((internalMode >> 1) & 3);
    if (kind == 1)              // eccentric anomaly
      return E;

    double f = sqrt((e + 1)/ad);
    double tau;
    if (e > 1)
      tau = f*tanh(E/2);
    else
      tau = f*tan(E/2);

    if (kind == 2)              // tau, tangent of half true anomaly
      return tau;

    return 2*atan(tau);         // output is true anomaly
  }
}
BIND(kepler_v, d_dd_iaibrq_01_2, f, kepler, 2, 2, "0meananomaly:1perifocalanomaly:0trueanomaly:2eccentricanomaly:4tau:8itercount");
//--------------------------------------------------------------------------
double interpolate_angle(double a1, double a2, double f)
     /* interpolates between angles <a1> and <a2> (measured in
        radians) at fraction <f> from <a1> to <a2>, taking into
        accounts that angles are specified modulo 2 pi only.  It
        assumes the smallest possible absolute difference between the
        angles. */
{
  a1 = famod(a1, TWOPI);
  a2 = fasmod(a2 - a1, TWOPI);
  a2 += a1;
  a2 = a1*(1 - f) + a2*f;
  return a2;
}
//--------------------------------------------------------------------------
int32_t extraHeliocentric(double JDE, int32_t object, double *equinox,
                      double *f, double *r)
/* For <JDE>, return the polar coordinates of <object> in <f>, and the
 equinox for which it was specified in <equinox>. */
{
  int32_t       findint(int32_t, int32_t *, int32_t), i, low, high, mid;
  double        m, e, k, d1, d2, q;
  struct extraInfo      *pp;
  struct orbitParams    *qq;

  if (!nExtraObj && readExtra(NULL, 0) == LUX_ERROR)
    return luxerror("Error reading orbital data.", 0);

  // first see if the object number is known
  i = findint(object, extraIDs, nExtraObj);
  if (i < 0)
    return luxerror("No orbital data for object %d.", 0, object);

  // OK, it is known.  Now find the ephemerides time closest to T
  pp = extraOrbits + i;                 // data for ith object in file
  low = 0;
  high = pp->nterms - 1;
  while (low <= high) {
    mid = (low + high)/2;
    if (JDE < pp->orbits[mid].JDE)
      high = mid - 1;
    else if (JDE > pp->orbits[mid].JDE)
      low = mid + 1;
    else
      break;
    //      return mid;
  }
  /* If JDE is found in pp->orbits[].JDE, then low = high = mid = the
   appropriate index.  If JDE is not found, then its value is between
   those at indices high and low.  (If JDE is before the first value,
   then high = -1; if JDE is after the last value, then low = pp->nterms. */

  if (low == high || high == -1 || low == pp->nterms) {
    // just take fixed parameters
    if (low == pp->nterms)
      low--;
    qq = pp->orbits + low;
    m = qq->M + (JDE - qq->JDE)*qq->n; // mean anomaly
    e = qq->e;
    k = qq->v_factor;
    q = qq->q;
    kepler(m, e, k, &m, r); // solve Kepler's equation
    *r = q**r;                  // distance
    f[0] = *r*qq->xfac*sin(qq->xangle + m);
    f[1] = *r*qq->yfac*sin(qq->yangle + m);
    f[2] = *r*qq->zfac*sin(qq->zangle + m);
  } else {                      // linear interpolation
    qq = pp->orbits + high;
    d1 = (JDE - qq->JDE)/(qq[1].JDE - qq->JDE);
    d2 = 1 - d1;
    m = interpolate_angle(famod(qq[0].M + (JDE - qq[0].JDE)*qq[0].n, TWOPI),
                          famod(qq[1].M + (JDE - qq[1].JDE)*qq[1].n, TWOPI),
                          d1);
    e = qq->e*d2 + qq[1].e*d1;
    k = qq->v_factor*d2 + qq[1].v_factor*d1;
    q = qq->q*d2 + qq[1].q*d1;
    kepler(m, e, k, &m, r);
    *r = q**r;                  // distance
    f[0] = *r*(qq->xfac*d2 + qq[1].xfac*d1)
      * sin((qq->xangle*d2 + qq[1].xangle*d1) + m);
    f[1] = *r*(qq->yfac*d2 + qq[1].yfac*d1)
      * sin((qq->yangle*d2 + qq[1].yangle*d1) + m);
    f[2] = *r*(qq->zfac*d2 + qq[1].zfac*d1)
      * sin((qq->zangle*d2 + qq[1].zangle*d1) + m);
  }
  *equinox = pp->equinox;
  return 1;
}
//--------------------------------------------------------------------------
void extraElementsHeliocentric(double JDE, double *equinox, double *f,
                               double *r)
{
  static double         xfac, yfac, zfac, xangle, yangle, zangle, epoch, m0, n,
        v_factor, e, a, q, theequinox;
  double m;

  if (haveExtraElements & 4) {  // new
    double i, node, peri, ci, si, cn, sn,
      ff, g, p, qq;
    int32_t ii;

    theequinox = extraElements[0];
    epoch = extraElements[1];
    e = fabs(extraElements[3]);
    i = extraElements[4]*DEG;
    node = extraElements[5]*DEG;
    peri = extraElements[6]*DEG;
    v_factor = (e == 1)? 0: sqrt(fabs((1 + e)/(1 - e)));
    switch (haveExtraElements & 3) {
      case 2:                   // /QELEMENTS
        q = fabs(extraElements[2]);
        if (e == 1) {           // parabola
          n = q? 0.03649116245/(q*sqrt(q)): 0.0; // n rad/d
        } else {
          a = fabs(q/(1 - e));
          n = a? 0.01720209895/(a*sqrt(a)): 0.0; // n rad/d
          // if (e > 1) n = -n;
        }
        m0 = (epoch - extraElements[7])*n;
        break;
      case 1:                   // default
        if (e == 1) {
          luxerror("Parabolic orbits have an infinite semimajor axis", 0);
          for (ii = 0; ii < 3; ii++)
            f[ii] = 0.0;
          *r = 0.0;
          return;
        }
        a = fabs(extraElements[2]);
        q = fabs(a*(1 - e));
        n = a? 0.01720209895/(a*sqrt(a)): 0.0; // n rad/d
        // if (e > 1) n = -n;
        m0 = extraElements[7]*DEG;
        break;
    }
    cn = cos(node);
    sn = sin(node);
    ci = cos(i);
    si = sin(i);
    ff = cn;
    g = sn;
    p = -sn*ci;
    qq = cn*ci;
    xfac = sqrt(ff*ff + p*p); // "a"
    xangle = atan2(ff,p) + peri; // "A"
    yfac = sqrt(g*g + qq*qq); // "b"
    yangle = atan2(g,qq) + peri; // "B"
    zfac = si; // "c"
    zangle = peri; // "C"
    haveExtraElements &= ~4;
  }
  *equinox = theequinox;
  m = m0 + (JDE - epoch)*n;
  kepler(m, e, v_factor, &m, r);
  *r = q**r;
  f[0] = *r*xfac*sin(xangle + m);
  f[1] = *r*yfac*sin(yangle + m);
  f[2] = *r*zfac*sin(zangle + m);
}
//--------------------------------------------------------------------------
void heliocentricXYZr(double JDE, int32_t object, double equinox,
                      double *pos, double *r, double tolerance,
                      int32_t vocal, int32_t source)
// returns in <f> the cartesian heliocentric ecliptic coordinates of
// object <object> for the desired <equinox> at <JDE>, and in <r> the
// heliocentric distance
{
  double        T, standardEquinox;
  int32_t       i;

  T = (JDE - J2000)/365250;     // Julian millennia since J2000.0
  switch (object) {
  case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
  case 8:
    switch (source) {
    case S_VSOP87A:
      XYZJ2000fromVSOPA(T, object, pos, tolerance);
      /* heliocentric cartesian coordinates referred to the mean
         dynamical ecliptic and equinox of J2000.0 */
      if (vocal) {
        printf("ASTRON: VSOP87A (%s) geometric heliocentric ecliptic coordinates J2000.0:\n",
               objectName(object));
        printXYZtoLBR(pos);
      }
      if ((internalMode & S_BARE) == 0) {
        if (internalMode & S_FK5) {
          if (J2000 != JDE) {
#if HAVE_LIBGSL
            if (vocal)
              printf("ASTRON: precess ecliptic coordinates from J2000.0 to ecliptic of date (JD %1$.14g = %1$-#24.6J)\n", JDE);
            XYZ_eclipticPrecession(pos, J2000, JDE); // precess to equinox of date
#else
            luxerror("Precession is not supported because the application was compiled without libgsl", 0);
#endif
            if (vocal)
              printXYZtoLBR(pos);
          }
          XYZ_VSOPtoFK5(T, pos); // transform to FK5 (must be equinox of date)
          if (vocal) {
            printf("ASTRON: FK5 (%s) geometric heliocentric ecliptic coordinates, ecliptic of date (JD %2$.14g = %2$-#24.6J):\n", objectName(object), JDE);
            printXYZtoLBR(pos);
          }
          if (JDE != equinox) {
#if HAVE_LIBGSL
            if (vocal)
              printf("ASTRON: precess ecliptic coordinates from JD %1$.14g = %1$-#24.6J to JD %2$.14g = %2$-#24.6J\n", JDE, equinox);
            XYZ_eclipticPrecession(pos, JDE, equinox); // precess to desired equinox
#else
            luxerror("Precession is not supported because the application was compiled without libgsl", 0);
#endif
            if (vocal)
              printXYZtoLBR(pos);
          }
        } else {
          if (J2000 != equinox) {
#if HAVE_LIBGSL
            if (vocal)
              printf("ASTRON: precess ecliptic coordinates from J2000.0 to JD %1$.14g = %1$-#24.6J\n", equinox);
            XYZ_eclipticPrecession(pos, J2000, equinox); // precess to desired equinox
#else
            luxerror("Precession is not supported because the application was compiled without libgsl", 0);
#endif
          }
        }
      }
      break;
    case S_VSOP87C:
      XYZdatefromVSOPC(T, object, pos, tolerance);
      /* heliocentric cartesian coordinates referred to the mean
         dynamical ecliptic and equinox of the date */
      if (vocal) {
        printf("ASTRON: VSOP87C (%s) geometric heliocentric ecliptic coordinates equinox/ecliptic of date (JD %2$.14g = %2$-#24.6J):\n",
               objectName(object), JDE);
        printXYZtoLBR(pos);
      }
      if ((internalMode & S_BARE) == 0) {
        if (internalMode & S_FK5) {
          XYZ_VSOPtoFK5(T, pos); // transform to FK5
          if (vocal) {
            printf("ASTRON: FK5 (%s) geometric heliocentric ecliptic coordinates, equinox/ecliptic of date (JD %2$.14g = %2$-#24.6J):\n", objectName(object), JDE);
            printXYZtoLBR(pos);
          }
        }
        if (JDE != equinox) {
#if HAVE_LIBGSL
          if (vocal)
            printf("ASTRON: precess ecliptic coordinates from JD %1$.14g = %1$-#24.6J to JD %2$.14g = %2$-#24.6J\n", JDE, equinox);
          XYZ_eclipticPrecession(pos, JDE, equinox); // precess to desired equinox
#else
          luxerror("Precession is not supported because the application was compiled without libgsl", 0);
#endif
        }
      }
      break;
    default:
      luxerror("Illegal VSOP model type", 0);
      pos[0] = pos[1] = pos[2] = *r = 0.0;
      return;
    }
    *r = hypota(3, pos);        // heliocentric distance
    if (vocal) {
      printf("ASTRON: (%s) geometric heliocentric ecliptic coordinates for equinox JD %2$.14g = %2$-#24.6J:\n",
             objectName(object), equinox);
      printXYZtoLBR(pos);
    }
    break;
  case 10:                      // the Moon
    {
      double lmoon, elon, msun, mmoon, nodedist, a1, a2, a3, suml, sumr,
        sumb, E[5], XYZmoon[3], Tc;
      struct moonlrTerm *mlrt;
      struct moonbTerm *mbt;
      int32_t i;

      Tc = T*10; // convert from julian millennia to
      // julian centuries
      lmoon = mpol4(218.3164591, 481267.88134236, -0.0013268, 1.0/538841,
                    -1.0/65194000, Tc, 360);
      lmoon *= DEG;
      elon = mpol4(297.8502042, 445267.1115168, -0.0016300, 1.0/545868,
                   -1.0/113065000, Tc, 360);
      elon *= DEG;
      msun = mpol3(357.5291092, 35999.0502909, -0.0001536, 1.0/24490000,
                   Tc, 360);
      msun *= DEG;
      mmoon = mpol4(134.9634114, 477198.8676313, 0.0089970, 1.0/69699,
                    -1.0/14712000, Tc, 360);
      mmoon *= DEG;
      nodedist = mpol4(93.2720993, 483202.0175273, -0.0034029,
                       -1.0/3526000, 1.0/863310000, Tc, 360);
      nodedist *= DEG;
      a1 = famod(119.75 + 131.849*Tc,360);
      a1 *= DEG;
      a2 = famod(53.09 + 479264.290*Tc,360);
      a2 *= DEG;
      a3 = famod(313.45 + 481266.484*Tc,360);
      a3 *= DEG;
      suml = sumr = sumb = 0.0;
      mlrt = moonlr;
      mbt = moonb;
      E[1] = E[3] = pol2(1,-0.002516,-0.0000074,Tc);
      E[2] = 1;
      E[0] = E[4] = E[1]*E[1];
      for (i = 0; i < sizeof(moonlr)/sizeof(struct moonlrTerm); i++) {
        double arg, fac;
        fac = E[mlrt->m + 2];
        arg = mlrt->d*elon + mlrt->m*msun + mlrt->mm*mmoon
          + mlrt->f*nodedist;
        suml += mlrt->l*sin(arg)*fac;
        sumr += mlrt->r*cos(arg)*fac;
        mlrt++;
      }
      for (i = 0; i < sizeof(moonb)/sizeof(struct moonbTerm); i++) {
        double arg, fac;
        fac = E[mbt->m + 2];
        arg = mbt->d*elon + mbt->m*msun + mbt->mm*mmoon + mbt->f*nodedist;
        sumb += mbt->b*sin(arg)*fac;
        mbt++;
      }
      suml += 3958*sin(a1) + 1962*sin(lmoon - nodedist) + 318*sin(a2);
      sumb += -2235*sin(lmoon) + 382*sin(a3) + 175*sin(a1 - nodedist)
        + 175*sin(a1 + nodedist) + 127*sin(lmoon - mmoon)
        - 115*sin(lmoon + mmoon);
      pos[0] = famod(lmoon + suml*DEG/1000000,TWOPI); // geocentric longitude referred
                                          // to the mean equinox of the date
      pos[1] = sumb*DEG/1000000;        // geocentric latitude
      pos[2] = (385000.56 + sumr/1000)/149597870; // AU (center-center)
      if (vocal) {
        printf("ASTRON: lunar ecliptic geocentric coordinates for equinox of date:\n");
        printLBRtoXYZ(pos);
      }
      if (JDE != equinox) {
        if (vocal)
          printf("ASTRON: precess ecliptic coordinates from JD %1$.14g = %1$-#24.6J to JD %2$.14g = %2$-#24.6J\n", JDE, equinox);
        eclipticPrecession(pos, JDE, equinox);
      }
      if (vocal) {
        printf("ASTRON: lunar ecliptic geocentric coordinates for equinox:\n");
        printLBRtoXYZ(pos);
      }
      LBRtoXYZ(pos, XYZmoon);

      heliocentricXYZr(JDE, 3, equinox, pos, r, tolerance, vocal, source);

      pos[0] += XYZmoon[0];
      pos[1] += XYZmoon[1];
      pos[2] += XYZmoon[2];
      *r = hypota(3, pos);
      if (vocal) {
        printf("ASTRON: lunar ecliptic heliocentric coordinates for equinox JD %1$.14g = %1$-#24.6J:\n", equinox);
        printXYZtoLBR(pos);
      }
    }
    break;
  case -1:                      // ELEMENTS object - if defined
    if (!haveExtraElements) {
      luxerror("Illegal object number %1d", 0, object);
      for (i = 0; i < 3; i++)
        pos[i] = 0.0;
      *r = 0.0;
      return;
    }
    extraElementsHeliocentric(JDE, &standardEquinox, pos, r);
    if (standardEquinox == EQUINOX_OF_DATE)
      standardEquinox = JDE;
    if (fabs(standardEquinox - equinox) > 1) { // precession
        // currently circuitous -> inefficient
      double f[3];
      XYZtoLBR(pos, f);
      eclipticPrecession(f, standardEquinox, equinox);
      LBRtoXYZ(f, pos);
    }
    break;
  default:
    if (extraHeliocentric(JDE, object, &standardEquinox, pos, r)
        == LUX_ERROR) {                 // cartesian
      for (i = 0; i < 3; i++)
        pos[i] = 0.0;
      *r = 0.0;
      return;
    }
    if (standardEquinox == EQUINOX_OF_DATE)
      standardEquinox = JDE;
    if (fabs(standardEquinox - equinox) > 1) { // precession
      // currently circuitous -> inefficient
      double f[3];
      XYZtoLBR(pos, f);
      eclipticPrecession(f, standardEquinox, equinox);
      LBRtoXYZ(f, pos);
    }
    break;
  }
}

static Ellipsoid
earth_ellipsoid(AstronomicalConstants::Earth_equatorial_radius_m
                / AstronomicalConstants::AU_m,
                AstronomicalConstants::Earth_flattening);

//--------------------------------------------------------------------------
/// Calculates geocentric ecliptic cartesian coordinates from
/// geographic polar ones.
///
/// \param[in] geograhic_latitude_rad is the geographic latitude in
/// radians.
///
/// \param[in] sidereal_rad is the local sidereal time in radians.
///
/// \param[in] height_m is the height above sea level in meters
///
/// \param[in] obliquity_rad is the obliquity of the ecliptic in
/// radians.
///
/// \param[out] xyz_AU is a pointer to the geocentric ecliptic
/// cartesian coordinates (X, Y, Z) measured in Astronomical Units.
void
get_geocentric_ecliptic_xyz_AU(double geographic_latitude_rad,
                               double sidereal_rad,
                               double height_m,
                               double obliquity_rad,
                               double* xyz_AU)
{
  earth_ellipsoid.xyz(geographic_latitude_rad, sidereal_rad, height_m, xyz_AU);
  Rotate3d::rotate_x(xyz_AU, -obliquity_rad);
}
//--------------------------------------------------------------------------
double
get_geocentric_latitude_rad(double geographic_latitude_rad,
                            double height_m)
{
  return earth_ellipsoid.geocentric_latitude_rad(geographic_latitude_rad, height_m);
}
//--------------------------------------------------------------------------
bool heliocentricXYZr_obs(double JDE, int32_t object, double equinox,
                          double latitude_rad, double sidereal_time_rad,
                          double height_m,
                          double *pos, double *r, double tolerance,
                          int32_t vocal, int32_t source)
{
  heliocentricXYZr(JDE, object, equinox, pos, r, tolerance, vocal, source);
  if (latitude_rad != S_PLANETOCENTRIC
      && object == 3) {      // Earth
    double pos_topocentric[3];
    double epsilon_rad = obliquity(equinox, 0);
    get_geocentric_ecliptic_xyz_AU(latitude_rad, sidereal_time_rad, height_m,
                                   epsilon_rad, pos_topocentric);
    for (int i = 0; i < 3; ++i)
      pos[i] += pos_topocentric[i];
    *r = hypota(3, pos);
    return true;
  }
  return false;
}
//--------------------------------------------------------------------------
void LBRtoXYZ(double *pos, double *pos2)
     /* calculates cartesian coordinates XYZ from polar coordinates LBR.
      pos must be unequal to pos2. */
{
  double        cl, cb, sl, sb;

  // calculate X Y Z from L B R
  cl = cos(pos[0]);
  cb = cos(pos[1]);
  sl = sin(pos[0]);
  sb = sin(pos[1]);
  pos2[0] = pos[2]*cb*cl;       // X
  pos2[1] = pos[2]*cb*sl;       // Y
  pos2[2] = pos[2]*sb;          // Z
}
//--------------------------------------------------------------------------
void XYZtoLBR(double *pos, double *pos2)
/* transform from cartesian to polar coordinates.  pos must be unequal to
 pos2 */
{
  double        r, h;

  h = hypot(pos[0], pos[1]);
  r = hypot(h, pos[2]);
  pos2[2] = r;                              // R
  pos2[0] = h? atan2(pos[1], pos[0]): 0.0; // L
  if (pos2[0] < 0)
    pos2[0] += TWOPI;
  pos2[1] = r? asin(pos[2]/pos2[2]): 0.0;       // B
}
//--------------------------------------------------------------------------
void ectoeq(double *pos, double ceps, double seps, char forward)
// transforms from ecliptical to equatorial coordinates or vice versa
{
  double        alpha, delta, sl, cl, sb, cb;

  sl = sin(pos[0]);
  cl = cos(pos[0]);
  sb = sin(pos[1]);
  cb = cos(pos[1]);
  if (forward == 0)             // reverse transform
    seps = -seps;
  alpha = atan2(sl*ceps - sb*seps/cb, cl);
  if (alpha < 0)
    alpha += TWOPI;
  delta = asin(sb*ceps + cb*seps*sl);
  pos[0] = alpha;
  pos[1] = delta;
}
//--------------------------------------------------------------------------
void galtoeq(double *pos, double equinox, char forward)
 // transforms from galactic to equatorial coordinates or vice versa
{
  double x;

  if (forward) {                // from galactic to equatorial
    double A = 123*DEG;
    double B = 27.4*DEG;

    pos[0] = pos[0] - A;
    x = 12.25*DEG + atan2(sin(pos[0]), cos(pos[0])*sin(B) - tan(pos[1])*cos(B));
    pos[1] = asin(sin(pos[1])*sin(B) + cos(pos[1])*cos(B)*cos(pos[0]));
    pos[0] = famod(x, TWOPI);
    precessEquatorial(pos, pos + 1, B1950, equinox);
  } else {                      // from equatorial to galactic
    double A = 192.25*DEG;
    double B = 27.4*DEG;

    precessEquatorial(pos, pos + 1, equinox, B1950);
    pos[0] = A - pos[0];
    pos[1] = pos[1];
    x = 303*DEG - atan2(sin(pos[0]), cos(pos[0])*sin(B) - tan(pos[1])*cos(B));
    pos[1] = asin(sin(pos[1])*sin(B) + cos(pos[1])*cos(B)*cos(pos[0]));
    pos[0] = famod(x, TWOPI);
  }
}
//--------------------------------------------------------------------------
int32_t lux_astrf(ArgumentCount narg, Symbol ps[]) {
/* ASTRF(<coords>[, <equinox>, /JULIAN, /BESSELIAN]
   [, /FROMEQUATORIAL, /FROMECLIPTICAL, /FROMGALACTIC]
   [, /TOEQUATORIAL, /TOECLIPTICAL, /TOGALACTIC])
   transforms between the main astronomical coordinate systems
   (with all coordinates measured in degrees)
   <coords>(0,*) are longitudes
   <coords>(1,*) are latitudes.
   <equinox> indicates the equinox of the input coordinates.
   <equinox> is a Julian Date, if no keywords are specified.
   <equinox> is a Julian year (e.g., 2000.0 = J2000.0) if /JULIAN is specified.
   <equinox> is a Besselian year (e.g., 1875.0 = B1875.0) if /BESSELIAN
   is specified.  If <equinox> is not specified, then J2000.0 is assumed.
   the returned array has the same structure as <coords>.
   By default, /FROMEQUATORIAL, /TOECLIPTICAL is assumed.
   LS 2004may30
*/
  LoopInfo srcinfo, tgtinfo;
  Pointer src, tgt;
  int32_t result, from, to;
  double pos[2], ceps, seps, epsilon, equinox;

  if (internalMode & 2)
    from = 2;                   // ecliptical
  else if (internalMode & 4)
    from = 3;                   // galactic
  else
    from = 1;                   // equatorial
  if (internalMode & 8)                 // equatorial
    to = 1;
  else if (internalMode & 32)
    to = 3;                     // galactic
  else
    to = 2;                     // ecliptical
  if (from == to)
    return ps[0];               // nothing to do
  if (standardLoop(ps[0], LUX_ZERO, SL_UPGRADE | SL_AXISCOORD | SL_EACHROW,
                   LUX_FLOAT, &srcinfo, &src, &result, &tgtinfo, &tgt)
      == LUX_ERROR)
    return LUX_ERROR;
  if (srcinfo.ndim < 1 || srcinfo.dims[0] < 2)
    return luxerror("Need at least two elements in first dimension", -1);
  if (narg > 1) {
    equinox = double_arg(ps[1]);
    if (internalMode & 128)     // /BESSELIAN to /JULIAN
      equinox = 0.99997860193253*equinox + 0.0413818359375; // to Julian
    if (internalMode & 3)       // /JULIAN to JD
      equinox = (equinox - 2000.0)*365.25 + J2000;
  } else                        // JD
    equinox = J2000;
  epsilon = obliquity(equinox, NULL);
  ceps = cos(epsilon);
  seps = sin(epsilon);
  do {
    switch (tgtinfo.type) {
    case LUX_FLOAT:
      switch (srcinfo.type) {
      case LUX_INT8:
        pos[0] = (double) src.ui8[0]*DEG;
        pos[1] = (double) src.ui8[1]*DEG;
        break;
      case LUX_INT16:
        pos[0] = (double) src.i16[0]*DEG;
        pos[1] = (double) src.i16[1]*DEG;
        break;
      case LUX_INT32:
        pos[0] = (double) src.i32[0]*DEG;
        pos[1] = (double) src.i32[1]*DEG;
        break;
      case LUX_FLOAT:
        pos[0] = (double) src.f[0]*DEG;
        pos[1] = (double) src.f[1]*DEG;
        break;
      default:
        break;
      }
      switch (from) {
      case 2:                   // ecliptical
        ectoeq(pos, ceps, seps, 1);
        break;
      case 3:                   // galactic
        galtoeq(pos, equinox, 1);
        break;
      }
      switch (to) {
      case 2:                   // ecliptical
        ectoeq(pos, ceps, seps, 0);
        break;
      case 3:
        galtoeq(pos, equinox, 0);
        break;
      }
      tgt.f[0] = (float) pos[0]*RAD;
      tgt.f[1] = (float) pos[1]*RAD;
      break;
    case LUX_DOUBLE:
      switch (srcinfo.type) {
      case LUX_INT8:
        pos[0] = (double) src.ui8[0]*DEG;
        pos[1] = (double) src.ui8[1]*DEG;
        break;
      case LUX_INT16:
        pos[0] = (double) src.i16[0]*DEG;
        pos[1] = (double) src.i16[1]*DEG;
        break;
      case LUX_INT32:
        pos[0] = (double) src.i32[0]*DEG;
        pos[1] = (double) src.i32[1]*DEG;
        break;
      case LUX_FLOAT:
        pos[0] = (double) src.f[0]*DEG;
        pos[1] = (double) src.f[1]*DEG;
        break;
      case LUX_DOUBLE:
        pos[0] = src.d[0]*DEG;
        pos[1] = src.d[1]*DEG;
        break;
      default:
        break;
      }
      switch (from) {
      case 2:                   // ecliptical
        ectoeq(pos, ceps, seps, 1);
        break;
      case 3:                   // galactic
        galtoeq(pos, equinox, 1);
        break;
      }
      switch (to) {
      case 2:                   // ecliptical
        ectoeq(pos, ceps, seps, 0);
        break;
      case 3:
        galtoeq(pos, equinox, 0);
        break;
      }
      tgt.d[0] = pos[0]*RAD;
      tgt.d[1] = pos[1]*RAD;
      break;
    default:
      break;
    }
    src.ui8 += srcinfo.rdims[0]*srcinfo.stride;
    tgt.ui8 += tgtinfo.rdims[0]*tgtinfo.stride;
  } while (srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
  return result;
}
//--------------------------------------------------------------------------
void refract(double *pos, double height)
/* corrects height above the horizon (pos[1]) for average
   refraction - but only if the refracted height is nonnegative */
{
  double        R, h;

  h = pos[1];
  R = 1/tan(h + 7.31/(h*RAD + 4.4));
  if (R < 0)                    // 0.08" error for h = 90 deg
    R = 0;
  else
    R *= exp(-h*1.22626e-4);    // correction for altitude
  h += R;
  if (h >= 0)
    pos[1] = h*DEG/60;
}
//--------------------------------------------------------------------------
void eqtohor(double *pos, double cphi, double sphi, char forward)
// transforms from equatorial to horizontal coordinates, or vice versa
{
  double        A, h, sH, cH, sd, cd;

  sH = sin(pos[0]);
  cH = cos(pos[0]);
  sd = sin(pos[1]);
  cd = cos(pos[1]);
  if (forward == 0)             // reverse transformation
    cphi = -cphi;
  A = atan2(sH, cH*sphi - sd*cphi/cd);
  if (forward == 0 && A < 0)
    A += TWOPI;
  h = asin(sphi*sd + cphi*cd*cH);
  pos[0] = A;
  pos[1] = h;
}
//--------------------------------------------------------------------------
double meanDistance(int32_t obj1, int32_t obj2)
{
  if (obj1 < 0 || obj1 > 8 || obj2 < 0 || obj2 > 8)
    return 0.0;
  else
    return meanDistances[obj1][obj2];
}
//--------------------------------------------------------------------------
void showraddms(char const* prefix, double x)
{
  printf("%1$s%2$.10g rad = %3$.10g deg = %3$-13.2T dms\n", prefix, x, x*RAD);
}
//--------------------------------------------------------------------------
void showradhms(char const * prefix, double x)
{
  printf("%1$s%2$.10g rad = %3$.10g deg = %3$#-13.2T hms\n", prefix, x, x*RAD);
}
//--------------------------------------------------------------------------
void printXYZtoLBR(double *xyz)
{
  double lbr[3];
  XYZtoLBR(xyz, lbr);
  printf(" X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
  showraddms(" L = ", lbr[0]);
  showraddms(" B = ", lbr[1]);
  printf(" R = %.10g\n", lbr[2]);
}
//--------------------------------------------------------------------------
void printLBRtoXYZ(double *lbr)
{
  double xyz[3];
  LBRtoXYZ(lbr, xyz);
  showraddms(" L = ", lbr[0]);
  showraddms(" B = ", lbr[1]);
  printf(" R = %.10g\n", lbr[2]);
  printf(" X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
}
//--------------------------------------------------------------------------
void printHBRtoXYZ(double *lbr)
{
  double xyz[3];
  LBRtoXYZ(lbr, xyz);
  showradhms(" H = ", lbr[0]);
  showraddms(" B = ", lbr[1]);
  printf(" R = %.10g\n", lbr[2]);
  printf(" X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
int32_t lux_astropos(ArgumentCount narg, Symbol ps[])
     // returns the positions of a set of heavenly bodies at a specific
     // set of times, for equinox 2000.0
     // Syntax: X = ASTRON(JDs, OBJECTS [, OBJECT0, OBSERVER=OBSERVER,
     //                    EQUINOX=EQUINOX, ELEMENTS=elements]
     //                            [, /XYZ, /EQUATORIAL, /ECLIPTICAL, /ELONGATION,
     //                             /HORIZONTAL, /DATE, /ERROR, /ABERRATION,
     /*                     /GEOMETRICAL, /TDT, /QELEMENTS, /FK5, /NUTATION,
                            /LIGHTTIME]) */
     // JDs = Julian dates for which elements are desired, in UTC
     // OBJECTS = scalar or array of object numbers.
     // OBJECT2 = number of the object relative to which the other
     //                   coordinates are requested
     // OBSERVER = [latitude,W longitude,height] of the observer,
     //           in degrees or meters
     // EQUINOX = equinox year of the coordinate system (default: 2000.0)
     //                   in Julian years TDT.
     // /XYZ: return ecliptical X Y Z coordinates in AU; otherwise return
     //       polar coordinates (longitude, latitude (degrees),
     //       distance (AU)
     // /EQUATORIAL: return right ascension, declination, and distance,
     //              or equivalent XYZ
     // /ECLIPTICAL: return ecliptic longitude, latitude, and distance
     //              or equivalent XYZ
     // /ELONGATION: return elongation, phase angle, and estimated
     //              visual magnitude
     // /HORIZONTAL: return azimuth (west from south), height, and distance
     //              or equivalent XYZ
     /* /PLANETOCENTRIC: ecliptical or equatorial coordinates relative to
        the orbit or equator of the planet */
     // if you don't specify an OBJECT2), then OBJECT2 is assumed to be
     //              the Earth.
     // current object numbers:  0 - Sun, 1 - Mercury, 2 - Venus,
     // 3 - Earth, 4 - Mars, 5 - Jupiter, 6 - Saturn, 7 - Uranus,
     // 8 - Neptune, 9 - Pluto, 10 - Moon.
{
  char  tdt, *string;
  int32_t       iq, nJD, *object, nObjects, object0,
    i, j, result, coordSystem, vocal;
  double        *JD, *f, *f0, longitude, latitude, height = 0.0,
    clat, slat, equinox;
  double        tolerance;

  if (internalMode & S_CONJSPREAD)      // /CONJSPREAD
    internalMode = internalMode | S_XYZ | S_ECLIPTICAL;
  coordSystem = (internalMode & S_COORDS); // desired coordinate system
  if (!coordSystem)
    coordSystem = S_ECLIPTICAL;         // default: ecliptical
  vocal = (internalMode & S_VOCAL); // print intermediate results

  std::vector<int> outDims;
  outDims.push_back(3);         // output values per JD/object

  iq = ps[1];                   // objects
  if (!iq) {
    if (narg > 5 && ps[5])
      iq = LUX_MINUS_ONE;       // specified elements
    else
      return cerror(ILL_ARG, ps[1]);
  }
  switch (symbol_class(iq)) {
    case LUX_SCALAR:
      iq = lux_long(1, &iq);
      object = &scalar_value(iq).i32;
      nObjects = 1;
      if (internalMode & S_KEEPDIMS) // always include objects dimensions
        outDims.push_back(1);
      break;
    case LUX_ARRAY:
      iq = lux_long(1, &iq);
      object = (int32_t *) array_data(iq);
      nObjects = array_size(iq);
      if (((internalMode & S_KEEPDIMS) // always include objects dimensions
          || nObjects > 1)
          && (internalMode & S_CONJSPREAD) == 0) { // not /conjspread
        outDims.insert(outDims.end(), array_dims(iq),
                       array_dims(iq) + array_num_dims(iq));
      }
      break;
    default:
      return cerror(ILL_CLASS, ps[1]);
  }

  iq = *ps;                     // JDs
  if (!iq)
    return cerror(ILL_ARG, iq);
  switch (symbol_class(iq)) {
    case LUX_SCALAR:
      iq = lux_double(1, &iq);
      JD = &scalar_value(iq).d;
      nJD = 1;
      if (internalMode & S_KEEPDIMS) // always include JD dimensions
        outDims.push_back(1);
      break;
    case LUX_ARRAY:
      iq = lux_double(1, &iq);
      JD = (double *) array_data(iq);
      nJD = array_size(iq);
      if ((internalMode & S_KEEPDIMS) // always include objects dimensions
          || nJD > 1)
        outDims.insert(outDims.end(), array_dims(iq),
                       array_dims(iq) + array_num_dims(iq));
      break;
    default:
      return cerror(ILL_CLASS, *ps);
  }

  if (narg > 2 && ps[2]) {      // object0
    object0 = int_arg(ps[2]);
  } else
    object0 = EARTH;            // no object0: use Earth
  if (object0 != EARTH) {       // not relative to Earth
    if (coordSystem != S_ECLIPTICAL && coordSystem != S_EQUATORIAL)
      return luxerror("Non-ecliptic/non-equatorial coordinates are only "
                      "returned relative to the Earth", 0);
  }

  double geocentric_latitude_rad;

  if (narg > 3 && ps[3])        // OBSERVER
  {
    if (symbol_class(ps[3]) != LUX_ARRAY
        || array_size(ps[3]) != 3)
      return
        luxerror("OBSERVER must be a three-element array "
                 "(lat/[deg], lon/[deg], h/[m])", ps[3]);
    iq = lux_double(1, ps + 3);
    f = (double *) array_data(iq);
    latitude = f[0];            // in degrees
    longitude = f[1];           // in degrees
    height = f[2];              // in meters above mean sea level
    if (latitude < -90 || latitude > 90)
      return luxerror("Illegal latitude: %14.6g; must be between "
                      "-90 and +90 degrees", ps[3], latitude);
    latitude *= DEG;
    longitude *= DEG;
    sincos(latitude, &slat, &clat);
    geocentric_latitude_rad = get_geocentric_latitude_rad(latitude, height);
  } else {                      // assume planetocentric
    latitude = S_PLANETOCENTRIC;
    if (coordSystem == S_HORIZONTAL)
      return luxerror("Need OBSERVER to return horizontal coordinates", 0);
    // avoid compiler warnings about possibly uninitialized variables
    longitude = 0;
    clat = 1;
    slat = 0;
    geocentric_latitude_rad = S_PLANETOCENTRIC;
  }

  if (narg > 4 && ps[4]) {      // EQUINOX
                                // equinox date (JDE)
    if (internalMode & S_DATE)
      return luxerror("Multiple equinoxes specified", ps[4]);
    switch (symbol_class(ps[4])) {
      case LUX_SCALAR:          // assume Julian year
        equinox = (double_arg(ps[4]) - 2000.0)*365.25 + J2000;
        break;
      case LUX_STRING:
        string = string_value(ps[4]);
        equinox = atof(string + 1); // year
        switch (toupper(*string)) {
          case 'J':             // Julian
            break;
          case 'B':             // Besselian (approximate) -> Julian
            equinox = 0.99997860193253*equinox + 0.0413818359375;
            break;
          default:
            return luxerror("Illegal EQUINOX specification", ps[4]);
        }
        equinox = (equinox - 2000.0)*365.25 + J2000;
        break;
      default:
        return cerror(ILL_CLASS, ps[4]);
    }
  } else
    equinox = J2000;            // default: J2000.0

  if (narg > 5 && ps[5]) {      // ELEMENTS
    /* elements: equinox epoch a e i node peri M absmag
              or equinox epoch q e i node peri Tperi absmag /QELEMENTS */
    j = ps[5];
    if (symbol_class(j) != LUX_ARRAY
        || array_size(j) != 9)
      return luxerror("Need a 9-element array with ELEMENTS", j);
    j = lux_double(1, &j);
    memcpy(extraElements, (double*) array_data(j), 9*sizeof(double));
    haveExtraElements = (internalMode & S_QELEMENTS)? 6: 5; // 5->A, 6->Q
  } else haveExtraElements = 0;                 // none

                                // create result array
  result = array_scratch(LUX_DOUBLE, outDims.size(), &outDims[0]);
  f = f0 = (double *) array_data(result);

  tdt = internalMode & S_TDT;   // time is specified in TDT rather than UT

  tolerance = 0;
  if (narg > 6 && ps[6])
    tolerance = double_arg(ps[6]);
  if (internalMode & S_TRUNCATEVSOP && !tolerance)
    tolerance = 1e-4;

  // calculate coordinates
  for (j = 0; j < nJD; j++) {   // all dates
    if (vocal)
      printf("ASTRON: calculating for JD = %1$.7f = %1$#-24.6J\n", JD[j]);
    double jd = tdt? JD[j]: JDE(JD[j], +1); // calculate date in TDT
    if (vocal) {
      if (tdt) {
        puts("ASTRON: JD is in TDT already");
      } else {
        printf("ASTRON: UT => TDT:      JD = %.7f\n", jd);
        printf("ASTRON: delta T = %.10g s\n", (jd - JD[j])*86400);
      }
    }
    double dPsi, cdPsi, sdPsi, dEps;
    if (internalMode & S_NUTATION) { // nutation
      nutation(jd, &dPsi, &cdPsi, &sdPsi, &dEps); // nutation parameters
      if (vocal) {
        printf("ASTRON: nutation constants:\n");
        showraddms(" dPsi = ", dPsi);
        showraddms(" dEps = ", dEps);
      }
    } else {
      dPsi = sdPsi = dEps = 0.0;        // ignore nutation
      cdPsi = 1.0;
      if (vocal)
        puts("ASTRON: no nutation correction.");
    }
    if (internalMode & S_DATE)
      equinox = jd;
    if (vocal)
      printf("ASTRON: equinox:        JD = %.7f\n", equinox);
    double epsilon = obliquity(equinox, &dEps);
    if (vocal) {
      if (dPsi)
        printf("ASTRON: obliquity of ecliptic for equinox, corrected for nutation:\n");
      else
        printf("ASTRON: mean obliquity of ecliptic for equinox:\n");
      showraddms(" epsilon = ", epsilon);
    }
    double ceps = cos(epsilon);
    double seps = sin(epsilon);
    double Tsid = siderealTime(JDE(jd, -1), &dPsi, ceps);
    /* apparent sidereal time at longitude zero (possibly corrected
       for nutation), at UT time, in radians */
    if (vocal) {
      if (dPsi)
        printf("ASTRON: apparent sidereal time (0 longitude, nutation)\n");
      else
        printf("ASTRON: mean sidereal time (0 longitude)\n");
      showradhms(" Tsid = ", Tsid);
    }
    if (vocal && !(internalMode & S_FK5))
      puts("ASTRON: coordinates relative to VSOP axes, not FK5");

    double mean[3];
    if (internalMode & S_CONJSPREAD)
      mean[0] = mean[1] = mean[2] = 0;

    double pos_sun_obs[3], r_sun_obs;
    // calculate the position of the observer's planet
    // bool applied_observer_planetocentric_offset
    //   = heliocentricXYZr_obs(jd, object0, equinox, geocentric_latitude_rad,
    //                          Tsid, height, pos_sun_obs, &r_sun_obs, tolerance,
    //                          vocal, internalMode & S_VSOP);
    /* cartesian ecliptic heliocentric coordinates of the observer's
       planet at target time */
    // pos_sun_obs[0] = X, pos_sun_obs[1] = Y, pos_sun_obs[2] = Z,
    // r_sun_obs = heliocentric distance of the observer
    if (vocal) {
      printf("ASTRON: observer (%s) geometric heliocentric ecliptic coordinates for equinox:\n", objectName(object0));
      printXYZtoLBR(pos_sun_obs);
    }

    for (i = 0; i < nObjects; i++) { // all objects
      double lighttime = 0.0;
      double r_sun_tgt, r_obs_tgt, pos_sun_tgt[9], pos_obs_tgt[9];
      if (internalMode & (S_LIGHTTIME | S_ABERRATION)) {
        if (vocal)              // print geometric position
          heliocentricXYZr(jd, object[i], equinox, pos_sun_tgt, &r_sun_tgt,
                           tolerance, 1, internalMode & S_VSOP);
        // calculate light time
        // get initial guess of "average" distance between the objects
        // for an initial guess of the light time
        r_sun_tgt = meanDistance(object[i],object0 >= 0? object0: 0);
        lighttime = r_sun_tgt*AUtoJD;   // initial estimate of light time
        double prev_lighttime = lighttime + 1; // previous estimate
        /* to get loop going, set initial value of prev_lighttime
           different from current value of lighttime */
        r_obs_tgt = 0.0; // initial estimate of geometrical distance

        // now we converge upon the light-time to the target object
        uint32_t count = 0;
        while (fabs(lighttime - prev_lighttime) > DBL_EPSILON*1000
               && count < 25) {
          // no convergence yet
          prev_lighttime = lighttime; // old estimate is previous estimate
          double jd_lt = jd - lighttime; // time corrected for light time
          heliocentricXYZr(jd_lt, object[i], equinox, pos_sun_tgt, &r_sun_tgt,
                           tolerance, 0, internalMode & S_VSOP);
          /* pos_tgt = cartesian ecliptic heliocentric coordinates of the
             target, r_sun_tgt = heliocentric distance of the target */
          pos_obs_tgt[0] = pos_sun_tgt[0] - pos_sun_obs[0]; // dX/AU
          pos_obs_tgt[1] = pos_sun_tgt[1] - pos_sun_obs[1]; // dY/AU
          pos_obs_tgt[2] = pos_sun_tgt[2] - pos_sun_obs[2]; // dZ/AU
          // apparent distance
          r_obs_tgt = hypota(3, pos_obs_tgt);
          lighttime = r_obs_tgt*AUtoJD; // new estimate of light time
          count++;
        } // end of while
        if (vocal)
          printf("ASTRON: light-time = %.10g min = %.10g days\n",
                 lighttime*24*60, lighttime);
      }

      /*
        now we have cartesian ecliptic coordinates
        pos3 = observer helioceontric at jd
        pos2 = target heliocentric at jd (geometrical) or jd - lighttime
        pos = target (at jd or jd - lighttime) relative to observer (at jd)
      */

      /* we include the effects of light-time and (if requested)
         aberration due to the observer's motion.

         The light-time dt is the time that light takes to travel from
         the object at time t - dt to the observer at time t.

         Light-time only: combine the position of the object at time t
         - dt with the position of the observer at time t.

         Aberration only: combine the position of the object at time t
         with the position of the observer at time t - dt.

         Light-time and aberration: combine the position of the object
         at time t - dt with the position of the observer at time t -
         dt.

         (These procedures assume that the motion of the observer
         during the light-time period is linear.)
      */

      switch (internalMode & (S_LIGHTTIME | S_ABERRATION)) {
      case 0:                   // geometrical; obs + target at jd
        heliocentricXYZr(jd, object[i], equinox, pos_sun_tgt, &r_sun_tgt,
                         tolerance, vocal, internalMode & S_VSOP);
        pos_obs_tgt[0] = pos_sun_tgt[0] - pos_sun_obs[0];
        pos_obs_tgt[1] = pos_sun_tgt[1] - pos_sun_obs[1];
        pos_obs_tgt[2] = pos_sun_tgt[2] - pos_sun_obs[2];
        r_obs_tgt = hypota(3, pos_obs_tgt);
        if (vocal) {
          puts("ASTRON: target geometric planetocentric ecliptic coordinates:");
          printXYZtoLBR(pos_obs_tgt);
        }
        break;
      case S_LIGHTTIME:         // observer at jd, target at jd - lighttime
        // calculated this already when figuring out the lighttime
        if (vocal) {
          puts("ASTRON: target planetocentric ecliptic coordinates corrected for light-time:");
          printXYZtoLBR(pos_obs_tgt);
        }
        break;
      case S_ABERRATION:        // observer at jd - lighttime, target at jd
        {
          double pos_sun_obs_lt[3], r_sun_obs_lt, pos_sun_tgt_nolt[3],
            r_sun_tgt_nolt;
          heliocentricXYZr(jd - lighttime, object0, equinox, pos_sun_obs_lt,
                           &r_sun_obs_lt, tolerance, vocal,
                           internalMode & S_VSOP);
          heliocentricXYZr(jd, object[i], equinox, pos_sun_tgt_nolt,
                           &r_sun_tgt_nolt, tolerance, vocal,
                           internalMode & S_VSOP);
          pos_obs_tgt[0] = pos_sun_tgt_nolt[0] - pos_sun_obs_lt[0];
          pos_obs_tgt[1] = pos_sun_tgt_nolt[1] - pos_sun_obs_lt[1];
          pos_obs_tgt[2] = pos_sun_tgt_nolt[2] - pos_sun_obs_lt[2];
        }
        /* pos_obs_tgt[i] now contain cartesian coordinates that
           point in the right direction but do not indicate the
           right distance, which is r_sun_obs.  correct. */
        {
          double f = r_obs_tgt/hypota(3, pos_obs_tgt);
          pos_obs_tgt[0] *= f;
          pos_obs_tgt[1] *= f;
          pos_obs_tgt[2] *= f;
          if (vocal) {
            puts("ASTRON: target planetocentric ecliptic coordinates corrected for aberration:");
            printXYZtoLBR(pos_obs_tgt);
          }
        }
        break;
      case (S_LIGHTTIME | S_ABERRATION): // observer + target at jd - lighttime
        {
          double pos_sun_obs_lt[9], r_sun_obs_lt;
          heliocentricXYZr(jd - lighttime, object0, equinox, pos_sun_obs_lt,
                           &r_sun_obs_lt, tolerance, vocal,
                           internalMode & S_VSOP);
          pos_obs_tgt[0] = pos_sun_tgt[0] - pos_sun_obs_lt[0];
          pos_obs_tgt[1] = pos_sun_tgt[1] - pos_sun_obs_lt[1];
          pos_obs_tgt[2] = pos_sun_tgt[2] - pos_sun_obs_lt[2];
        }
        double fac = r_obs_tgt/hypota(3, pos_obs_tgt);
        pos_obs_tgt[0] *= fac;
        pos_obs_tgt[1] *= fac;
        pos_obs_tgt[2] *= fac;
        if (vocal) {
          puts("ASTRON: target planetocentric ecliptic coordinates corrected for lighttime and aberration:");
          printXYZtoLBR(pos_obs_tgt);
        }
        break;
      }

      /* now have (in pos_obs_tgt[]) the planetocentric ecliptic
         cartesian coordinates referred to the desired equinox */

      if (dPsi) {
        // apply nutation
        double pos_nut[3];
        pos_nut[0] = pos_obs_tgt[0]*cdPsi - pos_obs_tgt[1]*sdPsi;
        pos_nut[1] = pos_obs_tgt[0]*sdPsi + pos_obs_tgt[1]*cdPsi;
        pos_nut[2] = pos_obs_tgt[2];
        memcpy(pos_obs_tgt, pos_nut, 3*sizeof(double));
        if (vocal) {
          puts("ASTRON: planetocentric ecliptic coordinates corrected for nutation:");
          printXYZtoLBR(pos_obs_tgt);
        }
      }

      double final[3];
      if (coordSystem == S_ELONGATION) { // elongation, phase angle, magn
        // calculate the elongation
        final[0] = (r_sun_obs*r_sun_obs + r_obs_tgt*r_obs_tgt
                     - r_sun_tgt*r_sun_tgt)/(2*r_sun_obs*r_obs_tgt);
        if (final[0] > 1)      // ignore round-off errors
          final[0] = 1;
        else if (final[0] < -1) // ignore round-off errors
          final[0] = -1;
        final[0] = acos(final[0]); // elongation

        // calculate the phase angle
        final[1] = (r_obs_tgt*r_obs_tgt + r_sun_tgt*r_sun_tgt
                     - r_sun_obs*r_sun_obs)/(2*r_sun_tgt*r_obs_tgt);
        if (final[1] > 1)
          final[1] = 1;
        else if (final[1] < -1)
          final[1] = -1;
        final[1] = acos(final[1]); // phase angle

        // calculate the magniture
        final[2] = magnitude(r_sun_tgt, r_obs_tgt, final[1]*RAD, object[i]);
        if (vocal) {
          puts("ASTRON: transform to elongation, phase angle, magnitude");
          showraddms(" el = ", final[0]);
          showraddms(" ph = ", final[1]);
          printf(" mag = %.10g\n", final[2]);
        }
      } else {
        XYZtoLBR(pos_obs_tgt, final);   // to polar coordinates
        if (internalMode & S_FK5)
          VSOPtoFK5(TC2000(jd), final);     // to FK5
        if (vocal) {
          puts("ASTRON: planetocentric ecliptic coordinates:");
          printLBRtoXYZ(final);
        }
        if (latitude != S_PLANETOCENTRIC // topocentric -> parallax
            || coordSystem == S_EQUATORIAL || coordSystem == S_HORIZONTAL) {
          ectoeq(final, ceps, seps, 1); // to equatorial coordinates
          if (vocal) {
            puts("ASTRON: planetocentric equatorial coordinates:");
            printHBRtoXYZ(final);
          }
          if (latitude != S_PLANETOCENTRIC) {
            // we need to take parallax into account
            final[0] = Tsid - longitude - final[0]; // RA to local hour angle
            if (vocal) {
              printf("ASTRON: local hour angle\n");
              showradhms(" H = ", final[0]);
            }
            if (vocal) {
              puts("ASTRON: topocentric equatorial coordinates:");
              printHBRtoXYZ(final);
            }
            if (coordSystem == S_ECLIPTICAL || coordSystem == S_EQUATORIAL)
              final[0] = Tsid - longitude - final[0]; // back to RA
            if (coordSystem == S_ECLIPTICAL)
              ectoeq(final, ceps, seps, 0); // back to ecliptical
          }
          /* we have ecliptical coordinates if S_ECLIPTICAL
             or equatorial coordinates if S_EQUATORIAL
             or hour angle - declination - distance if S_HORIZONTAL */
          if (coordSystem == S_HORIZONTAL
              && latitude != S_PLANETOCENTRIC) { // to horizontal coordinates
            eqtohor(final, clat, slat, 1);
            if (vocal) {
              puts("ASTRON: horizontal coordinates:");
              printLBRtoXYZ(final);
            }
          }
        }
        final[0] = famod(final[0], TWOPI);
        if ((internalMode & S_XYZ) != 0) {
          // back to cartesian coordinates
          double pos[3];
          memcpy(pos, final, sizeof(pos));
          LBRtoXYZ(pos, final);
          if (vocal) {
            puts("ASTRON: back to cartesian coordinates:");
            printXYZtoLBR(final);
          }
        }
      }
      if (internalMode & S_CONJSPREAD) { // /CONJSPREAD
        double r;

        r = hypota(3, final);
        if (r) {
          final[0] /= r;
          final[1] /= r;
          final[2] /= r;
        }
        mean[0] += final[0];
        mean[1] += final[1];
        mean[2] += final[2];
      } else {
        memcpy(f, final, 3*sizeof(double));
        f += 3;
      }
    } // end of for all objects
    if (internalMode & S_CONJSPREAD) {
      double w, final[3];
      w = hypota(3, mean)/nObjects;
      w = sqrt(-26262.45*log(w));
      XYZtoLBR(mean, final);
      memcpy(f, final, 2*sizeof(double));
      f[2] = w;
      f += 3;
    }
  } // end of for all dates

  // turn variances into standard deviations and radians into degrees
  f = f0;
  if ((internalMode & S_XYZ) == 0)
    for (i = 0; i < nJD*nObjects; i++) {
      f[0] *= RAD;
      f[1] *= RAD;
      f += 3;
    }
  else if (internalMode & S_CONJSPREAD)
    for (i = 0; i < nJD; i++) {
      f[0] *= RAD;
      f[1] *= RAD;
      f += 3;
    }

  return result;
}
//-------------------------------------------------------------------
/*
    Covariances

 In the transformation from x to y, dy_j = Sum_k dy_j/dx_k dx_k, where
 dy_j/dx_k is the partial derivative of y_j with respect to x_k.  If
 we take the dx_k to have a distribution with E[dx_k] = 0 then E[dx_k
 dx_l] = cov[x_k,x_l], and
   cov[y_j,y_m] = E[dy_j dy_m] = E[Sum_k Sum_l (dy_j/dx_k)(dy_m/dx_l)
     dx_k dx_l] = Sum_k Sum_l (dy_j/dx_k)(dy_m/dx_l) cov[x_k,x_l].

 The VSOP theory yields heliocentric ecliptical longitude and
 latitude, referred to the mean dynamical ecliptic and equinox of the
 date, and the heliocentric distance of the planets and also estimates
 for their variances.  I assume that the covariances (but not the
 variances) of these quantities are zero, i.e. that the errors in
 these quantities are uncorrelated.

 The first transformation is from the mean dynamical ecliptic to the
 mean FK5 ecliptic, and this I assume does not change the covariances.

 The second transformation is for ecliptic precession, and this I
 assumed does not change the covariances.

 The third transformation is from polar L, B, R to cartesian X, Y, Z,
 according to
   X = R cos(B) cos(L)
   Y = R cos(B) sin(L)
   Z = R sin(B).
 The partial derivatives are
          L:                 B:               R:
  X:  -R cos(B) sin(L)   -R sin(B) cos(L)    cos(B) cos(L)
  Y:   R cos(B) cos(L)   -R sin(B) sin(L)    cos(B) sin(L)
  Z:   0                  R cos(B)           sin(B)

 The fourth transformation is to subtract the heliocentric cartesian
 coordinates of the observer's position from those of the observed
 object, according to
   x = X - X0
   y = Y - Y0
   z = Z - Z0
 The covariances of the differences are equal to the sums of the
 covariances of the terms:
   cov[x_k,x_l] = cov[X_k,X_l] + cov[X0_k,X0_l].

 The fifth transformation is the correction for nutation, and this I
 assume does not change the covariances.

 Further transformations depend on which quantities are desired by the user.

 Transformation from ecliptic planetocentric cartesian coordinates to
 elongation E and phase angle P:
   robs = sqrt(xobs^2 + yobs^2 + zobs^2)         observer to Sun
   robj = sqrt(xobj^2 + yobj^2 + zobj^2)         object to Sun
   d = sqrt((xobs - xobj)^2 + (yobs - yobj)^2 + (zobs - zobj)^2)
                                                 observer to object
   E = arccos((robs^2 + d^2 - robj^2)/(2*robs*d))
   P = arccos((robj^2 + d^2 - robs^2)/(2*robj*d))

 Transformation from cartesian coordinates to polar coordinates, according to
   l = arctan(y/x)
   b = arctan(z/sqrt(x^2 + y^2))
   r = sqrt(x^2 + y^2 + z^2)
 The partial derivatives are
       x:            y:            z:
   l: -y/h^2         x/h^2         0
   b: -x z/(h r^2)  -y z/(h r^2)   h/r^2
   r:  x/r           y/r           z/r
 where h = sqrt(x^2 + y^2)

 Transformation from apparent geocentric ecliptical angular
 coordinates to apparent geocentric equatorial angular coordinates
 (right ascension and declination), according to
   a = arctan((sin(l) cos(eps) - tan(b) sin(eps))/cos(l))
   d = arcsin(sin(b) cos(eps) + cos(b) sin(eps) sin(l))
 where eps is the apparent obliquity of the ecliptic, which I assume
 has no effect on the covariances.  The partial derivatives are
         l:
   a: (cos(eps) cos(l) + sin(l) tan(a)) cos^2(a)
   d: cos(b) cos(l) sin(eps)/cos(d)
         b:
   a: -sin(eps) cos^2(a)/(cos(l) cos^2(b))
   d: (cos(b) cos(eps) - sin(b) sin(eps) sin(l))/cos(d)

 Transformation from geocentric to topocentric coordinates, according to
   A = cos(d) sin(H)
   B = cos(d) cos(H) - rcp s/r
   C = sin(d) - rsp s/r
   q = sqrt(A^2 + B^2 + C^2)
   H' = arctan(A/B)
   d' = arcsin(C/q)
 where H is the hour angle, rcp = rho' cos(phi') and rsp = rho'
 sin(phi') are the geocentric coordinates of the observer, and s is
 a constant, all of which I assume do not contribute to the
 covariances.  The partial derivatives are
          H:
  H': cos(d) (r cos(d) - cos(H) rcp s)/(r U^2)
  d': -A C rcp s/(U r q^2)
          d:
  H': sin(d) sin(H) rcp s/(r U^2)
  d': (r q^2 cos(d) + C s (rsp cos(d) - rcp sin(d) cos(H)))/(U r q^2)
          r:
  H': cos(d) sin(H) rcp s/(r U^2)
  d': s (rsp - C (B rcp + C rsp))/(r^2 U)
 where U = sqrt(A^2 + B^2)

 Transformation from equatorial to horizontal coordinates (azimuth and
 elevation) according to
  A = arctan(sin(H)/(cos(H) sin(phi) - tan(d) cos(phi))
  h = arcsin(sin(phi) sin(d) + cos(phi) cos(d) cos(H)).
 The partial derivatives are
            H:
  A:  sin(A) cos(A) cot(H) + sin^2(A) sin(phi)
  h: -cos(d) cos(phi) sin(H)/cos(h)
            d:
  A:  cos(phi) sin^2(A)/(sin(H) cos^2(d))
  h: (-cos(H) cos(phi) sin(d) + cos(d) sin(phi))/cos(h).

*/
