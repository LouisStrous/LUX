/* This is file anasofa.cc.

Copyright 2013-2014 Louis Strous

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
#include "sofam.h"
#include "action.hh"
#include <string.h>
#include <time.h>

/* This file provides LUX bindings for SOFA routines */

/* NOT INCLUDED, because suitable LUX equivalents are already available:

   iauA2af   - decompose radians into degrees, arcminutes, arcseconds, fraction
   iauA2tf   - decompose radians into hours, minutes, seconds, fraction
   iauAf2a   - convert degrees, arcminutes, arcseconds to radians
   iauAnp    - normalize angle into the range 0 <= a < 2pi
   iauAnpm   - normalize angle into the range -pi <= a < +pi
   iauCp     - copy a p-vector
   iauCpv    - copy a position/velocity vector
   iauCr     - copy an r-matrix
   iauD2dtf  - decompose a (UTC) JD into a Gregorian date
   iauD2tf   - decompose days to hours, minutes, seconds, fraction
   iauDtf2d  - compose a (UTC) JD from a Gregorian date
   iauJdcalf - Julian Date to formatted Gregorian Calendar
   iauP2pv   - extend a p-vector to a pv-vector by appending zero velocity
   iauPmp    - subtract two p-vectors
   iauPpp    - add two p-vectors
   iauPpsp   - add a p-vector and a scaled p-vector
   iauPv2p   - discard velocity component of a pv-vector
   iauPvmpv  - subtract two pv-vectors
   iauPvppv  - add two pv-vectors
   iauPvu    - update a pv-vector
   iauPvup   - update a pv-vector, discarding the velocity component
   iauRxp    - multiply a p-vector by an r-matrix
   iauRxr    - multiply two r-matrices
   iauS2xpv  - multiply a pv-vector by two scalars
   iauSxp    - multiply a p-vector by a scalar
   iauSxpv   - multiply a pv-matrix by a scalar
   iauTf2a   - convert hours, minutes, seconds to radians
   iauTf2d   - convert hours, minutes, seconds to days
   iauZp     - zero a p-vector
   iauZpv    - zero a pv-vector
   iauZr     - initialize an r-matrix to the null matrix
 */

/* bi00() returns the frame bias components of the IAU 2000
   precession-nutation models (part of MHB2000 with additions), in a
   3-element DOUBLE array.  The elements are: (0) the longitude
   correction, (1) the obliquity correction, (2) the ICRS RA of the
   J2000.0 mean equinox. */
BIND(iauBi00, v_dpT3_r3_000, f, bi00, 0, 0, 0)
/*-----------------------------------------------------------------------*/
/* bp00, jd, rb, rp, rbp

   returns the frame bias and precession, IAU 2000.  <jd> are the
   Julian Dates for which results are desired.  <rb> receives the
   frame bias maxtrix, <rp> the precession matrix, and <rbp> the
   bias-precession matrix. */
