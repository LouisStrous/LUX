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
#include "action.h"
#include "astron.h"
#include "astrodat2.h"
#include "astrodat3.h"
#include "calendar.h"
/* #include "astrodat.h" */
static char rcsid[] __attribute__ ((unused)) =
 "$Id: astron.c,v 4.0 2001/02/07 20:36:57 strous Exp $";

#define extractbits(value, base, bits) ((value >> base) & ((1 << bits) - 1))

#define S_ECLIPTICAL	1
#define S_EQUATORIAL	2
#define S_HORIZONTAL	3
#define S_ELONGATION	4
#define S_COORDS	7
#define S_XYZ		(1<<3)
#define S_FAST		(1<<4)
#define S_DATE		(1<<5)
#define S_TDT		(1<<6)
#define S_ERROR		(1<<7)
#define S_ABBERATION	(1<<8)
#define S_GEOMETRIC	(1<<9)
#define S_QELEMENTS	(1<<10)
#define S_FK5		(1<<11)
#define S_TRUNCATEVSOP  (1<<12)
#define S_CONJSPREAD    (1<<13)
#define S_PLANETOCENTRIC (1<<14)
#define S_KEEPDIMS	(1<<15)
#define S_VOCAL         (1<<16)
#define AUtoJD		((149.59787e9/299792458)/86400)
#define NOBJECTS	9

#define EARTH		3

int	findint(int, int *, int);
void	UTC_to_TAI(double *), TAI_to_UTC(double *);

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

int	nonae[] = {
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
int calendarType[] = {
  CAL_COMMON, CAL_ISLAMIC, CAL_HEBREW, CAL_EGYPTIAN
};

/* 2nd-degree polynomial evaluation */
#define pol2(a0,a1,a2,t) ((a0) + (t)*((a1) + (t)*(a2)))

/* 3rd-degree polynomial evaluation */
#define pol3(a0,a1,a2,a3,t) ((a0) + (t)*((a1) + (t)*((a2) + (t)*(a3))))

/* 4th-degree polynomial evaluation */
#define pol4(a0,a1,a2,a3,a4,t) (a0 + t*(a1 + t*(a2 + t*(a3 + t*a4))))

double JDE(double, int);
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

static int	*extraIDs;
static struct extraInfo {
  int		nterms;
  double	equinox;
  double	absmag;
  char		*comment;
  struct orbitParams {
    double	JDE;
    double	q;
    double	e;
    double	v_factor;
    double	xfac;
    double	yfac;
    double	zfac;
    double	xangle;
    double	yangle;
    double	zangle;
    double	M;
    double	n;
  } *orbits;
} *extraOrbits = NULL;
  
static int	nExtraObj = 0;

static double	extraElements[9];
static char	haveExtraElements;

extern int getAstronError;
extern int fullVSOP;

int idiv(int x, int y)
     /* returns the largest integer n such that x >= y*n */
{
  return (int) floor(((float) x)/y);
}

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
double meanTropicalYear(double JD)
     /* the mean length of the tropical year, in days, as a function of */
     /* Julian Date JD(TDT) [ES12.11-1,Laskar1986]. */
     /* The time from one vernal equinox to the next may vary from this */
     /* mean by several minutes. */
{
  double	T;

  T = (JD - 2451545.0)/36525;
  return pol3(365.2421896698, -6.15359e-6, -7.29e-10, 2.64e-10, T);
}
/*--------------------------------------------------------------------------*/
double meanSynodicMonth(double JD)
     /* the mean length of the synodic month, in days, as a function of */
     /* Julian Date JD(TDT) [ES12.11-2,Chapront-Touz\'e&Chapront1988]. */
     /* Any particular phase cycle may vary from the mean by up to */
     /* seven hours. */
{
  double	T;

  T = (JD - 2451545.0)/36525;
  return pol2(29.5305888531, 2.1621e-7, -3.64e-10, T);
}
/*--------------------------------------------------------------------------*/
int EasterDate(int year, int *month, int *day)
     /* the date of Easter in year y (Julian until 1582, Gregorian after */
     /* 1582) is returned in *month and *day. [ES12.22,Oudin1940; JM4] */
{
  int	c, n, k, i, j, l;

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
double lunarToJD(double k)
{
  double	T;

  k -= 83017;
  T = k/1236.85;
  return 2451550.09765 + 29.530588853*k
    + T*T*(1.337e-4 + T*(-1.5e-7 + 7.3e-10*T));
}
/*--------------------------------------------------------------------------*/
double JDtoLunar(double JD)
{
  double	k, JD2;

  k = floor((JD - 2451550.09765)/29.530588853) + 83017;
  JD2 = lunarToJD(k);
  k += (JD - JD2)/29.530588853;
  return k;
}
/*--------------------------------------------------------------------------*/
int DatetoCJDN(int year, int month, int day, int calendar)
/* Calculated the Chronological Julian Day Number at the given
   calendar date.  "calendar" indicates the calendar that year, month,
   and date are in.  possible values for calendar: CAL_GREGORIAN
   (Gregorian, started 15 October 1582); CAL_ISLAMIC (civil calendar);
   CAL_JULIAN (Julian proleptic calendar). */
{
  static int (*f[])(int, int, int) =
    { NULL, CommontoCJDN, GregoriantoCJDN, IslamictoCJDN,
      JuliantoCJDN, HebrewtoCJDN, EgyptiantoCJDN };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f)
      || !f[calendar])
    return 0;
  
  return f[calendar](year, month, day);
}
/*--------------------------------------------------------------------------*/
double DatetoCJD(int year, int month, double day, int calendar)
/* Calculates the Chronological Julian Date at the given date.
   "calendar" indicates the calendar that year, month, and date are
   in.  possible values for calendar: CAL_GREGORIAN (Gregorian,
   started 15 October 1582); CAL_ISLAMIC (civil calendar); CAL_JULIAN
   (Julian proleptic calendar).  The Chronological Julian Date is
   returned in the same time base as its arguments (TAI, UTC, TT, or
   something else altogether) */
{
  static double (*f[])(int, int, double) =
    { NULL, CommontoCJD, GregoriantoCJD, IslamictoCJD,
      JuliantoCJD, HebrewtoCJD, EgyptiantoCJD };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f)
      || !f[calendar])
    return 0.0;
  
  return f[calendar](year, month, day);
}
/*--------------------------------------------------------------------------*/
void CJDNtoDate(int CJDN, int *year, int *month, int *day, int calendar)
     /* returns the date corresponding to Julian Date JD. */
     /* possible values for calendar (see JulianDate): CAL_GREGORIAN, */
     /* CAL_ISLAMIC, CAL_JULIAN, CAL_HEBREW, CAL_EGYPTIAN, CAL_COMMON */
{
  static void (*f[])(int, int *, int *, int *) =
    { CJDNtoCommon, CJDNtoGregorian, CJDNtoIslamic,
      CJDNtoJulian, CJDNtoHebrew, CJDNtoEgyptian };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f))
    *year = *month = *day = 0;
  else
    f[calendar](CJDN, year, month, day);
}
/*--------------------------------------------------------------------------*/
void CJDtoDate(double CJD, int *year, int *month, double *day, int calendar)
     /* returns the date corresponding to Chronological Julian Day
        CJD.  possible values for calendar (see JulianDate):
        CAL_GREGORIAN, CAL_ISLAMIC, CAL_JULIAN, CAL_HEBREW,
        CAL_EGYPTIAN, CAL_COMMON */
{
  static void (*f[])(double, int *, int *, double *) =
    { CJDtoCommon, CJDtoGregorian, CJDtoIslamic,
      CJDtoJulian, CJDtoHebrew, CJDtoEgyptian };

  if (calendar < 0 || calendar >= sizeof(f)/sizeof(*f)
      || !f[calendar])
    *year = *month = *day = 0;
  else
    f[calendar](CJD, year, month, day);
}
/*--------------------------------------------------------------------------*/
int tishri(int year)
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
  n = idiv(year,19);		/* number of cycles of Meton */
  year -= n*19;			/* year within current cycle */
  leap = nLeap[year];		/* number of leap days in current cycle */

  halakim = 595*n + 876*year - 287*leap + 204;
  j = idiv(halakim,1080);
  hours = j + 16*n + 8*year + 13*leap + 5;
  halakim -= 1080*j;
  j = idiv(hours,24);
  dow = j + 6939*n + 354*year + 29*leap + 1;
  hours -= 24*j;
  weeks = idiv(dow,7);
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
void findTextDate(char *text, int *year, int *month, double *day, int *cal,
		  int order)
