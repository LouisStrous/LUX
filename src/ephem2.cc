/* This is file ephem2.cc.

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
// File ephem2.c
// Ephemerides having to do with the appearance of the Sun as seen from
// Earth.  LS 1997
#include <math.h>
#include "luxdefs.hh"
#include "action.hh"

// 2nd-degree polynomial evaluation
#define pol2(a0,a1,a2,t) (a0 + t*(a1 + t*a2))

// 3rd-degree polynomial evaluation
#define pol3(a0,a1,a2,a3,t) (a0 + t*(a1 + t*(a2 + t*a3)))

// 2nd-degree polynomial evaluation (modulo)
#define mpol2(a0,a1,a2,t,n) (fmod(a0 + t*(a1 + t*a2),n))

// 3rd-degree polynomial evaluation (modulo)
#define mpol3(a0,a1,a2,a3,t,n) (fmod(a0 + t*(a1 + t*(a2 + t*a3)),n))

static double        solar_stuff;
void solar_physical(double jd, int32_t select)
// from J. Meeus: Astronomical Algorithms, Willman-Bell, ISBN 0-943396-35-2,
// chapter 28
// /MODERN (internalMode & 2): corrections from P. Giles (Ph.D. thesis,
// Stanford University)
{
  double        l, m, e, c, T, o, l2, eps, x, y, k, i, theta, r, l0, theta0;
  double        JDE(double);

  i = ((internalMode & 2)? 7.16: 7.25)*DEG;
  k = (((internalMode & 2)? 73.49: 73 + 4.0/6) + 1.3958333*(jd - 2396758.0)/36525.0)*DEG;
  theta0 = (jd - 2398220.0)*360.0/25.38;
  theta = theta0*DEG;
  T = (jd - 2451545.0)/36525.0;        // julian centuries since 2000.0
  // mean solar longitude
  l0 = pol2(280.46645, 36000.76983, 0.0003032, T);
  l = fmod(l0, 360)*DEG;
  // mean solar anomaly
  m = mpol3(357.52910, 35999.05030, -0.0001559, -4.8e-7, T, 360)*DEG;
  // eccentricity of the Earth's orbit
  e = pol2(0.016708617, -4.2037e-5, -1.236e-7, T);
  // equation of center
  c = (pol2(1.914600, -0.004817, -1.4e-5, T)*sin(m)
    + (0.019993 - 1.01e-4*T)*sin(2*m) + 0.000290*sin(3*m))*DEG;
  // true anomaly
  m += c;
  r = (1 - e*e)/(1 + e*cos(m));        // solar distance/AU
  // sun's true geometric longitude, corrected for aberration
  l += c - 0.00569*DEG;
  o = (125.04 - 1934.136*T)*DEG;
  l2 = l - 0.00478*sin(o)*DEG;        // corrected for nutation
  eps = pol3(23.43929111, -0.013004166, -1.6389e-7, 5.03611e-7, T)*DEG;
  eps += 2.56e-3*cos(o)*DEG;        // correction for nutation
  x = atan(-cos(l2)*tan(eps));
  y = atan(-cos(l - k)*tan(i));
  switch (select) {
    case 0:                        // solar P
      solar_stuff = (x + y)*RAD;
      break;
    case 1:                        // solar B
      solar_stuff = asin(sin(l - k)*sin(i))*RAD;
      break;
    case 2: case 4:                // solar L
      solar_stuff = fmod(atan2(-sin(l - k)*cos(i), -cos(l - k)) - theta,
                         2*M_PI)*RAD;
      if (solar_stuff < 0)
        solar_stuff += 360;
      if (select == 4) {        // add full Carrington rotations
        l0 = theta0 - l0 - 51584.933;
        l = 360*ceil(l0/360);
        solar_stuff -= l;
        if (solar_stuff + l0 > 180)
          solar_stuff -= 360;
        else if (solar_stuff + l0 < -180)
          solar_stuff += 360;
      }
      break;
    case 3:                        // solar angular radius/arcsec
      solar_stuff = 959.6276/r;
      break;
  }
}
//------------------------------------------------------------------------
int32_t lux_solar_physical(ArgumentCount narg, Symbol ps[], int32_t select)
// LS < 1998
{
  int32_t        n, result;
  Pointer        src, trgt;

  /* get copy of ps[0] upgraded to LUX_FLOAT if necessary, return pointer
     in <src>, number of elements in <n>.  Also create garbage clone of
     (updated) <iq> and return pointer in <trgt>, symbol number in
     <result>. */
  if (getNumerical(ps[0], LUX_FLOAT, &n, &src, GN_UPDATE | GN_UPGRADE,
                   &result, &trgt) < 0)
    return LUX_ERROR;                // some error
  switch (symbol_type(result)) {
    case LUX_FLOAT:
      while (n--) {
        solar_physical((double) *src.f++, select);
        *trgt.f++ = (float) solar_stuff;
      }
      break;
    case LUX_DOUBLE:
      while (n--) {
        solar_physical(*src.d++, select);
        *trgt.d++ = solar_stuff;
      }
      break;
  }
  return result;
}
//------------------------------------------------------------------------
int32_t lux_solar_p(ArgumentCount narg, Symbol ps[])
{
  return lux_solar_physical(narg, ps, 0);
}
//------------------------------------------------------------------------
int32_t lux_solar_b(ArgumentCount narg, Symbol ps[])
{
  return lux_solar_physical(narg, ps, 1);
}
//------------------------------------------------------------------------
int32_t lux_solar_l(ArgumentCount narg, Symbol ps[])
{
  return lux_solar_physical(narg, ps, (internalMode & 1)? 4: 2);
}
//------------------------------------------------------------------------
int32_t lux_solar_r(ArgumentCount narg, Symbol ps[])
{
  return lux_solar_physical(narg, ps, 3);
}
//------------------------------------------------------------------------