BIND(iauBp00, v_dddp33T3_iao33op3p3qocq_0z1T3, s, bp00, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* bp06, jd, rb, rp, rbp

   returns the frame bias and precession, IAU 2006.  <jd> are the
   Julian Dates for which results are desired.  <rb> receives the
   frame bias matrix, <rp> the precession matrix, and <rbp> the
   bias-precession matrix.  */
BIND(iauBp06, v_dddp33T3_iao33op3p3qocq_0z1T3, s, bp06, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* BPN2XY, rbpn, x, y

   extracts the x,y coordinates of the Celestial Intermediate Pole
   from the bias-precesion-nutation matrix. */
BIND(iauBpn2xy, v_dp33dpdp_i33aommqocq_0T2, s, bpn2xy, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* c2i00a(jd)

 Returns the celestial-to-intermedia matrix for the Julian Dates <jd>
 using the IAU 2000A precession-nutation model. */
BIND(iauC2i00a, v_dddp33_iarp3p3q_0z1, f, c2i00a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* c2i00b(jd)

 Returns the celestial-to-intermedia matrix for the Julian Dates <jd>
 using the IAU 2000B precession-nutation model. */
BIND(iauC2i00b, v_dddp33_iarp3p3q_0z1, f, c2i00b, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* c2i06a(jd)

 Returns the celestial-to-intermedia matrix for the Julian Dates <jd>
 using the IAU 2006 precession model and IAU 2000A nutation model. */
BIND(iauC2i06a, v_dddp33_iarp3p3q_0z1, f, c2i06a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* rc2i = c2ibpn(jd, rbpn)

   Forms the celestial-to-intermediate matrix <rc2i> for Julian Dates
   <jd>, given the bias-precession-nutation matrix <rbpn>. */
BIND(iauC2ibpn, v_dddp33T2_iaip3p3qrcq_0z12, f, c2ibpn, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* c2ixy(jd, x, y)

   returns the celestial-to-intermediate-frame-of-date matrix for
   Julian Date <jd> and CIP x, y coordinates.
 */
BIND(iauC2ixy, v_dT4dp33_iaiqiqrp3p3q_0z1T3, f, c2ixy, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* rc2i = c2ixys(x, y, s)

   returns the celestial-to-intermediate-frame-of-date matrix given
   the CIP x, y and the CIO locator s.
 */
BIND(iauC2ixys, v_dT3dp33_iaiqiqrp3p3q_0T3, f, c2ixys, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* angles = c2s(p)

   Translates p-vector <p> to spherical coordinates.
   <angles(0,/all)> = longitude angle (radians)
   <angles(1,/all)> = latitude angle (radians)
   */
BIND(iauC2s, v_dp3dpdp_i3arm3p2q_011, f, c2s, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* rc2t = c2t00a(jdtt, jdut, x, y)

   Return the celestial-to-terrestrial matrix given <jdtt>, <jdut>,
   and the polar coordinates <x>, <y>, using the IAU 2000A nutation
   model.  <jdtt> and <jdut> should express the same instants in time,
   in the TT and UT1 timescales, respectively */
BIND(iauC2t00a, v_dT6dp33_iaiqT3rp3p3q_0z11T4, f, c2t00a, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* rc2t = c2t00b(jdtt, jdut, x, y)

   Return the celestial-to-terrestrial matrix given <jdtt>, <jdut>,
   and the polar coordinates <x>, <y>, using the IAU 2000B nutation
   model.  <jdtt> and <jdut> should express the same instants in time,
   in the TT and UT1 timescales, respectively */
BIND(iauC2t00b, v_dT6dp33_iaiqT3rp3p3q_0z11T4, f, c2t00b, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* rc2t = c2t06a(jd, x, y)

   Return the celestial-to-terrestrial matrix given <jdtt>, <jdut>,
   and the polar coordinates <x>, <y>, using the IAU 2006 precession
   model and the IAU 2000A nutation model.  <jdtt> and <jdut> should
   express the same instants in time, in the TT and UT1 timescales,
   respectively */
BIND(iauC2t06a, v_dT6dp33_iaiqT3rp3p3q_0z11T4, f, c2t06a, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* c2tcio(rc2i, era, rpom)

   Returns the celestial-to-terrestrial matrix based on the CIO-based
   components (the celestial-to-intermediate matrix <rc2i>, the Earth
   Rotation Angle <era>, and the polar motion matrix <rpom>) */
BIND(iauC2tcio, v_dp33ddp33T2_i33aimmqiqrq_0T3, f, c2tcio, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* c2teqx(rbpn, gst, rpom)

   Returns the celestial-to-terrestrial matrix based on the
   celestial-to-true matrix <rbpn>, the Greenwich Apparent Sidereal
   Time <gst>, and the polar motion matrix <rpom>) */
BIND(iauC2teqx, v_dp33ddp33T2_i33aimmqiqrq_0T3, f, c2teqx, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* c2tpe(jd, dpsi, deps, xp, yp)

   Returns the celestial-to-terrestrial matrix given the Julian Date
   <jd>, the nutation <dpsi>, <deps>, and the polar coordinates <xp>,
   <yp> */
BIND(iauC2tpe, v_dT8dp33_iaiqT4rp3p3q_0zzz1T5, f, c2tpe, 5, 5, 0)
/*-----------------------------------------------------------------------*/
/* c2txy(jd, x, y, xp, yp)

   Returns the celestial-to-terrestrial matrix given the Julian Date
   <jd>, the CIP coordinates <x>, <y>, and the polar motion <xp>,
   <yp> */
BIND(iauC2txy, v_dT8dp33_iaiqT4rp3p3q_0zzz1T5, f, c2txy, 5, 5, 0)
/*-----------------------------------------------------------------------*/
/* cal2jd(<caldates>)

   returns the Julian Dates corresponding to the specified Gregorian
   calendar dates <caldates>, which must have 3 elements in its first
   dimension (year, month, day).  Unexpected results may be given if
   illegal year, month, or day numbers are specified.  Dates before
   -4799-01-01 or after 1465073-02-28 are rejected. */
int32_t lux_iauCal2jd(int32_t narg, int32_t ps[])
{
  double djm0, djm;
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "i>L3*;r>L-3*", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  /* iauCal2jd(int32_t iy, int32_t im, int32_t id, double *djm0, double *djm) */
  switch (infos[0].type) {
  case LUX_INT32:
    while (infos[1].nelem--) {
      if (iauCal2jd(ptrs[0].l[0], ptrs[0].l[1], ptrs[0].l[2], &djm0, &djm))
        *ptrs[1].l = 0;
      else
        *ptrs[1].l = (djm0 + 0.5) + djm;
      ptrs[0].l += 3;
      ptrs[1].l++;
    }
    break;
  case LUX_INT64:
    while (infos[1].nelem--) {
      if (iauCal2jd(ptrs[0].q[0], ptrs[0].q[1], ptrs[0].q[2], &djm0, &djm))
        *ptrs[1].q = 0;
      else
        *ptrs[1].q = (djm0 + 0.5) + djm;
      ptrs[0].q += 3;
      ptrs[1].q++;
    }
    break;
  case LUX_FLOAT:
    while (infos[1].nelem--) {
      int32_t day = floor(ptrs[0].f[2]);
      double daypart = ptrs[0].f[2] - day;
      if (iauCal2jd((int32_t) ptrs[0].f[0], (int32_t) ptrs[0].f[1], day,
                    &djm0, &djm))
        *ptrs[1].f = 0;
      else
        *ptrs[1].f = djm0 + djm + daypart;
      ptrs[0].f += 3;
      ptrs[1].f++;
    }
    break;
  case LUX_DOUBLE:
    while (infos[1].nelem--) {
      int32_t day = floor(ptrs[0].d[2]);
      double daypart = ptrs[0].d[2] - day;
      if (iauCal2jd((int32_t) ptrs[0].d[0], (int32_t) ptrs[0].d[1], day,
                    &djm0, &djm))
        *ptrs[1].d = 0;
      else
        *ptrs[1].d = 2400000.5 + djm + daypart;
      ptrs[0].d += 3;
      ptrs[1].d++;
    }
    break;
  default:
    return cerror(ILL_TYPE, ps[0]);
  }
  return sa.result();
}
REGISTER(iauCal2jd, f, cal2jd, 1, 1, 0);
/*-----------------------------------------------------------------------*/
/* dat(date) returns delta(AT) = TAI - UTC for the given UTC date
   ([year, month, day]) */
int32_t lux_iauDat(int32_t narg, int32_t ps[])
{
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t iq;

  if (internalMode & 1) {       /* /VALID */
    time_t t;
    int32_t jdlo, jdhi, y, m, d;
    double f, dt;

    /* determine the last date of validity of the current implementation */

    jdlo = 2441318;         /* 1972-01-01 */
    t = time(0);
    jdhi = (double) t/86400.0 + 2440587.5 + 10000;
    iauJd2cal(jdlo, 0.0, &y, &m, &d, &f);
    iauJd2cal(jdhi, 0.0, &y, &m, &d, &f);
    do {
      int32_t jd, s;
      jd = (jdhi + jdlo)/2;
      iauJd2cal(jd, 0.0, &y, &m, &d, &f);
      s = iauDat(y, m, d, f, &dt);
      if (s) {
        jdhi = jd;
      } else {
        jdlo = jd;
      }
    } while (jdhi - jdlo > 1);
    iq = scalar_scratch(LUX_INT32);
    scalar_value(iq).l = jdlo;
    return iq;
  } else {
    if (narg < 1)
      return luxerror("Need 1 argument", 0);
    StandardArguments_RAII sa;
    if ((iq = sa.set(narg, ps, "i>L3*;rD-3", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    switch (infos[0].type) {
    case LUX_INT32:
      while (infos[1].nelem--) {
        iauDat(ptrs[0].l[0], ptrs[0].l[1], ptrs[0].l[2], 0.0, ptrs[1].d++);
        ptrs[0].l += 3;
      }
      break;
    case LUX_INT64:
      while (infos[1].nelem--) {
        iauDat(ptrs[0].q[0], ptrs[0].q[1], ptrs[0].q[2], 0.0, ptrs[1].d++);
        ptrs[0].q += 3;
      }
      break;
    case LUX_FLOAT:
      while (infos[1].nelem--) {
        int32_t d = (int32_t) floor(ptrs[0].f[2]);
        double f = ptrs[0].f[2] - d;
        iauDat((int32_t) ptrs[0].f[0], (int32_t) ptrs[0].f[1], d, f,
               ptrs[1].d++);
        ptrs[0].f += 3;
      }
      break;
    case LUX_DOUBLE:
      while (infos[1].nelem--) {
        int32_t d = (int32_t) floor(ptrs[0].d[2]);
        double f = ptrs[0].d[2] - d;
        iauDat((int32_t) ptrs[0].d[0], (int32_t) ptrs[0].d[1], d, f,
               ptrs[1].d++);
        ptrs[0].d += 3;
      }
      break;
    default:
      break;
    }
  }
  return iq;
}
REGISTER(iauDat, f, dat, 0, 1, "1valid");
/*-----------------------------------------------------------------------*/
/* dtdb(jdtt, ut, elong, u, v)

   Returns an approximation of TDB-TT in seconds, the difference
   between barycentric dynamical time and terrestrial time, for an
   observer on the Earth. <jd> is the Julian Date in TDB or TT,
   <elong> is the east longitude in radians, <u> is the distance in km
   from the Earth spin axis, and <v> is the distance in km north of
   the equatorial plane. */
BIND(iauDtdb, d_dT6_iaiT4rq_0z1T4_5, f, dtdb, 5, 5, 0)
/*-----------------------------------------------------------------------*/
/* ee00(<jd>, <epsa>, <dpsi>)

   Returns the equation of the equinoxes (IAU 2000) given the nutation
   in longitude <epsa> and the mean obliquity <dpsi> */
double do_iauEe00(double jd, double epsa, double dpsi)
{
  return iauEe00(jd, 0, epsa, dpsi);
}
BIND(do_iauEe00, d_dT3_iaiqiqrq_012, f, ee00, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* ee00a(<jd>)

   Returns the equation of the equinoxes, compatible with IAU 2000
   resolutions */
BIND(iauEe00a, d_dd_iarq_0z_1, f, ee00a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* ee00b(<jd>)

   Returns the equation of the equinoxes, compatible with IAU 2000
   resolutions but using the truncated nutation model IAU 2000B */
BIND(iauEe00b, d_dd_iarq_0z_1, f, ee00b, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* ee06a(<jd>)

   Returns the equation of the equinoxes, compatible with IAU 2000
   resolutions and the IAU 2006/200A precession-nutation */
BIND(iauEe06a, d_dd_iarq_0z_1, f, ee06a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* eect00(<jd>)

   Returns the equation of the equinoxes complementary term,
   consistent with IAU 2000 resolutions. */
BIND(iauEect00, d_dd_iarq_0z_1, f, eect00, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* eform(<ellid>)

   Returns the equatorial radius (in meters) and the flattening of the
   Earth reference ellipsoid with the given ellipsoid identifier
   <ellid>: 1 = WGS84, 2 = GRS80, 3 = WGS72.  If an unsupported
   <ellid> is specified, then zero radius and flattening are
   returned. */
BIND(iauEform, i_idpdp_iarp2q_011, f, eform, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* eo06a(<jd>)

   Returns the equation of the origins, using IAU 2006 precession and
   IAU 2000A nutation. */
BIND(iauEo06a, d_dd_iarq_0z_1, f, eo06a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* eors(<rnpb>, <s>)

   Returns the equation of the origins, given the classical NPB matrix
   <rnpb> and the CIO locator <s> */
BIND(iauEors, d_dp33d_i33aimmqrcq_01_2, f, eors, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* epb(<jd>)

   Returns the Besselian epoch corresponding to Julian Date <jd>. */
BIND(iauEpb, d_dd_iarq_0z_1, f, epb, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* epb2jd(<bepoch>)

   Returns the Julian Date corresponding to Besselian epoch <bepoch>
   (e.g., 1957.3). */
int32_t lux_iauEpb2jd(int32_t narg, int32_t ps[])
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "i>D*;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    double djm0, djm;

    iauEpb2jd(*ptrs[0].d++, &djm0, &djm);
    *ptrs[1].d++ = djm0 + djm;
  }
  return sa.result();
}
REGISTER(iauEpb2jd, f, epb2jd, 1, 1, 0);
/*-----------------------------------------------------------------------*/
/* epj(<jd>)

   Returns the Julian epoch corresponding to Julian Date <jd>. */
BIND(iauEpj, d_dd_iarq_0z_1, f, epj, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* epj2jd(<bepoch>)

   Returns the Julian Date corresponding to Julian epoch <bepoch>
   (e.g., 1957.3). */
int32_t lux_iauEpj2jd(int32_t narg, int32_t ps[])
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "i>D*;rD*", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    double djm0, djm;

    iauEpj2jd(*ptrs[0].d++, &djm0, &djm);
    *ptrs[1].d++ = djm0 + djm;
  }
  return sa.result();
}
REGISTER(iauEpj2jd, f, epj2jd, 1, 1, 0);
/*-----------------------------------------------------------------------*/
/* epv00, jd, pvh, pvb

   Returns the Earth position and velocity, heliocentric (pvh) and
   barycentric (pvb), with respect to the Barycentric Celestial
   Reference System.  pvh(0,*) = heliocentric position (AU), pvh(1,*)
   = heliocentric velocity (AU/d), pvb(0,*) = barycentric position
   (AU), pvb(1,*) = barycentric velocity (AU/d) */
BIND(iauEpv00, i_dddp23T2_iaop2p3qocq_0z12, s, epv00, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* eqeq94(<jd>)

   Returns the equation of the equinoxes according to the IAU 1994 model.
 */
BIND(iauEqeq94, d_dd_iarq_0z_1, f, eqeq94, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* era00(<jd>)

   Returns the Earth rotation angle according to the IAU 2000 model.
*/
BIND(iauEra00, d_dd_iarq_0z_1, f, era00, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* fad03(<t>)

   Returns the mean elongation of the Moon from the Sun, a fundamental
   argument according to the IERS conventions of 2003, as a function
   of the number <t> of Julian centuries since J2000.0 TDB.  It makes
   no practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFad03, d_d_iarq_0_1, f, fad03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* fae03(<t>)

   Returns the mean longitude of the Earth, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFae03, d_d_iarq_0_1, f, fae03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* faf03(<t>)

   Returns the mean longitude of the Moon minus the mean longitude of
   the ascending node, a fundamental argument according to the IERS
   conventions of 2003, as a function of the number <t> of Julian
   centuries since J2000.0 TDB.  It makes no practical difference if
   <t> is measured since J2000.0 TT instead. */
BIND(iauFaf03, d_d_iarq_0_1, f, faf03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* faju03(<t>)

   Returns the mean longitude of Jupiter, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFaju03, d_d_iarq_0_1, f, faju03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* fal03(<t>)

   Returns the mean anomaly of the Moon, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFal03, d_d_iarq_0_1, f, fal03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* falp03(<t>)

   Returns the mean anomaly of the Sun, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFalp03, d_d_iarq_0_1, f, falp03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* fama03(<t>)

   Returns the mean longitude of Mars, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFama03, d_d_iarq_0_1, f, fama03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* fame03(<t>)

   Returns the mean longitude of Mercury, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFame03, d_d_iarq_0_1, f, fame03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* fane03(<t>)

   Returns the mean longitude of Neptune, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFane03, d_d_iarq_0_1, f, fane03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* faom03(<t>)

   Returns the mean longitude of the Moon's ascending node, a
   fundamental argument according to the IERS conventions of 2003, as
   a function of the number <t> of Julian centuries since J2000.0 TDB.
   It makes no practical difference if <t> is measured since J2000.0
   TT instead. */
BIND(iauFaom03, d_d_iarq_0_1, f, faom03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* fapa03(<t>)

   Returns the general accumulated precession in longitude, a
   fundamental argument according to the IERS conventions of 2003, as
   a function of the number <t> of Julian centuries since J2000.0 TDB.
   It makes no practical difference if <t> is measured since J2000.0
   TT instead. */
BIND(iauFapa03, d_d_iarq_0_1, f, fapa03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* fasa03(<t>)

   Returns the mean longitude of Saturn, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFasa03, d_d_iarq_0_1, f, fasa03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* faur03(<t>)

   Returns the mean longitude of Uranus, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFaur03, d_d_iarq_0_1, f, faur03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* fave03(<t>)

   Returns the mean longitude of Venus, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFave03, d_d_iarq_0_1, f, fave03, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* fk52h, <ra5>, <dec5>, <dra5>, <ddec5>, <px5>, <rv5>, <rah>,
   <dech>, <drah>, <ddech>, <pxh>, <rvh>

   Transforms FK5 (J2000.0) star data into the Hipparcos system.  The
   FK5 data is: right ascension <ra5>, declination <dec5>, proper
   motion in right ascension <dra5>, proper motion in declination
   <ddec5>, parallax <px5>, radial velocity <rv5>.  The Hipparcos data
   is: right ascension <rah>, declination <dech>, proper motion in
   right ascension <drah>, proper motion in declination <ddech>,
   parallax <pxh>, radial velocity <rvh>. */
BIND(iauFk52h, v_dT6dpT6_iaiqT5oqT6_0T11, s, fk52h, 12, 12, 0)
/*-----------------------------------------------------------------------*/
/* fk5hip, <r5h>, <s5h>

   Returns the FK5-to-Hipparcos rotation <r5h> and spin <s5h> */
BIND(iauFk5hip, v_dp3dp_o33o3_01, s, fk5hip, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* fk5hz, <ra5>, <dec5>, <jd>, <rah>, <dech>

   Transforms an FK5 (J2000.0) star position (right ascension <ra5>,
   declination <dec5>) determined at Julian Date <jd> into the system
   of the Hipparcos catalog, assuming zero Hipparcos proper motion.
   The returned coordinates in the Hipparcos system are right
   ascension <rah>, declination <dech> */
BIND(iauFk5hz, v_dT4dpdp_iaiqiqoqoq_0T2z34, s, fk5hz, 5, 5, 0)
/*-----------------------------------------------------------------------*/
/* fw2m(<gamma_bar>, <phi_bar>, <psi>, <eps>)

   Forms a rotation matrix given the Fukushima-Williams angles
   <gamma_bar>, <phi_bar>, <psi>, and <eps> */
BIND(iauFw2m, v_dT4dp33_iaiqT3rp3p3q_0T4, f, fw2m, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* fw2xy, <gamma_bar>, <phi_bar>, <psi>, <eps>, <x>, <y>

   Returns CIP <x>, <y>, given the Fukushima-Williams
   bias-precession-nutation angles <gamma_bar>, <phi_bar>, <psi>,
   <eps>. */
BIND(iauFw2xy, v_dT4dpdp_iaiqT3oqoq_0T5, s, fw2xy, 6, 6, 0)
/*-----------------------------------------------------------------------*/
/* gc2gd(<xyz>, <ellid>)

   Returns geodetic coordinates (elongation, phi, height)
   corresponding to geocentric coordinates <xyz> using the specified
   reference ellipsoid <ellid> */
BIND(iauGc2gd, i_idp3dpT3_ii3arcq_0T222, f, gc2gd, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* gc2gde(<xyz>, <a>, <f>)

   Returns geodetic coordinates (elongation, phi, height)
   corresponding to geocentric coordinates <xyz> using the reference
   ellipsoid with equatorial radius <a> and flattening <f> */
BIND(iauGc2gde, v_dddp3dpT3_i3aiirq_120333, f, gc2gde, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* gd2gc(<eph>, <ellid>)

   Returns geocentric coordinates (x, y, z) corresponding to geodetic
   coordinates <eph> (elongation, phi, height) for the specified
   reference ellipsoid <ellid> */
BIND(iauGd2gc, i_idT3dp3_i3airq_10002, f, gd2gc, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* gd2gce(<eph>, <a>, <f>)

   Returns geocentric coordinates (x, y, z) corresponding to geodetic
   coordinates <eph> (elongation, phi, height) for the reference
   ellipsoid with equatorial radius <a> and flattening <f> */
BIND(iauGd2gce, i_dT5dp3_i3aiirq_120003, f, gd2gce, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* gmst00(<jdut>, <jdtt>)

   Returns Greenwich mean sidereal time according to a model that is
   consistent with IAU 2000 resolution.  <jdut> is the Julian Date
   (UT1).  <jdtt> is the Julian Date (TT) of the same instant. <jdut>
   is needed to predict the Earth rotation, and TT to predict the
   effects of precession.  If <jdtt> is set equal to <jdut> then
   errors of order 100 microarcseconds result. */
BIND(iauGmst00, d_dT4_iaiqrq_01_2, f, gmst00, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* gmst06(<jdut>, <jdtt>)

   Returns Greenwich mean sidereal time according to a model that is
   consistent with IAU 2006 precession.  <jdut> is the Julian Date
   (UT1).  <jdtt> is the Julian Date (TT) of the same instant. <jdut>
   is needed to predict the Earth rotation, and TT to predict the
   effects of precession.  If <jdtt> is set equal to <jdut> then
   errors of order 100 microarcseconds result. */
BIND(iauGmst06, d_dT4_iaiqrq_01_2, f, gmst06, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* gmst82(<jd>)

   Returns Greenwich mean sidereal time for the given UT1 Julian Date
   <jd>, according to the IAU 1982 model */
BIND(iauGmst82, d_dd_iarq_0z_1, f, gmst82, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* gst00a(<jdut>, <jdtt>)

   Returns Greenwich apparent sidereal time consisten with IAU 2000
   resolutions.  <jdut> is the Julian Date (UT1).  <jdtt> is the
   Julian Date (TT) of the same instant. <jdut> is needed to predict
   the Earth rotation, and TT to predict the effects of precession.
   If <jdtt> is set equal to <jdut> then errors of order 100
   microarcseconds result. */
BIND(iauGst00a, d_dT4_iaiqrq_01_2, f, gst00a, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* gst00b(<jdut>)

   Returns Greenwich apparent sidereal time consistent with IAU 2000
   resolutions but using the truncated nutation model IAU 2000B.
   <jdut> is the Julian Date (UT1). */
BIND(iauGst00b, d_dd_iarq_0z_1, f, gst00b, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* gst06(<jdut>, <jdtt>, <rnpb>)

   Returns Greenwich apparent sidereal time consistent with IAU 2006
   resolutions, given the nutation-precession-bias matrix
   <rnpb>. <jdtt> is the Julian Date (TT) of the same instant. <jdut>
   is needed to predict the Earth rotation, and TT to predict the
   effects of precession.  If <jdtt> is set equal to <jdut> then
   errors of order 100 microarcseconds result. */
BIND(iauGst06, d_dT4dp33_iaiqip3p3qrq_00112_3, f, gst06, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* gst06a(<jdut>, <jdtt>)

   Returns Greenwich apparent sidereal time consistent with IAU 2000
   and IAU 2006 resolutions.  <jdut> is the Julian Date (UT1).  <jdtt>
   is the Julian Date (TT) of the same instant. <jdut> is needed to
   predict the Earth rotation, and TT to predict the effects of
   precession.  If <jdtt> is set equal to <jdut> then errors of order
   100 microarcseconds result. */
BIND(iauGst06a, d_dT4_iaiqrq_01_2, f, gst06a, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* gst94(<jd>)

   Returns Greenwich apparent sidereal time for the given UT1 Julian
   Date <jd>, consistent with IAU 1982/94 resolutions */
BIND(iauGst94, d_dd_iarq_0z_1, f, gst94, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* h2fk5, <rah>, <dech>, <drah>, <ddech>, <pxh>, <rvh>, <ra5>,
   <dec5>, <dra5>, <ddec5>, <px5>, <rv5>

   Transforms Hipparcos star data into the FK5 (J2000.0) system.  The
   Hipparcos data is: right ascension <rah>, declination <dech>,
   proper motion in right ascension <drah>, proper motion in
   declination <ddech>, parallax <pxh>, radial velocity <rvh>.  The
   FK5 data is: right ascension <ra5>, declination <dec5>, proper
   motion in right ascension <dra5>, proper motion in declination
   <ddec5>, parallax <px5>, radial velocity <rv5>. */
BIND(iauH2fk5, v_dT6dpT6_iaiqT5oqT6_0T11, s, h2fk5, 12, 12, 0)
/*-----------------------------------------------------------------------*/
/* hfk5z, <rah>, <dech>, <jd>, <ra5>, <dec5>

   Transforms a Hipparcos star position (right ascension <rah>,
   declination <dech>) determined at Julian Date <jd> into the FK5
   (J2000.0) system, assuming zero Hipparcos proper motion.  The
   returned coordinates in the FK5 system are right ascension <ra5>,
   declination <dec5> */
BIND(iauHfk5z, v_dT4dpT4_iaiqiqoqT4_0T2z3T6, s, hfk5z, 7, 7, 0)
/*-----------------------------------------------------------------------*/
/* ir()

   Returns an identity r-matrix */
BIND(iauIr, v_dp33_r33_0, f, ir, 0, 0, 0)
/*-----------------------------------------------------------------------*/
/* jd2cal(<jd>) returns the Gregorian calendar dates corresponding
   to the specified Julian Dates <jd>.  The return value has the same
   dimensions as <jd> but with one extra dimension of size 3 prefixed,
   for the year, month, and day, respectively.  Zeros are returned for
   input JD outside of the supported range.  Julian day numbers before
   -68569.5 or after 1e9 are considered illegal -- actually, JDs
   greater than 536802342 yield erroneous results, but the SOFA
   routine does not indicate problems until the JD exceeds 1e9. */
BIND(iauJd2cal, i_ddipT3dp_iarp3q_0z1111, f, jd2cal, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* num00a(<jd>)

   Returns the matrix for nutation for a given Julian Date <jd>,
   according to the IAU 2000A model. */
BIND(iauNum00a, v_dddp33_iarp3p3q_0z1, f, num00a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* num00b(<jd>)

   Returns the matrix for nutation for a given Julian Date <jd>,
   according to the IAU 2000B model. */
BIND(iauNum00b, v_dddp33_iarp3p3q_0z1, f, num00b, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* num06a(<jd>)

   Returns the matrix for nutation for a given Julian Date <jd>,
   according to the IAU 2006/2000A model. */
BIND(iauNum06a, v_dddp33_iarp3p3q_0z1, f, num06a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* numat(<epsa>, <dpsi>, <deps>)

   Returns the matrix of nutation based on the mean obliquity of the
   date <epsa> and the nutation components <dpsi> and <deps>. */
BIND(iauNumat, v_dT3dp33_iaiqiqrp3p3q_0T3, f, numat, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* nut00a, <jd>, <dpsi>, <deps>

   Returns the nutation for Julian Date <jd> according to the IAU
   2000A model (MHB2000 luni-solar and planetary nutation with fre
   core nutation omitted).  <dpsi> and <deps> are the returned
   nutation components (luni-solar + planetary) */
BIND(iauNut00a, v_dddpdp_iaoqoq_0z12, s, nut00a, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* nut00b, <jd>, <dpsi>, <deps>

   Returns the nutation for Julian Date <jd> according to the IAU
   2000B model.  <dpsi> and <deps> are the returned nutation
   components (luni-solar + planetary) */
BIND(iauNut00b, v_dddpdp_iaoqoq_0z12, s, nut00b, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* nut06a, <jd>, <dpsi>, <deps>

   Returns the nutation for Julian Date <jd> according to the IAU
   2000A model, with adjustments to match the IAU 2006 precession.
   <dpsi> and <deps> are the returned nutation components (luni-solar
   + planetary) */
BIND(iauNut06a, v_dddpdp_iaoqoq_0z12, s, nut06a, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* nut80, <jd>, <dpsi>, <deps>

   Returns the nutation for Julian Date <jd> according to the IAU 1980
   model.  <dpsi> and <deps> are the returned nutation components
   (luni-solar + planetary) */
BIND(iauNut80, v_dddpdp_iaoqoq_0z12, s, nut80, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* nutm80(<jd>)

   Returns the nutation matrix for Julian Date <jd>, according to the
   IAU 1980 model.
 */
BIND(iauNutm80, v_dddp33_iarp3p3q_0z1, f, nutm80, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* obl06(<jd>)

   Returns the mean obliquity of the ecliptic according to the IAU
   2006 precession model.
 */
BIND(iauObl06, d_dd_iarq_0z_1, f, obl06, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* obl80(<jd>)

   Returns the mean obliquity of the ecliptic according to the IAU
   1980 model. */
BIND(iauObl80, d_dd_iarq_0z_1, f, obl80, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* p06e, <jd>, <eps0>, <psia>, <oma>, <bpa>, <bqa>, <pia>, <bpia>,
   <epsa>, <chia>, <za>, <zetaa>, <thetaa>, <pa>, <gam>, <phi>, <psi>

   Returns IAU 2006 equinox-based precession angles for Julian Date
   <jd>.  The returned precession angles are: epsilon_0, psi_A,
   omega_A, P_A, Q_A, pi_A, Pi_A, obliquity epsilon_A, chi_A, z_A,
   zeta_A, theta_A, p_A, F-W angle gamma_J2000, F-W angle phi_J2000,
   F-W angle psi_J2000.
 */
BIND(iauP06e, v_dddpT16_iaoqT16_0z1T16, s, p06e, 17, 17, 0)
/*-----------------------------------------------------------------------*/
/* p2s(<p>)

   Returns the spherical polar coordinates (theta, phi, r)
   corresponding to the p-vector <p> */
BIND(iauP2s, v_dp3dpT3_i3arq_0111, f, p2s, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* pap(<ref>, <tgt>)

   Returns the position angle of p-vector <tgt> from p-vector <ref>
*/
BIND(iauPap, d_dp3dp3_i3aiqrm3q_01_2, f, pap, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* pas(<al>, <ap>, <bl>, <bp>)

   Returns the position angle of B with respect to A.  <al> =
   longitude of point A in radians, <ap> = latitude of point A in
   radians, <bl> = longitude of point B in radians, <bp> = latitude of
   point B in radians
*/
BIND(iauPas, d_dT4_iaiqT3rq_0T3_4, f, pas, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* pb06, <jd>, <bzeta>, <bz>, <btheta>

   Returns three Euler angles which implement general precesion from
   epoch J2000.0, using the IAU 2006 model, for Julian Date <jd>.
   Frame bias (the offset between ICRS and J2000.0) is included. */
BIND(iauPb06, v_dddpT3_iaoqT3_0z1T3, s, pb06, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* pdp(<a>, <b>)

   Returns the inner product of the two p-vectors <a> and <b> */
BIND(iauPdp, d_dp3dp3_i3aiqrm3q_01_2, f, pdp, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* pfw06, <jd>, <gamb>, <phib>, <psib>, <epsa>

   Returns the IAU 2006 precession angles in Fukushima-Williams
   4-angle formulation. */
BIND(iauPfw06, v_dddpT4_iaoqT4_0z1T4, s, pfw06, 5, 5, 0)
/*-----------------------------------------------------------------------*/
/* plan94(<jd>, <planet>)

   Returns the approximate heliocentric position and velocity at
   Julian Date <jd> of major planet <planet> (1 = Mercury, 2 = Venus,
   3 = EMB, 4 = Mars, ..., 8 = Neptune)
*/
BIND(iauPlan94, i_ddidp23_iairp3p2q_0z12, f, plan94, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* pm(<p>)

   Returns the modulus of the p-vector <p>
*/
BIND(iauPm, d_dp3_i3arm3q_0_1, f, pm, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* pmat00(<jd>)

   Returns the precession matrix (including frame bias) from GCRS to a
   specified Julian Date <jd> according to the IAU 2000 model.
*/
BIND(iauPmat00, v_dddp33_iarp3p3q_0z1, f, pmat00, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* pmat06(<jd>)

   Returns the precession matrix (including frame bias) from GCRS to a
   specified Julian Date <jd> according to the IAU 2006 model.
*/
BIND(iauPmat06, v_dddp33_iarp3p3q_0z1, f, pmat06, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* pmat76(<jd>)

   Returns the precession matrix from J2000.0 to a specified Julian
   Date <jd> according to the IAU 1976 model. */
BIND(iauPmat76, v_dddp33_iarp3p3q_0z1, f, pmat76, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* pn, <p>, <r>, <u>

   Converts p-vector <p> into a modulus <r> and unit vector <u> */
BIND(iauPn, v_dp3dpdp3_i3aomqoq_0T2, s, pn, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* pn00, <jd>, <dpsi>, <deps>, <epsa>, <rb>, <rp>, <rbp>, <rn>, <rbpn>

   Returns precession-nutation according to the IAU 2000 model for the
   specified Julian Date <jd> and nutation <dpsi>, <deps>.  The output
   parameters are the mean obliquity <epsa>, the frame bias matrix
   <rb>, the precession matrix <rp>, the bias-precession matrix <rbp>,
   the nutation matrix <rn>, and the GCRS-to-true matrix <rbpn>. */
BIND(iauPn00, v_dT4dpdp3T5_iaiqiqoqop3p3qocqT4_0z1T8, s, pn00, 9, 9, 0)
/*-----------------------------------------------------------------------*/
/* pn00a, <jd>, <dpsi>, <deps>, <epsa>, <rb>, <rp>, <rbp>, <rn>, <rbpn>

   Returns precession-nutation according to the IAU 200A model for the
   specified Julian Date <jd>.  The output parameters are the nutation
   <dpsi>, <deps>, the mean obliquity <epsa>, the frame bias matrix
   <rb>, the precession matrix <rp>, the bias-precession matrix <rbp>,
   the nutation matrix <rn>, and the GCRS-to-true matrix <rbpn>. */
BIND(iauPn00a, v_dddpT3dp33T5_iaoqT3op3p3qocqT4_0z1T8, s, pn00a, 9, 9, 0)
/*-----------------------------------------------------------------------*/
/* pn00b, <jd>, <dpsi>, <deps>, <epsa>, <rb>, <rp>, <rbp>, <rn>, <rbpn>

   Returns precession-nutation according to the IAU 200A model for the
   specified Julian Date <jd>.  The output parameters are nutation
   <dpsi>, <deps>, the mean obliquity <epsa>, the frame bias matrix
   <rb>, the precession matrix <rp>, the bias-precession matrix <rbp>,
   the nutation matrix <rn>, and the GCRS-to-true matrix <rbpn>. */
BIND(iauPn00b, v_dddpT3dp33T5_iaoqT3op3p3qocqT4_0z1T8, s, pn00b, 9, 9, 0)
/*-----------------------------------------------------------------------*/
/* pn06, <jd>, <dpsi>, <deps>, <epsa>, <rb>, <rp>, <rbp>, <rn>, <rbpn>

   Returns precession-nutation according to the IAU 2006 model for the
   specified Julian Date <jd> and nutation <dpsi>, <deps>.  The output
   parameters are the mean obliquity <epsa>, the frame bias matrix
   <rb>, the precession matrix <rp>, the bias-precession matrix <rbp>,
   the nutation matrix <rn>, and the GCRS-to-true matrix <rbpn>. */
BIND(iauPn06, v_dT4dpdp3T5_iaiqiqoqop3p3qocqT4_0z1T8, s, pn06, 9, 9, 0)
/*-----------------------------------------------------------------------*/
/* pn06a, <jd>, <dpsi>, <deps>, <epsa>, <rb>, <rp>, <rbp>, <rn>, <rbpn>

   Returns precession-nutation according to the IAU 2006/2000A models
   for the specified Julian Date <jd>.  The output parameters are the
   nutation <dpsi>, <deps>, the mean obliquity <epsa>, the frame bias
   matrix <rb>, the precession matrix <rp>, the bias-precession matrix
   <rbp>, the nutation matrix <rn>, and the GCRS-to-true matrix
   <rbpn>. */
BIND(iauPn06a, v_dddpT3dp33T5_iaoqT3op3p3qocqT4_0z1T8, s, pn06a, 9, 9, 0)
/*-----------------------------------------------------------------------*/
/* pnm00a(<jd>)

   Returns the matrix of precession-nutation (including frame bias)
   for Julian Date <jd>, equinox-based, IAU 2000A model. */
BIND(iauPnm00a, v_dddp33_iarp3p3q_0z1, f, pnm00a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* pnm00b(<jd>)

   Returns the matrix of precession-nutation (including frame bias)
   for Julian Date <jd>, equinox-based, IAU 2000B model. */
BIND(iauPnm00b, v_dddp33_iarp3p3q_0z1, f, pnm00b, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* pnm06a(<jd>)

   Returns the matrix of precession-nutation (including frame bias)
   for Julian Date <jd>, equinox-based, IAU 2006 precession and IAU
   2000A nutation models. */
BIND(iauPnm06a, v_dddp33_iarp3p3q_0z1, f, pnm06a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* pnm80(<jd>)

   Returns the matrix of precession-nutation (including frame bias)
   for Julian Date <jd>, equinox-based, IAU 1976 precession and IAU
   1980 nutation models. */
BIND(iauPnm80, v_dddp33_iarp3p3q_0z1, f, pnm80, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* pom00(<xp>, <yp>, <sp>)

   Returns the matrix of polar motion, IAU 2000, given the coordinates
   <xp>, <yp> of the pole and TIO locator <sp>. */
BIND(iauPom00, v_dT3dp33_iaiqiqrp3p3q_0T3, f, pom00, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* pr00, <jd>, <dpsipr>, <depspr>

   Returns the precession-rate part of the IAU 2000
   precession-nutation models (part of MHB2000). */
BIND(iauPr00, v_dddpdp_iaoqoq_0z12, s, pr00, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* prec76, <jd1>, <jd2>, <zeta>, <z>, <theta>

   Returns the three Euler angles <zeta>, <z>, <theta> which implement
   general precession between epochs (Julian Dates) <jd1> and <jd2>,
   using the IAU 1976 model (as for the FK5 catalog).  <zeta> is the
   1st rotation angle in radians clockwise around the z axis, <z> the
   3rd rotation angle in radians clockwise around the z axis, and
   <theta> the 2nd rotation angle in radians counterclockwise around
   the y axis. */
BIND(iauPrec76, v_dT4dpT3_iaiqoqT3_0z1z2T4, s, prec76, 5, 5, 0)
/*-----------------------------------------------------------------------*/
/* pv2s, <pv>, <theta>, <phi>, <r>, <td>, <pd>, <rd>

   Converts position/velocity vector <pv> from Cartesian to spherical
   coordinates.  <theta> = longitude angle, <phi> = latitude angle,
   <r> = radial distance, <td> = rate of change of theta, <pd> = rate
   of change of <phi>, <rd> = rate of change of <r> */
BIND(iauPv2s, v_dp23dpT6_iD23aommqocqT5_0T6, s, pv25, 7, 7, 0)
/*-----------------------------------------------------------------------*/
/* pvdpv(<a>, <b>)

   Returns the inner product of pv-vectors <a> and <b> */
BIND(iauPvdpv, v_dp23T2dp2_i23aiqrkmq_0T2, f, pvdpv, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* pvm, <pv>, <r>, <s>

   Returns the modulus of pv-vector <pv>.  <r> is the modulus of the
   position component, and <s> is the modulus of the velocity
   component. */
BIND(iauPvm, v_dp23dpdp_i23aommqocq_0T2, s, pvm, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* pvstar, <pv>, <ra>, <dec>, <pmr>, <pmd>, <px>, <rv>

   Converts position-velocity matrix <pv> to catalog coordinates right
   ascension <ra> (rad), declination <dec> (rad), right ascension
   proper motion <pmr> in rad/a, declination proper motion <pmd>
   (rad/a), parallax <px> (arcsec), and radial velocity <rv> (km/s
   receding) */
BIND(iauPvstar, i_dp23dpT6_i23aom2m3qomqT5_0T6, s, pvstar, 7, 7, 0)
/*-----------------------------------------------------------------------*/
/* pvxpv(<a>, <b>)

   Returns the outer product of two pv-vectors <a> and <b> */
BIND(iauPvxpv, v_dp23T3_i23aiqrq_0T2, f, pvxpv, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* pxp(<a>, <b>)

   Returns the outer product of two p-vectors <a> and <b> */
BIND(iauPxp, v_dp3T3_i3aiqrq_0T2, f, pxp, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* rm2v(<r>)

   Returns the rotation vector corresponding to rotation matrix <r>.
   The rotation vector has the same direction as the Euler axis of
   matrix <r> and has its magnitude equal to the rotation angle in
   radians of the rotation matrix. */
/* void iauRm2v(double r[3][3], double [3]) */
BIND(iauRm2v, v_dp33dp3_i33armq_01, f, rm2v, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* rxp(<r>, <p>)

   Returns the matrix product of an r-matrix and a p-vector. */
BIND(iauRxp, v_dp33dp3T2_i33aimqrcq_0T2, f, rxp, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* rxr(<r1>, <r2>)

   Returns the matrix product of two r-matrices. */
BIND(iauRxr, v_dp33T3_i33aiqrq_0T2, f, rxr, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* rv2m(<w>)

   Returns the rotation matrix corresponding to rotation vector <w>.
   The rotation matrix Euler axis gets the direction of <w>, and the
   rotation matrix gets a rotation angle equal to the modulus of
   <w>. */
BIND(iauRv2m, v_dp3dp33_i3arp3q_01, f, rv2m, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* rx, <phi>, <r>

   Rotates r-matrix <r> about the x axis over angle <phi> */
BIND(iauRx, v_ddp33_iao33q_01, s, rx, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* rxpv(<r>, <pv>)

   Returns the matrix product of r-matrix <r> and pv-vector <pv> */
BIND(iauRxpv, v_dp33dp23T2_i33aim3p2qrcq_0T2, f, rxpv, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* ry, <phi>, <r>

   Rotates r-matrix <r> about the y axis over angle <phi> */
BIND(iauRy, v_ddp33_iao33q_01, s, ry, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* rz, <phi>, <r>

   Rotates r-matrix <r> about the z axis over angle <phi> */
BIND(iauRz, v_ddp33_iao33q_01, s, rz, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* s00(<jd>, <x>, <y>)

   Returns the CIO locator for the specified Julian Date <jd> given
   the CIP's <x> and <y> coordinates, using IAU 2000A
   precession-nutation */
double do_iauS00(double jd, double x, double y)
{
  return iauS00(jd, 0, x, y);
}
BIND(do_iauS00, d_dT3_iaiqiqrq_012, f, s00, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* s00a(<jd>)

   Returns the CIO locator for the specified Julian Date <jd>, using
   IAU 2000A precession-nutation */
BIND(iauS00a, d_dd_iarq_0z_1, f, s00a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* s00b(<jd>)

   Returns the CIO locator for the specified Julian Date <jd>, using
   IAU 2000B precession-nutation */
BIND(iauS00b, d_dd_iarq_0z_1, f, s00b, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* s06(<jd>, <x>, <y>)

   Returns the CIO locator for the specified Julian Date <jd> given
   the CIP's <x> and <y> coordinates, using IAU 2006/2000A
   precession-nutation */
double do_iauS06(double jd, double x, double y)
{
  return iauS06(jd, 0, x, y);
}
BIND(do_iauS06, d_dT3_iaiqiqrq_012, f, s06, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* s06a(<jd>)

   Returns the CIO locator for the specified Julian Date <jd>, using
   IAU 2006 precession and IAU 2000A nutation */
BIND(iauS06a, d_dd_iarq_0z_1, f, s06a, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* s2c(<theta>, <phi>)

   Returns the cartesian coordinates (direction cosines) corresponding
   to spherical coordinates longitude <theta> and latitude <phi> */
BIND(iauS2c, v_dddp3_iaiqrp3q_0T2, f, S2C, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* s2p(<theta>, <phi>, <r>)

   Converts spherical polar coordinates longitude <theta>, latitude
   <phi>, radial distance <r> to a p-vector */
BIND(iauS2p, v_dT3dp3_iaiqiqrp3q_0T3, f, s2p, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* s2pv(<theta>, <phi>, <r>, <td>, <pd>, <rd>)

   Returns the pv-vector that corresponds to the spherical coordinates
   longitude <theta>, latitude <phi>, radial distance <r>, rate of
   change of theta <td>, rate of change of phi <pd>, rate of change of
   r <rd> */
BIND(iauS2pv, v_dT6dp23_iaiqT5op2p3q_0T6, f, s2pv, 6, 6, 0)
/*-----------------------------------------------------------------------*/
/* sepp(<a>, <b>)

   Returns the angular separation between two p-vectors <a> and <b> */
BIND(iauSepp, d_dp3dp3_i3aiqrm3q_01_2, f, sepp, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* seps(<lon1>, <lat1>, <lon2>, <lat2>)

   Returns the angular separation between the direction with longitude
   <lon1>, latitude <lat1> and the direction with longitude <lon2>,
   latitude <lat2> */
BIND(iauSeps, d_dT4_iaiqT3rq_0T3_4, f, seps, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* sp00(<jd>)

   Returns the TIO locator s' for Julian Date <jd>, positioning the
   Terrestrial Intermediate Origin on the equator of the Celestial
   Intermediate Pole */
BIND(iauSp00, d_dd_iarq_0z_1, f, sp00, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* starpm, <ra1>, <dec1>, <pmr1>, <pmd1>, <px1>, <rv1>, <jd1>,
   <jd2>, <ra2>, <dec2>, <pmr2>, <pmd2>, <px2>, <rv2>

   Updates star catalog data for space motion. <ra> = right ascension,
   <dec> = declination, <pmr> = right ascension proper motion, <pmd> =
   declination proper motion, <px> = parallax, <rv> = radial velocity;
   *1 = before, *2 = after */
BIND(iauStarpm, i_dT10dpT6_iaiqT7oqT6_0T6z7z8T13, s, starpm, 14, 14, 0)
/*-----------------------------------------------------------------------*/
/* starpv(<ra>, <dec>, <pmr>, <pmd>, <px>, <rv>)

   Converts star catalog coordinates to a position-velocity matrix */
BIND(iauStarpv, i_dT6dp23_iaiqT5op2p3q_0T6, f, starpv, 6, 6, 0)
/*-----------------------------------------------------------------------*/
/* taitt(<jdtai>)

   Translates Julian Date <jdtai> from TAI to TT */
BIND(iauTaitt, i_dddpdp_iarq_0z11, f, taitt, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* taiut1(<jdtai>, <dta>)

   Translates Julian Date <jdtai> from TAI to UT1, given <dta> = UT1 -
   TAi in seconds */
BIND(iauTaiut1, i_dT3dpdp_iaiqrq_0z122, f, taiut1, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* taiutc(<jdtai>)

   Translates Julian Date <jdtai> from TAI to UTC */
BIND(iauTaiutc, i_dddpdp_iarq_0z11, f, taiutc, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* tcbtdb(<jdtcb>)

   Translates Julian Date <jdtcb> from TCB to TDB */
BIND(iauTcbtdb, i_dddpdp_iarq_0z11, f, tcbtdb, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* tcgtt(<jdtcg>)

   Translates Julian Date <jdtcg> from TCG to TT */
BIND(iauTcgtt, i_dddpdp_iarq_0z11, f, tcgtt, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* tdbtcb(<jdtdb>)

   Translates Julian Date <jdtdb> from TDB to TCB */
BIND(iauTdbtcb, i_dddpdp_iarq_0z11, f, tdbtcb, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* tdbtt(<jdtdb>, <dtr>)

   Translates Julian Date <jdtbd> from TDB to TT, given <dtr> = TDB -
   TT in seconds  */
BIND(iauTdbtt, i_dT3dpdp_iaiqrq_0z122, f, tdbtt, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* tr(<r>)

   Transposes r-matrix <r> */
BIND(iauTr, v_dp33T2_i33arq_01, f, tr, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* trxp(<r>, <p>)

   Multiply p-vector <p> by the transpose of r-matrix <r> */
BIND(iauTrxp, v_dp33dp3T2_i33aimqrcq_0T2, f, trxp, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* trxpv(<r>, <pv>)

   Multiply pv-vector <pv> by the transpose of r-matrix <r> */
BIND(iauTrxpv, v_dp33dp23T2_i33aim3p2qrcq_0T2, f, trxpv, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* tttai(<jdtt>)

   Transforms Julian Date <jdtt> from TT to TAI */
BIND(iauTttai, i_dddpdp_iarq_0z11, f, tttai, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* tttcg(<jdtt>)

   Transforms Julian Date <jdtt> from TT to TCG */
BIND(iauTttcg, i_dddpdp_iarq_0z11, f, tttcg, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* tttdb(<jdtt>, <dt>)

   Transforms Julian Date <jdtt> from TT to TDB, given <dt> = TDB -
   TT in seconds */
BIND(iauTttdb, i_dT3dpdp_iaiqrq_0z122, f, tttdb, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* ttut1(<jdtt>, <dt>)

   Transforms Julian Date <jdtt> from TT to UT1, given <dt> = TT - UT1
   in seconds */
BIND(iauTtut1, i_dT3dpdp_iaiqrq_0z122, f, ttut1, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* ut1tai(<jdut>, <dt>)

   Transforms Julian Date <jdut> from UT1 to TAI, given <dt> = UT1 -
   TAI in seconds */
BIND(iauUt1tai, i_dT3dpdp_iaiqrq_0z122, f, ut1tai, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* ut1tt(<jdut>, <dt>)

   Transforms Julian Date <jdut> from UT1 to TT, given <dt> = UT1 -
   TT in seconds */
BIND(iauUt1tt, i_dT3dpdp_iaiqrq_0z122, f, ut1tt, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* ut1utc(<jdut>, <dt>)

   Transforms Julian Date <jdut> from UT1 to UTC, given <dt> = UT1 -
   UTC in seconds */
BIND(iauUt1utc, i_dT3dpdp_iaiqrq_0z122, f, ut1utc, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* utctai(<jdut>)

   Transforms Julian Date <jdut> from UTC to TAI */
BIND(iauUtctai, i_dddpdp_iarq_0z11, f, utctai, 1, 1, 0)
/*-----------------------------------------------------------------------*/
/* utcut1(<jdut>, <dt>)

   Transforms Julian Date <jdut> from UTC to UT1, given <dt> = UT1 -
   UTC in seconds */
BIND(iauUtcut1, i_dT3dpdp_iaiqrq_0z122, f, utcut1, 2, 2, 0)
/*-----------------------------------------------------------------------*/
/* xy06, <jd>, <x>, <y>

   Returns the Celestial Intermediate Pole <x>, <y> coordinates for
   the given Julian Date <jd>, based on IAU 2006 precession and IAU
   2000A nutation. */
BIND(iauXy06, v_dddpdp_iaoqoq_0z12, s, xy06, 3, 3, 0)
/*-----------------------------------------------------------------------*/
/* xys00a, <jd>, <x>, <y>, <s>

   Returns the Celestial Intermediate Pole <x>, <y> coordinates and
   the CIO locator <s> for Julian Date <jd>, based on the IAU 2000A
   precession-nutation model */
BIND(iauXys00a, v_dddpT3_iaoqT3_0z1T3, s, xys00a, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* xys00b, <jd>, <x>, <y>, <s>

   Returns the Celestial Intermediate Pole <x>, <y> coordinates and
   the CIO locator <s> for Julian Date <jd>, based on the IAU 2000B
   precession-nutation model */
BIND(iauXys00b, v_dddpT3_iaoqT3_0z1T3, s, xys00b, 4, 4, 0)
/*-----------------------------------------------------------------------*/
/* xys06a, <jd>, <x>, <y>, <s>

   Returns the Celestial Intermediate Pole <x>, <y> coordinates and
   the CIO locator <s> for Julian Date <jd>, based on the IAU 2006
   precession and IAU 2000A nutation models */
BIND(iauXys06a, v_dddpT3_iaoqT3_0z1T3, s, xys06a, 4, 4, 0)
/*-----------------------------------------------------------------------*/