/* interpret the <text> as a date string in the calendar indicated by */
/* <*cal> (e.g., CAL_HEBREW).  If <*cal> is CAL_DEFAULT, then the */
/* used calendar is deduced from (the start of) the month name.  <order> */
/* indicates the order of the year and day numbers, it must be either */
/* CAL_YMD or CAL_DMY. The */
/* deduced calendar is returned in <*cal>, and the year, month number, */
/* and day are returned in <*year>, <*month>, and <*day>, respectively. */
{
  int	i, j, nnum = 0, ntext = 0, c, kind;
  char	*daystring = NULL, *monthstring = NULL, *yearstring = NULL, *text0;

  /* first we seek the starts of the day, month, and year.  We assume that */
  /* the day and year are numerical, and the month is in text form. */
  /* If <order> == CAL_DMY, then we assume that the first number is the day */
  /* and the second number is the year; if <order> == CAL_YMD, then */
  /* the first number is the */
  /* year and the second number is the day.  The month string may come */
  /* anywhere before, between, or after the numbers. */
  text0 = text;
  while (isspace((int) *text))	/* skip whitespace */
    text++;
  do {
    if (*text == '-' || *text == '+' || isdigit((int) *text)) {	/* a number */
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
      while (*text && !isspace((int) *text)) /* find next whitespace */
	text++;
      while (*text && isspace((int) *text)) /* skip it */
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
	while (*text && !isdigit((int) *text) && *text != '-' && *text != '+')
	  text++;
	if (!isspace((int) text[-1]))
	  while (*text && !isspace((int) *text))
	    text++;
      } while (*text
	       && (!(isdigit((int) *text) || *text == '+' || *text == '-')
		   || !isspace((int) text[-1])));
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
      while (text > text0 && isspace((int) text[-1]))
	text--;
      c = *text;
      *text = '\0';
    } else if (yearstring > monthstring) {
      text = yearstring;
      while (text > text0 && isspace((int) text[-1]))
	text--;
      c = *text;
      *text = '\0';
    }
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
void roman_numeral(int number)
/* Stores at curScrat the number in Roman numeral form if it is positive and
   less than 4,000, otherwise in regular digital form. */
{
  char	*p;
  int	i;

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
int construct_output_dims(int *input_dims, int num_input_dims,
			  int inputs_per_entry,
			  int *output_dims, int output_dims_size,
			  int outputs_per_entry)
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
  int num_output_dims = 0;

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
int gcd(int a, int b)
{
  int c;

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
int ana_calendar(int narg, int ps[])
{
  int result, input_elem_per_date, output_elem_per_date, iq;
  int *dims, ndim;
  loopInfo tgtinfo, srcinfo;
  pointer src, tgt;
  enum Calendar_order fromorder, toorder;
  enum Calendar fromcalendar, tocalendar;
  enum Calendar_timescale fromtime, totime;
  enum Symboltype inputtype, internaltype, outputtype;
  enum Calendar_outputtype outputkind;
  static struct {
    int to_elements_per_date;   /* translating to calendar date */
    int from_elements_per_date; /* translating from calendar date */
    void (*CJDNtoCal)(int const *CJDN, int *date);
    void (*CJDtoCal)(double const *CJD, double *date);
    void (*CaltoCJDN)(int const *date, int *CJDN);
    void (*CaltoCJD)(double const *date, double *CJD);
    void (*CJDNtoCalS)(int const *CJDN, char **date);
    void (*CJDtoCalS)(double const *CJD, char **date);
    void (*CalStoCJDN)(char * const *date, int *CJDN);
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

  fromcalendar = extractbits(internalMode, CAL_CALENDAR_BASE,
			     CAL_CALENDAR_BITS);
  tocalendar = extractbits(internalMode, CAL_CALENDAR_BASE + CAL_CALENDAR_BITS,
			   CAL_CALENDAR_BITS);
  outputkind = extractbits(internalMode, CAL_OUTPUT_BASE, CAL_OUTPUT_BITS); /* /NUMERIC, /TEXT, /ISOTEXT */
  fromorder = extractbits(internalMode, CAL_ORDER_BASE, CAL_ORDER_BITS); /* /FROMYMD, /FROMDMY */
  toorder = extractbits(internalMode, CAL_ORDER_BASE + CAL_ORDER_BITS,
			CAL_ORDER_BITS); /* /TOYMD, /TODMY */
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

  void (*CaltoCJDN)(int const *date, int *CJDN)
    = cal_data[fromcalendar].CaltoCJDN;
  void (*CaltoCJD)(double const *date, double *CJDN)
    = cal_data[fromcalendar].CaltoCJD;
  void (*CalStoCJDN)(char * const *date, int *CJDN)
    = cal_data[fromcalendar].CalStoCJDN;
  void (*CalStoCJD)(char * const *date, double *CJD)
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

  if (fromcalendar == tocalendar) /* no conversion */
    return *ps;

  void (*CJDNtoCal)(int const *CJDN, int *date)
    = cal_data[tocalendar].CJDNtoCal;
  void (*CJDtoCal)(double const *CJD, double *date)
    = cal_data[tocalendar].CJDtoCal;
  void (*CJDNtoCalS)(int const *CJDN, char **date)
    = cal_data[tocalendar].CJDNtoCalS;
  void (*CJDtoCalS)(double const *CJD, char **date)
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
    if (CaltoCJDN)
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
    if (CaltoCJD)
      internaltype = ANA_DOUBLE;
    else {
      internaltype = ANA_LONG;
      if (input_elem_per_date != 1) {
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
    outputtype = CaltoCJDN? ANA_LONG: ANA_DOUBLE;
  else if (outputkind == CAL_DOUBLE || internaltype == ANA_DOUBLE)
    outputtype = CaltoCJD? ANA_DOUBLE: ANA_LONG;
  else                          /* should not happen */
    outputtype == internaltype;

  {
    int more[1], less[1], nMore, nLess, m, l, q;

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
    int dims[MAX_DIMS];

    assert(srcinfo.dims[0] % input_elem_per_date == 0);
    memcpy(dims, srcinfo.dims, srcinfo.ndim*sizeof(*dims));
    if (srcinfo.ndim == 1) {    /* there is only one dimension */
      dims[1] = 1;              /* add a 2nd dimension */
      srcinfo.ndim = 2;
    }
    int d = srcinfo.dims[0]/input_elem_per_date;
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
        temp.l = (int) floor(*src.d); /* translate from DOUBLE to LONG */
        CaltoCJDN(&temp.l, &timestamp.l); /* use LONG translation */
        src.d += input_elem_per_date;
        break;
      }
      break;
    case ANA_DOUBLE:            /* translate to CJD */
      switch (inputtype) {
      case ANA_LONG: /* only cases with one element per date reach here */
        assert(input_elem_per_date == 1);
        temp.d = (double) *src.l; /* translate from LONG to DOUBLE */
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
        temp.d = (double) timestamp.l; /* translate from LONG to DOUBLE */
        CJDtoCal(&temp.d, tgt.d);      /* use DOUBLE translation */
        tgt.d += output_elem_per_date;
        break;
      case ANA_STRING_ARRAY:
        CJDNtoCalS(&timestamp.l, tgt.sp);
        tgt.sp += output_elem_per_date;
        break;
      }
      break;
    case ANA_DOUBLE:
      switch (outputtype) {
      case ANA_LONG:
        temp.l = (int) floor(timestamp.d); /* translate from DOUBLE to LONG */
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
      }
      break;
    }
  } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
  if (!loopIsAtStart(&tgtinfo))
    return anaerror("Source loop is finished but target loop is not!", ps[0]);

  return result;
}
/*--------------------------------------------------------------------------*/
int ana_calendar_OLD(int narg, int ps[])
     /* general calendar conversion routine */
     /* syntax: DATE2 = CALENDAR(DATE1, /FROMCALENDAR, /TOCALENDAR) */
     /* "/FROMCALENDAR":  /FROMCOMMON /FROMGREGORIAN /FROMISLAMIC */
     /*                   /FROMJULIAN /FROMJD /FROMCJD
                          /FROMUTC /FROMTAI /FROMTT */
     /* "/TOCALENDAR": /TOCOMMON /TOGREGORIAN /TOISLAMIC /TOJULIAN /TOJD */
     /*                /TOCJD /TOMAYAN /TOLONGCOUNT /TOUTC /TOTAI /TOTT */
{
  int	n, *dims, ndim, nRepeat, type, i, iq, cal, newDims[MAX_DIMS],
    num_newDims, year, month, nd, d, t, v, m, t1, t2, time, sec, min, hour,
    fromcalendar, tocalendar, fromtime, totime, output, fromorder, toorder,
    outtype, iday;
  char	isFree = 0, *line, **monthNames;
  pointer	data, JD;
  double	day;

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
    data.l = (int *) array_data(iq);
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
    JD.l = (int *) malloc(nRepeat*sizeof(int));
    if (!JD.l)
      return cerror(ALLOC_ERR, *ps);
    break;
  case ANA_DOUBLE: case ANA_TEMP_STRING:
    JD.d = (double *) malloc(nRepeat*sizeof(double));
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
	int level = 0;
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
      return error("Cannot parse text-based dates in this calendar", *ps);
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
            JD.d[i] = CJDtoJD((double) data.l[i]);
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
          JD.d[i] = lunarToJD((double) data.l[i]);
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
            JD.l[i] = DatetoCJDN((int) data.d[3*i], (int) data.d[3*i + 1],
                                 (int) data.d[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.l[i] = DatetoCJDN((int) data.d[3*i + 2], (int) data.d[3*i + 1],
                                 (int) data.d[3*i], cal);
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
                                (double) data.l[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD(data.l[3*i + 2], data.l[3*i + 1],
                                (double) data.l[3*i], cal);
          break;
        }
        break;
      case ANA_DOUBLE:
        switch (fromorder) {
        case CAL_YMD:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD((int) data.d[3*i], (int) data.d[3*i + 1],
                                data.d[3*i + 2], cal);
          break;
        case CAL_DMY:
          for (i = 0; i < nRepeat; i++)
            JD.d[i] = DatetoCJD((int) data.d[3*i + 2], (int) data.d[3*i + 1],
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
      data.d = (double *) array_data(iq);
    }
    for (i = 0; i < nRepeat; i++) {
      if (outtype == ANA_DOUBLE)
        CJDtoDate(JD.d[i], &year, &month, &day, CAL_COMMON);
      else
        CJDNtoDate(JD.l[i], &year, &month, &iday, CAL_COMMON);
      switch (output) {
      case 999 /* CAL_ISOTEXT */:
        if (outtype == ANA_DOUBLE) {
          sec = (day - (int) day)*86400;
          min = sec/60;
          sec = sec % 60;
          hour = min/60;
          min = min % 60;
          sprintf(line,"%4d-%02d-%02dT%02d:%02d:%02d", year, month,
                  (int) day, hour, min, sec);
        } else
          sprintf(line,"%4d-%02d-%02d", year, month, iday);
        *data.sp++ = strsave(line);
        break;
      case CAL_TEXT:
        switch (toorder) {
        case CAL_YMD:
          if (outtype == ANA_DOUBLE)
            sprintf(line, "%1d %s %1d", year,
                    GregorianMonths[month - 1], (int) day);
          else
            sprintf(line, "%1d %s %1d", year,
                    GregorianMonths[month - 1], iday);
          break;
        case CAL_DMY:
          if (outtype == ANA_DOUBLE)
            sprintf(line, "%1d %s %1d", (int) day,
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
      int	l;

      if (outtype == ANA_DOUBLE) {
        CJDtoDate(JD.d[i], &year, &month, &day, CAL_COMMON);
        iday = (int) day;
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
      data.d = (double *) array_data(iq);
    }
    memcpy(data.d, JD.d, nRepeat*sizeof(double));
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
      data.l = (int *) array_data(iq);
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
      data.d = (double *) array_data(iq);
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
      int n;

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
    data.d = (double *) array_data(iq);
  }
  for (i = 0; i < nRepeat; i++) {
    if (outtype == ANA_DOUBLE) {
      CJDtoDate(JD.d[i], &year, &month, &day, tocalendar);
      iday = (int) day;
    } else
      CJDNtoDate(JD.l[i], &year, &month, &iday, tocalendar);
    switch (output) {
    case 999 /* CAL_ISOTEXT */:
      if (outtype == ANA_DOUBLE) {
        sec = (day - (int) day)*86400;
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
int ana_EasterDate(int narg, int ps[])
     /* returns dates of Easter in the common calendar (Gregorian-Julian). */
     /* syntax:  X = EASTERDATE(YEARS), where YEARS is an array.  X will */
     /* have three elements in its first dimension, in the order year, */
     /* month, day */
{
  int	*year, nYear, iq, *dims, nDim, newDims[MAX_DIMS], *ptr, month, day;

  switch (symbol_class(*ps))
  { case ANA_SCALAR:
      iq = ana_long(1, ps);	/* make ANA_LONG */
      year = &scalar_value(iq).l;
      nYear = 1;  nDim = 0;
      break;
    case ANA_ARRAY:
      iq = ana_long(1, ps);
      year = (int *) array_data(iq);
      nYear = array_size(iq);  dims = array_dims(iq);
      nDim = array_num_dims(iq);
      break;
    default:
      return cerror(ILL_CLASS, *ps); }
  /* create result */
  *newDims = 3;
  if (nDim) memcpy(newDims + 1, dims, nDim);
  iq = array_scratch(ANA_LONG, nDim + 1, newDims);
  ptr = (int *) array_data(iq);
  while (nYear--)
  { if (EasterDate(*year, &month, &day) < 0)
      return -1;		/* some error */
    *ptr++ = *year;  *ptr++ = month;  *ptr++ = day; }
  return iq;
}
/*--------------------------------------------------------------------------*/
double deltaT(double JD)
/* returns the difference between TAI and UTC for a given JD(UTC), in seconds.
   For UTC between 1961-01-01 and 2007-01-01. LS 24sep98 25dec06 */
{
  static double	JDs[] = {
    2437300.5, 2437512.5, 2437665.5, 2438334.5, 2438395.5, 2438486.5,
    2438639.5, 2438761.5, 2438820.5, 2438942.5, 2439004.5, 2439126.5,
    2439887.5, 2441317.5, 2441499.5, 2441683.5, 2442048.5, 2442413.5,
    2442778.5, 2443144.5, 2443509.5, 2443874.5, 2444239.5, 2444786.5,
    2445151.5, 2445516.5, 2446247.5, 2447161.5, 2447892.5, 2448257.5,
    2448804.5, 2449169.5, 2449534.5, 2450083.5, 2450630.5, 2451179.5,
    2453736.5,
  };
  static double	t0s[] = {
    2437300, 2437300, 2437665, 2437665, 2438761, 2438761,
    2438761, 2438761, 2438761, 2438761, 2438761, 2439126,
    2439126, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
  };
  static double	offsets[] = {
    1.4228180, 1.3728180, 1.8457580, 1.9458580, 3.2401300, 3.3401300,
    3.4401300, 3.5401300, 3.6401300, 3.7401300, 3.8401300, 4.3131700,
    4.2131700, 10.0, 11.0, 12.0, 13.0, 14.0,
    15.0, 16.0, 17.0, 18.0, 19.0, 20.0,
    21.0, 22.0, 23.0, 24.0, 25.0, 26.0,
    27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0,
  };
  static double	units[] = {
    0.001296, 0.001296, 0.0011232, 0.0011232, 0.001296, 0.001296,
    0.001296, 0.001296, 0.001296, 0.001296, 0.001296, 0.002592,
    0.002592, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
  };
  static int	n = sizeof(JDs)/sizeof(double);
  double	T, dT;
  int	lo, hi, mid;

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
/*--------------------------------------------------------------------------*/
void UTC_to_TAI(double *JD)
{
  *JD += deltaT(*JD)/86400;
}
/*--------------------------------------------------------------------------*/
void TAI_to_UTC(double *JD)
{
  double	d1;
  
  d1 = *JD - deltaT(*JD)/86400;
  *JD = *JD - deltaT(d1)/86400;
}
/*--------------------------------------------------------------------------*/
double JDE(double JD, int direction)
     /* corrects JD for changes in the rotation rate of the Earth. */
     /* (i.e., takes JD (UT) and returns an approximate JDE (TDT). */
     /* The algorithms are based on observations between years -390 */
     /* and +1986 [ES2.553].  if <direction> = +1, then goes from
      UT to TDT, if <direction> = -1, then goes from TDT to UT. */
{
  double	dT, T;

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
void rotate(double *pos, double angle, int axis)
/* rotate X, Y, and Z (pos[0] through pos[2]) over the specified angle */
/* (in radians) along the specified axis (X = 0, etc.) */
{
  double	p1, p2, c, s;
  static double	se = 0.3977771559, ce = 0.917482062; /* sine and cosine of */
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
void precess(double *pos, double equinox, int forward)
     /* precess the rectangular coordinates from equinox 2000.0 to */
     /* the specified one (JDE) (forward > 0) or backward (forward <= 0) */
     /* ES3.212 */
{
  static double	z_a = 0, theta_a = 0, zeta_a = 0, oldEquinox = J2000;
  double	T;

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
double	p_ce, p_se, p_pi, p_p;
void initEclipticPrecession(double JDE, double equinox)
     /* initialize for precession from equinox <JDE> to equinox <equinox> */
{
  double	t, eta, T;

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
void eclipticPrecession(double *pos, double JDE, double equinox)
/* precess the ecliptical polar coordinates <pos> from the equinox of
   <JDE> (in JDE) to that of <equinox> (in JDE) */
{
  static double	oldJDE = DBL_MAX, oldEquinox = DBL_MAX;
  double	a, b, c, cb, sb, s;
  
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
/*--------------------------------------------------------------------------*/
int ana_precess(int narg, int ps[])
/* PRECESS(coords, JDfrom, JDto) precesses the equatorial coordinates
   <coords> from the equinox of <JDfrom> to <JDto>, both measured in
   Julian Days.  <coords>(0,*) is taken to contain right ascensions
   measured in degrees, and <coords>(1, *) declinations, measured in
   degrees.  LS 2004may03*/
{
  double JDfrom, JDto, alpha, delta;
  pointer src, tgt;
  loopInfo srcinfo, tgtinfo;
  int n, outtype, result, done;

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
      advanceLoop(&srcinfo);
      delta = *src.b*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      advanceLoop(&tgtinfo);
      *tgt.f = delta*RAD;
      do {
	done = advanceLoop(&srcinfo);
	advanceLoop(&tgtinfo);
	if (!done)
	  *tgt.f = *src.b;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case ANA_WORD:
    do {
      alpha = *src.w*DEG;
      advanceLoop(&srcinfo);
      delta = *src.w*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      advanceLoop(&tgtinfo);
      *tgt.f = delta*RAD;
      do {
	done = advanceLoop(&srcinfo);
	advanceLoop(&tgtinfo);
	if (!done)
	  *tgt.f = *src.w;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case ANA_LONG:
    do {
      alpha = *src.l*DEG;
      advanceLoop(&srcinfo);
      delta = *src.l*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      advanceLoop(&tgtinfo);
      *tgt.f = delta*RAD;
      do {
	done = advanceLoop(&srcinfo);
	advanceLoop(&tgtinfo);
	if (!done)
	  *tgt.f = *src.l;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case ANA_FLOAT:
    do {
      alpha = *src.f*DEG;
      advanceLoop(&srcinfo);
      delta = *src.f*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.f = alpha*RAD;
      advanceLoop(&tgtinfo);
      *tgt.f = delta*RAD;
      do {
	done = advanceLoop(&srcinfo);
	advanceLoop(&tgtinfo);
	if (!done)
	  *tgt.f = *src.f;
      } while (!done);
    } while (done < srcinfo.rndim);
    break;
  case ANA_DOUBLE:
    do {
      alpha = *src.d*DEG;
      advanceLoop(&srcinfo);
      delta = *src.d*DEG;
      precessEquatorial(&alpha, &delta, JDfrom, JDto);
      *tgt.d = alpha*RAD;
      advanceLoop(&tgtinfo);
      *tgt.d = delta*RAD;
      do {
	done = advanceLoop(&srcinfo);
	advanceLoop(&tgtinfo);
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
int constellation(double alpha, double delta)
/* returns the identity of the official constellation that the
   position is in that is indicated by right ascension <alpha> and
   declination <delta>, both measured in degrees relative to the
   equinox of B1875.0.  LS 2004may03
 */
{
  int nb = sizeof(constellation_boundaries)/sizeof(struct constellation_struct);
  int i;
  struct constellation_struct *cb = constellation_boundaries;

  if (delta < -90)
    delta = -90;
  else if (delta > 90)
    delta = +90;
  alpha = famod(alpha, 360.0);	/* reduce to interval 0 - 360 degrees */
  alpha /= 15;			/* from degrees to hours */

  for (i = 0; delta < cb[i].delta || alpha >= cb[i].alpha2
	 || alpha < cb[i].alpha1; i++);
  return cb[i].constellation;
}
/*--------------------------------------------------------------------------*/
#define B1875 (2405889.25855)
int ana_constellation(int narg, int ps[])
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
  int n, result, done;
  double equinox, alpha, delta;
  loopInfo srcinfo, tgtinfo;
  pointer src, tgt;
  int vocal;

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
      advanceLoop(&srcinfo);
      delta = *src.b*DEG;
      do
	done = advanceLoop(&srcinfo);
      while (!done);
      if (vocal) {
        printf("CONSTELLATION: (equinox %.10g) %.10g %.10g rad\n", equinox, alpha, delta);
        printf("CONSTELLATION: (equinox %.10g) %.10g %.10g deg\n", equinox, alpha*RAD, delta*RAD);
      }
      precessEquatorial(&alpha, &delta, equinox, B1875);
      if (vocal) {
        printf("CONSTELLATION: (equinox B1875.0) %.10g %.10g rad\n", alpha, delta);
        printf("CONSTELLATION: (equinox B1875.0) %.10g %.10g deg\n", alpha*RAD, delta*RAD);
      }
      *tgt.b = constellation(alpha*RAD, delta*RAD);
      advanceLoop(&tgtinfo);
    } while (done < srcinfo.rndim);
    break;
  case ANA_WORD:
    do {
      alpha = *src.w*DEG;
      advanceLoop(&srcinfo);
      delta = *src.w*DEG;
      do
	done = advanceLoop(&srcinfo);
      while (!done);
      if (vocal) {
        printf("CONSTELLATION: (equinox %.10g) %.10g %.10g rad\n", equinox, alpha, delta);
        printf("CONSTELLATION: (equinox %.10g) %.10g %.10g deg\n", equinox, alpha*RAD, delta*RAD);
      }
      precessEquatorial(&alpha, &delta, equinox, B1875);
      if (vocal) {
        printf("CONSTELLATION: (equinox B1875.0) %.10g %.10g rad\n", alpha, delta);
        printf("CONSTELLATION: (equinox B1875.0) %.10g %.10g deg\n", alpha*RAD, delta*RAD);
      }
      *tgt.b = constellation(alpha*RAD, delta*RAD);
      advanceLoop(&tgtinfo);
    } while (done < srcinfo.rndim);
    break;
  case ANA_LONG:
    do {
      alpha = *src.l*DEG;
      advanceLoop(&srcinfo);
      delta = *src.l*DEG;
      do
	done = advanceLoop(&srcinfo);
      while (!done);
      if (vocal) {
        printf("CONSTELLATION: (equinox %.10g) %.10g %.10g rad\n", equinox, alpha, delta);
        printf("CONSTELLATION: (equinox %.10g) %.10g %.10g deg\n", equinox, alpha*RAD, delta*RAD);
      }
      precessEquatorial(&alpha, &delta, equinox, B1875);
      if (vocal) {
        printf("CONSTELLATION: (equinox B1875.0) %.10g %.10g rad\n", alpha, delta);
        printf("CONSTELLATION: (equinox B1875.0) %.10g %.10g deg\n", alpha*RAD, delta*RAD);
      }
      *tgt.b = constellation(alpha*RAD, delta*RAD);
      advanceLoop(&tgtinfo);
    } while (done < srcinfo.rndim);
    break;
  case ANA_FLOAT:
    do {
      alpha = *src.f*DEG;
      advanceLoop(&srcinfo);
      delta = *src.f*DEG;
      do
	done = advanceLoop(&srcinfo);
      while (!done);
      if (vocal) {
        printf("CONSTELLATION: (equinox %.10g) %.10g %.10g rad\n", equinox, alpha, delta);
        printf("CONSTELLATION: (equinox %.10g) %.10g %.10g deg\n", equinox, alpha*RAD, delta*RAD);
      }
      precessEquatorial(&alpha, &delta, equinox, B1875);
      if (vocal) {
        printf("CONSTELLATION: (equinox B1875.0) %.10g %.10g rad\n", alpha, delta);
        printf("CONSTELLATION: (equinox B1875.0) %.10g %.10g deg\n", alpha*RAD, delta*RAD);
      }
      *tgt.b = constellation(alpha*RAD, delta*RAD);
      advanceLoop(&tgtinfo);
    } while (done < srcinfo.rndim);
    break;
  case ANA_DOUBLE:
    do {
      alpha = *src.d*DEG;
      advanceLoop(&srcinfo);
      delta = *src.d*DEG;
      do
	done = advanceLoop(&srcinfo);
      while (!done);
      if (vocal) {
        printf("CONSTELLATION: (equinox %.10g) %.10g %.10g rad\n", equinox, alpha, delta);
        printf("CONSTELLATION: (equinox %.10g) %.10g %.10g deg\n", equinox, alpha*RAD, delta*RAD);
      }
      precessEquatorial(&alpha, &delta, equinox, B1875);
      if (vocal) {
        printf("CONSTELLATION: (equinox B1875.0) %.10g %.10g rad\n", alpha, delta);
        printf("CONSTELLATION: (equinox B1875.0) %.10g %.10g deg\n", alpha*RAD, delta*RAD);
      }
      *tgt.b = constellation(alpha*RAD, delta*RAD);
      advanceLoop(&tgtinfo);
    } while (done < srcinfo.rndim);
    break;
  }
  return result;
}
/*--------------------------------------------------------------------------*/
int ana_constellationname(int narg, int ps[]) 
/* CONSTELLATIONNAME(<index>) returns the official abbreviation of the
   name of the constellation with the given <index> (may be an array),
   as returned by function CONSTELLATION.  LS 2004may03
 */
{
  int result, n, nc;
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
	*tgt = strdup(constellation_names[(int) *src.f]);
      tgt++;
      src.f++;
    }
    break;
  case ANA_DOUBLE:
    while (n--) {
      if (*src.d < 0 || *src.d >= nc)
	*tgt = strdup("***");
      else
	*tgt = strdup(constellation_names[(int) *src.d]);
      tgt++;
      src.d++;
    }
    break;
  }
  return result;
}
/*--------------------------------------------------------------------------*/
float magnitude(double d, double r, double beta, int objNum)
     /* Estimates the visual magnitude from the square of the distance */
     /* to the object, the square of the distance of the object to the Sun, */
     /* and the phase angle in degrees*/
     /* [ES7, 15] */
{
  double	temp;
  int	i;

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
void nutation(double JDE, double *dPsi, double *cdPsi, double *sdPsi,
	      double *dEps)
/* calculates the nutation in longitude and/or obliquity */
{
  double	d, m, mm, f, o, *amp, angle, T;
  short int	*mul;
  int	i;

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
double obliquity(double JDE, double *dEps)
/* returns the obliquity of the ecliptic: calculates nutation if */
/* dEps == NULL, otherwise adds *dEps. */
{
  double	eps, T;

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
double standardSiderealTime(int JD, double *dPsi, double ceps)
/* returns the sidereal time at longitude zero at 0 UT of the day of which */
/* 12 UT corresponds to Julian Day number <JD>, in radians; mean if */
/* dPsi == NULL, apparent otherwise.  */
{
  double	c, T, jd;

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
double siderealTime(double JD, double *dPsi, double ceps)
/* returns the sidereal time at longitude zero at the indicated JD (UT), */
/* in radians; mean if dPsi == NULL, apparent otherwise */
{
  double	c, T, jd;

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
int ana_siderealtime(int narg, int ps[]) 
/* SIDEREALTIME(<jd>) returns the mean sidereal time at the indicated */
/* julian dates, in hours */
/* LS 31mar2002 */
{
  double *jd, *out, d;
  int n, iq, result;
  double dPsi, cdPsi, sdPsi, dEps, epsilon;

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
    jd = (double *) array_data(iq);
    n = array_size(iq);
    result = array_clone(iq, ANA_DOUBLE);
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
void VSOPtoFK5(double T, double *pos)
/* transforms from VSOP polar coordinates to FK5 polar coordinates */
{
  double	ll, cll, sll;
  
  ll = pos[0] - 0.024382*(1 + 0.000222);
  cll = cos(ll);
  sll = sin(ll);
  pos[0] += -4.3793e-7 + 1.8985e-7*(cll + sll)*tan(pos[1]);
  pos[1] += 1.8985e-7*(cll - sll);
}
/*--------------------------------------------------------------------------*/
#define EQUINOX_OF_DATE	DBL_MAX

int readExtra(char *file, char mode)
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
  int	obj, id, rec, c, format, n, nmore, indx, nterm;
  double	jd, a, e, i, node, peri, m, equinox;
  double	sn, cn, si, ci, f, g, p, q, mag;

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
  for (pp = orbitLine; isspace((byte) *pp); pp++);
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
    extraIDs = (int *) malloc(nExtraObj*sizeof(int));
  } else {			/* merge */
    extraOrbits =
      (struct extraInfo *) realloc(extraOrbits,
				   nExtraObj*sizeof(struct extraInfo));
    extraIDs = (int *) realloc(extraIDs, nExtraObj*sizeof(int));
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
      memmove(extraIDs + indx + 1, extraIDs + indx, nmore*sizeof(int));
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
    extraIDs = (int *) realloc(extraIDs, indx*sizeof(int));
    nExtraObj = indx;
  }

  return 1;
}
/*--------------------------------------------------------------------------*/
int ana_readorbits(int narg, int ps[])
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
  int	i;

  if (nExtraObj) {
    puts("Auxilliary orbits were read for:");
    for (i = 0; i < nExtraObj; i++)
      printf("%d %s\n", extraIDs[i],
	     extraOrbits[i].comment? extraOrbits[i].comment: "(no comment)");
  } else
    puts("No auxilliary orbits have been read.");
}
/*--------------------------------------------------------------------------*/
int ana_showorbits(int narg, int ps[])
{
  showExtra();
  return 1;
}
/*--------------------------------------------------------------------------*/
void kepler(double m, double e, double v_factor, double *v, double *rf)
/* solves Kepler's equation for mean anomaly <m> (in radians) and
 eccentricity <e>.  Returns true anomaly <*v> and radius factor <*rf>.
 <v_factor> must be equal to sqrt(abs((1 + <e>)/(1 - <e>))).  The radius
 will be equal to the perihelion distance times <*rf>.  */
{
  double	E, de, p;

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
/*--------------------------------------------------------------------------*/
int extraHeliocentric(double JDE, int object, double *equinox,
		       double *f, double *r)
/* For <JDE>, return the polar coordinates of <object> in <f>, and the
 equinox for which it was specified in <equinox>. */
{
  int	findint(int, int *, int), i, low, high, mid;
  double	m, e, k, d1, d2, q;
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
  if (getAstronError)			/* no errors known */
    f[3] = f[4] = f[5] = f[6] = f[7] = f[8] = 0.0;
  return 1;
}
/*--------------------------------------------------------------------------*/
void extraElementsHeliocentric(double JDE, double *equinox, double *f,
			       double *r)
{
  static double	xfac, yfac, zfac, xangle, yangle, zangle, epoch, m0, n,
	v_factor, e, a, q, theequinox;
  double m;

  if (haveExtraElements & 4) {	/* new */
    double i, node, peri, absmag, ci, si, cn, sn,
      ff, g, p, qq;
    int ii;

    theequinox = extraElements[0];
    epoch = extraElements[1];
    e = fabs(extraElements[3]);
    i = extraElements[4]*DEG;
    node = extraElements[5]*DEG;
    peri = extraElements[6]*DEG;
    absmag = extraElements[8];
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
	  for (ii = 0; ii < (getAstronError? 9: 3); ii++)
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
  if (getAstronError)			/* no errors known */
    f[3] = f[4] = f[5] = f[6] = f[7] = f[8] = 0.0;
}
/*--------------------------------------------------------------------------*/
void heliocentricXYZr(double JDE, int object, double equinox, double *f,
		      double *r, int vocal)
     /* returns in <f> the cartesian heliocentric eclipic coordinates of
	object <object> for the desired <equinox> at <JDE>, and in
	<r> the heliocentric distance */
{
  void	LBRtoXYZ(double *, double *), XYZtoLBR(double *, double *);
  double	pos[9], T, standardEquinox;
  int	i;

  T = (JDE - J2000)/365250;	/* Julian millennia since J2000.0 */
  switch (object) {
  case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
  case 8:
    LBRfromVSOPD(T, object, pos); /* heliocentric longitude, latitude,
				       and distance referred to the mean
				       dynamical ecliptic and equinox of
				       the date */
    if (vocal) {
      printf("ASTRON: VSOP (%d) ecliptic heliocentric coordinates, equinox/ecliptic of date:\n", object);
      printf("lon = %.10g, lat = %.10g rad, r = %.10g AU\n",
             pos[0], pos[1], pos[2]);
      printf("lon = %.10g, lat = %.10g deg\n",
             pos[0]*RAD, pos[1]*RAD);
      double xyz[3];
      LBRtoXYZ(pos, xyz);
      printf("X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
    }
    *r = pos[2];		/* heliocentric distance */
    if (internalMode & S_FK5) {
      VSOPtoFK5(10*T, pos);	/* to FK5 system (equinox of date)*/
      if (vocal) {
        printf("ASTRON: FK5 (%d) ecliptic heliocentric coordinates, equinox/ecliptic of date:\n", object);
        printf("lon = %.10g, lat = %.10g rad, r = %.10g AU\n",
               pos[0], pos[1], pos[2]);
        printf("lon = %.10g, lat = %.10g deg\n",
               pos[0]*RAD, pos[1]*RAD);
        double xyz[3];
        LBRtoXYZ(pos, xyz);
        printf("X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
      }
    } else if (vocal)
      puts("ASTRON: no transformation to FK5 system");
    if (fabs(JDE - equinox) > 1) { 	/* precession */
	/* we ignore precession when the difference in time between the
	 calculation date and the equinox is less than or equal to 1 day */
      eclipticPrecession(pos, JDE, equinox);
      if (vocal) {
        printf("ASTRON: (%d) ecliptic heliocentric coordinates for equinox:\n", object);
        printf("lon = %.10g, lat = %.10g rad, r = %.10g AU\n",
               pos[0], pos[1], pos[2]);
        printf("lon = %.10g, lat = %.10g deg\n",
               pos[0]*RAD, pos[1]*RAD);
        double xyz[3];
        LBRtoXYZ(pos, xyz);
        printf("X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
      }
    }
    LBRtoXYZ(pos, f);		/* to cartesian coordinates */
    break;
  case 10:			/* the Moon */
    {
      double lmoon, elon, msun, mmoon, nodedist, a1, a2, a3, suml, sumr,
	sumb, E[5], lambda, beta, delta, XYZmoon[3], Tc;
      struct moonlrTerm *mlrt;
      struct moonbTerm *mbt;
      int i;

      /* Tc = T*10 + 4.069459e-10; /* convert from julian millennia to */
      Tc = T*10; /* convert from julian millennia to */
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
	double arg, fac;
	fac = E[mlrt->m + 2];
	arg = mlrt->d*elon + mlrt->m*msun + mlrt->mm*mmoon + mlrt->f*nodedist;
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
      pos[0] = lmoon + suml*DEG/1000000; /* geocentric longitude referred */
					  /* to the mean equinox of the date */
      pos[1] = sumb*DEG/1000000;	/* geocentric latitude */
      pos[2] = (385000.56 + sumr/1000)/149597870; /* AU (center-center) */
      if (vocal) {
        printf("ASTRON: lunar ecliptic geocentric coordinates for equinox of date:\n");
        printf("lon = %.10g, lat = %.10g rad, r = %.10g AU\n",
               pos[0], pos[1], pos[2]);
        printf("lon = %.10g, lat = %.10g deg\n",
               pos[0]*RAD, pos[1]*RAD);
        double xyz[3];
        LBRtoXYZ(pos, xyz);
        printf("X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
      }
      if (fabs(JDE - equinox) > 1) {
	eclipticPrecession(pos, JDE, equinox);
        printf("ASTRON: lunar ecliptic geocentric coordinates for equinox:\n");
        printf("lon = %.10g, lat = %.10g rad, r = %.10g AU\n",
               pos[0], pos[1], pos[2]);
        printf("lon = %.10g, lat = %.10g deg\n",
               pos[0]*RAD, pos[1]*RAD);
        double xyz[3];
        LBRtoXYZ(pos, xyz);
        printf("X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
      }
      LBRtoXYZ(pos, XYZmoon);
      LBRfromVSOPD(T, 3, pos);	/* position of Earth */
      if (internalMode & 2048)
	VSOPtoFK5(10*T, pos);	/* to FK5 system (equinox of date) */
      if (fabs(JDE - equinox) > 1) 	/* precession */
	/* we ignore precession when the difference in time between the
	   calculation date and the equinox is less than or equal to 1 day */
	eclipticPrecession(pos, JDE, equinox);
      if (fabs(JDE - equinox) > 1) {
	eclipticPrecession(pos, JDE, equinox);
        if (vocal) {
          printf("ASTRON: earth ecliptic heliocentric coordinates for equinox:\n");
          printf("lon = %.10g, lat = %.10g rad, r = %.10g AU\n",
                 pos[0], pos[1], pos[2]);
          printf("lon = %.10g, lat = %.10g deg\n",
                 pos[0]*RAD, pos[1]*RAD);
          double xyz[3];
          LBRtoXYZ(pos, xyz);
          printf("X = %.10g, Y = %.10g, Z = %.10g\n", xyz[0], xyz[1], xyz[2]);
        }
      }
      LBRtoXYZ(pos, f);		/* to cartesian coordinates */
      f[0] += XYZmoon[0];
      f[1] += XYZmoon[1];
      f[2] += XYZmoon[2];
      *r = sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
      if (vocal) {
        printf("ASTRON: lunar ecliptic heliocentric coordinates for equinox:\n");
        printf("X = %.10g, Y = %.10g, Z = %.10g AU\n",
               f[0], f[1], f[2]);
        double lbr[3];
        XYZtoLBR(f, lbr);
        printf("lon = %.10g, lat = %.10g rad, r = %.10g AU\n",
               lbr[0], lbr[1], lbr[2]);
        printf("lon = %.10g, lat = %.10g deg\n",
               lbr[0]*RAD, lbr[1]*RAD);
      }
    }
    break;
  case -1:			/* ELEMENTS object - if defined */
    if (!haveExtraElements) {
      anaerror("Illegal object number %1d", 0, object);
      for (i = 0; i < (getAstronError? 9: 3); i++)
	f[i] = 0.0;
      *r = 0.0;
      return;
    }
    extraElementsHeliocentric(JDE, &standardEquinox, pos, r);
    if (standardEquinox == EQUINOX_OF_DATE)
      standardEquinox = JDE;
    if (fabs(standardEquinox - equinox) > 1) { /* precession */
	/* currently circuitous -> inefficient */
      XYZtoLBR(pos, f);
      eclipticPrecession(f, standardEquinox, equinox);
      LBRtoXYZ(f, pos);
    }
    memcpy(f, pos, (getAstronError? 9: 3)*sizeof(double));
    break;
  default:
    if (extraHeliocentric(JDE, object, &standardEquinox, pos, r)
	== ANA_ERROR) {		/* cartesian */
      for (i = 0; i < (getAstronError? 9: 3); i++)
	f[i] = 0.0;
      *r = 0.0;
      return;
    }
    if (standardEquinox == EQUINOX_OF_DATE)
      standardEquinox = JDE;
    if (fabs(standardEquinox - equinox) > 1) { /* precession */
      /* currently circuitous -> inefficient */
      XYZtoLBR(pos, f);
      eclipticPrecession(f, standardEquinox, equinox);
      LBRtoXYZ(f, pos);
    }
    memcpy(f, pos, (getAstronError? 9: 3)*sizeof(double));
    break;
  }
}
/*--------------------------------------------------------------------------*/
static double der[9];
void propagateError(double *der, double *var, double *var2)
/* takes the covariances in <var> and calculates the transformed covariances */
/* according to the partial derivatives in <der>, and stores the results */
/* in <var2>. <var> = [var[x_0],var[x_1],var[x_2],cov[x_0,x_1],cov[x_0,x_2], */
/* cov[x_1,x_2]].  <var2> = similar but for y.  <der(i,j)> = dy_i/dx_j. */
{
  var2[0] = (der[0]*der[0]*var[0] + /* cov_y[00] = d00^2 cov_x[00] + */
	     der[3]*der[3]*var[1] + /* d01^2 cov_x[11] + */
	     der[6]*der[6]*var[2] + /* d02^2 cov_x[22] + */
	     2*(der[0]*der[3]*var[3] + /* 2*(d00 d01 cov_x[01] + */
		der[0]*der[6]*var[4] + /* d00 d02 cov_x[02] + */
		der[3]*der[6]*var[5]));	/* d01 d02 cov_x[12]) */
  var2[1] = (der[1]*der[1]*var[0] + /* cov_y[11] = d10^2 cov_x[00] + */
	     der[4]*der[4]*var[1] + /* d11^2 cov_x[11] + */
	     der[7]*der[7]*var[2] + /* d12^2 cov_x[22] + */
	     2*(der[1]*der[4]*var[3] + /* 2*(d10 d11 cov_x[01] + */
		der[1]*der[7]*var[4] + /* d10 d12 cov_x[02] + */
		der[4]*der[7]*var[5]));	/* d11 d12 cov_x[12]) */
  var2[2] = (der[2]*der[2]*var[0] + /* cov_y[22] = d20^2 cov_x[00] + */
	     der[5]*der[5]*var[1] + /* d21^2 cov_x[11] + */
	     der[8]*der[8]*var[2] + /* d22^2 cov_x[22] + */
	     2*(der[2]*der[5]*var[3] + /* 2*(d20 d21 cov_x[01] + */
		der[2]*der[8]*var[4] + /* d20 d22 cov_x[02] + */
		der[5]*der[8]*var[5]));	/* d21 d22 cov_x[12]) */
  var2[3] = (der[0]*der[1]*var[0] + /* cov_y[01] = d00 d10 cov_x[00] + */
	     der[3]*der[4]*var[1] + /* d01 d11 cov_x[11] + */
	     der[6]*der[7]*var[2] + /* d02 d12 cov_x[22] + */
	     2*(der[0]*der[4]*var[3] + /* 2*(d00 d11 cov_x[01] + */
		der[0]*der[7]*var[4] + /* d00 d12 cov_x[02] + */
		der[3]*der[7]*var[5]));	/* d01 d12 cov_x[12]) */
  var2[4] = (der[0]*der[2]*var[0] + /* cov_y[02] = d00 d20 cov_x[00] + */
	     der[3]*der[5]*var[1] + /* d01 d21 cov_x[11] + */
	     der[6]*der[8]*var[2] + /* d02 d22 cov_x[22] + */
	     2*(der[0]*der[5]*var[3] + /* 2*(d00 d21 cov_x[01] + */
		der[0]*der[8]*var[4] + /* d00 d22 cov_x[02] + */
		der[3]*der[8]*var[5]));	/* d01 d22 cov_x[12]) */
  var2[5] = (der[1]*der[2]*var[0] + /* cov_y[12] = d10 d20 cov_x[00] + */
	     der[4]*der[5]*var[1] + /* d11 d21 cov_x[11] + */
	     der[7]*der[8]*var[2] + /* d12 d22 cov_x[22] + */
	     2*(der[1]*der[5]*var[3] + /* 2*(d10 d21 cov_x[01] + */
		der[1]*der[8]*var[4] + /* d10 d22 cov_x[02] + */
		der[4]*der[8]*var[5]));	/* d11 d22 cov_x[12]) */
}
/*--------------------------------------------------------------------------*/
void LBRtoXYZ(double *pos, double *pos2)
     /* calculates cartesian coordinates XYZ from polar coordinates LBR.
      pos must be unequal to pos2. */
{
  double	cl, cb, sl, sb;

  /* calculate X Y Z from L B R */
  cl = cos(pos[0]);
  cb = cos(pos[1]);
  sl = sin(pos[0]);
  sb = sin(pos[1]);
  pos2[0] = pos[2]*cb*cl;	/* X */
  pos2[1] = pos[2]*cb*sl;	/* Y */
  pos2[2] = pos[2]*sb;		/* Z */
  if (getAstronError)
  { /* calculate vX vY vZ from vL vB vR */
    der[0] = -pos[2]*cb*sl;	/* dX/dL */
    der[1] = pos[2]*cb*cl;	/* dY/dL */
    der[2] = 0.0;		/* dZ/dL */
    der[3] = -pos[2]*sb*cl;	/* dX/dB */
    der[4] = -pos[2]*sb*sl;	/* dY/dB */
    der[5] = pos[2]*cb;		/* dZ/dB */
    der[6] = cb*cl;		/* dX/dR */
    der[7] = cb*sl;		/* dY/dR */
    der[8] = sb;		/* dZ/dR */
    propagateError(der, pos + 3, pos2 + 3); }
}
/*--------------------------------------------------------------------------*/
void XYZtoLBR(double *pos, double *pos2)
/* transform from cartesian to polar coordinates.  pos must be unequal to
 pos2 */
{
  double	r2, r, h2, h;

  h2 = pos[0]*pos[0] + pos[1]*pos[1];
  r2 = h2 + pos[2]*pos[2];
  pos2[2] = r = sqrt(r2);		/* R */
  pos2[0] = h2? atan2(pos[1], pos[0]): 0.0; /* L */
  if (pos2[0] < 0)
    pos2[0] += TWOPI;
  pos2[1] = r? asin(pos[2]/pos2[2]): 0.0;	/* B */
  if (getAstronError) {		/* covariances */
    h = sqrt(h2);
    if (h2) {
      der[0] = -pos[1]/h2;	/* dL/dX */
      der[3] = pos[0]/h2;	/* dL/dY */
      if (r2) {
	der[1] = -pos[0]*pos[2]/(h*r2); /* dB/dX */
	der[4] = -pos[1]*pos[2]/(h*r2); /* dB/dY */
      } else
	der[1] = der[4] = 0.0;
    } else der[0] = der[1] = der[3] = der[4] = 0.0;
    if (r) {
      der[2] = pos[0]/r;	/* dR/dX */
      der[5] = pos[1]/r;	/* dR/dY */
      der[7] = h/r2;		/* dB/dZ */
      der[8] = pos[2]/r;	/* dR/dZ */
    } else der[2] = der[5] = der[7] = der[8] = 0.0;
    der[6] = 0.0;		/* dL/dZ */
    propagateError(der, pos + 3, pos2 + 3);
  }
}
/*--------------------------------------------------------------------------*/
void ectoeq(double *pos, double ceps, double seps, char forward)
/* transforms from ecliptical to equatorial coordinates or vice versa */
{ 
  double	alpha, delta, sl, cl, sb, cb, d[6], ca, sa, cd, sd;

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
  if (getAstronError)
  { ca = cos(pos[0]);
    sa = sin(pos[0]);
    cd = cos(pos[1]);
    sd = sin(pos[1]);
    der[0] = (ceps*cl*ca + sl*sa)*ca; /* da/dl */
    der[1] = cb*cl*seps/cd;	/* dd/dl */
    der[2] = 0.0;		/* dr/dl */
    der[3] = -seps*ca*ca/(cl*cb*cb); /* da/db */
    der[4] = (cb*ceps - sb*seps*sl)/cd;	/* dd/db */
    der[5] = der[6] = der[7] = 0.0; /* dr/db, da/dr, dd/dr */
    der[8] = 1.0;		/* dr/dr */
    propagateError(der, pos + 3, d);
    memcpy(pos + 3, d, 6*sizeof(double)); }
}
/*--------------------------------------------------------------------------*/
void galtoeq(double *pos, double equinox, char forward)
 /* transforms from galactic to equatorial coordinates or vice versa */
{
  double x;

  if (forward) {		/* from galactic to equatorial */
    double A = 123*DEG;
    double B = 27.4*DEG;

    pos[0] = pos[0] - A;
    x = 12.25*DEG + atan2(sin(pos[0]), cos(pos[0])*sin(B) - tan(pos[1])*cos(B));
    pos[1] = asin(sin(pos[1])*sin(B) + cos(pos[1])*cos(B)*cos(pos[0]));
    pos[0] = famod(x, TWOPI);
    precessEquatorial(pos, pos + 1, B1950, equinox);
  } else {			/* from equatorial to galactic */
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
/*--------------------------------------------------------------------------*/
int ana_astrf(int narg, int ps[], int forward) {
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
  int result, from, to;
  double pos[2], ceps, seps, epsilon, equinox;

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
	pos[0] = (double) src.b[0]*DEG;
	pos[1] = (double) src.b[1]*DEG;
	break;
      case ANA_WORD:
	pos[0] = (double) src.w[0]*DEG;
	pos[1] = (double) src.w[1]*DEG;
	break;
      case ANA_LONG:
	pos[0] = (double) src.l[0]*DEG;
	pos[1] = (double) src.l[1]*DEG;
	break;
      case ANA_FLOAT:
	pos[0] = (double) src.f[0]*DEG;
	pos[1] = (double) src.f[1]*DEG;
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
      tgt.f[0] = (float) pos[0]*RAD;
      tgt.f[1] = (float) pos[1]*RAD;
      break;
    case ANA_DOUBLE:
      switch (srcinfo.type) {
      case ANA_BYTE:
	pos[0] = (double) src.b[0]*DEG;
	pos[1] = (double) src.b[1]*DEG;
	break;
      case ANA_WORD:
	pos[0] = (double) src.w[0]*DEG;
	pos[1] = (double) src.w[1]*DEG;
	break;
      case ANA_LONG:
	pos[0] = (double) src.l[0]*DEG;
	pos[1] = (double) src.l[1]*DEG;
	break;
      case ANA_FLOAT:
	pos[0] = (double) src.f[0]*DEG;
	pos[1] = (double) src.f[1]*DEG;
	break;
      case ANA_DOUBLE:
	pos[0] = src.d[0]*DEG;
	pos[1] = src.d[1]*DEG;
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
    }
    src.b += srcinfo.rdims[0]*srcinfo.stride;
    tgt.b += tgtinfo.rdims[0]*tgtinfo.stride;
  } while (advanceLoop(&srcinfo) < srcinfo.rndim);
  return result;
}
/*--------------------------------------------------------------------------*/
#define s_parallax (4.263451e-5)
void parallax(double *pos, double r0, double rcp, double rsp)
/* corrects equatorial planetocentric coordinates for parallax, i.e. */
/* transforms to equatorial topocentric coordinates.  Tsid is the */
/* local sidereal time, r0 is the apparent planetocentric distance, */
/* rcp and rsp indicate the topocentric coordinates of the observer */
{
  double	r, cd, ch, sd, sh, u, d1, d2, A, B, C, q;

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
  if (getAstronError)
  { u = sqrt(u);
    d1 = r*u*u;
    d2 = u*r*q;
    der[0] = cd*(r*cd - ch*rcp*s_parallax)/d1; /* da'/da */
    der[1] = -A*C*rcp*s_parallax/d2; /* dd'/da */
    der[2] = 0.0;		/* dr'/da */
    der[3] = sd*sh*rcp*s_parallax/d1; /* da'/dd */
				/* dd'/dd */
    der[4] = (r*q*q*cd + C*s_parallax*(rsp*cd - rcp*sd*ch))/d2;
    der[5] = 0.0;		/* dr'/dd */
    der[6] = cd*sh*rcp*s_parallax/d1; /* da'/dr */
				/* dd'/dr */
    der[7] = s_parallax*(rsp - C*(B*rcp + C*rsp))/(r*r*u);
    der[8] = 1.0;		/* dr'/dr */
    propagateError(der, pos + 3, pos + 3); }
}
/*--------------------------------------------------------------------------*/
void refract(double *pos, double height)
/* corrects height above the horizon (pos[1]) for average 
   refraction - but only if the refracted height is nonnegative */
{
  double	R, h;

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
void eqtohor(double *pos, double cphi, double sphi, char forward)
/* transforms from equatorial to horizontal coordinates, or vice versa */
{
  double	A, h, sH, cH, sd, cd, sa, ca, ch;

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
  if (getAstronError)
  { sa = sin(pos[0]);
    ca = cos(pos[0]);
    ch = cos(pos[1]);
    der[0] = sa*ca*sH/cH + sa*sa*sphi; /* dA/dH */
    der[1] = -cd*cphi*sH/ch;	/* dh/dH */
    der[2] = 0.0;		/* dr/dH */
    der[3] = cphi*sa*sa/(sH*cd*cd); /* dA/dd */
    der[4] = (-cH*cphi*sd + cd*sphi)/ch; /* dh/dd */
    der[5] = 0.0;		/* dr/dd; */
    der[6] = 0.0;		/* dA/dr */
    der[7] = 0.0;		/* dh/dr */
    der[8] = 1.0;		/* dr/dr */
    propagateError(der, pos + 3, pos + 3); }
}
/*--------------------------------------------------------------------------*/
#define FLAT	0.99664719
#define R_EARTH	6378140
void geocentricCoords(double latitude, double height, double *rcp, double *rsp)
/* returns the geocentric quantities rho cos phi and rho sin phi */
{ 
  double	u;

  u = atan(FLAT*tan(latitude));
  *rcp = cos(u) + height*cos(latitude)/R_EARTH;
  *rsp = FLAT*sin(u) + height*sin(latitude)/R_EARTH;
}
/*--------------------------------------------------------------------------*/
double meanDistance(int obj1, int obj2)
{
  if (obj1 < 0 || obj1 > 8 || obj2 < 0 || obj2 > 8)
    return 0.0;
  else
    return meanDistances[obj1][obj2];
}
/*--------------------------------------------------------------------------*/
int ana_astropos(int narg, int ps[])
     /* returns the positions of a set of heavenly bodies at a specific */
     /* set of times, for equinox 2000.0 */
     /* Syntax: X = ASTRON(JDs, OBJECTS [, OBJECT0, OBSERVER=OBSERVER, */
     /*                    EQUINOX=EQUINOX, ELEMENTS=elements] */
     /*			   [, /XYZ, /EQUATORIAL, /ECLIPTICAL, /ELONGATION, */
     /*			    /HORIZONTAL, /DATE, /ERROR, /ABBERATION, */
     /*                     /GEOMETRICAL, /TDT, /QELEMENTS, /FK5 ]) */
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
  int	iq, nJD, nJDdims, *JDdims, *object, nObjects, object0, dims[MAX_DIMS],
    nDims, i, j, n, result, coordSystem, count, vocal;
  double	*JD, *f, *f0, jd0, djd, djd0, r, r0, dPsi,
    dEps, epsilon, ceps, seps, longitude, latitude, height = 0.0, rsp,
    rcp, clat, slat, Tsid, equinox, robject, robject0, jd, cdPsi, sdPsi;
  double	pos[9], pos2[9], pos3[9], mean[3];

  if (internalMode & S_CONJSPREAD) 	/* /CONJSPREAD */
    internalMode = (internalMode & ~(S_ERROR)) | S_XYZ
      | S_ECLIPTICAL;
  coordSystem = (internalMode & S_COORDS); /* desired coordinate system */
  if (!coordSystem)
    coordSystem = S_ECLIPTICAL;	/* default: ecliptical */
  fullVSOP = !(internalMode & S_TRUNCATEVSOP);
  vocal = (internalMode & S_VOCAL); /* print intermediate results */

  iq = *ps;			/* JDs */
  if (!iq)
    return cerror(ILL_ARG, iq);
  switch (symbol_class(iq)) {
    case ANA_SCALAR:
      iq = ana_double(1, &iq);
      JD = &scalar_value(iq).d;
      nJD = 1;
      nJDdims = 1;
      JDdims = &nJD;
      break;
    case ANA_ARRAY:
      iq = ana_double(1, &iq);
      JD = (double *) array_data(iq);
      nJD = array_size(iq);
      nJDdims = array_num_dims(iq);
      JDdims = array_dims(iq);
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
      object = (int *) array_data(iq);
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
    f = (double *) array_data(iq);
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
    memcpy(extraElements, array_data(j), 9*sizeof(double));
    haveExtraElements = (internalMode & S_QELEMENTS)? 6: 5; /* 5->A, 6->Q */
  } else haveExtraElements = 0;		/* none */

  				/* create result array */
  getAstronError = (internalMode & S_ERROR)? 1: 0;
  dims[0] = getAstronError? 6: 3;	/* include error bars if requested */
  nDims = 1;
  if ((nObjects > 1 || internalMode & S_KEEPDIMS)
      && (internalMode & S_CONJSPREAD) == 0)
    dims[nDims++] = nObjects;
  if (nJD > 1 || internalMode & S_KEEPDIMS)
    dims[nDims++] = nJD;
  result = array_scratch(ANA_DOUBLE, nDims, dims);
  f = f0 = (double *) array_data(result);
  
  tdt = internalMode & S_TDT;	/* time is specified in TDT rather than UT */
  if ((internalMode & S_GEOMETRIC) && (internalMode & S_ABBERATION)) {
    /* puts("ASTRON - ignoring /ABBERATION because of /GEOMETRIC"); */
    internalMode = internalMode & ~S_ABBERATION;
  }

  /* calculate coordinates */
  for (j = 0; j < nJD; j++) {	/* all dates */
    if (vocal)
      printf("ASTRON: calculating for JD = %.7f\n", JD[j]);
    jd = tdt? JD[j]: JDE(JD[j], +1); /* calculate date in TDT */
    if (vocal) {
      if (tdt) {
        puts("ASTRON: JD is in TDT already");
      } else {
        printf("ASTRON: UT => TDT:      JD = %.7f\n", jd);
        printf("ASTRON: delta T = %.10g s\n", (jd - JD[j])*86400);
      }
    }
    jd0 = jd;
    if (internalMode & S_ABBERATION) { /* nutation and abberation */
      nutation(jd0, &dPsi, &cdPsi, &sdPsi, &dEps); /* nutation */
    } else {
      dPsi = sdPsi = dEps = 0.0;	/* ignore abberation */
      cdPsi = 1.0;
    }
    if (vocal) {
      printf("ASTRON: nutation/abberation constants:\n");
      printf(" dPsi = %.10g, dEps = %.10g rad\n", dPsi, dEps);
      printf(" dPsi = %.10g, dEps = %.10g deg\n", dPsi*RAD, dEps*RAD);
    }
    epsilon = obliquity(jd0, &dEps); /* obliquity of the ecliptic corrected */
				    /* for nutation */
    if (vocal) {
      printf("ASTRON: obliquity of ecliptic corrected for nutation: %.10g rad\n",
             epsilon);
      printf("ASTRON: obliquity of ecliptic corrected for nutation: %.10g deg\n",
             epsilon*RAD);
    }
    ceps = cos(epsilon);
    seps = sin(epsilon);
    Tsid = siderealTime(JDE(jd, -1), &dPsi, ceps); /* apparent sidereal time */
				/* at longitude zero (corrected for */
				/* nutation), at UT time, in radians */
    if (vocal) {
      printf("ASTRON: apparent sidereal time (0 longitude, nutation) = %.10g rad\n", Tsid);
      printf("ASTRON: apparent sidereal time (0 longitude, nutation) = %.10g deg\n", Tsid*RAD);
    }
    if (internalMode & S_DATE)
      equinox = jd;
    if (vocal)
      printf("ASTRON: equinox:        JD = %.7f\n", equinox);

    if (internalMode & S_CONJSPREAD)
      mean[0] = mean[1] = mean[2] = 0;
    for (i = 0; i < nObjects; i++) { /* all objects */
      if (internalMode & S_GEOMETRIC) { /* geometric position: no correction */
				        /* for light-time */
	djd = r = 0.0;
	djd0 = 1.0;
      } else {			/* apparent */
	/* we include the effects of light-time and abberation due to the
	   observer's motion by combining the positions of the object and
	   the observer at time t - dt where dt is the light-time correction
	   that fits the distance between the object at time t - dt and
	   the observer at time t (NOT t - dt) */
	/* get initial guess of "average" distance between the objects */
	/* for an initial guess of the light time */
	r = meanDistance(object[i],object0 >= 0? object0: 0);
	djd = r*AUtoJD;		/* initial estimate of light time */
	djd0 = djd + 1;		/* old estimate (should here be different */
				/* from dT to get iteration going) */
	r0 = 0.0; 		/* initial estimate of geometrical distance */
      }	/* end of if (internalMode & S_GEOMETRIC) else */
      /* first get position of observer: it is assumed constant during the
	 calculation of the light time */
      heliocentricXYZr(jd0, object0, equinox, pos3, &robject0, vocal);
      /* cartesian ecliptic heliocentric coordinates of the observer: */
      /* pos3[0] = X, pos3[1] = Y, pos3[2] = Z, */
      /* robject0 = heliocentric distance of the observer */
      if (vocal) {
        printf("ASTRON: observer (%d) geometric ecliptic heliocentric coordinates for equinox:\n", object0);
        printf("X = %.10g, Y = %.10g, Z = %.10g, r = %.10g AU\n",
               pos3[0], pos3[1], pos3[2], robject0);
        double pol[3];
        XYZtoLBR(pos3, pol);
        printf("lon = %.10g, lat = %.10g rad\n", pol[0], pol[1]);
        pol[0] *= RAD;
        pol[1] *= RAD;
        printf("lon = %.10g, lat = %.10g deg\n", pol[0], pol[1]);
      }

      /* now we converge upon the light-time to the target object */
      count = 0;
      while (fabs(djd - djd0) > DBL_EPSILON*1000 && count < 25) {
			/* no convergence yet */ 
	djd0 = djd;		/* old estimate is previous estimate */
	jd = jd0 - djd;		/* time corrected for light time */
	heliocentricXYZr(jd, object[i], equinox, pos2, &robject, vocal);
	/* pos2 = cartesian ecliptic heliocentric coordinates of the target, */
	/* robject = heliocentric distance of the target */
	pos[0] = pos2[0] - pos3[0]; /* dX/AU */
	pos[1] = pos2[1] - pos3[1]; /* dY/AU */
	pos[2] = pos2[2] - pos3[2]; /* dZ/AU */
				/* apparent distance */
	r0 = sqrt(pos[0]*pos[0] + pos[1]*pos[1] + pos[2]*pos[2]);
	if (internalMode & S_GEOMETRIC) /* no light-time */
	  break;
        if (vocal && !count) {
          printf("ASTRON: target (%d) geometric ecliptic heliocentric coordinates for equinox:\n", object[i]);
          printf("X = %.10g, Y = %.10g, Z = %.10g, r = %.10g AU\n",
                 pos2[0], pos2[1], pos2[2], robject);
          double pol[3];
          XYZtoLBR(pos2, pol);
          printf("lon = %.10g, lat = %.10g rad\n", pol[0], pol[1]);
          pol[0] *= RAD;
          pol[1] *= RAD;
          printf("lon = %.10g, lat = %.10g deg\n", pol[0], pol[1]);
        }
	djd = r0*AUtoJD;	/* new estimate of light time */
        if (vocal)
          printf("light-time = %.10g min\n", djd*24*60);
	count++;
      }	/* end of while */
      /* now pos3 = cartesian ecliptic heliocentric observer object coordinates
	 pos2 = apparent cartesian ecliptic heliocentric target object coordinates
	 pos = apparent cartesian ecliptic planetocentric target object coordinates */
      if (vocal) {
        if (internalMode & S_GEOMETRIC)
          puts("ASTRON: No correction for light time: geometric coordinates");
        else
          puts("ASTRON: Corrected for light time: apparent coordinates");
        printf("ASTRON: target (%d) ecliptic heliocentric coordinates for equinox:\n", object[i]);
        printf("X = %.10g, Y = %.10g, Z = %.10g, r = %.10g AU\n",
               pos2[0], pos2[1], pos2[2], robject);
        double pol[3];
        XYZtoLBR(pos2, pol);
        printf("lon = %.10g, lat = %.10g rad\n", pol[0], pol[1]);
        pol[0] *= RAD;
        pol[1] *= RAD;
        printf("lon = %.10g, lat = %.10g deg\n", pol[0], pol[1]);
        printf("ASTRON: target (%d) ecliptic planetocentric coordinates for equinox:\n", object[i]);
        printf("X = %.10g, Y = %.10g, Z = %.10g, r = %.10g AU\n",
               pos[0], pos[1], pos[2], r0);
        XYZtoLBR(pos, pol);
        printf("lon = %.10g, lat = %.10g rad\n", pol[0], pol[1]);
        pol[0] *= RAD;
        pol[1] *= RAD;
        printf("lon = %.10g, lat = %.10g deg\n", pol[0], pol[1]);
      }

      /* now have the planetocentric apparent position corrected for
	 light-time (if not S_GEOMETRIC) but not yet for abberation. */
      if (internalMode & S_ABBERATION) {
	/* correct for abberation */
	heliocentricXYZr(jd0 - djd, object0, equinox, pos3, &r, vocal);
	pos[0] = pos2[0] - pos3[0];
	pos[1] = pos2[1] - pos3[1];
	pos[2] = pos2[2] - pos3[2];
	/* pos[i] now contain cartesian coordinates that point in the
	 right direction but do not indicate the right distance,
	 which is r0.  correct. */
        r = r0? r0/sqrt(pos[0]*pos[0] + pos[1]*pos[1] + pos[2]*pos[2]): 0;
        pos[0] *= r;
	pos[1] *= r;
	pos[2] *= r;
        if (vocal) {
          puts("ASTRON: target ecliptic planetocentric coordinates corrected for abberation:");
          printf("X = %.10g, Y = %.10g, Z = %.10g, r = %.10g AU\n",
                 pos[0], pos[1], pos[2], r0);
          double pol[3];
          XYZtoLBR(pos, pol);
          printf("lon = %.10g, lat = %.10g rad\n", pol[0], pol[1]);
          pol[0] *= RAD;
          pol[1] *= RAD;
          printf("lon = %.10g, lat = %.10g deg\n", pol[0], pol[1]);
        }
      } else if (vocal) {
        puts("ASTRON: no correction for abberation");
      }
      if (getAstronError)
	for (n = 3; n < 9; n++) /* update covariance matrix */
	  pos[n] = r*(pos2[n] + pos3[n]); /* covariances add */

      /* now have (in pos[]) the planetocentric ecliptic cartesian */
      /* coordinates referred to the desired equinox, FK5 */
      
      /* apply nutation */
      pos2[0] = pos[0]*cdPsi - pos[1]*sdPsi;
      pos2[1] = pos[0]*sdPsi + pos[1]*cdPsi;
      pos2[2] = pos[2];
      if (getAstronError)
	memcpy(pos2 + 3, pos + 3, 6*sizeof(double));
      if (vocal) {
        puts("ASTRON: ecliptic planetocentric coordinates corrected for nutation:");
        printf("X = %.10g, Y = %.10g, Z = %.10g, r = %.10g AU\n",
               pos2[0], pos2[1], pos2[2], r0);        
        double pol[3];
        XYZtoLBR(pos2, pol);
        printf("lon = %.10g, lat = %.10g rad\n", pol[0], pol[1]);
        pol[0] *= RAD;
        pol[1] *= RAD;
        printf("lon = %.10g, lat = %.10g deg\n", pol[0], pol[1]);
      }

      if (coordSystem == S_ELONGATION) { /* elongation, phase angle, magn */
	pos2[0] = (robject0*robject0 + r0*r0 - robject*robject)/
	  (2*robject0*r0);
        if (pos2[0] > 1)	/* ignore round-off errors */
	  pos2[0] = 1;
	else if (pos2[0] < -1)	/* ignore round-off errors */
	  pos2[0] = -1;
	pos2[0] = acos(pos2[0]); /* elongation */
	pos2[1] = (robject*robject + r0*r0 - robject0*robject0)/
	  (2*robject*r0);
        if (pos2[1] > 1)
	  pos2[1] = 1;
	else if (pos2[1] < -1)
	  pos2[1] = -1;
	pos2[1] = acos(pos2[1]); /* phase angle */
	pos2[2] = magnitude(r0, robject, pos2[1]*RAD, object[i]);
	if (getAstronError)		/* not yet implemented */
	  pos2[3] = pos2[4] = pos2[5] = pos2[6] = pos2[7] = pos2[8] = 0.0;
        if (vocal) {
          puts("ASTRON: transform to elongation, phase angle, magnitude");
          printf("el = %.10g rad, ph = %.10g rad, mag = %.10\n",
                 pos2[0], pos2[1], pos2[2]);        
        }
      } else if ((internalMode & S_XYZ) == 0 || latitude != S_PLANETOCENTRIC) {
	XYZtoLBR(pos2, pos);	/* to polar coordinates */
	memcpy(pos2, pos, (getAstronError? 9: 3)*sizeof(double));
        if (vocal) {
          puts("ASTRON: ecliptic planetocentric coordinates:");
          printf("lon = %.10g rad, lat = %.10g rad, r = %.10g AU\n",
                 pos2[0], pos2[1], pos2[2]);        
          printf("lon = %.10g, lat = %.10g deg\n",
                 pos2[0]*RAD, pos2[1]*RAD);
        }
	if (latitude != S_PLANETOCENTRIC /* topocentric -> parallax */
	    || coordSystem == S_EQUATORIAL || coordSystem == S_HORIZONTAL) {
	  ectoeq(pos2, ceps, seps, 1); /* to equatorial coordinates */
          if (vocal) {
            puts("ASTRON: equatorial planetocentric coordinates:");
            printf("lon = %.10g rad, lat = %.10g rad, r = %.10g AU\n",
                   pos2[0], pos2[1], pos2[2]);        
            printf("lon = %.10g, lat = %.10g deg\n",
                   pos2[0]*RAD, pos2[1]*RAD);
          }
	  if (latitude != S_PLANETOCENTRIC) {
	    /* we need to take parallax into account */
	    pos2[0] = Tsid - longitude - pos2[0]; /* RA to local hour angle */
            if (vocal) {
              printf("ASTRON: local hour angle = %.10g rad\n", pos2[0]);
              printf("ASTRON: local hour angle = %.10g deg\n", pos2[0]*RAD);
            }
	    parallax(pos2, r0, rcp, rsp);
            if (vocal) {
              puts("ASTRON: equatorial topocentric coordinates (parallax):");
              printf("lon = %.10g rad, lat = %.10g rad, r = %.10g AU\n",
                     pos2[0], pos2[1], pos2[2]);        
              printf("lon = %.10g, lat = %.10g deg\n",
                     pos2[0]*RAD, pos2[1]*RAD);
            }
	    if (coordSystem == S_ECLIPTICAL || coordSystem == S_EQUATORIAL)
	      pos2[0] = Tsid - longitude - pos2[0]; /* back to RA */
	    if (coordSystem == S_ECLIPTICAL)
	      ectoeq(pos2, ceps, seps, 0); /* back to ecliptical */
	  }
	  /* we have ecliptical coordinates if S_ECLIPTICAL
	     or equatorial coordinates if S_EQUATORIAL
	     or hour angle - declination - distance if S_HORIZONTAL */
	  if (coordSystem == S_HORIZONTAL
	      && latitude != S_PLANETOCENTRIC) { /* to horizontal coordinates */
	    eqtohor(pos2, clat, slat, 1);
            if (vocal) {
              puts("ASTRON: horizontal coordinates:");
              printf("az = %.10g rad, el = %.10g rad, r = %.10g AU\n",
                     pos2[0], pos2[1], pos2[2]);  
              printf("az = %.10g, el = %.10g deg\n",
                     pos2[0]*RAD, pos2[1]*RAD);
            }
          }
	}
	pos2[0] = famod(pos2[0], TWOPI);
	if ((internalMode & S_XYZ) != 0) {
	  /* back to cartesian coordinates */
	  LBRtoXYZ(pos2, pos);
	  memcpy(pos2, pos, (getAstronError? 9: 3)*sizeof(double));
          if (vocal) {
            puts("ASTRON: back to cartesian coordinates:");
            printf("X = %.10g, Y = %.10g, Z = %.10g AU\n",
                   pos2[0], pos2[1], pos2[2]);  
          }
	}
      }
      if (internalMode & S_CONJSPREAD) { /* /CONJSPREAD */
	double r;

	r = sqrt(pos2[0]*pos2[0] + pos2[1]*pos2[1] + pos2[2]*pos2[2]);
	if (r) {
	  pos2[0] /= r;
	  pos2[1] /= r;
	  pos2[2] /= r;
	}
	mean[0] += pos2[0];
	mean[1] += pos2[1];
	mean[2] += pos2[2];
	/* TODO: errors */
      } else {
	memcpy(f, pos2, dims[0]*sizeof(double));
	f += dims[0];
      }
    } /* end of for all objects */
    if (internalMode & S_CONJSPREAD) {
      double w;
      w = sqrt(mean[0]*mean[0] + mean[1]*mean[1] + mean[2]*mean[2])/nObjects;
      w = sqrt(-26262.45*log(w));
      XYZtoLBR(mean, pos);
      memcpy(f, pos, 2*sizeof(double));
      f[2] = w;
      if (getAstronError)
	f[3] = f[4] = f[5] = 0;
      f += dims[0];
    }
  } /* end of for all dates */

  /* turn variances into standard deviations and radians into degrees */
  f = f0;
  if (getAstronError) {
    if (internalMode & S_XYZ)	/* XYZ */
      for (i = 0; i < nJD*nObjects; i++) {
	f[3] = sqrt(f[3]);
	f[4] = sqrt(f[4]);
	f[5] = sqrt(f[5]);
	f += 6;
      }
    else
      for (i = 0; i < nJD*nObjects; i++) {
	f[0] *= RAD;
	f[1] *= RAD;
	f[3] = sqrt(f[3])*RAD;
	f[4] = sqrt(f[4])*RAD;
	f[5] = sqrt(f[5]);
	f += 6;
      }
  } else if ((internalMode & S_XYZ) == 0)
    for (i = 0; i < nJD*nObjects; i++) {
      if (vocal) {
        printf("ASTRON: %.10g rad => %.10g deg\n", f[0], f[0]*RAD);
        printf("ASTRON: %.10g rad => %.10g deg\n", f[1], f[1]*RAD);
      }
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

