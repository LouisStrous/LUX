/* This is file ephem.cc.

Copyright 2013 Louis Strous, Richard Shine
Copyright 2014 Louis Strous

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
// file ephem, solar ephemeris routines
// 10/23/97 - add Rick Bogart's timerep stuff, not fully used yet
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include<stdio.h>
#include<string.h>
#include <math.h>
#include "action.hh"

 typedef double TIME;
 static        int32_t        m[]  =  {31,28,31,30,31,30,31,31,30,31,30,31};
 static        float        rq = 57.2957795;
 static        float        pi = 3.141592654, b, r, d, p;
 static        int32_t        choice;
 //--------------------------------------------------------------------------
 /* this section has a copy of part of Bogart's time routines, we want to be
 able to get UTC time strings from TAI in seconds and the reverse */
 static struct date_time {
    double second;
    double julday;
    double delta;
    int32_t year;
    int32_t month;
    int32_t dofm;
    int32_t dofy;
    int32_t hour;
    int32_t minute;
    int32_t civil;
    int32_t ut_flag;
    char zone[8];
 } dattim;
 double        time_tai,  last_tai = 0.0;
#define JD_EPOCH        (2443144.5)
#define EPOCH_2000_01_01        ( 725760000.0)
#define EPOCH_1601_01_01        (-11865398400.0)
#define EPOCH_1600_01_01        (-11897020800.0)
#define EPOCH_1582_10_15        (-12440217600.0)
#define EPOCH_1581_01_01        (-12495686400.0)
#define SEC_DAY         (86400.0)                             //       1 d
#define SEC_YEAR        (31536000.0)                          //     365 d
#define SEC_BSYR        (31622400.0)                          //     366 d
#define SEC_YEAR4       (126144000.0)                         //    1460 d
#define SEC_4YRS        (126230400.0)                         //    1461 d
#define SEC_GCNT        (3155673600.0)                        //   36524 d
#define SEC_JCNT        (3155760000.0)                        //   36525 d
#define SEC_GR4C        (12622780800.0)                       //  146097 d
#define SEC_JL4C        (12623040000.0)                       //  146100 d
 static int32_t molen[] = {31, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
 static double ut_leap_time[] = {
 -536543999.0,                                                //  1960.01.01
 -457747198.0,                                                //  1962.07.01
 -394588797.0,                                                //  1964.07.01
 -363052796.0,                                                //  1965.07.01
 -331516795.0,                                                //  1966.07.01
 -284083194.0,                                                //  1968.01.01
 -252460793.0,                                                //  1969.01.01
 -220924792.0,                                                //  1970.01.01
 -189388791.0,                                                //  1971.01.01
 -157852790.0,                                                //  1972.01.01
 -142127989.0,                                                //  1972.07.01
 -126230388.0,                                                //  1973.01.01
  -94694387.0,                                                //  1974.01.01
  -63158386.0,                                                //  1975.01.01
  -31622385.0,                                                //  1976.01.01
         16.0,                                                //  1977.01.01
   31536017.0,                                                //  1978.01.01
   63072018.0,                                                //  1979.01.01
   94608019.0,                                                //  1980.01.01
  141868820.0,                                                //  1981.07.01
  173404821.0,                                                //  1982.07.01
  204940822.0,                                                //  1983.07.01
  268099223.0,                                                //  1985.07.01
  347068824.0,                                                //  1988.01.01
  410227225.0,                                                //  1990.01.01
  441763226.0,                                                //  1991.01.01
  489024027.0,                                                //  1992.07.01
  520560028.0,                                                //  1993.07.01
  552096029.0,                                                //  1994.07.01
  599529630.0,                                                //  1996.01.01
  646790431.0,                                                //  1997.07.01
  // the 7/1/98 was deferred to 12/31/98
  // 678326432.0, */                                          /*  1998.07.01
  694224032.0,                                                //  1999.01.01
  709862433.0,                                                //  1999.07.01
  725760034.0,                                                //  2000.01.01
  757382435.0,                                                //  2001.01.01
  788918436.0,                                                //  2002.01.01
  820454437.0,                                                //  2003.01.01
  851990438.0,                                                //  2004.01.01
  883612839.0,                                                //  2005.01.01
  899251240.0,                                                //  2005.07.01
  930787241.0,                                                //  2006.07.01
  962323242.0,                                                //  2007.07.01
  993945643.0,                                                //  2008.07.01
 1025481644.0                                                 //  2009.07.01
 };
 // -------------------------------------------------------------
TIME tai_adjustment (TIME t)
 /* modified from Rick Bogart's code, assumes UTC and make adjustment
 for TAI */
 {
  TIME dt;
  int32_t leapsecs, ct;

  leapsecs = sizeof (ut_leap_time) / sizeof (TIME);
  dt = 0.0;
  if (t >= ut_leap_time[0]) {
    t += 1.0;
    for (ct=0; ct<leapsecs && t>=ut_leap_time[ct]; ct++) {
      t += 1.0;
      dt -= 1.0;
    }
  }
  return dt;
  }
 // -------------------------------------------------------------
void date_from_epoch_time (TIME t) {
  double century, four_year, one_year;
  int32_t year, month, day;

  if (t < EPOCH_1582_10_15) {
                           //  time < 1582.10.15_00:00: use Julian calendar
    year = 1585;
    t -= EPOCH_1581_01_01 + SEC_4YRS;
    while (t < -SEC_JL4C) {
      t += SEC_JL4C;
      year -= 400;
    }
    while (t < -SEC_JCNT) {
      t += SEC_JCNT;
      year -= 100;
    }
    while (t < -SEC_4YRS) {
      t += SEC_4YRS;
      year -= 4;
    }
    one_year = SEC_BSYR;
    while (t < 0.0) {
      t += one_year;
      year -= 1;
      one_year = SEC_YEAR;
    }
  }
  else {
    year = 1600;
    t -= EPOCH_1600_01_01;
    while (t < -SEC_4YRS) {
      t += SEC_4YRS;
      year -= 4;
    }
    one_year = SEC_YEAR;
    while (t < 0.0) {
      t += one_year;
      year -= 1;
      one_year = (year % 4 == 1) ? SEC_BSYR : SEC_YEAR;
    }
  }

  century = SEC_JCNT;
  while (t >= century) {
    t -= century;
    year += 100;
    century = (year % 400) ? SEC_GCNT : SEC_JCNT;
  }
  four_year = (year % 100) ? SEC_4YRS : (year % 400) ? SEC_YEAR4 : SEC_4YRS;
  while (t >= four_year) {
    t -= four_year;
    year += 4;
    four_year = SEC_4YRS;
  }
  one_year = (year % 4) ? SEC_YEAR : (year % 100) ? SEC_BSYR : (year % 400) ?
      SEC_YEAR : SEC_BSYR;
  while (t >= one_year) {
    t -= one_year;
    year += 1;
    one_year = SEC_YEAR;
  }

  dattim.year = year;
  if (year%4 == 0)
    molen[2] = 29;
  if ((year%100 == 0) && (year > 1600) && (year%400 != 0))
    molen[2] = 28;
  month = 1;
  day = t / SEC_DAY;
  while (day >= molen[month]) {
    day -= molen[month];
    t -= SEC_DAY * molen[month];
    month++;
  }
  molen[2] = 28;
  dattim.month = month;
  dattim.dofm = t / SEC_DAY;
  t -= SEC_DAY * dattim.dofm;
  dattim.dofm++;
  dattim.hour = t / 3600.0;
  t -= 3600.0 * dattim.hour;
  dattim.minute = t / 60.0;
  t -= 60.0 * dattim.minute;
  dattim.second = t;
 }
 //--------------------------------------------------------------------------
TIME epoch_time_from_date ()
 {
 /* modified from one used by Rick Bogart to work with day of year rather
 than month, day of month */
  TIME t;
  int32_t        yr1601;

  t = dattim.second + 60.0 * (dattim.minute + 60.0 * (dattim.hour));
  t += SEC_DAY * (dattim.dofy - 1);
  // we don't use the month here
  yr1601 = dattim.year - 1601;
  if (yr1601 < 0) {
    if (dattim.year%4 ==0)
    while (yr1601 < 1) {
      t -= SEC_JL4C;
      yr1601 += 400;
    }
    while (yr1601 > 99) {
      t += SEC_JCNT;
      yr1601 -= 100;
    }
  }
  else {
    while (yr1601 > 399) {
      t += SEC_GR4C;
      yr1601 -= 400;
    }
    while (yr1601 > 99) {
      t += SEC_GCNT;
      yr1601 -= 100;
    }
  }
  while (yr1601 > 3) {
    t += SEC_4YRS;
    yr1601 -= 4;
  }
  while (yr1601 > 0) {
    t += SEC_YEAR;
    yr1601 -= 1;
  }
  t +=  EPOCH_1601_01_01;
  //  Correct for adjustment to Gregorian calendar
  if (t < EPOCH_1582_10_15) t += 10 * SEC_DAY;
  t -= tai_adjustment (t);
  return (t);
 }
 //--------------------------------------------------------------------------
TIME utc_adjustment (TIME t, char const* zone) {
  TIME dt;
  int32_t leapsecs, ct;

  dattim.ut_flag = 0;
  // _raise_case (zone);
  if (!strcmp (zone, "TAI")) {
    dattim.civil = 0;
    return 0.0;
  }
  if (!strcmp (zone, "TDT") || !strcmp (zone, "TT")) {
    dattim.civil = 0;
    return 32.184;
  }
       //  All others civil time, so use universal time coordination offset
  dattim.civil = 1;
  leapsecs = sizeof (ut_leap_time) / sizeof (TIME);
  dt = 0.0;
  for (ct=0; ct<leapsecs; ct++) {
    if (t < (ut_leap_time[ct] - 1.0))
      break;
    dt -= 1.0;
    if (t < ut_leap_time[ct])
      dattim.ut_flag = 1;
 /*
    else
      t -= 1.0;
 */
  }
  // return (dt + zone_adjustment (zone));
  return (dt);
 }
 //--------------------------------------------------------------------------
void sprint_time (char *out, TIME t, char const* zone, int32_t precision)
 {
  char format[64];

  t += utc_adjustment (t, zone);
  date_from_epoch_time (t);
  if (dattim.ut_flag) {
    dattim.second += 1.0;
  }
  if (precision > 0) {
    sprintf (format, "%s%02d.%df_%%s", "%04d.%02d.%02d_%02d:%02d:%",
      precision+3, precision);
    sprintf (out, format, dattim.year, dattim.month, dattim.dofm,
      dattim.hour, dattim.minute, dattim.second, zone);
  }
  else if (precision == 0)
    sprintf (out, "%04d.%02d.%02d_%02d:%02d:%02.0f_%s",
      dattim.year, dattim.month, dattim.dofm,
      dattim.hour, dattim.minute, dattim.second, zone);
 /* Rick's else case eliminated here, not too interesting, it removed the
 seconds from the string */
 }
 //--------------------------------------------------------------------------
int32_t lux_tai_from_date(ArgumentCount narg, int32_t ps[]) // returns TAI
 // call is tai =  tai_from_date(year, doy, hour, minute, second)
 // may want to upgrade to accept strings using Rick's routines
 {
 int32_t result_sym;
 if (int_arg_stat(ps[0], &dattim.year) != 1) return -1;
 if (int_arg_stat(ps[1], &dattim.dofy) != 1) return -1;
 if (int_arg_stat(ps[2], &dattim.hour) != 1) return -1;
 if (int_arg_stat(ps[3], &dattim.minute) != 1) return -1;
 if (double_arg_stat(ps[4], &dattim.second) != 1) return -1;
 result_sym = scalar_scratch(LUX_DOUBLE);
 sym[result_sym].spec.scalar.d = epoch_time_from_date ();
 return result_sym;
 }
 //--------------------------------------------------------------------------
int32_t lux_date_from_tai(ArgumentCount narg, int32_t ps[]) // returns date string
 // call is s =  date_from_tai(tai)
 {
 char        utc[64];                // just to hold times
 int32_t result_sym, prec = 0;
 TIME        t;
 if (double_arg_stat(ps[0], &t) != 1) return -1;
 if (narg > 1) {
 if (int_arg_stat(ps[1], &prec) != 1) return -1;
 }
 sprint_time (utc, t, "UTC", prec);
 result_sym = string_scratch(strlen(utc)); // null added by string_scratch
 strcpy(string_value(result_sym), utc);
 return result_sym;
 }
 //--------------------------------------------------------------------------
int32_t lux_tri_name_from_tai(ArgumentCount narg, int32_t ps[]) // returns date string
 // call is s =  tri_name_tai(tai)
 {
 char        *p;
 int32_t result_sym;
 TIME        t;
 if (double_arg_stat(ps[0], &t) != 1) return -1;
#define TRILENGTH 16
 t += utc_adjustment (t, "UTC");
  date_from_epoch_time (t);
  if (dattim.ut_flag) {
    dattim.second += 1.0;
  }
 result_sym = string_scratch(TRILENGTH); // null added by string_scratch
 p = (char *) sym[result_sym].spec.array.ptr;

 sprintf (p, "tri%04d%02d%02d.%02d00",
      dattim.year, dattim.month, dattim.dofm,
      dattim.hour);

 return result_sym;
 }
 //--------------------------------------------------------------------------
int32_t ephem_setup(int32_t, int32_t []);
int32_t lux_sun_b(ArgumentCount narg, int32_t ps[])// sun_b function
 // returns solar B angle, b = sun_b(day_of_year, year)
{
  choice = 0;
  return ephem_setup(narg,ps);
}
 //--------------------------------------------------------------------------
int32_t lux_sun_r(ArgumentCount narg, int32_t ps[]) // sun_r function
 // returns solar radius, r = sun_r(day_of_year, year)
 {
 choice = 1;
 return        ephem_setup(narg,ps);
 }
 //--------------------------------------------------------------------------
int32_t lux_sun_d(ArgumentCount narg, int32_t ps[]) // sun_d function
 // returns solar d angle, d = sun_d(day_of_year, year)
 {
 choice = 2;
 return        ephem_setup(narg,ps);
 }
 //--------------------------------------------------------------------------
int32_t lux_sun_p(ArgumentCount narg, int32_t ps[]) // sun_p function
 // returns solar P angle, p = sun_p(day_of_year, year)
 {
 choice = 3;
 return        ephem_setup(narg,ps);
 }
 //--------------------------------------------------------------------------
int32_t        execute_error(int32_t), sephem(int32_t, float);
int32_t ephem_setup(ArgumentCount narg, int32_t ps[])
 {
 int32_t        nsym, result_sym, j, nd, n, iy;
 float        day;
 struct        ahead        *h;
 union        types_ptr q1,q3;

 // first arg is the day, can be scalar or array
 nsym= ps[0];
 nsym = lux_float(1, &nsym);
 //switch on the class
 switch (symbol_class(nsym))        {
 case LUX_SCAL_PTR:             //scalar ptr
 result_sym = scalar_scratch(LUX_FLOAT); n=1; q1.i32 = scal_ptr_pointer(nsym).i32;
 q3.i32 = &sym[result_sym].spec.scalar.i32; break;
 case LUX_SCALAR:               //scalar
 result_sym = scalar_scratch(LUX_FLOAT); n=1; q1.i32 = &sym[nsym].spec.scalar.i32;
 q3.i32 = &sym[result_sym].spec.scalar.i32; break;
 case LUX_ARRAY:                //array
 h = (struct ahead *) sym[nsym].spec.array.ptr;
 q1.i32 = (int32_t *) ((char *)h + sizeof(struct ahead));
 nd = h->ndim;
 n = 1; for (j=0;j<nd;j++) n *= h->dims[j];        // # of elements for nsym
 result_sym = array_clone(nsym,LUX_FLOAT);
 h = (struct ahead *) sym[result_sym].spec.array.ptr;
 q3.f = (float *) ((char *)h + sizeof(struct ahead));
 break;
 default:        return execute_error(32);
 }
 // second arg is the year which must be a scalar
 iy = int_arg( ps[1]);
 /* this is important, we really only accept years in the 20th century
 because we want to denote years as either 19xx or xx. The formula
 probably are not really accurate for other centuries anyhow. sephem
 wants the 2 digit form of the year, so we take off a leading 19 here
 and also check for years that we don't want to process */
 if (iy >= 1900) iy = iy -1900;
 // 4/7/99 - patched to accept year 2000 to 2099
 if (iy < 0 || iy > 199 )  return execute_error(117);
 while (n>0) {
 n--;        day = *q1.f++;        sephem( iy, day);
 switch (choice) {
 case 0: *q3.f++ = b; break;
 case 1: *q3.f++ = r; break;
 case 2: *q3.f++ = d; break;
 case 3: *q3.f++ = p; break;
 }
 }
 return        result_sym;
 }
 //--------------------------------------------------------------------------
int32_t admo(int32_t idoy, int32_t iyr, int32_t *idm, int32_t *imy)
 {
 // convert day of year to month and day of month
 int32_t        ndt, i;
 if (iyr%4 == 0) m[1]  =  29; else m[1]  =  28;
 ndt = 0;
 for (i=0;i<12;i++) {ndt = ndt + m[i];if (ndt >= idoy) break;}
 *imy = i+1;  *idm = idoy - ndt + m[i];
 return 1;
 }
 //--------------------------------------------------------------------------
int32_t julian(int32_t iy, int32_t im, int32_t id)
 /*  (j.meeus: astr. formulae for calculators, willmann bell inc. 1982)
 c   parameters in :
 c         iy  -  year no. after 1900
 c         im  -  month no. of the year
 c         id  -  day no. of the month
 c   parameters out :
 c         jd  -  julian day
 */
 {
 double        b, did, a;
 int32_t        ii, iyy, ij;
 iyy  =  1900. + iy;
 if (im <= 2) { iyy = iyy - 1;        im = im+12; }
 a = ((float) iyy)/100.;        b = 2.-a+a/4.;        did = id;
 ii = 365.25 * (float) iyy;        ij = 30.6001* (float) (im+1);
 return        (int32_t) (ii + ij + b + 1720994.5 + did);
 }
 //--------------------------------------------------------------------------
int32_t sephem(int32_t ny, float day)
 {
 /*        returns solar b angle (in radians) and solar radius (in arcsec)
         input is year and day of year (including fraction of day)*/
 /*
   time variable for newcomb's folmulae:
   fraction of julian centuries elapsed since 1900.05 (noon 1st of january)
   =  j.d.2415020.0)
 */
 double        sday, jd, h, hh, ehel, eks, sml, anm, cc, el, sl, san, av, om, ba;
 double        year, eincl, t;
 int32_t        id, im, idoy, iy;
 /* the day is done as a fp value such that the first day is 0 - 0.9999999,
 this means that to use the julian date function, we have to add 1,
 we finally get the f.p. day since 1900.05 */
 idoy = (int32_t) (day+1.0);        iy = ny;        sday = (day- (int32_t) day);
 admo(idoy, iy, &id, &im);
 // printf("iy,im,id = %d %d %d\n",iy,im,id);
 jd = (double) julian(iy,im,id)+0.5;
 h = (jd + sday - 2415020.0)/36525.0;        hh = h * h;
 //printf("sday, jd, h0, h, hh = %f, %10.1f %g %g %g\n",sday, jd, h0, h, hh);
         /* newcomb's formulae. (page 98 explanatory suppl. to the ephemeris)
            mean obliquity of the ecliptic*/
 ehel = 0.4093198 - 2.2703e-4 * h - 2.86e-8 * hh;
                                         // eccentricity of earth's orbit
 eks = 0.01675104 - 0.0000418*h;
                                                 // mean longitude of sun
 sml = 279.6967 + 36000.769*h;
 sml = sml - 360.* floor( sml/360.);
 sml = sml * pi/180.0;
 //printf("sml = %f\n",sml);
                                                 // mean anomaly
 anm = 358.4758+35999.0498*h - 0.00015*hh;        anm = anm*pi/180.0;
                                         // true longitude of sun (sl)
 cc = (1.91946-0.00479*h) * sin(anm) + 0.020 * sin(2*anm);
 cc = cc * pi/180.0;        sl = sml + cc;
                                                         // true anomaly
 //printf("anm, cc = %f %f\n", anm, cc);
 san = anm + cc;        el = sl;
                                         // no correction for aberration
                                         // distance to sun
 //printf("san, eks = %f %f\n",san,eks);
 av = (1-eks*eks)/(1+eks*cos(san));
                                                         // radius of sun
 r = atan(0.0046555/av)*(180./pi)*3600.;
                                                         // in arcseconds
 // need years since 1850 to calculate long. of ascending node
 year = 1900.+ny+(day/365.);
 // longitude of ascending node (page 171 smart)
 om = (73.66666+0.01395833*(year-1850.0)) * (pi/180.);
 // inclination of solar equator on the ecliptic (7.25 degrees ('constant'))
 eincl = 0.12653637;
                         // heliographic latitude of centre of disc
 ba = sin(el-om) * sin(eincl); b = asin(ba) * rq;
 ba = sin(el-om+0.5*pi) * sin(eincl); d = asin(ba) * rq;
                         // position angle of northern rotation pole of sun
 t = atan(-1.0*cos(el)*tan(ehel))+atan(-1.0*cos(el-om)*tan(eincl));
 p = t * rq;                                        // p angle in degrees
 return 1;
 }
