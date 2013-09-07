/* File astron.c */
/* ANA routines for calculating various astronomical ephemerides and */
/* for transforming dates between various calendars */
/* Formulas from "Astronomical Algorithms" */
/* by Jean Meeus, Willmann-Bell, Inc. (1991) ("JM"), and from "Explanatory */
/* Supplement to the Astronomical Almanac", edited by P. K. Seidelmann, */
/* University Science Books (1992) ("ES"). */

/* Note on time systems: 
   UTC = "Universal Coordinated Time": basis of everyday time; local
         official time is commonly equal to UTC plus a fixed amount,
	 usually an integer number of hours.  UTC differs from TAI
	 by an integral number of seconds.  UTC days may differ
	 in length from 86400 seconds because of leap seconds that
	 are inserted irregularly and infrequently to keep time in step
	 with the Earth's rotation.
   UT1 = "Universal Time 1": time sequence linked to the Earth's rotation
         by (slight) variation of the length of its second.  UTC leap
	 seconds are inserted such that UT1 never differs more than
	 0.9 seconds from UTC.
   TAI = "International Atomic Time": a time sequence without regards
         to the Earth's rotation, without leap seconds.
   TDT = "Terrestrial Dynamical Time": a time sequence introduced as
         independent variable in calculations of planetary motion.
	 It is currently defined equal to TAI + 32.184 seconds.
   GMT = "Greenwich Mean Time": historically, the official time of
         Great Britain.  Currently used as a synonym of UTC.

   UTC leap seconds are introduced such that the sequence of UTC second
   markers is 23:59:59 23:59:60 00:00:00.

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
   since 0 UT on 1 January 1970.    LS 24sep98
   */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <assert.h>
#include <time.h>
#include "action.h"
#include "astron.h"
#include "astrodat2.h"
#include "astrodat3.h"
#include "calendar.h"
#include "vsop.h"
/* #include "astrodat.h" */
static char rcsid[] __attribute__ ((unused)) =
 "$Id: astron.c,v 4.0 2001/02/07 20:36:57 strous Exp $";

#define extractbits(value, base, bits) (((value) >> (base)) & ((1 << (bits)) - 1))

#define S_ECLIPTICAL	(1)
#define S_EQUATORIAL	(2)
#define S_HORIZONTAL	(3)
#define S_ELONGATION	(4)
#define S_COORDS	(7)
#define S_XYZ		(1<<3) // 8
#define S_LIGHTTIME	(1<<4) // 16
#define S_DATE		(1<<5) // 32
#define S_TDT		(1<<6) // 64
#define S_ABERRATION	(1<<8) // 256
#define S_NUTATION	(1<<9) // 512
#define S_QELEMENTS	(1<<10) // 1024
#define S_FK5		(1<<11) // 2048
#define S_TRUNCATEVSOP  (1<<12) // 4096
#define S_CONJSPREAD    (1<<13) // 8192
#define S_PLANETOCENTRIC (1<<14) // 16384
#define S_KEEPDIMS	(1<<15) // 32768
#define S_VOCAL         (1<<16) // 65536
#define S_VSOP87A       (0)
#define S_VSOP87C       (1<<17) // 131072
#define S_VSOP          (S_VSOP87A|S_VSOP87C)
#define S_BARE          (1<<18) // 262144
#define AUtoJD		((149.597870691e9/299792458)/86400)
#define NOBJECTS	9

#define EARTH		3

Int	findint(Int, Int *, Int);
void	UTC_to_TAI(Double *), TAI_to_UTC(Double *);
void	CJDLT_to_TAI(Double *), TAI_to_CJDLT(Double *);

char *GregorianMonths[] = {
  "January", "February", "March", "April", "May", "June", "July",
  "August", "September", "October", "November", "December"
};

char *IslamicMonths[] = {
  "Muharram", "Safar", "Rabi`a I", "Rabi`a II", "Jumada I", "Jumada II",
  "Rajab", "Sha`ban", "Ramadan", "Shawwal", "Dhu al-Q`adah",
  "Dhu al-Hijjah"
};

char *tikalVenteina[] = {
  "Imix'", "Ik'", "Ak'b'al", "K'an", "Chikchan", "Kimi", "Manik'", "Lamat",
  "Muluk", "Ok", "Chuwen", "Eb'", "B'en", "Ix", "Men", "Kib'", "Kab'an",
  "Etz'nab'", "Kawak", "Ajaw"
};

char *tikalMonth[] = {
  "Pop", "Wo", "Sip", "Sotz'", "Sek", "Xul", "Yaxk'in", "Mol", "Ch'en",
  "Yax", "Sac", "Keh", "Mak", "K'ank'in", "Muwan", "Pax", "K'ayab'", "Kumk'u",
  "Wayeb'"
};

char *HebrewMonths[] = {
  "Tishri", "Heshvan", "Kislev", "Tevet", "Shevat", "Adar", "Nisan",
  "Iyar", "Sivan", "Tammuz", "Av", "Elul"
};

char *EgyptianMonths[] = {
  "Thoth", "Phaophi", "Athyr", "Choiak", "Tybi", "Mecheir", "Phamenoth",
  "Pharmuthi", "Pachon", "Payni", "Epiphi", "Mesore", "epagomenai"
};

Int	nonae[] = {
  5, 5, 7, 5, 7, 5, 7, 5, 5, 7, 5, 5
}, idus[] = {
  13, 13, 15, 13, 15, 13, 15, 13, 13, 15, 13, 13
};
char *latinMonthsI[] = {
  "Ianuariis", "Februariis", "Martiis", "Aprilibus", "Maiis", "Iuniis",
  "Iuliis", "Augustis", "Septembribus", "Octobribus", "Novembribus",
  "Decembribus"
}, *latinMonthsII[] = {
  "Ianuarias", "Februarias", "Martias", "Apriles", "Maias", "Iunias",
  "Iulias", "Augustas", "Septembres", "Octobres", "Novembres", "Decembres"
};

char **calendars[] = {
  GregorianMonths, IslamicMonths, HebrewMonths, EgyptianMonths
};
Int calendarType[] = {
  CAL_COMMON, CAL_ISLAMIC, CAL_HEBREW, CAL_EGYPTIAN
};

/* 2nd-degree polynomial evaluation */
#define pol2(a0,a1,a2,t) ((a0) + (t)*((a1) + (t)*(a2)))

/* 3rd-degree polynomial evaluation */
#define pol3(a0,a1,a2,a3,t) ((a0) + (t)*((a1) + (t)*((a2) + (t)*(a3))))

/* 4th-degree polynomial evaluation */
#define pol4(a0,a1,a2,a3,a4,t) (a0 + t*(a1 + t*(a2 + t*(a3 + t*a4))))

Double JDE(Double, Int);
/* time in centuries since epoch 1900.0 */
#define TC1900(JD) ((JD - 2415020.0)/36525)
/* time in centuries since epoch 2000.0 */
#define TC2000(JD) ((JD - 2451545.0)/36525)
/* time in millennia since epoch 2000.0 */
#define TM2000(JD) ((JD - 2451545.0)/365250)

/* 2nd-degree polynomial evaluation (modulo) */
#define mpol2(a0,a1,a2,t,n) (fmod(a0 + t*(a1 + t*a2),n))

/* 3rd-degree polynomial evaluation (modulo) */
#define mpol3(a0,a1,a2,a3,t,n) (fmod(a0 + t*(a1 + t*(a2 + t*a3)),n))

/* 4th-degree polynomial evaluation (modulo) */
#define mpol4(a0,a1,a2,a3,a4,t,n) (fmod(a0 + t*(a1 + t*(a2 + t*(a3 + t*a4))),n))

#define J2000	(2451545.0)
#define B1950   (2433282.4235)

#define ecos	0.9174369451	/* cos(ecliptic2000) */
#define esin	0.3978812030	/* sin(ecliptic2000) */

static Int	*extraIDs;
static struct extraInfo {
  Int		nterms;
  Double	equinox;
  Double	absmag;
  char		*comment;
  struct orbitParams {
    Double	JDE;
    Double	q;
    Double	e;
    Double	v_factor;
    Double	xfac;
    Double	yfac;
    Double	zfac;
    Double	xangle;
    Double	yangle;
    Double	zangle;
    Double	M;
    Double	n;
  } *orbits;
} *extraOrbits = NULL;
  
static Int	nExtraObj = 0;

static Double	extraElements[9];
static char	haveExtraElements;

extern Int getAstronError;
extern Int fullVSOP;

void LBRtoXYZ(Double *pos, Double *pos2),
  XYZtoLBR(Double *, Double *),
  XYZ_eclipticPrecession(Double *pos, Double equinox1, Double equinox2);

Int idiv(Int x, Int y)
     /* returns the largest integer n such that x >= y*n */
{
  return (Int) floor(((Float) x)/y);
}

void printXYZtoLBR(Double *xyz);
void printLBRtoXYZ(Double *lbr);
void printHBRtoXYZ(Double *lbr);
void showraddms(char *prefix, Double x);
void showradhms(char *prefix, Double x);

#define TAI_to_TT(jd)	(*(jd) += 32.184/86400)
#define TT_to_TAI(jd)	(*(jd) -= 32.184/86400)

/* Calendars: (all dates in the Gregorian calendar (from 15 October 1582) */
/*            or the Julian proleptic calendar (before 15 October 1582), */
/*            unless indicated otherwise).  The year preceding Year 1 */
/*            is called Year 0, and preceding that is Year -1 (astronomical */
/*            reckoning). */
/* Gregorian: the liturgical calendar of the Christian faith, and the civil */
/*            calendar of many countries in the world.  It was installed as */
/*            liturgical calendar by Pope Gregory XIII on 15 October 1582 */
/*            (following 4 October 1582 on the Julian calendar).  The start */
/*            of the calendar was fixed by Dionysius Exiguus in the 6th */
/*            century. */
/* Islamic (civil): the religious calendar of Islam is based on the first */
/*            sighting of the lunar crescent after New Moon.  For civil use, */
/*            various tabulated calenders have been used.  The one used here */
/*            is the "civil" one most often used by historians, in */
/*            which A.H. 1 Muharram 1 corresponds to 16 July 622.  If the */
/*            astronomical epoch is preferred, then 1 Muharram 1 corresponds */
/*            to 15 July 622. */
/* Julian Proleptic: a calendar which differs slightly from the Gregorian */
/*            in the occurrence of leap years.  Devised by Julius Caesar */
/*            in or just after -45.  Used by historians with extension */
/*            into the past (proleptic).  */
/*--------------------------------------------------------------------------*/
Double meanTropicalYear(Double JD)
     /* the mean length of the tropical year, in days, as a function of */
     /* Julian Date JD(TDT) [ES12.11-1,Laskar1986]. */
     /* The time from one vernal equinox to the next may vary from this */
     /* mean by several minutes. */
{
  Double	T;

  T = (JD - 2451545.0)/36525;
  return pol3(365.2421896698, -6.15359e-6, -7.29e-10, 2.64e-10, T);
}
/*--------------------------------------------------------------------------*/
Double meanSynodicMonth(Double JD)
     /* the mean length of the synodic month, in days, as a function of */
     /* Julian Date JD(TDT) [ES12.11-2,Chapront-Touz\'e&Chapront1988]. */
     /* Any particular phase cycle may vary from the mean by up to */
     /* seven hours. */
{
  Double	T;

  T = (JD - 2451545.0)/36525;
  return pol2(29.5305888531, 2.1621e-7, -3.64e-10, T);
}
/*--------------------------------------------------------------------------*/
Int EasterDate(Int year, Int *month, Int *day)
     /* the date of Easter in year y (Julian until 1582, Gregorian after */
     /* 1582) is returned in *month and *day. [ES12.22,Oudin1940; JM4] */
{
  Int	c, n, k, i, j, l;

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
/*--------------------------------------------------------------------------*/
Double lunarToJD(Double k)
{
  Double	T;

  k -= 83017;
  T = k/1236.85;
  return 2451550.09765 + 29.530588853*k
    + T*T*(1.337e-4 + T*(-1.5e-7 + 7.3e-10*T));
}
/*--------------------------------------------------------------------------*/
Double JDtoLunar(Double JD)
{
  Double	k, JD2;

  k = floor((JD - 2451550.09765)/29.530588853) + 83017;
  JD2 = lunarToJD(k);
  k += (JD - JD2)/29.530588853;
  return k;
}
/*--------------------------------------------------------------------------*/
Int DatetoCJDN(Int year, Int month, Int day, Int calendar)
/* Calculated the Chronological Julian Day Number at the given
   calendar date.  "calendar" indicates the calendar that year, month,
   and date are in.  possible values for calendar: CAL_GREGORIAN
   (Gregorian, started 15 October 1582); CAL_ISLAMIC (civil calendar);
   CAL_JULIAN (Julian proleptic calendar). */
{
  static Int (*f[])(Int, Int, Int) =
    { NULL, CommontoCJDN, GregoriantoCJDN, IslamictoCJDN,
      JuliantoCJDN, HebrewtoCJDN, EgyptiantoCJDN };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f)
      || !f[calendar])
    return 0;
  
  return f[calendar](year, month, day);
}
/*--------------------------------------------------------------------------*/
Double DatetoCJD(Int year, Int month, Double day, Int calendar)
/* Calculates the Chronological Julian Date at the given date.
   "calendar" indicates the calendar that year, month, and date are
   in.  possible values for calendar: CAL_GREGORIAN (Gregorian,
   started 15 October 1582); CAL_ISLAMIC (civil calendar); CAL_JULIAN
   (Julian proleptic calendar).  The Chronological Julian Date is
   returned in the same time base as its arguments (TAI, UTC, TT, or
   something else altogether) */
{
  static Double (*f[])(Int, Int, Double) =
    { NULL, CommontoCJD, GregoriantoCJD, IslamictoCJD,
      JuliantoCJD, HebrewtoCJD, EgyptiantoCJD };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f)
      || !f[calendar])
    return 0.0;
  
  return f[calendar](year, month, day);
}
/*--------------------------------------------------------------------------*/
void CJDNtoDate(Int CJDN, Int *year, Int *month, Int *day, Int calendar)
     /* returns the date corresponding to Julian Date JD. */
     /* possible values for calendar (see JulianDate): CAL_GREGORIAN, */
     /* CAL_ISLAMIC, CAL_JULIAN, CAL_HEBREW, CAL_EGYPTIAN, CAL_COMMON */
{
  static void (*f[])(Int, Int *, Int *, Int *) =
    { CJDNtoCommon, CJDNtoGregorian, CJDNtoIslamic,
      CJDNtoJulian, CJDNtoHebrew, CJDNtoEgyptian };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f))
    *year = *month = *day = 0;
  else
    f[calendar](CJDN, year, month, day);
}
/*--------------------------------------------------------------------------*/
void CJDtoDate(Double CJD, Int *year, Int *month, Double *day, Int calendar)
     /* returns the date corresponding to Chronological Julian Day
        CJD.  possible values for calendar (see JulianDate):
        CAL_GREGORIAN, CAL_ISLAMIC, CAL_JULIAN, CAL_HEBREW,
        CAL_EGYPTIAN, CAL_COMMON */
{
  static void (*f[])(Double, Int *, Int *, Double *) =
    { CJDtoCommon, CJDtoGregorian, CJDtoIslamic,
      CJDtoJulian, CJDtoHebrew, CJDtoEgyptian };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f)
      || !f[calendar])
    *year = *month = *day = 0;
  else
    f[calendar](CJD, year, month, day);
}
/*--------------------------------------------------------------------------*/
void findTextDate(char *text, Int *year, Int *month, Double *day, Int *cal,
		  Int order)
/* interpret the <text> as a date string in the calendar indicated by */
/* <*cal> (e.g., CAL_HEBREW).  If <*cal> is CAL_DEFAULT, then the */
/* used calendar is deduced from (the start of) the month name.  <order> */
/* indicates the order of the year and day numbers, it must be either */
/* CAL_YMD or CAL_DMY. The */
/* deduced calendar is returned in <*cal>, and the year, month number, */
/* and day are returned in <*year>, <*month>, and <*day>, respectively. */
{
  Int	i, j, nnum = 0, ntext = 0, c;
  char	*daystring = NULL, *monthstring = NULL, *yearstring = NULL, *text0;

  /* first we seek the starts of the day, month, and year.  We assume that */
  /* the day and year are numerical, and the month is in text form. */
  /* If <order> == CAL_DMY, then we assume that the first number is the day */
  /* and the second number is the year; if <order> == CAL_YMD, then */
  /* the first number is the */
  /* year and the second number is the day.  The month string may come */
  /* anywhere before, between, or after the numbers. */
  text0 = text;
  while (isspace((Int) *text))	/* skip whitespace */
    text++;
  do {
    if (*text == '-' || *text == '+' || isdigit((Int) *text)) {	/* a number */
      switch (nnum) {
	case 0:			/* the first number */
	  switch (order) {
	    case CAL_DMY:
	      daystring = text;	/* take it for the day */
	      break;
	    case CAL_YMD:
	      yearstring = text; /* take it for the year */
	      break;
	  }
	  break;
	case 1:			/* the second number */
	  switch (order) {
	    case CAL_DMY:
	      yearstring = text; /* take it for the year */
	      break;
	    case CAL_YMD:	/* take it for the day */
	      daystring = text;
	      break;
	  }
	  break;
      }
      nnum++;
      while (*text && !isspace((Int) *text)) /* find next whitespace */
	text++;
      while (*text && isspace((Int) *text)) /* skip it */
	text++;
    } else {			/* a non-number */
      switch (ntext) {
	case 0:			/* take it for the month name */
	  monthstring = text;
	  break;
      }
      ntext++;
      text++;
      do {
	while (*text && !isdigit((Int) *text) && *text != '-' && *text != '+')
	  text++;
	if (!isspace((Int) text[-1]))
	  while (*text && !isspace((Int) *text))
	    text++;
      } while (*text
	       && (!(isdigit((Int) *text) || *text == '+' || *text == '-')
		   || !isspace((Int) text[-1])));
    }
  } while (*text);

  if (yearstring) { 		/* have a year number */
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
    /* we must temporarily terminate the month string, or else any following */
    /* numbers (day, year) may be interpreted as part of the month name */
    if (daystring > monthstring) {
      /* the day number follows the month name; we seek the whitespace */
      /* after the last character of the month name */
      text = daystring;
      while (text > text0 && isspace((Int) text[-1]))
	text--;
    } else if (yearstring > monthstring) {
      text = yearstring;
      while (text > text0 && isspace((Int) text[-1]))
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
	if (i == 12) {		/* didn't find it */
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
	if (i == 12) {		/* didn't find it */
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
	if (i == 12) {		/* didn't find it */
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
	if (i == 12) {		/* didn't find it */
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
    /* now we must restore the original string */
    if (daystring > monthstring || yearstring > monthstring)
      *text = c;
  } else
    *month = 1;
}
/*--------------------------------------------------------------------------*/
void roman_numeral(Int number)
/* Stores at curScrat the number in Roman numeral form if it is positive and
   less than 4,000, otherwise in regular digital form. */
{
  char	*p;
  Int	i;

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
/*--------------------------------------------------------------------------*/
Int construct_output_dims(Int *input_dims, Int num_input_dims,
			  Int inputs_per_entry,
			  Int *output_dims, Int output_dims_size,
			  Int outputs_per_entry)
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
  Int num_output_dims = 0;

  if (!input_dims || num_input_dims < 1 || inputs_per_entry < 1
      || !output_dims || output_dims_size < 1 || outputs_per_entry < 1)
    /* illegal values */
    return -1;
  if (input_dims[0] % inputs_per_entry != 0)
    /* first input dimension doesn't fit a whole number of entries */
    return -2;
  
  if (outputs_per_entry > 1) {
    /* put the output values for each entry into the first dimension */
    *output_dims++ = outputs_per_entry;
    output_dims_size--;
    num_output_dims++;
  }
  if (inputs_per_entry > 1) {
    /* don't need these in the output */
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
/*--------------------------------------------------------------------------*/
Int gcd(Int a, Int b)
{
  Int c;

  if (!a || !b)
    return 1;
  if (a < 0)
    a = -a;
  if (b < 0)
    b = -b;
  if (b > a) {
    c = b; b = a; a = c;
  }
  /* now a >= b > 0 */
  c = a % b;
  while (c) {
    a = b; b = c;
  }
  return b;
}
/*--------------------------------------------------------------------------*/
Int ana_calendar(Int narg, Int ps[])
{
  Int result, input_elem_per_date, output_elem_per_date, iq;
  Int *dims = NULL, ndim = 0;
  loopInfo tgtinfo, srcinfo;
  pointer src, tgt;
  /* enum Calendar_order fromorder, toorder; */
  enum Calendar fromcalendar, tocalendar;
  enum Calendar_timescale fromtime, totime;
  enum Symboltype inputtype, internaltype, outputtype;
  enum Calendar_outputtype outputkind;
  static struct {
    Int to_elements_per_date;   /* translating to calendar date */
    Int from_elements_per_date; /* translating from calendar date */
    void (*CJDNtoCal)(Int const *CJDN, Int *date);
    void (*CJDtoCal)(Double const *CJD, Double *date);
    void (*CaltoCJDN)(Int const *date, Int *CJDN);
    void (*CaltoCJD)(Double const *date, Double *CJD);
    void (*CJDNtoCalS)(Int const *CJDN, char **date);
    void (*CJDtoCalS)(Double const *CJD, char **date);
    void (*CalStoCJDN)(char * const *date, Int *CJDN);
    void (*CalStoCJD)(char * const *date, Double *CJD);
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

  /* initialize to zero */
  memset(&srcinfo, 0, sizeof(srcinfo));
  memset(&tgtinfo, 0, sizeof(tgtinfo));
  src.v = NULL;
  tgt.v = NULL;

  fromcalendar = extractbits(internalMode, CAL_CALENDAR_BASE,
			     CAL_CALENDAR_BITS);
  tocalendar = extractbits(internalMode, CAL_CALENDAR_BASE + CAL_CALENDAR_BITS,
			   CAL_CALENDAR_BITS);
  outputkind = extractbits(internalMode, CAL_OUTPUT_BASE, CAL_OUTPUT_BITS); /* /NUMERIC, /TEXT, /ISOTEXT */
  /*
  fromorder = extractbits(internalMode, CAL_ORDER_BASE, CAL_ORDER_BITS); / /FROMYMD, /FROMDMY /
  toorder = extractbits(internalMode, CAL_ORDER_BASE + CAL_ORDER_BITS,
			CAL_ORDER_BITS); / /TOYMD, /TODMY /
  */
  fromtime = extractbits(internalMode, CAL_TIME_BASE, CAL_TIME_BITS);
  totime = extractbits(internalMode, CAL_TIME_BASE + CAL_TIME_BITS,
		       CAL_TIME_BITS);

  numerical_or_string(ps[0], &dims, &ndim, NULL, NULL);

  /* If no specific "from" calendar is specified, then assume the Common
     calendar if the input is a string or string array, or if the input
     has 3 elements in its first dimension.  Otherwise, assume CJD. */
  if (fromcalendar == CAL_DEFAULT) { 
    if (symbolIsString(ps[0])
        || dims[0] == 3)
      fromcalendar = CAL_COMMON;
    else
      fromcalendar = CAL_CJD;
  }

  void (*CaltoCJDN)(Int const *date, Int *CJDN)
    = cal_data[fromcalendar].CaltoCJDN;
  void (*CaltoCJD)(Double const *date, Double *CJDN)
    = cal_data[fromcalendar].CaltoCJD;
  /*
  void (*CalStoCJDN)(char * const *date, Int *CJDN)
    = cal_data[fromcalendar].CalStoCJDN;
  */
  void (*CalStoCJD)(char * const *date, Double *CJD)
    = cal_data[fromcalendar].CalStoCJD;

  iq = ps[0];
  {
    enum Symboltype type;

    /* if the input type is integer, then promote to LONG.  If the
       input type is floating point, then promote to DOUBLE. */
    type = symbol_type(iq);
    if (isIntegerType(type)) {
      iq = ana_long(1, &iq);
      inputtype = ANA_LONG;
    } else if (isFloatType(type)) {
      iq = ana_double(1, &iq);
      inputtype = ANA_DOUBLE;
    } else                        /* must be text type */
      inputtype = ANA_STRING_ARRAY;
  }

  /* number of input elements expected per calendar date */
  input_elem_per_date = cal_data[fromcalendar].from_elements_per_date;
  if (isStringType(inputtype))
    input_elem_per_date = 1; /* strings have all elements of a date in a single string */

  if (dims[0] % input_elem_per_date)
    return anaerror("Incompatible first dimension: expected a multiple "
                    "of %d but found %d", ps[0], input_elem_per_date,
                    dims[0]);
  
  /* If no specific "to" calendar is specified, then assume the Common
     calendar if the (effective) "from" calendar is a
     one-element-per-date calendar, and otherwise CJD */
  if (tocalendar == CAL_DEFAULT) {
    if (input_elem_per_date == 1)
      tocalendar = CAL_COMMON;
    else
      tocalendar = CAL_CJD;
  }

  if (fromcalendar == tocalendar
      && fromtime == totime)    /* no conversion */
    return *ps;

  void (*CJDNtoCal)(Int const *CJDN, Int *date)
    = cal_data[tocalendar].CJDNtoCal;
  void (*CJDtoCal)(Double const *CJD, Double *date)
    = cal_data[tocalendar].CJDtoCal;
  void (*CJDNtoCalS)(Int const *CJDN, char **date)
    = cal_data[tocalendar].CJDNtoCalS;
  void (*CJDtoCalS)(Double const *CJD, char **date)
    = cal_data[tocalendar].CJDtoCalS;
  
  assert(CaltoCJDN || CaltoCJD);
  assert(CJDNtoCal || CJDtoCal);

  /* number of output elements per calendar date */
  output_elem_per_date = cal_data[tocalendar].to_elements_per_date;

  /* if the (promoted) input type is LONG, then the internal type is
     LONG if CaltoCJDN is not NULL, or DOUBLE if CaltoCJDN is NULL.
     If CaltoCJDN is NULL and the number of input elements per date
     is not equal to 1, then work with a DOUBLE copy of the input
     symbol. */
  if (inputtype == ANA_LONG) {
    if (CaltoCJDN && fromtime == totime)
      internaltype = ANA_LONG;
    else {
      internaltype = ANA_DOUBLE;
      if (input_elem_per_date != 1) {
        iq = ana_double(1, &iq);
        inputtype = ANA_DOUBLE;
      }
    }
  }

  /* if the (promoted) input type is DOUBLE, then the internal type
     is DOUBLE if CaltoCJD is not NULL, or LONG if CaltoCJD is NULL.
     If CaltoCJD is NULL and the number of input elements per date
     is not equal to 1, then work with a LONG ("floor") copy of the
     input symbol. */
  
  else if (inputtype == ANA_DOUBLE) {
    if (CaltoCJD || fromtime != totime)
      internaltype = ANA_DOUBLE;
    else {
      internaltype = ANA_LONG;
      if (input_elem_per_date != 1) {
        Int ana_floor(Int, Int *);
        iq = ana_floor(1, &iq);
        inputtype = ANA_LONG;
      }
    }
  }

  /* if the input type is a string type, then the internal type
     depends on the output kind. */
  else {
    if (outputkind == CAL_DOUBLE)
      internaltype = ANA_DOUBLE;
    else
      internaltype = ANA_LONG;
    input_elem_per_date = 1; /* all input elements in a single text value */
  }
  
  /* for numeric output, the output type is DOUBLE if CJDNtoCal is
     NULL (i.e., a conversion routine to LONG is not available), or
     LONG if CJDtoCal is NULL (i.e., a conversion routine to DOUBLE is
     not available), or else is equal to the internal type.  For
     non-numerical (i.e., text) output, then output type indicates
     text.
  */
  if (outputkind == CAL_TEXT) {
    outputtype = ANA_STRING_ARRAY;
    output_elem_per_date = 1;   /* all date components in a single text value */
  } else if (outputkind == CAL_LONG || internaltype == ANA_LONG)
    outputtype = CJDNtoCal? ANA_LONG: ANA_DOUBLE;
  else if (outputkind == CAL_DOUBLE || internaltype == ANA_DOUBLE)
    outputtype = CJDtoCal? ANA_DOUBLE: ANA_LONG;
  else                          /* should not happen */
    outputtype = internaltype;

  {
    Int more[1], less[1], nMore, nLess;

    /* does the output need more elements than the input? */
    nMore = nLess = 0;
    if (output_elem_per_date != input_elem_per_date) { /* yes, different */
      if (output_elem_per_date > 1) { /* output needs more elements */
        nMore = 1;                    /* prefix one dimension */
        more[0] = output_elem_per_date;
      }
      if (input_elem_per_date > 1) { /* output needs fewer elements */
        nLess = 1;
        less[0] = input_elem_per_date; /* reduce 1st dimension */
      }
    }
    
    if (standardLoopX(iq, ANA_ZERO,
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
      return ANA_ERROR;
  }

  if (srcinfo.rdims[0] != input_elem_per_date) {
    /* we have a multiple of the expected number of input elements per
       date, but the first dimension must be exactly equal to the
       expected number else the standard loop ignores the excess
       elements.  We shift the excess to the second dimension. */
    Int dims[MAX_DIMS];

    assert(srcinfo.dims[0] % input_elem_per_date == 0);
    memcpy(dims, srcinfo.dims, srcinfo.ndim*sizeof(*dims));
    if (srcinfo.ndim == 1) {    /* there is only one dimension */
      dims[1] = 1;              /* add a 2nd dimension */
      srcinfo.ndim = 2;
    }
    Int d = srcinfo.dims[0]/input_elem_per_date;
    dims[1] *= d;
    dims[0] /= d;
    setupDimensionLoop(&srcinfo, srcinfo.ndim, dims, srcinfo.type,
                       srcinfo.naxes, srcinfo.axes, srcinfo.data,
                       srcinfo.mode);
  }    

  /* complain if the desired type of translation is not available.  We
     don't need to check for numerical types, because we demand that
     at least one of the translations (to LONG/DOUBLE) is available,
     and are prepared to handle the case where only one is
     available. */
  if (inputtype == ANA_STRING_ARRAY && !CalStoCJD)
    return anaerror("Translating from STRING is not supported for this calendar", ps[0]);

  /* complain if the desired type of translation is not available.  We
     don't need to check for numerical types, because we demand that
     at least one of the translations (from LONG/DOUBLE) is available,
     and are prepared to handle the case where only one is
     available. */
  if (outputtype == ANA_STRING_ARRAY)
    switch (internaltype) {
    case ANA_LONG:
      if (!CJDNtoCalS)
        return anaerror("Translating CJDN to STRING is not supported for this calendar", ps[0]);
      break;
    case ANA_DOUBLE:
      if (!CJDtoCalS)
        return anaerror("Translating CJD to STRING is not supported for this calendar", ps[0]);
      break;
    default:
      break;
    }

  /* now loop over all dates to translate */
  do {
    scalar timestamp, temp;

    /* translate input to CJD or CJDN */
    switch (internaltype) {
    case ANA_LONG:              /* translate to CJDN */
      switch (inputtype) {
      case ANA_LONG:
        CaltoCJDN(src.l, &timestamp.l);
        src.l += input_elem_per_date;
        break;
      case ANA_DOUBLE: /* only cases with one element per date reach here */
        assert(input_elem_per_date == 1);
        temp.l = (Int) floor(*src.d); /* translate from DOUBLE to LONG */
        CaltoCJDN(&temp.l, &timestamp.l); /* use LONG translation */
        src.d += input_elem_per_date;
        break;
      default:
        break;
      }
      break;
    case ANA_DOUBLE:            /* translate to CJD */
      switch (inputtype) {
      case ANA_LONG: /* only cases with one element per date reach here */
        assert(input_elem_per_date == 1);
        temp.d = (Double) *src.l; /* translate from LONG to DOUBLE */
        CaltoCJD(&temp.d, &timestamp.d); /* use DOUBLE translation */
        src.l += input_elem_per_date;
        break;
      case ANA_DOUBLE:
        CaltoCJD(src.d, &timestamp.d);
        src.d += input_elem_per_date;
        break;
      case ANA_STRING_ARRAY: case ANA_LSTRING:
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

    /* translate CJD or CJDN to output */
    switch (internaltype) {
    case ANA_LONG:
      switch (outputtype) {
      case ANA_LONG:
        CJDNtoCal(&timestamp.l, tgt.l);
        tgt.l += output_elem_per_date;
        break;
      case ANA_DOUBLE: /* only cases with one element per date reach here */
        assert(output_elem_per_date == 1);
        temp.d = (Double) timestamp.l; /* translate from LONG to DOUBLE */
        CJDtoCal(&temp.d, tgt.d);      /* use DOUBLE translation */
        tgt.d += output_elem_per_date;
        break;
      case ANA_STRING_ARRAY:
        CJDNtoCalS(&timestamp.l, tgt.sp);
        tgt.sp += output_elem_per_date;
        break;
      default:
        break;
      }
      break;
    case ANA_DOUBLE:
      switch (outputtype) {
      case ANA_LONG:
        temp.l = (Int) floor(timestamp.d); /* translate from DOUBLE to LONG */
        CJDNtoCal(&temp.l, tgt.l);         /* use LONG translation */
        tgt.l += output_elem_per_date;
        break;
      case ANA_DOUBLE:
        CJDtoCal(&timestamp.d, tgt.d);
        tgt.d += output_elem_per_date;
        break;
      case ANA_STRING_ARRAY:
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
  } while (advanceLoop(&tgtinfo, &tgt), 
	   advanceLoop(&srcinfo, &src) < srcinfo.rndim);
  if (!loopIsAtStart(&tgtinfo))
    return anaerror("Source loop is finished but target loop is not!", ps[0]);

  return result;
}
/*--------------------------------------------------------------------------*/
Int ana_calendar_OLD(Int narg, Int ps[])
     /* general calendar conversion routine */
     /* syntax: DATE2 = CALENDAR(DATE1, /FROMCALENDAR, /TOCALENDAR) */
     /* "/FROMCALENDAR":  /FROMCOMMON /FROMGREGORIAN /FROMISLAMIC */
     /*                   /FROMJULIAN /FROMJD /FROMCJD
                          /FROMUTC /FROMTAI /FROMTT */
     /* "/TOCALENDAR": /TOCOMMON /TOGREGORIAN /TOISLAMIC /TOJULIAN /TOJD */
     /*                /TOCJD /TOMAYAN /TOLONGCOUNT /TOUTC /TOTAI /TOTT */
{
  Int	n, *dims, ndim, nRepeat, type, i, iq, cal, newDims[MAX_DIMS],
    num_newDims, year, month, nd, d, t, v, m, sec, min, hour,
    fromcalendar, tocalendar, fromtime, totime, output, fromorder, toorder,
    outtype, iday;
  char	isFree = 0, *line, **monthNames;
  pointer	data, JD;
  Double	day;

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
    return *ps;			/* no conversion */
  iq = *ps;
  switch (symbol_class(iq)) {
  case ANA_SCALAR:
    n = 1;
    nRepeat = 1;
    ndim = 1;
    dims = &n;
    type = scalar_type(iq);
    if (type < ANA_LONG) {
      iq = ana_long(1, ps);
      type = ANA_LONG;
    } else if (type == ANA_FLOAT) {
      iq = ana_double(1, ps);
      type = ANA_DOUBLE;
    }
    data.b = &scalar_value(iq).b;
    break;
  case ANA_ARRAY:
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
    if (type < ANA_LONG) {
      iq = ana_long(1, ps);
      type = ANA_LONG;
    } else if (type == ANA_FLOAT) {
      iq = ana_double(1, ps);
      type = ANA_DOUBLE;
    }
    data.l = (Int *) array_data(iq);
    break;
  case ANA_STRING:
    nRepeat = 1;
    n = 1;
    ndim = 1;
    dims = &n;
    type = ANA_STRING_ARRAY;
    data.sp = &string_value(iq);
    break;
  default:
    return cerror(ILL_CLASS, *ps);
  }

  switch (fromcalendar) {	/* source calendar */
  case CAL_DEFAULT:
    /* if the first dimension has 3 elements or if it is a string
       array, then we assume it is in the common calendar; otherwise
       we assume it is measured in Julian Days */
    if (type != ANA_STRING_ARRAY) {
      if (n == 3)
        fromcalendar = cal = CAL_COMMON;
      else if (type == ANA_DOUBLE)
        fromcalendar = cal = CAL_JD;
      else
        fromcalendar = cal = CAL_CJD;
    } else
      cal = CAL_DEFAULT;
    break;
  case CAL_JD: case CAL_CJD: case CAL_LUNAR: /* /FROMJD, /FROMCJD, /FROMLUNAR */
    nRepeat *= n;
    n = 1;
    /* fall-through */
  default:
    cal = fromcalendar;
    break;
  }

  outtype = type;               /* default */
  if (fromcalendar == CAL_LUNAR || tocalendar == CAL_LUNAR
      || fromcalendar == CAL_JD || tocalendar == CAL_JD 
      || type == ANA_STRING_ARRAY)
    outtype = ANA_DOUBLE;

  /* if outtype == ANA_LONG, then JD.l contains CJDN values
     if outtype == ANA_DOUBLE, then JD.d contains JD values
     CJDN = floor(JD + 0.5) */

  /* temporary space for JDs */
  switch (outtype) {
  case ANA_LONG:
    JD.l = (Int *) malloc(nRepeat*sizeof(Int));
    if (!JD.l)
      return cerror(ALLOC_ERR, *ps);
    break;
  case ANA_DOUBLE: case ANA_TEMP_STRING:
    JD.d = (Double *) malloc(nRepeat*sizeof(Double));
    if (!JD.d)
      return cerror(ALLOC_ERR, *ps);
    break;
  default:
    return cerror(ILL_CLASS, ps[0]);
  }

  if (type == ANA_STRING_ARRAY) {
    assert(outtype == ANA_DOUBLE);
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
	Int level = 0;
	long stride = 1;
	long d = 0, this = 0;
	
	p = data.sp[i];
	q = strchr(p,'\0');	/* end of string */
	while (q > p) {
	  while (q > p && !isdigit(q[-1]))
	    q--;
	  if (q > p) {
	    while (q > p && isdigit(q[-1]))
	      q--;
	    this = atol(q);
	    d += this*stride;
	    stride *= (level == 1? 18: 20);
	    level++;
	  }
	}	  
	JD.d[i] = 584282.5 + d;
      }
      break;
    default:
      return anaerror("Cannot parse text-based dates in this calendar", *ps);
    }
    cal = CAL_JD;               /* signify that translation to JD is complete */
  } else switch (cal) {		/* from calendar */
    case CAL_COMMON: case CAL_GREGORIAN: case CAL_ISLAMIC: case CAL_JULIAN:
    case CAL_HEBREW: case CAL_EGYPTIAN:
      if (n != 3)
	return
	  anaerror("Need 3 numbers (year, month, day) per calendar date!", *ps);
      break;
    case CAL_JD:		/* from Julian Day */
      assert(outtype == ANA_DOUBLE);
      switch (type) {
      case ANA_LONG:
        for (i = 0; i < nRepeat; i++)
          JD.d[i] = data.l[i];
        break;
      case ANA_DOUBLE:
        memcpy(JD.d, data.d, nRepeat*sizeof(*JD.d));
        break;
      }
      break;
    case CAL_CJD:		/* from Chronological Julian Day */
      switch (outtype) {
      case ANA_LONG:
        switch (type) {
        case ANA_LONG:
          memcpy(JD.l, data.l, nRepeat*sizeof(*JD.l));
          break;
        case ANA_DOUBLE:
          /* from JD to CJDN */
          for (i = 0; i < nRepeat; i++)
            JD.l[i] = floor(JDtoCJD(data.d[i]));
          break;
        }
        break;
      case ANA_DOUBLE:
        switch (type) {
        case ANA_LONG:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = CJDtoJD((Double) data.l[i]);
          break;
        case ANA_DOUBLE:
          memcpy(JD.d, data.d, nRepeat*sizeof(*JD.d));
          break;
        }
      }
      break;
    case CAL_LUNAR:		/* from lunar calendar */
      assert(outtype == ANA_DOUBLE);
      switch (type) {
      case ANA_LONG:
        for (i = 0; i < nRepeat; i++)
          JD.d[i] = lunarToJD((Double) data.l[i]);
        break;
      case ANA_DOUBLE:
        for (i = 0; i < nRepeat; i++)
          JD.d[i] = lunarToJD(data.d[i]);
        break;
      }
      cal = CAL_JD;
      break;
    default:
      switch (type) {
      case ANA_LONG:
        free(JD.l);
        break;
      case ANA_DOUBLE:
        free(JD.d);
        break;
      }
      return anaerror("Illegal source calendar specification (%1d)", 0, cal);
    }

  /* go from FROM_CALENDAR to Julian Date */
  if (cal != CAL_JD)		/* not done yet, calculate Julian date */
    switch (outtype) {
    case ANA_LONG:              /* convert to CJDN */
      switch (type) {
      case ANA_LONG:
        switch (fromorder) {
        case CAL_YMD:
          for (i = 0; i < nRepeat; i++)
            JD.l[i] = DatetoCJDN(data.l[3*i], data.l[3*i + 1],
                                 data.l[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.l[i] = DatetoCJDN(data.l[3*i + 2], data.l[3*i + 1],
                                 data.l[3*i], cal);
          break;
        }
        break;
      case ANA_DOUBLE:
        switch (fromorder) {
        case CAL_YMD:
          for (i = 0; i < nRepeat; i++)
            JD.l[i] = DatetoCJDN((Int) data.d[3*i], (Int) data.d[3*i + 1],
                                 (Int) data.d[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.l[i] = DatetoCJDN((Int) data.d[3*i + 2], (Int) data.d[3*i + 1],
                                 (Int) data.d[3*i], cal);
          break;
        }
        break;
      }
      break;
    case ANA_DOUBLE:            /* convert to JD */
      switch (type) {
      case ANA_LONG:
        switch (fromorder) {
        case CAL_YMD:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD(data.l[3*i], data.l[3*i + 1],
                                (Double) data.l[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD(data.l[3*i + 2], data.l[3*i + 1],
                                (Double) data.l[3*i], cal);
          break;
        }
        break;
      case ANA_DOUBLE:
        switch (fromorder) {
        case CAL_YMD:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD((Int) data.d[3*i], (Int) data.d[3*i + 1],
                                data.d[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD((Int) data.d[3*i + 2], (Int) data.d[3*i + 1],
                                data.d[3*i], cal);
          break;
        }
        break;
      }
      break;
    }

  switch (outtype) {
  case ANA_DOUBLE:
    /* now do any required time base translation */
    if (fromtime != totime) {	/* we need some conversion */
      /* we go through TAI, which is a uniform time base */
      switch (fromtime) {
      case CAL_UTC:		/* /FROMUTC */
	for (i = 0; i < nRepeat; i++)
	  UTC_to_TAI(&JD.d[i]);
	break;
      case CAL_TT:		/* /FROMTT */
	for (i = 0; i < nRepeat; i++)
	  TT_to_TAI(&JD.d[i]);
	break;
      }
      /* and now to the target base */
      switch (totime) {
      case CAL_UTC:		/* /TOUTC */
	for (i = 0; i < nRepeat; i++)
	  TAI_to_UTC(&JD.d[i]);
	break;
      case CAL_TT:		/* /TOTT */
	for (i = 0; i < nRepeat; i++)
	  TAI_to_TT(&JD.d[i]);
	break;
      }
      break;
    case ANA_LONG:                /* no time translation */
      break;
    }
  }
  
  /* n = number of input values per input calendar date */

  /* now from JD or CJDN to the required target calendar */
  if (tocalendar == CAL_DEFAULT)/* default */
    tocalendar = (fromcalendar == CAL_JD)? CAL_COMMON: CAL_JD;
  switch (tocalendar) {
  case CAL_COMMON:		/* to common */
    if (output != CAL_NUMERIC) { /* text output */
      if (nRepeat > 1) {
        num_newDims = construct_output_dims(dims, ndim, n,
                                            newDims, MAX_DIMS, 1);
        iq = array_scratch(ANA_TEMP_STRING, num_newDims, newDims);
        data.sp = (char **) array_data(iq);
      } else {
        iq = string_scratch(-1);
        data.sp = (char **) &string_value(iq);
      }
      allocate(line, 80, char);
    } else {			/* numeric output */
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 3);
      iq = array_scratch(outtype, num_newDims, newDims);
      data.d = (Double *) array_data(iq);
    }
    for (i = 0; i < nRepeat; i++) {
      if (outtype == ANA_DOUBLE)
        CJDtoDate(JD.d[i], &year, &month, &day, CAL_COMMON);
      else
        CJDNtoDate(JD.l[i], &year, &month, &iday, CAL_COMMON);
      switch (output) {
      case 999 /* CAL_ISOTEXT */:
        if (outtype == ANA_DOUBLE) {
          sec = (day - (Int) day)*86400;
          min = sec/60;
          sec = sec % 60;
          hour = min/60;
          min = min % 60;
          sprintf(line,"%4d-%02d-%02dT%02d:%02d:%02d", year, month,
                  (Int) day, hour, min, sec);
        } else
          sprintf(line,"%4d-%02d-%02d", year, month, iday);
        *data.sp++ = strsave(line);
        break;
      case CAL_TEXT:
        switch (toorder) {
        case CAL_YMD:
          if (outtype == ANA_DOUBLE)
            sprintf(line, "%1d %s %1d", year,
                    GregorianMonths[month - 1], (Int) day);
          else
            sprintf(line, "%1d %s %1d", year,
                    GregorianMonths[month - 1], iday);
          break;
        case CAL_DMY:
          if (outtype == ANA_DOUBLE)
            sprintf(line, "%1d %s %1d", (Int) day,
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
          if (outtype == ANA_DOUBLE) {
            *data.d++ = year;
            *data.d++ = month;
            *data.d++ = day;
          } else {
            *data.l++ = year;
            *data.l++ = month;
            *data.l++ = iday;
          }
          break;
        case CAL_DMY:
          if (outtype == ANA_DOUBLE) {
            *data.d++ = day;
            *data.d++ = month;
            *data.d++ = year;
          } else {
            *data.l++ = iday;
            *data.l++ = month;
            *data.l++ = year;
          }
          break;
        }
        break;
      }
    }
    if (!isFree)
      free(JD.l);
    if (output != CAL_NUMERIC)
      free(line);
    if (symbol_class(iq) == ANA_STRING)
      symbol_memory(iq) = strlen(string_value(iq)) + 1;
    return iq;
  case CAL_GREGORIAN: case CAL_JULIAN: /* to Gregorian */
    monthNames = GregorianMonths;
    break;
  case CAL_ISLAMIC:		/* to Islamic */
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
      iq = array_scratch(ANA_TEMP_STRING, num_newDims, newDims);
      data.sp = (char **) array_data(iq);
    } else {
      iq = string_scratch(-1);
      data.sp = (char **) &string_value(iq);
    }
    for (i = 0; i < nRepeat; i++) {
      char	*p = curScrat, type = 0;
      Int	l;

      if (outtype == ANA_DOUBLE) {
        CJDtoDate(JD.d[i], &year, &month, &day, CAL_COMMON);
        iday = (Int) day;
      } else
        CJDNtoDate(JD.l[i], &year, &month, &iday, CAL_COMMON);
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
      free(JD.l);
    if (symbol_class(iq) == ANA_STRING)
      symbol_memory(iq) = strlen(data.sp[-1]) + 1;
    return iq;
    break;
  case CAL_JD:		/* /TOJD */
    assert(outtype == ANA_DOUBLE);
    if (nRepeat == 1) {	/* need scalar */
      iq = scalar_scratch(outtype);
      data.d = &scalar_value(iq).d;
    } else {			/* need array */
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(outtype, num_newDims, newDims);
      data.d = (Double *) array_data(iq);
    }
    memcpy(data.d, JD.d, nRepeat*sizeof(Double));
    if (!isFree)
      free(JD.l);
    return iq;
  case CAL_CJD:		/* /TOCJD */
    assert(outtype == ANA_LONG);
    if (nRepeat == 1) {	/* need scalar */
      iq = scalar_scratch(outtype);
      data.l = &scalar_value(iq).l;
    } else {			/* need array */
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(outtype, num_newDims, newDims);
      data.l = (Int *) array_data(iq);
    }
    memcpy(data.l, JD.l, nRepeat*sizeof(*JD.l));
    if (!isFree)
      free(JD.l);
    return iq;
  case CAL_LUNAR:
    assert(outtype == ANA_DOUBLE);
    if (nRepeat == 1) {	/* need scalar */
      iq = scalar_scratch(ANA_DOUBLE);
      data.d = &scalar_value(iq).d;
    } else {			/* need array */
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(ANA_DOUBLE, num_newDims, newDims);
      data.d = (Double *) array_data(iq);
    }
    for (i = 0; i < nRepeat; i++)
      *data.d++ = JDtoLunar(JD.d[i]);
    if (!isFree)
      free(JD.d);
    return iq;
  case CAL_MAYAN:		/* /TOMAYAN */
    if (nRepeat > 1) {
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(ANA_TEMP_STRING, num_newDims, newDims);
      data.sp = (char **) array_data(iq);
    } else {
      iq = string_scratch(-1);
      data.sp = (char **) &string_value(iq);
    }
    allocate(line, 80, char);
    for (i = 0; i < nRepeat; i++) {
      if (outtype == ANA_DOUBLE)
        d = floor(JDtoCJD(JD.d[i]));
      else
        d = JD.l[i];
      t = iamod(d + 5, 13) + 1;
      v = iamod(d + 16, 20);
      d = iamod(d + 65, 365);
      m = d/20;
      d = iamod(d, 20);
      sprintf(line, "%1d %s %1d %s", t, tikalVenteina[v], d, tikalMonth[m]);
      *data.sp++ = strsave(line);
    }
    if (!isFree)
      free(JD.l);
    return iq;
  case CAL_LONGCOUNT:		/* /TOLONGCOUNT */
    if (nRepeat > 1) {
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(ANA_TEMP_STRING, num_newDims, newDims);
      data.sp = (char **) array_data(iq);
    } else {
      iq = string_scratch(-1);
      data.sp = (char **) &string_value(iq);
    }
    allocate(line, 80, char);
    for (i = 0; i < nRepeat; i++) {
      Int n;

      if (outtype == ANA_DOUBLE)
        d = floor(JDtoCJD(JD.d[i]) - 584283);
      else
        d = JD.l[i] - 584283;
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
      free(JD.l);
    return iq;
  case CAL_HEBREW:
    monthNames = HebrewMonths;
    break;
  default:
    if (!isFree)
      free(JD.l);
    return anaerror("Illegal target calendar specification (%1d)", 0, cal);
  }
  if (output != CAL_NUMERIC) {
    if (nRepeat > 1) {
      num_newDims = construct_output_dims(dims, ndim, n,
                                          newDims, MAX_DIMS, 1);
      iq = array_scratch(ANA_TEMP_STRING, num_newDims, newDims);
      data.sp = (char **) array_data(iq);
    } else {
      iq = string_scratch(-1);
      data.sp = (char **) &string_value(iq);
    }
    allocate(line, 80, char);
  } else {
    num_newDims = construct_output_dims(dims, ndim, n,
                                        newDims, MAX_DIMS, 3);
    iq = array_scratch(outtype, num_newDims, newDims);
    data.d = (Double *) array_data(iq);
  }
  for (i = 0; i < nRepeat; i++) {
    if (outtype == ANA_DOUBLE) {
      CJDtoDate(JD.d[i], &year, &month, &day, tocalendar);
      iday = (Int) day;
    } else
      CJDNtoDate(JD.l[i], &year, &month, &iday, tocalendar);
    switch (output) {
    case 999 /* CAL_ISOTEXT */:
      if (outtype == ANA_DOUBLE) {
        sec = (day - (Int) day)*86400;
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
      case ANA_DOUBLE:
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
      case ANA_LONG:
        switch (toorder) {
        case CAL_YMD:
          *data.l++ = year;
          *data.l++ = month;
          *data.l++ = iday;
          break;
        case CAL_DMY:
          *data.l++ = iday;
          *data.l++ = month;
          *data.l++ = year;
          break;
        }
        break;
      }
      break;
    }
  }
  if (!isFree)
    free(JD.l);
  if (output != CAL_NUMERIC)
    free(line);
  if (symbol_class(iq) == ANA_STRING)
    symbol_memory(iq) = strlen(string_value(iq)) + 1;
  return iq;
}
/*--------------------------------------------------------------------------*/
Int ana_EasterDate(Int narg, Int ps[])
     /* returns dates of Easter in the common calendar (Gregorian-Julian). */
     /* syntax:  X = EASTERDATE(YEARS), where YEARS is an array.  X will */
     /* have three elements in its first dimension, in the order year, */
     /* month, day */
{
  Int	*year, nYear, iq, *dims, nDim, newDims[MAX_DIMS], *ptr, month, day;

  switch (symbol_class(*ps))
  { case ANA_SCALAR:
      iq = ana_long(1, ps);	/* make ANA_LONG */
      year = &scalar_value(iq).l;
      nYear = 1;  nDim = 0;
      break;
    case ANA_ARRAY:
      iq = ana_long(1, ps);
      year = (Int *) array_data(iq);
      nYear = array_size(iq);  dims = array_dims(iq);
      nDim = array_num_dims(iq);
      break;
    default:
      return cerror(ILL_CLASS, *ps); }
  /* create result */
  *newDims = 3;
  if (nDim) memcpy(newDims + 1, dims, nDim);
  iq = array_scratch(ANA_LONG, nDim + 1, newDims);
  ptr = (Int *) array_data(iq);
  while (nYear--)
  { if (EasterDate(*year, &month, &day) < 0)
      return -1;		/* some error */
    *ptr++ = *year;  *ptr++ = month;  *ptr++ = day; }
  return iq;
}
/*--------------------------------------------------------------------------*/
Double deltaT(Double CJD)
/* returns the difference between TAI and UTC for a given CJD(UTC), in seconds.
   For UTC between 1961-01-01 and 2007-01-01. LS 24sep98 25dec06 */
{
  static Double	JDs[] = {
    2437300.5, 2437512.5, 2437665.5, 2438334.5, 2438395.5, 2438486.5,
    2438639.5, 2438761.5, 2438820.5, 2438942.5, 2439004.5, 2439126.5,
    2439887.5, 2441317.5, 2441499.5, 2441683.5, 2442048.5, 2442413.5,
    2442778.5, 2443144.5, 2443509.5, 2443874.5, 2444239.5, 2444786.5,
    2445151.5, 2445516.5, 2446247.5, 2447161.5, 2447892.5, 2448257.5,
    2448804.5, 2449169.5, 2449534.5, 2450083.5, 2450630.5, 2451179.5,
    2453736.5,
  };
  static Double	t0s[] = {
    2437300, 2437300, 2437665, 2437665, 2438761, 2438761,
    2438761, 2438761, 2438761, 2438761, 2438761, 2439126,
    2439126, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
  };
  static Double	offsets[] = {
    1.4228180, 1.3728180, 1.8457580, 1.9458580, 3.2401300, 3.3401300,
    3.4401300, 3.5401300, 3.6401300, 3.7401300, 3.8401300, 4.3131700,
    4.2131700, 10.0, 11.0, 12.0, 13.0, 14.0,
    15.0, 16.0, 17.0, 18.0, 19.0, 20.0,
    21.0, 22.0, 23.0, 24.0, 25.0, 26.0,
    27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0,
  };
  static Double	units[] = {
    0.001296, 0.001296, 0.0011232, 0.0011232, 0.001296, 0.001296,
    0.001296, 0.001296, 0.001296, 0.001296, 0.001296, 0.002592,
    0.002592, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
  };
  static Int	n = sizeof(JDs)/sizeof(Double);
  Double	T, dT;
  Int	lo, hi, mid;

  Double JD = CJD - 0.5;
  if (JD < JDs[0]) {		/* before 1961-01-01 */
    /* do a rough estimate */
    /* The algorithm is based on observations between years -390 */
    /* and +1986 [ES2.553]. */
    T = (JD - 2378495)/36525;	/* centuries since 1800.0 */
    if (JD > 2314580.0)		/* year +1625 */
      dT = 5.156 + 13.3066*(T - 0.19)*(T - 0.19);
    else if (JD > 2067320)	/* year +948 */
      dT = 25.5*T*T - 3.61*T - 29.3; /* NOTE: I added the linear term and */
    /* offset to this formula to have this one and the previous one match */
    /* at the stated boundary of about 1625. LS 17dec95 */
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

  lo = (hi < mid)? hi: mid;	/* the last one from the table */
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
/*--------------------------------------------------------------------------*/
void UTC_to_TAI(Double *CJD)
{
  *CJD += deltaT(*CJD)/86400;
}
/*--------------------------------------------------------------------------*/
void TAI_to_UTC(Double *CJD)
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

  Double UTC_est = *CJD - deltaT(*CJD)/86400;
  *CJD -= -deltaT(UTC_est)/86400;
}
/*--------------------------------------------------------------------------*/
void CJDLT_to_TAI(Double *CJD)
{
  time_t t_LT = (time_t) ((*CJD - 2440588)*86400);
  struct tm *bt = localtime(&t_LT);
  time_t t_UTC = t_LT - bt->tm_gmtoff;
  *CJD = t_UTC/(Double) 86400.0 + 2440588;
  UTC_to_TAI(CJD);
}
/*--------------------------------------------------------------------------*/
void TAI_to_CJDLT(Double *CJD)
{
  TAI_to_UTC(CJD);
  time_t t_UTC = (time_t) ((*CJD - (Double) 2440588)*86400);
  struct tm *bt = localtime(&t_UTC);
  time_t t_LT = t_UTC + bt->tm_gmtoff;
  *CJD = t_LT/(Double) 86400.0 + 2440588;
}
/*--------------------------------------------------------------------------*/
Double JDE(Double JD, Int direction)
     /* corrects JD for changes in the rotation rate of the Earth. */
     /* (i.e., takes JD (UT) and returns an approximate JDE (TDT). */
     /* The algorithms are based on observations between years -390 */
     /* and +1986 [ES2.553].  if <direction> = +1, then goes from
      UT to TDT, if <direction> = -1, then goes from TDT to UT. */
{
  Double	dT, T;

  T = (JD - 2378495)/36525;	/* centuries since 1800.0 */
  if (JD > 2314580.0)		/* year +1625 */
    dT = 5.156 + 13.3066*(T - 0.19)*(T - 0.19);
  else if (JD > 2067320)	/* year +948 */
    dT = 25.5*T*T - 3.61*T - 29.3; /* NOTE: I added the linear term and */
  /* offset to this formula to have this one and the previous one match */
  /* at the stated boundary of about 1625. LS 17dec95 */
  else
    dT = 1360 + 320*T + 44.3*T*T;
  return (direction < 0)? JD - dT/86400: JD + dT/86400;
}
/*--------------------------------------------------------------------------*/
void rotate(Double *pos, Double angle, Int axis)
/* rotate X, Y, and Z (pos[0] through pos[2]) over the specified angle */
/* (in radians) along the specified axis (X = 0, etc.) */
{
  Double	p1, p2, c, s;
  static Double	se = 0.3977771559, ce = 0.917482062; /* sine and cosine of */
				/* angle of ecliptic at equinox 2000.0 */

  switch (axis)
  { case 0:			/* around X axis */
      c = cos(angle);
      s = sin(angle);
      p1 = pos[1]*c - pos[2]*s;
      p2 = pos[1]*s + pos[2]*c;
      pos[1] = p1;
      pos[2] = p2;
      c = c*c;
      s = s*s;
      p1 = pos[4]*c + pos[5]*s;
      p2 = pos[4]*s + pos[5]*c;
      pos[4] = p1;
      pos[5] = p2;
      break;
    case 1:			/* around Y axis */
      c = cos(angle);
      s = sin(angle);
      p1 = pos[0]*c + pos[2]*s;
      p2 = -pos[0]*s + pos[2]*c;
      pos[0] = p1;
      pos[2] = p2;
      c = c*c;
      s = s*s;
      p1 = pos[3]*c + pos[5]*s;
      p2 = pos[3]*s + pos[5]*c;
      pos[3] = p1;
      pos[5] = p2;
      break;
    case 2:			/* around Z axis */
      c = cos(angle);
      s = sin(angle);
      p1 = pos[0]*c - pos[1]*s;
      p2 = pos[0]*s + pos[1]*c;
      pos[0] = p1;
      pos[1] = p2;
      c = c*c;
      s = s*s;
      p1 = pos[3]*c + pos[4]*s;
      p2 = pos[3]*s + pos[4]*c;
      pos[3] = p1;
      pos[4] = p2;
      break;
    case 3:			/* + e2000 */
      p1 = pos[1]*ce + pos[2]*se;
      p2 = -pos[1]*se + pos[2]*ce;
      pos[1] = p1;
      pos[2] = p2;
      break;
    case 4:			/* - e2000 */
      p1 = pos[1]*ce - pos[2]*se;
      p2 = pos[1]*se + pos[2]*ce;
      pos[1] = p1;
      pos[2] = p2;
      break;
    }
}
/*--------------------------------------------------------------------------*/
void precess(Double *pos, Double equinox, Int forward)
     /* precess the rectangular coordinates from equinox 2000.0 to */
     /* the specified one (JDE) (forward > 0) or backward (forward <= 0) */
     /* ES3.212 */
{
  static Double	z_a = 0, theta_a = 0, zeta_a = 0, oldEquinox = J2000;
  Double	T;

  if (equinox == J2000)
    return;			/* already in 2000.0 */
  if (equinox != oldEquinox)
  { oldEquinox = equinox;
    T = (equinox - J2000)/36525;
    zeta_a = pol2(2306.2181, 0.30188, 0.017998, T)*T*DEG/3600;
    z_a = pol2(2306.2181, 1.09468, 0.018203, T)*T*DEG/3600;
    theta_a = pol2(2004.3109, -0.42665, -0.041833, T)*T*DEG/3600; }
  if (forward > 0)
  { rotate(pos, zeta_a, 2);
    rotate(pos, -theta_a, 1);
    rotate(pos, z_a, 2); }
  else
  { rotate(pos, -z_a, 2);
    rotate(pos, theta_a, 1);
    rotate(pos, -zeta_a, 2); }
  return;
}
/*--------------------------------------------------------------------------*/
Double	p_ce, p_se, p_pi, p_p;
void initEclipticPrecession(Double JDE, Double equinox)
     /* initialize for precession from equinox <JDE> to equinox <equinox> */
{
  Double	t, eta, T;

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
/*--------------------------------------------------------------------------*/
void eclipticPrecession(Double *pos, Double JDE, Double equinox)
/* precess the ecliptical polar coordinates <pos> from the equinox of
   <JDE> (in JDE) to that of <equinox> (in JDE) */
{
  static Double	oldJDE = DBL_MAX, oldEquinox = DBL_MAX;
  Double	a, b, c, cb, sb, s;
  
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
/*--------------------------------------------------------------------------*/
void precessEquatorial(Double *ra, Double *dec, Double JDfrom, Double JDto)
/* precess the equatorial coordinates *ra (right ascension) and *dec
   (declination), both measured in radians, from the equinox of JDfrom
   to the equinox of JDto.  LS 2004may03 */
{
  static Double zeta, z, theta, from = 0, to = 0;
  Double A, B, C;

  if (from != JDfrom || to != JDto || !from || !to) {
    Double T, t;

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
/*--------------------------------------------------------------------------*/
Int ana_precess(Int narg, Int ps[])
/* PRECESS(coords, JDfrom, JDto) precesses the equatorial coordinates
   <coords> from the equinox of <JDfrom> to <JDto>, both measured in
   Julian Days.  <coords>(0,*) is taken to contain right ascensions
   measured in degrees, and <coords>(1, *) declinations, measured in
   degrees.  LS 2004may03*/
{
  Double JDfrom, JDto, alpha, delta;
  pointer src, tgt;
  loopInfo srcinfo, tgtinfo;
  Int n, outtype, result, done;

  JDfrom = double_arg(ps[1]);
  JDto = double_arg(ps[2]);
  if (internalMode & 2) {
    JDfrom = 0.99997860193253*JDfrom + 0.0413818359375; /* to Julian */
    JDto = 0.99997860193253*JDto + 0.0413818359375; /* to Julian */
  }
  if (internalMode & 3) {
    JDfrom = (JDfrom - 2000.0)*365.25 + J2000;
    JDto = (JDto - 2000.0)*365.25 + J2000;
  }
  outtype = (symbol_type(ps[0]) > ANA_FLOAT)? ANA_DOUBLE: ANA_FLOAT;
  n = standardLoop(ps[0], ANA_ZERO, SL_AXISCOORD | SL_SRCUPGRADE,
		   outtype, &srcinfo, &src, &result, &tgtinfo, &tgt);
  if (n == ANA_ERROR)
    return n;
  if (srcinfo.ndim < 1) {
    zap(result);
    return cerror(NEED_ARR, ps[0]);
  }
  if (srcinfo.dims[0] < 2) {
    zap(result);
    return anaerror("Need at least 2 elements in the first dimension", ps[0]);
  }
  
  switch (symbol_type(ps[0])) {
  case ANA_BYTE:
    do {
      alpha = *src.b*DEG;
      advanceLoop(&srcinfo, &src);
      delta = *src.b*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      advanceLoop(&tgtinfo, &tgt);
      *tgt.f = delta*RAD;
      do {
	done = advanceLoop(&srcinfo, &src);
	advanceLoop(&tgtinfo, &tgt);
	if (!done)
	  *tgt.f = *src.b;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case ANA_WORD:
    do {
      alpha = *src.w*DEG;
      advanceLoop(&srcinfo, &src);
      delta = *src.w*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      advanceLoop(&tgtinfo, &tgt);
      *tgt.f = delta*RAD;
      do {
	done = advanceLoop(&srcinfo, &src);
	advanceLoop(&tgtinfo, &tgt);
	if (!done)
	  *tgt.f = *src.w;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case ANA_LONG:
    do {
      alpha = *src.l*DEG;
      advanceLoop(&srcinfo, &src);
      delta = *src.l*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      advanceLoop(&tgtinfo, &tgt);
      *tgt.f = delta*RAD;
      do {
	done = advanceLoop(&srcinfo, &src);
	advanceLoop(&tgtinfo, &tgt);
	if (!done)
	  *tgt.f = *src.l;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case ANA_FLOAT:
    do {
      alpha = *src.f*DEG;
      advanceLoop(&srcinfo, &src);
      delta = *src.f*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      advanceLoop(&tgtinfo, &tgt);
      *tgt.f = delta*RAD;
      do {
	done = advanceLoop(&srcinfo, &src);
	advanceLoop(&tgtinfo, &tgt);
	if (!done)
	  *tgt.f = *src.f;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case ANA_DOUBLE:
    do {
      alpha = *src.d*DEG;
      advanceLoop(&srcinfo, &src);
      delta = *src.d*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.d = alpha*RAD;
      advanceLoop(&tgtinfo, &tgt);
      *tgt.d = delta*RAD;
      do {
	done = advanceLoop(&srcinfo, &src);
	advanceLoop(&tgtinfo, &tgt);
	if (!done)
	  *tgt.d = *src.d;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  }
  return result;
}
/*--------------------------------------------------------------------------*/
#include "constellations.h"
Int constellation(Double alpha, Double delta)
/* returns the identity of the official constellation that the
   position is in that is indicated by right ascension <alpha> and
   declination <delta>, both measured in degrees relative to the
   equinox of B1875.0.  LS 2004may03
 */
{
  Int nb = sizeof(constellation_boundaries)/sizeof(struct constellation_struct);
  Int i;
  struct constellation_struct *cb = constellation_boundaries;

  if (delta < -90)
    delta = -90;
  else if (delta > 90)
    delta = +90;
  alpha = famod(alpha, 360.0);	/* reduce to interval 0 - 360 degrees */
  alpha /= 15;			/* from degrees to hours */

  for (i = 0; i < nb && (delta < cb[i].delta || alpha >= cb[i].alpha2
                         || alpha < cb[i].alpha1); i++);
  return cb[i].constellation;
}
/*--------------------------------------------------------------------------*/
#define B1875 (2405889.25855047) /* from SOFA Epb2jd routine */
Int ana_constellation(Int narg, Int ps[])
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
  Int n, result, done;
  Double equinox, alpha, delta;
  loopInfo srcinfo, tgtinfo;
  pointer src, tgt;
  Int vocal;

  if (narg > 1) {
    equinox = double_arg(ps[1]);
    if (internalMode & 2)
      equinox = 0.99997860193253*equinox + 0.0413818359375; /* to Julian */
    if (internalMode & 3)
      equinox = (equinox - 2000.0)*365.25 + J2000;
  } else
    equinox = J2000;
  vocal = internalMode & 4;

  n = standardLoop(ps[0], ANA_ZERO, SL_AXISCOORD | SL_COMPRESS,
		   ANA_BYTE, &srcinfo, &src, &result, &tgtinfo, &tgt);
  if (n == ANA_ERROR)
    return n;
  if (srcinfo.ndim < 1) {
    zap(result);
    return cerror(NEED_ARR, ps[0]);
  }
  if (srcinfo.dims[0] < 2) {
    zap(result);
    return anaerror("Need at least 2 elements in the first dimension", ps[0]);
  }

  switch (symbol_type(ps[0])) {
  case ANA_BYTE:
    do {
      alpha = *src.b*DEG;
      advanceLoop(&srcinfo, &src);
      delta = *src.b*DEG;
      do
	done = advanceLoop(&srcinfo, &src);
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
      *tgt.b = constellation(alpha*RAD, delta*RAD);
      advanceLoop(&tgtinfo, &tgt);
    } while (done < srcinfo.rndim);
    break;
  case ANA_WORD:
    do {
      alpha = *src.w*DEG;
      advanceLoop(&srcinfo, &src);
      delta = *src.w*DEG;
      do
	done = advanceLoop(&srcinfo, &src);
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
      *tgt.b = constellation(alpha*RAD, delta*RAD);
      advanceLoop(&tgtinfo, &tgt);
    } while (done < srcinfo.rndim);
    break;
  case ANA_LONG:
    do {
      alpha = *src.l*DEG;
      advanceLoop(&srcinfo, &src);
      delta = *src.l*DEG;
      do
	done = advanceLoop(&srcinfo, &src);
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
      *tgt.b = constellation(alpha*RAD, delta*RAD);
      advanceLoop(&tgtinfo, &tgt);
    } while (done < srcinfo.rndim);
    break;
  case ANA_FLOAT:
    do {
      alpha = *src.f*DEG;
      advanceLoop(&srcinfo, &src);
      delta = *src.f*DEG;
      do
	done = advanceLoop(&srcinfo, &src);
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
      *tgt.b = constellation(alpha*RAD, delta*RAD);
      advanceLoop(&tgtinfo, &tgt);
    } while (done < srcinfo.rndim);
    break;
  case ANA_DOUBLE:
    do {
      alpha = *src.d*DEG;
      advanceLoop(&srcinfo, &src);
      delta = *src.d*DEG;
      do
	done = advanceLoop(&srcinfo, &src);
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
      *tgt.b = constellation(alpha*RAD, delta*RAD);
      advanceLoop(&tgtinfo, &tgt);
    } while (done < srcinfo.rndim);
    break;
  }
  return result;
}
/*--------------------------------------------------------------------------*/
Int ana_constellationname(Int narg, Int ps[]) 
/* CONSTELLATIONNAME(<index>) returns the official abbreviation of the
   name of the constellation with the given <index> (may be an array),
   as returned by function CONSTELLATION.  LS 2004may03
 */
{
  Int result, n, nc;
  pointer src;
  char **tgt;

  if (numerical(ps[0], NULL, NULL, &n, &src) == ANA_ERROR)
    return ANA_ERROR;
  if (n > 1) {
    result = array_clone(ps[0], ANA_STRING_ARRAY);
    tgt = array_data(result);
  } else {
    result = string_scratch(0);
    tgt = &string_value(result);
  }
  nc = sizeof(constellation_names)/sizeof(char *);
  switch (symbol_type(ps[0])) {
  case ANA_BYTE:
    while (n--) {
      if (*src.b >= nc)
	*tgt = strdup("***");
      else
	*tgt = strdup(constellation_names[*src.b]);
      tgt++;
      src.b++;
    }
    break;
  case ANA_WORD:
    while (n--) {
      if (*src.w < 0 || *src.w >= nc)
	*tgt = strdup("***");
      else
	*tgt = strdup(constellation_names[*src.w]);
      tgt++;
      src.w++;
    }
    break;
  case ANA_LONG:
    while (n--) {
      if (*src.l < 0 || *src.l >= nc)
	*tgt = strdup("***");
      else
	*tgt = strdup(constellation_names[*src.l]);
      tgt++;
      src.l++;
    }
    break;
  case ANA_FLOAT:
    while (n--) {
      if (*src.f < 0 || *src.f >= nc)
	*tgt = strdup("***");
      else
	*tgt = strdup(constellation_names[(Int) *src.f]);
      tgt++;
      src.f++;
    }
    break;
  case ANA_DOUBLE:
    while (n--) {
      if (*src.d < 0 || *src.d >= nc)
	*tgt = strdup("***");
      else
	*tgt = strdup(constellation_names[(Int) *src.d]);
      tgt++;
      src.d++;
    }
    break;
  }
  return result;
}
/*--------------------------------------------------------------------------*/
Float magnitude(Double d, Double r, Double beta, Int objNum)
     /* Estimates the visual magnitude from the square of the distance */
     /* to the object, the square of the distance of the object to the Sun, */
     /* and the phase angle in degrees*/
     /* [ES7, 15] */
{
  Double	temp;
  Int	i;

  switch (objNum) {
    default:
      if (nExtraObj) {
	i = findint(objNum, extraIDs, nExtraObj);
	return extraOrbits[i].absmag + 5*log10(r*d);
      }
      return 0;
    case -1:			/* ELEMENTS */
      if (haveExtraElements)
	return extraElements[8] + 5*log10(r*d);
      return 0;
    case 0:			/* Sun */
      return -26.72 + 2.5*log10(d);
    case 1:			/* Mercury */
      temp = fabs(beta)/100;
      return -0.42 + 5*log10(r*d) + temp*(3.80 + temp*(-2.73 + 2.00*temp));
    case 2:			/* Venus */
      temp = fabs(beta)/100;
      return -4.40 + 5*log10(r*d) + temp*(0.09 + temp*(2.39 - 0.65*temp));
    case 3:			/* Earth */
      /* phase angle dependence derived from Redshift 2 planetarium program */
      temp = fabs(beta)/100;
      return -3.86 + 5*log10(r*d) + temp*(0.509 + temp*1.494);
    case 4:			/* Mars */
      return -1.52 + 5*log10(r*d) + 0.016*fabs(beta);
    case 5:			/* Jupiter */
      return -9.40 + 5*log10(r*d) + 0.005*fabs(beta);
    case 6:			/* Saturn */
      return -8.88 + 5*log10(r*d) + 0.044*fabs(beta);
    case 7:			/* Uranus */
      return -7.19 + 5*log10(r*d) + 0.0028*fabs(beta);
    case 8:			/* Neptune */
      /* assume same beta-dependence as for Uranus */
      return -6.87 + 5*log10(r*d) + 0.0028*fabs(beta);
    case 9:			/* Pluto */
      return -1.01 + 5*log10(r*d) + 0.041*fabs(beta);
    case 10:			/* Moon [ES 9.342-2] */
      temp = fabs(beta);
      temp *= temp;
      temp *= temp;
      return 0.21 + 5*log10(r*d) + 0.0217*fabs(beta) + 5.5e-9*temp;
  }
}
/*--------------------------------------------------------------------------*/
const char const *objectName(Int objNum)
{
  switch (objNum) {
    default:
      if (nExtraObj) {
	Int i = findint(objNum, extraIDs, nExtraObj);
	return extraOrbits[i].comment;
      }
      return "Unknown";
    case -1:			/* ELEMENTS */
      return "ELEMENTS";
    case 0:			/* Sun */
      return "Sun";
    case 1:			/* Mercury */
      return "Mercury";
    case 2:			/* Venus */
      return "Venus";
    case 3:			/* Earth */
      return "Earth";
    case 4:			/* Mars */
      return "Mars";
    case 5:			/* Jupiter */
      return "Jupiter";
    case 6:			/* Saturn */
      return "Saturn";
    case 7:			/* Uranus */
      return "Uranus";
    case 8:			/* Neptune */
      return "Neptune";
    case 9:			/* Pluto */
      return "Pluto";
    case 10:			/* Moon [ES 9.342-2] */
      return "Moon";
  }
}
/*--------------------------------------------------------------------------*/
void nutation(Double JDE, Double *dPsi, Double *cdPsi, Double *sdPsi,
	      Double *dEps)
/* calculates the nutation in longitude and/or obliquity */
{
  Double	d, m, mm, f, o, *amp, angle, T;
  int16_t	*mul;
  Int	i;

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
/*--------------------------------------------------------------------------*/
Double obliquity(Double JDE, Double *dEps)
/* returns the obliquity of the ecliptic: calculates nutation if */
/* dEps == NULL, otherwise adds *dEps. */
{
  Double	eps, T;

  T = (JDE - J2000)/3652500;	/* 10,000 Julian years! since 2000.0 */
  eps = (((((((((T*2.45 + 5.79)*T + 27.87)*T + 7.12)*T - 39.05)*T - 249.67)*T
	    - 51.38)*T + 1999.25)*T - 1.55)*T - 4680.93)*T*4.848136811e-6
	      + 0.4090928042;
  if (!dEps)
    nutation(JDE, NULL, NULL, NULL, dEps);
  if (dEps)
    eps += *dEps;
  return eps;
}
/*--------------------------------------------------------------------------*/
Double standardSiderealTime(Int JD, Double *dPsi, Double ceps)
/* returns the sidereal time at longitude zero at 0 UT of the day of which */
/* 12 UT corresponds to Julian Day number <JD>, in radians; mean if */
/* dPsi == NULL, apparent otherwise.  */
{
  Double	c, T, jd;

  jd = floor(JD) + 0.5;
  T = (jd - J2000)/36525;	/* Julian centuries */
  c = pol3(0.2790572732639,100.00213903780093,1.0775926e-6,-7.176e-11,T);
  if (dPsi)
    c += *dPsi*ceps;
  c -= floor(c);
  c *= TWOPI;
  return c;
}
/*--------------------------------------------------------------------------*/
Double siderealTime(Double JD, Double *dPsi, Double ceps)
/* returns the sidereal time at longitude zero at the indicated JD (UT), */
/* in radians; mean if dPsi == NULL, apparent otherwise */
{
  Double	c, T, jd;

  jd = floor(JD) + 0.5;
  T = (jd - J2000)/36525;	/* Julian centuries */
  c = pol3(0.2790572732639,100.00213903780093,1.0775926e-6,-7.176e-11,T);
  c += (pol2(100.00213903780093,2*1.0775926e-6,3*-7.176e-11,T)/36525 + 1)*(JD - jd);
  if (dPsi)
    c += *dPsi*ceps;
  c -= floor(c);
  c *= TWOPI;
  return c;  
}
/*--------------------------------------------------------------------------*/
Int ana_siderealtime(Int narg, Int ps[]) 
/* SIDEREALTIME(<jd>) returns the mean sidereal time at the indicated */
/* julian dates, in hours */
/* LS 31mar2002 */
{
  Double *jd, *out;
  Int n, iq, result;
  Double dPsi, cdPsi, sdPsi, dEps, epsilon;

  switch (symbol_class(ps[0])) {
  case ANA_SCALAR:
    iq = ana_double(1, ps);
    jd = &scalar_value(iq).d;
    n = 1;
    result = scalar_scratch(ANA_DOUBLE);
    out = &scalar_value(result).d;
    break;
  case ANA_ARRAY:
    iq = ana_double(1, ps);
    jd = (Double *) array_data(iq);
    n = array_size(iq);
    result = array_clone(iq, ANA_DOUBLE);
    out = (Double *) array_data(result);
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
  case 1:			/* /ATZEROTIME */
    while (n--) {
      nutation(*jd, &dPsi, &cdPsi, &sdPsi, &dEps);
      epsilon = obliquity(*jd, &dEps);
      *out++ = standardSiderealTime(*jd++, &dPsi, cos(epsilon))/TWOPI*24;
    }
    break;
  case 2:			/* /MEAN */
    while (n--)
      *out++ = siderealTime(*jd++, NULL, 0)/TWOPI*24;
    break;
  case 3:			/* /MEAN /ATZEROTIME */
    while (n--)
      *out++ = standardSiderealTime(*jd++, NULL, 0)/TWOPI*24;
    break;
  }

  return result;
}
/*--------------------------------------------------------------------------*/
void VSOPtoFK5(Double T, Double *pos)
/* transforms from VSOP polar coordinates to FK5 polar coordinates; T
   is in Julian centuries since J2000.0; pos[] contains polar
   coordinates (longitude, latitude, radius) in radians and AU,
   relative to the VSOP dynamical ecliptic and equinox of date */
{
  Double	ll, cll, sll;
  
  ll = pos[0] - 0.024382*T*(1 + 0.000222*T);
  cll = cos(ll);
  sll = sin(ll);
  pos[0] += -4.3793e-7 + 1.8985e-7*(cll + sll)*tan(pos[1]);
  pos[1] += 1.8985e-7*(cll - sll);
}
/*--------------------------------------------------------------------------*/
void XYZ_VSOPtoFK5(Double T, Double *pos)
/* transforms from VSOP cartesian coordinates (relative to the
   dynamical ecliptic and equinox of the date) to FK5 cartesian
   coordinates */
{
  Double tpos[3];
  XYZtoLBR(pos, tpos);
  VSOPtoFK5(T, tpos);
  LBRtoXYZ(tpos, pos);
}
/*--------------------------------------------------------------------------*/
#define EQUINOX_OF_DATE	DBL_MAX

Int readExtra(char *file, char mode)
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
  FILE	*fp;
  char	*defaultFile = "$ANADIR/orbits", orbitLine[256], *pp;
  Int	obj, id, rec, c, format, n, nmore, indx, nterm;
  Double	jd, a, e, i, node, peri, m, equinox;
  Double	sn, cn, si, ci, f, g, p, q, mag;

  if (!file)
    file = defaultFile;
  fp = fopen(expand_name(file, NULL), "r");
  if (!fp)
    return anaerror("Could not open orbital data file \"%s\".", 0, file);
  while ((c = getc(fp)) == '=') {	/* comment lines */
    ungetc(' ', fp);
    if (!fgets(orbitLine, 256, fp))
      return anaerror("Read error/unexpected data end in file %s", 0, file);
    if (mode & 1)	/* /LIST */
      printf(orbitLine);
  }
  ungetc(c, fp);
  fscanf(fp, "%d", &n);		/* number of objects in this file*/
  fgets(orbitLine, 256, fp);	/* rest of 1st data line */
  for (pp = orbitLine; isspace((Byte) *pp); pp++);
  if (*pp && mode & 1)
    printf(pp);
  
  if (extraOrbits && mode == 2) { /* replace */
    free(extraOrbits);
    free(extraIDs);
    nExtraObj = n;
    nmore = 0;
  } else {
    nmore = nExtraObj;
    nExtraObj += n;
  }

  if (mode == 2 || !extraOrbits) { /* replace or new */
    extraOrbits =
      (struct extraInfo *) malloc(nExtraObj*sizeof(struct extraInfo));
    extraIDs = (Int *) malloc(nExtraObj*sizeof(Int));
  } else {			/* merge */
    extraOrbits =
      (struct extraInfo *) realloc(extraOrbits,
				   nExtraObj*sizeof(struct extraInfo));
    extraIDs = (Int *) realloc(extraIDs, nExtraObj*sizeof(Int));
  }
    
  if (!extraOrbits || !extraIDs)
    return cerror(ALLOC_ERR, 0);

  for (obj = indx = 0; obj < n; obj++, indx++) {
    while ((c = getc(fp)) == '=') { /* comment line */
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
    if (extraIDs[indx] == id) {	/* object already exists: replace */
      printf(
	"Warning - object %1d (%s) already exists.\nReplacing orbital data.\n",
	id,
	extraOrbits[indx].comment? extraOrbits[indx].comment: "no comment" );
      free(extraOrbits[indx].orbits);
      free(extraOrbits[indx].comment);
      nmore--;
    } else {			/* insert object */
      memmove(extraIDs + indx + 1, extraIDs + indx, nmore*sizeof(Int));
      memmove(extraOrbits + indx + 1, extraOrbits + indx,
	      nmore*sizeof(struct extraInfo));
    }
    
    extraIDs[indx] = id;
    extraOrbits[indx].nterms = nterm;
    extraOrbits[indx].absmag = mag;

				/* seek equinox specification */
    format = 0;
    do 
      c = getc(fp);
    while (c != EOF && c != '\n' && isspace(c));
    switch (c) {
      case 'j': case 'J':	/* Julian */
	extraOrbits[indx].equinox = 0;
	break;
      case 'b': case 'B':
	extraOrbits[indx].equinox = 1;
	break;
      case '\n':
	extraOrbits[indx].equinox = 2; /* J2000 */
	break;
      case 'd': case 'D':
	extraOrbits[indx].equinox = EQUINOX_OF_DATE;
	break;
      case 'a': case 'A': case '=':
	format = 'A';
	extraOrbits[indx].equinox = 2; /* J2000 */
	if (c == '=')
	  ungetc(c, fp);
	break;
      case 'q': case 'Q':
	format = 'Q';
	extraOrbits[indx].equinox = 2; /* J2000 */
	break;
      default:
	fclose(fp);
	return anaerror("Illegal equinox specification \"%c\" in orbital data file \"%s\".", 0, c, file);
    }
    if (extraOrbits[indx].equinox <= 1) { /* read year */
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
      if (extraOrbits[indx].equinox) /* Besselian */
	equinox = 0.99997860193253*equinox + 0.0413818359375; /* to Julian */
      equinox = (equinox - 2000.0)*365.25 + J2000;
      extraOrbits[indx].equinox = equinox;
    } else if (extraOrbits[indx].equinox == 2) /* J2000 */
      extraOrbits[indx].equinox = J2000;

    if (!format) {		/* not yet found format */
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
	    return anaerror("Illegal data format specification \"%c\" in orbital data file \"%s\".", 0, c, file);
	}
      }
    }

    if (mode & 1)
      printf("%d ", extraIDs[indx]);
    if (c != '\n') {		/* read comment; skip to end of line */
      fgets(orbitLine, 256, fp);
      pp = orbitLine;
      while (*pp != '\n')
	pp++;
      *pp = '\0';		/* remove final \n */
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
      if (e == 1) {	/* parabolic orbit */
	if (format == 'A') {
	  fclose(fp);
	  return anaerror("Parablic orbit has undefined semimajor axis", 0);
	}
	extraOrbits[indx].orbits[rec].n =
	  a? 0.03649116245/(a*sqrt(a)): 0.0; /* n rad/d */
	extraOrbits[indx].orbits[rec].M =
	  (jd - m)*extraOrbits[indx].orbits[rec].n;
      } else {
	if (format == 'Q') {
	  a = fabs(a/(1 - e));	/* semimajor axis from perifocus distance */
	  extraOrbits[indx].orbits[rec].n =
	    a? 0.01720209895/(a*sqrt(a)): 0.0; /* n rad/d */
	  extraOrbits[indx].orbits[rec].M =
	    (jd - m)*extraOrbits[indx].orbits[rec].n;
	} else {
	  extraOrbits[indx].orbits[rec].M = m*DEG;
	  extraOrbits[indx].orbits[rec].n =
	    a? 0.01720209895/(a*sqrt(a)): 0.0; /* n rad/d */
	}
	/* if (e > 1) extraOrbits[indx].orbits[rec].n = -extraOrbits[indx].orbits[rec].n; */
      }
      cn = cos(node);
      sn = sin(node);
      ci = cos(i);
      si = sin(i);
      f = cn;
      g = sn;
      p = -sn*ci;
      q = cn*ci;
      extraOrbits[indx].orbits[rec].xfac = sqrt(f*f + p*p); /* "a" */
      extraOrbits[indx].orbits[rec].xangle = atan2(f,p) + peri; /* "A" */
      extraOrbits[indx].orbits[rec].yfac = sqrt(g*g + q*q); /* "b" */
      extraOrbits[indx].orbits[rec].yangle = atan2(g,q) + peri; /* "B" */
      extraOrbits[indx].orbits[rec].zfac = si; /* "c" */
      extraOrbits[indx].orbits[rec].zangle = peri; /* "C" */
    } /* end of for (rec,...) */
  } /* end of for (indx,...) */
  if (ferror(fp))
    return anaerror("Read error in orbital data file \"%s\".", 0, file);
  fclose(fp);

  indx += nmore;
  if (indx < nExtraObj) {
    /* some IDs were already present, so some of the extra allocated
     memory was not used */
    extraOrbits =
      (struct extraInfo *) realloc(extraOrbits,
				   indx*sizeof(struct extraInfo));
    extraIDs = (Int *) realloc(extraIDs, indx*sizeof(Int));
    nExtraObj = indx;
  }

  return 1;
}
/*--------------------------------------------------------------------------*/
Int ana_readorbits(Int narg, Int ps[])
/* READORBITS [,<file>,/LIST,/REPLACE] reads orbital information from <file>.
 Lists comments if /LIST is specified.  Replace preread data (if any)
 if /REPLACE is specified, else merges */
{
  char	*file;

  file = narg? string_arg(ps[0]): NULL;
  return readExtra(file, internalMode & 3);
}
/*--------------------------------------------------------------------------*/
void showExtra(void)
/* list the presently read auxilliary object */
{
  Int	i;

  if (nExtraObj) {
    puts("Auxilliary orbits were read for:");
    for (i = 0; i < nExtraObj; i++)
      printf("%d %s\n", extraIDs[i],
	     extraOrbits[i].comment? extraOrbits[i].comment: "(no comment)");
  } else
    puts("No auxilliary orbits have been read.");
}
/*--------------------------------------------------------------------------*/
Int ana_showorbits(Int narg, Int ps[])
{
  showExtra();
  return 1;
}
/*--------------------------------------------------------------------------*/
void kepler(Double m, Double e, Double v_factor, Double *v, Double *rf)
/* solves Kepler's equation for mean anomaly <m> (in radians) and
 eccentricity <e>.  Returns true anomaly <*v> and radius factor <*rf>.
 <v_factor> must be equal to sqrt(abs((1 + <e>)/(1 - <e>))).  The radius
 will be equal to the perihelion distance times <*rf>.  */
{
  Double	E, de, p;

  if (fabs(e) < 1) {		/* elliptical orbit */
    m = fmod(m, TWOPI);
    E = m;
    do {
      p = 1 - e*cos(E);
      de = (m + e*sin(E) - E)/p;
      E += de;
    } while (fabs(de) > 1e3*DBL_EPSILON);
    *v = 2*atan(v_factor*tan(E/2)); /* true anomaly */
    *rf = p/(1 - e);
  } else if (fabs(e) > 1) {	/* hyperbolic orbit */
    E = m/e;
    do {
      p = E;
      E = asinh((m + E)/e);
    } while (E != p);
    *v = 2*atan(v_factor*tanh(E/2));
    *rf = (1 - e*cosh(E))/(1 - e);
  } else {			/* parabolic orbit */
    E = m/2;
    e = cbrt(E + sqrt(E*E + 1));
    p = e - 1.0/e;
    *v = 2*atan(p);
    *rf = 1 + p*p;
  }
}
/*--------------------------------------------------------------------------*/
Double interpolate_angle(Double a1, Double a2, Double f)
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
/*--------------------------------------------------------------------------*/
Int extraHeliocentric(Double JDE, Int object, Double *equinox,
                      Double *f, Double *r)
/* For <JDE>, return the polar coordinates of <object> in <f>, and the
 equinox for which it was specified in <equinox>. */
{
  Int	findint(Int, Int *, Int), i, low, high, mid;
  Double	m, e, k, d1, d2, q;
  struct extraInfo	*pp;
  struct orbitParams	*qq;
  
  if (!nExtraObj && readExtra(NULL, 0) == ANA_ERROR)
    return anaerror("Error reading orbital data.", 0);
  
  /* first see if the object number is known */
  i = findint(object, extraIDs, nExtraObj);
  if (i < 0)
    return anaerror("No orbital data for object %d.", 0, object);

  /* OK, it is known.  Now find the ephemerides time closest to T */
  pp = extraOrbits + i;		/* data for ith object in file */
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
    /*      return mid; */
  }
  /* If JDE is found in pp->orbits[].JDE, then low = high = mid = the
   appropriate index.  If JDE is not found, then its value is between
   those at indices high and low.  (If JDE is before the first value,
   then high = -1; if JDE is after the last value, then low = pp->nterms. */

  if (low == high || high == -1 || low == pp->nterms) {
    /* just take fixed parameters */
    if (low == pp->nterms)
      low--;
    qq = pp->orbits + low;
    m = qq->M + (JDE - qq->JDE)*qq->n; /* mean anomaly */
    e = qq->e;
    k = qq->v_factor;
    q = qq->q;
    kepler(m, e, k, &m, r); /* solve Kepler's equation */
    *r = q**r;			/* distance */
    f[0] = *r*qq->xfac*sin(qq->xangle + m);
    f[1] = *r*qq->yfac*sin(qq->yangle + m);
    f[2] = *r*qq->zfac*sin(qq->zangle + m);
  } else {			/* linear interpolation */
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
    *r = q**r;			/* distance */
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
/*--------------------------------------------------------------------------*/
void extraElementsHeliocentric(Double JDE, Double *equinox, Double *f, 
                               Double *r)
{
  static Double	xfac, yfac, zfac, xangle, yangle, zangle, epoch, m0, n,
	v_factor, e, a, q, theequinox;
  Double m;

  if (haveExtraElements & 4) {	/* new */
    Double i, node, peri, ci, si, cn, sn,
      ff, g, p, qq;
    Int ii;

    theequinox = extraElements[0];
    epoch = extraElements[1];
    e = fabs(extraElements[3]);
    i = extraElements[4]*DEG;
    node = extraElements[5]*DEG;
    peri = extraElements[6]*DEG;
    v_factor = (e == 1)? 0: sqrt(fabs((1 + e)/(1 - e)));
    switch (haveExtraElements & 3) {
      case 2:			/* /QELEMENTS */
	q = fabs(extraElements[2]);
	if (e == 1) {		/* parabola */
	  n = q? 0.03649116245/(q*sqrt(q)): 0.0; /* n rad/d */
	} else {
	  a = fabs(q/(1 - e));
	  n = a? 0.01720209895/(a*sqrt(a)): 0.0; /* n rad/d */
	  /* if (e > 1) n = -n; */
	}
	m0 = (epoch - extraElements[7])*n;
	break;
      case 1:			/* default */
	if (e == 1) {
	  anaerror("Parabolic orbits have an infinite semimajor axis", 0);
	  for (ii = 0; ii < 3; ii++)
	    f[ii] = 0.0;
	  *r = 0.0;
	  return;
	}
	a = fabs(extraElements[2]);
	q = fabs(a*(1 - e));
	n = a? 0.01720209895/(a*sqrt(a)): 0.0; /* n rad/d */      
	/* if (e > 1) n = -n; */
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
    xfac = sqrt(ff*ff + p*p); /* "a" */
    xangle = atan2(ff,p) + peri; /* "A" */
    yfac = sqrt(g*g + qq*qq); /* "b" */
    yangle = atan2(g,qq) + peri; /* "B" */
    zfac = si; /* "c" */
    zangle = peri; /* "C" */
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
/*--------------------------------------------------------------------------*/
void heliocentricXYZr(Double JDE, Int object, Double equinox, Double *pos,
		      Double *r, Double tolerance, Int vocal, Int source)
     /* returns in <f> the cartesian heliocentric ecliptic coordinates of
	object <object> for the desired <equinox> at <JDE>, and in
	<r> the heliocentric distance */
{
  Double	T, standardEquinox;
  Int	i;

  T = (JDE - J2000)/365250;	/* Julian millennia since J2000.0 */
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
	    if (vocal)
	      printf("ASTRON: precess ecliptic coordinates from J2000.0 to ecliptic of date (JD %1$.14g = %1$-#24.6J)\n", JDE);
	    XYZ_eclipticPrecession(pos, J2000, JDE); /* precess to equinox of date */
	    if (vocal)
	      printXYZtoLBR(pos);
	  }
	  XYZ_VSOPtoFK5(T, pos); /* transform to FK5 (must be equinox of date) */
	  if (vocal) {
	    printf("ASTRON: FK5 (%s) geometric heliocentric ecliptic coordinates, ecliptic of date (JD %2$.14g = %2$-#24.6J):\n", objectName(object), JDE);
	    printXYZtoLBR(pos);
	  }
	  if (JDE != equinox) {
	    if (vocal)
	      printf("ASTRON: precess ecliptic coordinates from JD %1$.14g = %1$-#24.6J to JD %2$.14g = %2$-#24.6J\n", JDE, equinox);
	    XYZ_eclipticPrecession(pos, JDE, equinox); /* precess to desired equinox */
	    if (vocal)
	      printXYZtoLBR(pos);
	  }
	} else {
	  if (J2000 != equinox) {
	    printf("ASTRON: precess ecliptic coordinates from J2000.0 to JD %1$.14g = %1$-#24.6J\n", equinox);
	    XYZ_eclipticPrecession(pos, J2000, equinox); /* precess to desired equinox */
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
	  XYZ_VSOPtoFK5(T, pos); /* transform to FK5 */
	  if (vocal) {
	    printf("ASTRON: FK5 (%s) geometric heliocentric ecliptic coordinates, equinox/ecliptic of date (JD %2$.14g = %2$-#24.6J):\n", objectName(object), JDE);
	    printXYZtoLBR(pos);
	  }
	}
	if (JDE != equinox) {
	  if (vocal)
	    printf("ASTRON: precess ecliptic coordinates from JD %1$.14g = %1$-#24.6J to JD %2$.14g = %2$-#24.6J\n", JDE, equinox);
	  XYZ_eclipticPrecession(pos, JDE, equinox); /* precess to desired equinox */
	}
      }
      break;
    default:
      anaerror("Illegal VSOP model type", 0);
      pos[0] = pos[1] = pos[2] = *r = 0.0;
      return;
    }
    *r = hypota(3, pos);        /* heliocentric distance */
    if (vocal) {
      printf("ASTRON: (%s) geometric heliocentric ecliptic coordinates for equinox JD %2$.14g = %2$-#24.6J:\n", 
             objectName(object), equinox);
      printXYZtoLBR(pos);
    }
    break;
  case 10:			/* the Moon */
    {
      Double lmoon, elon, msun, mmoon, nodedist, a1, a2, a3, suml, sumr,
	sumb, E[5], XYZmoon[3], Tc;
      struct moonlrTerm *mlrt;
      struct moonbTerm *mbt;
      Int i;

      Tc = T*10 + 4.069459e-10; /* convert from julian millennia to */
      /* julian centuries and take out light-time correction which is */
      /* implicitly included in the used lunar ephemeris (assumed */
      /* equivalent to 385000.56 km) */
      lmoon = mpol4(218.3164591,481267.88134236,-0.0013268,1.0/538841,-1.0/65194000,Tc,360);
      lmoon *= DEG;
      elon = mpol4(297.8502042,445267.1115168,-0.0016300,1.0/545868,-1.0/113065000,Tc,360);
      elon *= DEG;
      msun = mpol3(357.5291092,35999.0502909,-0.0001536,1.0/24490000,Tc,360);
      msun *= DEG;
      mmoon = mpol4(134.9634114,477198.8676313,0.0089970,1.0/69699,-1.0/14712000,Tc,360);
      mmoon *= DEG;
      nodedist = mpol4(93.2720993,483202.0175273,-0.0034029,-1.0/3526000,1.0/863310000,Tc,360);
      nodedist *= DEG;
      a1 = fmod(119.75 + 131.849*Tc,360);
      a1 *= DEG;
      a2 = fmod(53.09 + 479264.290*Tc,360);
      a2 *= DEG;
      a3 = fmod(313.45 + 481266.484*Tc,360);
      a3 *= DEG;
      suml = sumr = sumb = 0.0;
      mlrt = moonlr;
      mbt = moonb;
      E[1] = E[3] = pol2(1,-0.002516,-0.0000074,Tc);
      E[2] = 1;
      E[0] = E[4] = E[1]*E[1];
      for (i = 0; i < sizeof(moonlr)/sizeof(struct moonlrTerm); i++) {
	Double arg, fac;
	fac = E[mlrt->m + 2];
	arg = mlrt->d*elon + mlrt->m*msun + mlrt->mm*mmoon + mlrt->f*nodedist;
	suml += mlrt->l*sin(arg)*fac;
	sumr += mlrt->r*cos(arg)*fac;
	mlrt++;
      }
      for (i = 0; i < sizeof(moonb)/sizeof(struct moonbTerm); i++) {
	Double arg, fac;
	fac = E[mbt->m + 2];
	arg = mbt->d*elon + mbt->m*msun + mbt->mm*mmoon + mbt->f*nodedist;
	sumb += mbt->b*sin(arg)*fac;
	mbt++;
      }
      suml += 3958*sin(a1) + 1962*sin(lmoon - nodedist) + 318*sin(a2);
      sumb += -2235*sin(lmoon) + 382*sin(a3) + 175*sin(a1 - nodedist)
	+ 175*sin(a1 + nodedist) + 127*sin(lmoon - mmoon)
	- 115*sin(lmoon + mmoon);
      pos[0] = lmoon + suml*DEG/1000000; /* geocentric longitude referred */
					  /* to the mean equinox of the date */
      pos[1] = sumb*DEG/1000000;	/* geocentric latitude */
      pos[2] = (385000.56 + sumr/1000)/149597870; /* AU (center-center) */
      if (vocal) {
        printf("ASTRON: lunar ecliptic geocentric coordinates for equinox of date:\n");
        printLBRtoXYZ(pos);
      }
      if (vocal && JDE != equinox)
	printf("ASTRON: precess ecliptic coordinates from JD %1$.14g = %1$-#24.6J to JD %2$.14g = %2$-#24.6J\n", JDE, equinox);
      eclipticPrecession(pos, JDE, equinox);
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
  case -1:			/* ELEMENTS object - if defined */
    if (!haveExtraElements) {
      anaerror("Illegal object number %1d", 0, object);
      for (i = 0; i < 3; i++)
	pos[i] = 0.0;
      *r = 0.0;
      return;
    }
    extraElementsHeliocentric(JDE, &standardEquinox, pos, r);
    if (standardEquinox == EQUINOX_OF_DATE)
      standardEquinox = JDE;
    if (fabs(standardEquinox - equinox) > 1) { /* precession */
	/* currently circuitous -> inefficient */
      Double f[3];
      XYZtoLBR(pos, f);
      eclipticPrecession(f, standardEquinox, equinox);
      LBRtoXYZ(f, pos);
    }
    break;
  default:
    if (extraHeliocentric(JDE, object, &standardEquinox, pos, r)
	== ANA_ERROR) {		/* cartesian */
      for (i = 0; i < 3; i++)
	pos[i] = 0.0;
      *r = 0.0;
      return;
    }
    if (standardEquinox == EQUINOX_OF_DATE)
      standardEquinox = JDE;
    if (fabs(standardEquinox - equinox) > 1) { /* precession */
      /* currently circuitous -> inefficient */
      Double f[3];
      XYZtoLBR(pos, f);
      eclipticPrecession(f, standardEquinox, equinox);
      LBRtoXYZ(f, pos);
    }
    break;
  }
}
/*--------------------------------------------------------------------------*/
void LBRtoXYZ(Double *pos, Double *pos2)
     /* calculates cartesian coordinates XYZ from polar coordinates LBR.
      pos must be unequal to pos2. */
{
  Double	cl, cb, sl, sb;

  /* calculate X Y Z from L B R */
  cl = cos(pos[0]);
  cb = cos(pos[1]);
  sl = sin(pos[0]);
  sb = sin(pos[1]);
  pos2[0] = pos[2]*cb*cl;	/* X */
  pos2[1] = pos[2]*cb*sl;	/* Y */
  pos2[2] = pos[2]*sb;		/* Z */
}
/*--------------------------------------------------------------------------*/
void XYZtoLBR(Double *pos, Double *pos2)
/* transform from cartesian to polar coordinates.  pos must be unequal to
 pos2 */
{
  Double	r, h;

  h = hypot(pos[0], pos[1]);
  r = hypot(h, pos[2]);
  pos2[2] = r;                              /* R */
  pos2[0] = h? atan2(pos[1], pos[0]): 0.0; /* L */
  if (pos2[0] < 0)
    pos2[0] += TWOPI;
  pos2[1] = r? asin(pos[2]/pos2[2]): 0.0;	/* B */
}
/*--------------------------------------------------------------------------*/
void ectoeq(Double *pos, Double ceps, Double seps, char forward)
/* transforms from ecliptical to equatorial coordinates or vice versa */
{ 
  Double	alpha, delta, sl, cl, sb, cb;

  sl = sin(pos[0]);
  cl = cos(pos[0]);
  sb = sin(pos[1]);
  cb = cos(pos[1]);
  if (forward == 0)		/* reverse transform */
    seps = -seps;
  alpha = atan2(sl*ceps - sb*seps/cb, cl);
  if (alpha < 0)
    alpha += TWOPI;
  delta = asin(sb*ceps + cb*seps*sl);
  pos[0] = alpha;
  pos[1] = delta;
}
/*--------------------------------------------------------------------------*/
void galtoeq(Double *pos, Double equinox, char forward)
 /* transforms from galactic to equatorial coordinates or vice versa */
{
  Double x;

  if (forward) {		/* from galactic to equatorial */
    Double A = 123*DEG;
    Double B = 27.4*DEG;

    pos[0] = pos[0] - A;
    x = 12.25*DEG + atan2(sin(pos[0]), cos(pos[0])*sin(B) - tan(pos[1])*cos(B));
    pos[1] = asin(sin(pos[1])*sin(B) + cos(pos[1])*cos(B)*cos(pos[0]));
    pos[0] = famod(x, TWOPI);
    precessEquatorial(pos, pos + 1, B1950, equinox);
  } else {			/* from equatorial to galactic */
    Double A = 192.25*DEG;
    Double B = 27.4*DEG;

    precessEquatorial(pos, pos + 1, equinox, B1950);
    pos[0] = A - pos[0];
    pos[1] = pos[1];
    x = 303*DEG - atan2(sin(pos[0]), cos(pos[0])*sin(B) - tan(pos[1])*cos(B));
    pos[1] = asin(sin(pos[1])*sin(B) + cos(pos[1])*cos(B)*cos(pos[0]));
    pos[0] = famod(x, TWOPI);
  }
}
/*--------------------------------------------------------------------------*/
Int ana_astrf(Int narg, Int ps[], Int forward) {
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
  loopInfo srcinfo, tgtinfo;
  pointer src, tgt;
  Int result, from, to;
  Double pos[2], ceps, seps, epsilon, equinox;

  if (internalMode & 2)
    from = 2;			/* ecliptical */
  else if (internalMode & 4)
    from = 3;			/* galactic */
  else
    from = 1;			/* equatorial */
  if (internalMode & 8)		/* equatorial */
    to = 1;
  else if (internalMode & 32)
    to = 3;			/* galactic */
  else
    to = 2;			/* ecliptical */
  if (from == to)
    return ps[0];		/* nothing to do */
  if (standardLoop(ps[0], ANA_ZERO, SL_UPGRADE | SL_AXISCOORD | SL_EACHROW,
		   ANA_FLOAT, &srcinfo, &src, &result, &tgtinfo, &tgt)
      == ANA_ERROR)
    return ANA_ERROR;
  if (srcinfo.ndim < 1 || srcinfo.dims[0] < 2)
    return anaerror("Need at least two elements in first dimension", -1);
  if (narg > 1) {
    equinox = double_arg(ps[1]);
    if (internalMode & 128)	/* /BESSELIAN to /JULIAN */
      equinox = 0.99997860193253*equinox + 0.0413818359375; /* to Julian */
    if (internalMode & 3)	/* /JULIAN to JD */
      equinox = (equinox - 2000.0)*365.25 + J2000;
  } else			/* JD */
    equinox = J2000;
  epsilon = obliquity(equinox, NULL);
  ceps = cos(epsilon);
  seps = sin(epsilon);
  do {
    switch (tgtinfo.type) {
    case ANA_FLOAT:
      switch (srcinfo.type) {
      case ANA_BYTE:
	pos[0] = (Double) src.b[0]*DEG;
	pos[1] = (Double) src.b[1]*DEG;
	break;
      case ANA_WORD:
	pos[0] = (Double) src.w[0]*DEG;
	pos[1] = (Double) src.w[1]*DEG;
	break;
      case ANA_LONG:
	pos[0] = (Double) src.l[0]*DEG;
	pos[1] = (Double) src.l[1]*DEG;
	break;
      case ANA_FLOAT:
	pos[0] = (Double) src.f[0]*DEG;
	pos[1] = (Double) src.f[1]*DEG;
	break;
      default:
        break;
      }
      switch (from) {
      case 2:			/* ecliptical */
	ectoeq(pos, ceps, seps, 1);
	break;
      case 3:			/* galactic */
	galtoeq(pos, equinox, 1);
	break;
      }
      switch (to) {
      case 2:			/* ecliptical */
	ectoeq(pos, ceps, seps, 0);
	break;
      case 3:
	galtoeq(pos, equinox, 0);
	break;
      }
      tgt.f[0] = (Float) pos[0]*RAD;
      tgt.f[1] = (Float) pos[1]*RAD;
      break;
    case ANA_DOUBLE:
      switch (srcinfo.type) {
      case ANA_BYTE:
	pos[0] = (Double) src.b[0]*DEG;
	pos[1] = (Double) src.b[1]*DEG;
	break;
      case ANA_WORD:
	pos[0] = (Double) src.w[0]*DEG;
	pos[1] = (Double) src.w[1]*DEG;
	break;
      case ANA_LONG:
	pos[0] = (Double) src.l[0]*DEG;
	pos[1] = (Double) src.l[1]*DEG;
	break;
      case ANA_FLOAT:
	pos[0] = (Double) src.f[0]*DEG;
	pos[1] = (Double) src.f[1]*DEG;
	break;
      case ANA_DOUBLE:
	pos[0] = src.d[0]*DEG;
	pos[1] = src.d[1]*DEG;
	break;
      default:
        break;
      }
      switch (from) {
      case 2:			/* ecliptical */
	ectoeq(pos, ceps, seps, 1);
	break;
      case 3:			/* galactic */
	galtoeq(pos, equinox, 1);
	break;
      }
      switch (to) {
      case 2:			/* ecliptical */
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
    src.b += srcinfo.rdims[0]*srcinfo.stride;
    tgt.b += tgtinfo.rdims[0]*tgtinfo.stride;
  } while (advanceLoop(&srcinfo, &src) < srcinfo.rndim);
  return result;
}
/*--------------------------------------------------------------------------*/
#define s_parallax (4.263451e-5)
void parallax(Double *pos, Double r0, Double rcp, Double rsp)
/* corrects equatorial planetocentric coordinates for parallax, i.e. */
/* transforms to equatorial topocentric coordinates.  Tsid is the */
/* local sidereal time, r0 is the apparent planetocentric distance, */
/* rcp and rsp indicate the topocentric coordinates of the observer */
{
  Double	r, cd, ch, sd, sh, u, A, B, C, q;

  r = s_parallax/r0;	/* sine of equatorial horizontal parallax */
  cd = cos(pos[1]);		/* declination */
  sd = sin(pos[1]);
  sh = sin(pos[0]);		/* hour angle */
  ch = cos(pos[0]);
  A = cd*sh;
  B = cd*ch - rcp*r;
  C = sd - rsp*r;
  u = A*A + B*B;
  q = sqrt(u + C*C);
  pos[0] = fmod(atan2(A, B), TWOPI);
  if (pos[0] < 0)
    pos[0] += TWOPI;	/* apparent hour angle */
  pos[1] = asin(C/q);	/* apparent declination */
}
/*--------------------------------------------------------------------------*/
void refract(Double *pos, Double height)
/* corrects height above the horizon (pos[1]) for average 
   refraction - but only if the refracted height is nonnegative */
{
  Double	R, h;

  h = pos[1];
  R = 1/tan(h + 7.31/(h*RAD + 4.4));
  if (R < 0)			/* 0.08" error for h = 90 deg */
    R = 0;
  else
    R *= exp(-h*1.22626e-4);	/* correction for altitude */
  h += R;
  if (h >= 0)
    pos[1] = h*DEG/60;
}
/*--------------------------------------------------------------------------*/
void eqtohor(Double *pos, Double cphi, Double sphi, char forward)
/* transforms from equatorial to horizontal coordinates, or vice versa */
{
  Double	A, h, sH, cH, sd, cd;

  sH = sin(pos[0]);
  cH = cos(pos[0]);
  sd = sin(pos[1]);
  cd = cos(pos[1]);
  if (forward == 0)		/* reverse transformation */
    cphi = -cphi;
  A = atan2(sH, cH*sphi - sd*cphi/cd);
  if (forward == 0 && A < 0)
    A += TWOPI;
  h = asin(sphi*sd + cphi*cd*cH);
  pos[0] = A;
  pos[1] = h;
}
/*--------------------------------------------------------------------------*/
#define FLAT	0.99664719
#define R_EARTH	6378140
void geocentricCoords(Double latitude, Double height, Double *rcp, Double *rsp)
/* returns the geocentric quantities rho cos phi and rho sin phi */
{ 
  Double	u;

  u = atan(FLAT*tan(latitude));
  *rcp = cos(u) + height*cos(latitude)/R_EARTH;
  *rsp = FLAT*sin(u) + height*sin(latitude)/R_EARTH;
}
/*--------------------------------------------------------------------------*/
Double meanDistance(Int obj1, Int obj2)
{
  if (obj1 < 0 || obj1 > 8 || obj2 < 0 || obj2 > 8)
    return 0.0;
  else
    return meanDistances[obj1][obj2];
}
/*--------------------------------------------------------------------------*/
void showraddms(char *prefix, Double x)
{
  printf("%1$s%2$.10g rad = %3$.10g deg = %3$-13.2T dms\n", prefix, x, x*RAD);
}
/*--------------------------------------------------------------------------*/
void showradhms(char *prefix, Double x)
{
  printf("%1$s%2$.10g rad = %3$.10g deg = %3$#-13.2T hms\n", prefix, x, x*RAD);
}
/*--------------------------------------------------------------------------*/
void printXYZtoLBR(Double *xyz)
{
  Double lbr[3];
  XYZtoLBR(xyz, lbr);
  printf(" X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
  showraddms(" L = ", lbr[0]);
  showraddms(" B = ", lbr[1]);
  printf(" R = %.10g\n", lbr[2]);
}
/*--------------------------------------------------------------------------*/
void printLBRtoXYZ(Double *lbr)
{
  Double xyz[3];
  LBRtoXYZ(lbr, xyz);
  showraddms(" L = ", lbr[0]);
  showraddms(" B = ", lbr[1]);
  printf(" R = %.10g\n", lbr[2]);
  printf(" X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
}
/*--------------------------------------------------------------------------*/
void printHBRtoXYZ(Double *lbr)
{
  Double xyz[3];
  LBRtoXYZ(lbr, xyz);
  showradhms(" H = ", lbr[0]);
  showraddms(" B = ", lbr[1]);
  printf(" R = %.10g\n", lbr[2]);
  printf(" X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
}
/*--------------------------------------------------------------------------*/
Int ana_astropos(Int narg, Int ps[])
     /* returns the positions of a set of heavenly bodies at a specific */
     /* set of times, for equinox 2000.0 */
     /* Syntax: X = ASTRON(JDs, OBJECTS [, OBJECT0, OBSERVER=OBSERVER, */
     /*                    EQUINOX=EQUINOX, ELEMENTS=elements] */
     /*			   [, /XYZ, /EQUATORIAL, /ECLIPTICAL, /ELONGATION, */
     /*			    /HORIZONTAL, /DATE, /ERROR, /ABERRATION, */
     /*                     /GEOMETRICAL, /TDT, /QELEMENTS, /FK5, /NUTATION,
                            /LIGHTTIME]) */
     /* JDs = Julian dates for which elements are desired, in UT */
     /* OBJECTS = scalar or array of object numbers. */
     /* OBJECT2 = number of the object relative to which the other */
     /*		  coordinates are requested */
     /* OBSERVER = [latitude,W longitude,height] of the observer, */
     /*           in degrees or meters */
     /* EQUINOX = equinox year of the coordinate system (default: 2000.0) */
     /*                   in Julian years TDT. */
     /* /XYZ: return ecliptical X Y Z coordinates in AU; otherwise return */
     /*       polar coordinates (longitude, latitude (degrees), */
     /*       distance (AU) */
     /* /EQUATORIAL: return right ascension, declination, and distance, */
     /*              or equivalent XYZ */
     /* /ECLIPTICAL: return ecliptic longitude, latitude, and distance */
     /*              or equivalent XYZ */
     /* /ELONGATION: return elongation, phase angle, and estimated */
     /*              visual magnitude */
     /* /HORIZONTAL: return azimuth (west from south), height, and distance */
     /*              or equivalent XYZ */
     /* /PLANETOCENTRIC: ecliptical or equatorial coordinates relative to
	the orbit or equator of the planet */
     /* if you don't specify an OBJECT2), then OBJECT2 is assumed to be */
     /*              the Earth. */
     /* current object numbers:  0 - Sun, 1 - Mercury, 2 - Venus, */
     /* 3 - Earth, 4 - Mars, 5 - Jupiter, 6 - Saturn, 7 - Uranus, */
     /* 8 - Neptune, 9 - Pluto, 10 - Moon. */
{
  char	tdt, *string;
  Int	iq, nJD, *object, nObjects, object0, dims[MAX_DIMS],
    nDims, i, j, result, coordSystem, vocal;
  Double	*JD, *f, *f0, longitude, latitude, height = 0.0, rsp,
    rcp, clat, slat, equinox;
  Double	tolerance;

  if (internalMode & S_CONJSPREAD) 	/* /CONJSPREAD */
    internalMode = internalMode | S_XYZ | S_ECLIPTICAL;
  coordSystem = (internalMode & S_COORDS); /* desired coordinate system */
  if (!coordSystem)
    coordSystem = S_ECLIPTICAL;	/* default: ecliptical */
  vocal = (internalMode & S_VOCAL); /* print intermediate results */

  iq = *ps;			/* JDs */
  if (!iq)
    return cerror(ILL_ARG, iq);
  switch (symbol_class(iq)) {
    case ANA_SCALAR:
      iq = ana_double(1, &iq);
      JD = &scalar_value(iq).d;
      nJD = 1;
      break;
    case ANA_ARRAY:
      iq = ana_double(1, &iq);
      JD = (Double *) array_data(iq);
      nJD = array_size(iq);
      break;
    default:
      return cerror(ILL_CLASS, *ps);
  }

  iq = ps[1];			/* objects */
  if (!iq) {
    if (narg > 5 && ps[5])
      iq = ANA_MINUS_ONE;	/* specified elements */
    else
      return cerror(ILL_ARG, ps[1]);
  }
  switch (symbol_class(iq)) {
    case ANA_SCALAR:
      iq = ana_long(1, &iq);
      object = &scalar_value(iq).l;
      nObjects = 1;
      break;
    case ANA_ARRAY:
      iq = ana_long(1, &iq);
      object = (Int *) array_data(iq);
      nObjects = array_size(iq);
      break;
    default:
      return cerror(ILL_CLASS, ps[1]);
  }

  if (narg > 2 && ps[2]) {	/* object0 */
    object0 = int_arg(ps[2]);
  } else
    object0 = EARTH;		/* no object0: use Earth */
  if (object0 != EARTH) {	/* not relative to Earth */
    if (coordSystem != S_ECLIPTICAL && coordSystem != S_EQUATORIAL)
      return anaerror("Non-ecliptic/non-equatorial coordinates are only returned relative to the Earth", 0);
  }

  if (narg > 3 && ps[3]) {	/* OBSERVER */
    if (symbol_class(ps[3]) != ANA_ARRAY
	|| array_size(ps[3]) != 3)
      return
	anaerror("OBSERVER must be a three-element array (lat/[deg], lon/[deg], h/[m])", ps[3]);
    iq = ana_double(1, ps + 3);
    f = (Double *) array_data(iq);
    latitude = f[0];		/* in degrees */
    longitude = f[1];		/* in degrees */
    height = f[2];		/* in meters above mean sea level */
    if (latitude < -90 || latitude > 90)
      return anaerror("Illegal latitude: %14.6g; must be between -90 and +90 degrees", ps[3], latitude);
    latitude *= DEG;
    longitude *= DEG;
    geocentricCoords(latitude, height, &rcp, &rsp);
    clat = cos(latitude);
    slat = sin(latitude);
  } else {			/* assume planetocentric */
    latitude = S_PLANETOCENTRIC;
    if (coordSystem == S_HORIZONTAL)
      return anaerror("Need OBSERVER to return horizontal coordinates", 0);
    /* avoid compiler warnings about possibly uninitialized variables */
    longitude = 0;
    clat = 1;
    slat = 0;
  }

  if (narg > 4 && ps[4]) { 	/* EQUINOX */
				/* equinox date (JDE) */
    if (internalMode & S_DATE)
      return anaerror("Multiple equinoxes specified", ps[4]);
    switch (symbol_class(ps[4])) {
      case ANA_SCALAR:		/* assume Julian year */
	equinox = (double_arg(ps[4]) - 2000.0)*365.25 + J2000;
	break;
      case ANA_STRING:
	string = string_value(ps[4]);
	equinox = atof(string + 1); /* year */
	switch (toupper(*string)) {
	  case 'J':		/* Julian */
	    break;
	  case 'B':		/* Besselian (approximate) -> Julian */
	    equinox = 0.99997860193253*equinox + 0.0413818359375;
	    break;
	  default:
	    return anaerror("Illegal EQUINOX specification", ps[4]);
	}
	equinox = (equinox - 2000.0)*365.25 + J2000;
	break;
      default:
	return cerror(ILL_CLASS, ps[4]);
    }
  } else
    equinox = J2000;		/* default: J2000.0 */

  if (narg > 5 && ps[5]) {	/* ELEMENTS */
    /* elements: equinox epoch a e i node peri M absmag
              or equinox epoch q e i node peri Tperi absmag /QELEMENTS */
    j = ps[5];
    if (symbol_class(j) != ANA_ARRAY
	|| array_size(j) != 9)
      return anaerror("Need a 9-element array with ELEMENTS", j);
    j = ana_double(1, &j);
    memcpy(extraElements, array_data(j), 9*sizeof(Double));
    haveExtraElements = (internalMode & S_QELEMENTS)? 6: 5; /* 5->A, 6->Q */
  } else haveExtraElements = 0;		/* none */

  				/* create result array */
  dims[0] = 3;
  nDims = 1;
  if ((nObjects > 1 || internalMode & S_KEEPDIMS)
      && (internalMode & S_CONJSPREAD) == 0)
    dims[nDims++] = nObjects;
  if (nJD > 1 || internalMode & S_KEEPDIMS)
    dims[nDims++] = nJD;
  result = array_scratch(ANA_DOUBLE, nDims, dims);
  f = f0 = (Double *) array_data(result);
  
  tdt = internalMode & S_TDT;	/* time is specified in TDT rather than UT */

  tolerance = 0;
  if (narg > 6 && ps[6])
    tolerance = double_arg(ps[6]);
  if (internalMode & S_TRUNCATEVSOP && !tolerance)
    tolerance = 1e-4;

  /* calculate coordinates */
  for (j = 0; j < nJD; j++) {	/* all dates */
    if (vocal)
      printf("ASTRON: calculating for JD = %1$.7f = %1$#-24.6J\n", JD[j]);
    Double jd = tdt? JD[j]: JDE(JD[j], +1); /* calculate date in TDT */
    if (vocal) {
      if (tdt) {
        puts("ASTRON: JD is in TDT already");
      } else {
        printf("ASTRON: UT => TDT:      JD = %.7f\n", jd);
        printf("ASTRON: delta T = %.10g s\n", (jd - JD[j])*86400);
      }
    }
    Double dPsi, cdPsi, sdPsi, dEps;
    if (internalMode & S_NUTATION) { /* nutation */
      nutation(jd, &dPsi, &cdPsi, &sdPsi, &dEps); /* nutation parameters */
      if (vocal) {
        printf("ASTRON: nutation constants:\n");
        showraddms(" dPsi = ", dPsi);
        showraddms(" dEps = ", dEps);
      }
    } else {
      dPsi = sdPsi = dEps = 0.0;	/* ignore nutation */
      cdPsi = 1.0;
      if (vocal)
        puts("ASTRON: no nutation correction.");
    }
    if (internalMode & S_DATE)
      equinox = jd;
    if (vocal)
      printf("ASTRON: equinox:        JD = %.7f\n", equinox);
    Double epsilon = obliquity(equinox, &dEps);
    if (vocal) {
      if (dPsi)
        printf("ASTRON: obliquity of ecliptic for equinox, corrected for nutation:\n");
      else
        printf("ASTRON: mean obliquity of ecliptic for equinox:\n");
      showraddms(" epsilon = ", epsilon);
    }
    Double ceps = cos(epsilon);
    Double seps = sin(epsilon);
    Double Tsid = siderealTime(JDE(jd, -1), &dPsi, ceps);
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
    
    Double mean[3];
    if (internalMode & S_CONJSPREAD)
      mean[0] = mean[1] = mean[2] = 0;

    Double pos_sun_obs[3], r_sun_obs;
    /* calculate the position of the observer */
    heliocentricXYZr(jd, object0, equinox, pos_sun_obs, &r_sun_obs, tolerance, 
                     vocal, internalMode & S_VSOP);
    /* cartesian ecliptic heliocentric coordinates of the observer
       at target time */
    /* pos_sun_obs[0] = X, pos_sun_obs[1] = Y, pos_sun_obs[2] = Z, */
    /* r_sun_obs = heliocentric distance of the observer */
    if (vocal) {
      printf("ASTRON: observer (%s) geometric heliocentric ecliptic coordinates for equinox:\n", objectName(object0));
      printXYZtoLBR(pos_sun_obs);
    }

    for (i = 0; i < nObjects; i++) { /* all objects */
      Double lighttime = 0.0;
      Double r_sun_tgt, r_obs_tgt, pos_sun_tgt[9], pos_obs_tgt[9];
      if (internalMode & (S_LIGHTTIME | S_ABERRATION)) {
        if (vocal)              /* print geometric position */
          heliocentricXYZr(jd, object[i], equinox, pos_sun_tgt, &r_sun_tgt,
                           tolerance, 1, internalMode & S_VSOP);
        /* calculate light time */
	/* get initial guess of "average" distance between the objects */
	/* for an initial guess of the light time */
	r_sun_tgt = meanDistance(object[i],object0 >= 0? object0: 0);
	lighttime = r_sun_tgt*AUtoJD;   /* initial estimate of light time */
	Double prev_lighttime = lighttime + 1; /* previous estimate */
        /* to get loop going, set initial value of prev_lighttime
           different from current value of lighttime */
	r_obs_tgt = 0.0; /* initial estimate of geometrical distance */

        /* now we converge upon the light-time to the target object */
        uint32_t count = 0;
        while (fabs(lighttime - prev_lighttime) > DBL_EPSILON*1000
               && count < 25) {
          /* no convergence yet */ 
          prev_lighttime = lighttime; /* old estimate is previous estimate */
          Double jd_lt = jd - lighttime; /* time corrected for light time */
          heliocentricXYZr(jd_lt, object[i], equinox, pos_sun_tgt, &r_sun_tgt,
                           tolerance, 0, internalMode & S_VSOP);
          /* pos_tgt = cartesian ecliptic heliocentric coordinates of the
             target, r_sun_tgt = heliocentric distance of the target */
          pos_obs_tgt[0] = pos_sun_tgt[0] - pos_sun_obs[0]; /* dX/AU */
          pos_obs_tgt[1] = pos_sun_tgt[1] - pos_sun_obs[1]; /* dY/AU */
          pos_obs_tgt[2] = pos_sun_tgt[2] - pos_sun_obs[2]; /* dZ/AU */
          /* apparent distance */
          r_obs_tgt = hypota(3, pos_obs_tgt);
          lighttime = r_obs_tgt*AUtoJD; /* new estimate of light time */
          count++;
        } /* end of while */
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
      case 0:                   /* geometrical; obs + target at jd */
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
      case S_LIGHTTIME:         /* observer at jd, target at jd - lighttime */
        /* calculated this already when figuring out the lighttime */
        if (vocal) {
          puts("ASTRON: target planetocentric ecliptic coordinates corrected for light-time:");
          printXYZtoLBR(pos_obs_tgt);
        }
        break;
      case S_ABERRATION:        /* observer at jd - lighttime, target at jd */
        {
          Double pos_sun_obs_lt[3], r_sun_obs_lt, pos_sun_tgt_nolt[3],
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
        Double f = r_obs_tgt/hypota(3, pos_obs_tgt);
        pos_obs_tgt[0] *= f;
        pos_obs_tgt[1] *= f;
        pos_obs_tgt[2] *= f;
        if (vocal) {
          puts("ASTRON: target planetocentric ecliptic coordinates corrected for aberration:");
          printXYZtoLBR(pos_obs_tgt);
        }
        break;
      case S_LIGHTTIME | S_ABERRATION: /* observer + target at jd - lighttime */
        {
          Double pos_sun_obs_lt[9], r_sun_obs_lt;
          heliocentricXYZr(jd - lighttime, object0, equinox, pos_sun_obs_lt,
                           &r_sun_obs_lt, tolerance, vocal,
                           internalMode & S_VSOP);
          pos_obs_tgt[0] = pos_sun_tgt[0] - pos_sun_obs_lt[0];
          pos_obs_tgt[1] = pos_sun_tgt[1] - pos_sun_obs_lt[1];
          pos_obs_tgt[2] = pos_sun_tgt[2] - pos_sun_obs_lt[2];
        }
        Double fac = r_obs_tgt/hypota(3, pos_obs_tgt);
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
        /* apply nutation */
        Double pos_nut[3];
        pos_nut[0] = pos_obs_tgt[0]*cdPsi - pos_obs_tgt[1]*sdPsi;
        pos_nut[1] = pos_obs_tgt[0]*sdPsi + pos_obs_tgt[1]*cdPsi;
        pos_nut[2] = pos_obs_tgt[2];
        memcpy(pos_obs_tgt, pos_nut, 3*sizeof(Double));
        if (vocal) {
          puts("ASTRON: planetocentric ecliptic coordinates corrected for nutation:");
          printXYZtoLBR(pos_obs_tgt);
        }
      }

      Double final[3];
      if (coordSystem == S_ELONGATION) { /* elongation, phase angle, magn */
        /* calculate the elongation */
        final[0] = (r_sun_obs*r_sun_obs + r_obs_tgt*r_obs_tgt
                     - r_sun_tgt*r_sun_tgt)/(2*r_sun_obs*r_obs_tgt);
        if (final[0] > 1)      /* ignore round-off errors */
	  final[0] = 1;
	else if (final[0] < -1) /* ignore round-off errors */
	  final[0] = -1;
	final[0] = acos(final[0]); /* elongation */

        /* calculate the phase angle */
	final[1] = (r_obs_tgt*r_obs_tgt + r_sun_tgt*r_sun_tgt
                     - r_sun_obs*r_sun_obs)/(2*r_sun_tgt*r_obs_tgt);
        if (final[1] > 1)
	  final[1] = 1;
	else if (final[1] < -1)
	  final[1] = -1;
	final[1] = acos(final[1]); /* phase angle */

        /* calculate the magniture */
	final[2] = magnitude(r_sun_tgt, r_obs_tgt, final[1]*RAD, object[i]);
        if (vocal) {
          puts("ASTRON: transform to elongation, phase angle, magnitude");
          showraddms(" el = ", final[0]);
          showraddms(" ph = ", final[1]);
          printf(" mag = %.10g\n", final[2]);
        }
      } else {
	XYZtoLBR(pos_obs_tgt, final);	/* to polar coordinates */
        if (internalMode & S_FK5)
          VSOPtoFK5(TC2000(jd), final);     /* to FK5 */
        if (vocal) {
          puts("ASTRON: planetocentric ecliptic coordinates:");
          printLBRtoXYZ(final);
        }
	if (latitude != S_PLANETOCENTRIC /* topocentric -> parallax */
	    || coordSystem == S_EQUATORIAL || coordSystem == S_HORIZONTAL) {
	  ectoeq(final, ceps, seps, 1); /* to equatorial coordinates */
          if (vocal) {
            puts("ASTRON: planetocentric equatorial coordinates:");
            printHBRtoXYZ(final);
          }
	  if (latitude != S_PLANETOCENTRIC) {
	    /* we need to take parallax into account */
	    final[0] = Tsid - longitude - final[0]; /* RA to local hour angle */
            if (vocal) {
              printf("ASTRON: local hour angle\n");
              showradhms(" H = ", final[0]);
            }
	    parallax(final, r_obs_tgt, rcp, rsp);
            if (vocal) {
              puts("ASTRON: topocentric equatorial coordinates (parallax):");
              printHBRtoXYZ(final);
            }
	    if (coordSystem == S_ECLIPTICAL || coordSystem == S_EQUATORIAL)
	      final[0] = Tsid - longitude - final[0]; /* back to RA */
	    if (coordSystem == S_ECLIPTICAL)
	      ectoeq(final, ceps, seps, 0); /* back to ecliptical */
	  }
	  /* we have ecliptical coordinates if S_ECLIPTICAL
	     or equatorial coordinates if S_EQUATORIAL
	     or hour angle - declination - distance if S_HORIZONTAL */
	  if (coordSystem == S_HORIZONTAL
	      && latitude != S_PLANETOCENTRIC) { /* to horizontal coordinates */
	    eqtohor(final, clat, slat, 1);
            if (vocal) {
              puts("ASTRON: horizontal coordinates:");
              printLBRtoXYZ(final);
            }
          }
	}
	final[0] = famod(final[0], TWOPI);
	if ((internalMode & S_XYZ) != 0) {
	  /* back to cartesian coordinates */
          Double pos[3];
          memcpy(pos, final, sizeof(pos));
	  LBRtoXYZ(pos, final);
          if (vocal) {
            puts("ASTRON: back to cartesian coordinates:");
            printXYZtoLBR(final);
          }
	}
      }
      if (internalMode & S_CONJSPREAD) { /* /CONJSPREAD */
	Double r;

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
	memcpy(f, final, 3*sizeof(Double));
	f += 3;
      }
    } /* end of for all objects */
    if (internalMode & S_CONJSPREAD) {
      Double w, final[3];
      w = hypota(3, mean)/nObjects;
      w = sqrt(-26262.45*log(w));
      XYZtoLBR(mean, final);
      memcpy(f, final, 2*sizeof(Double));
      f[2] = w;
      f += 3;
    }
  } /* end of for all dates */

  /* turn variances into standard deviations and radians into degrees */
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
/*-------------------------------------------------------------------*/
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
