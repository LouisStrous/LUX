#include "sofam.h"
#include "action.h"
#include <string.h>

/* This file provides ANA bindings for SOFA routines */

/* NOT INCLUDED, because suitable ANA equivalents are already available:

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

/* IAUBI00() returns the frame bias components of the IAU 2000
   precession-nutation models (part of MHB2000 with additions), in a
   3-element DOUBLE array.  The elements are: (0) the longitude
   correction, (1) the obliquity correction, (2) the ICRS RA of the
   J2000.0 mean equinox. */
BIND(iauBi00, oddd_combine, f, IAUBI00, 0, 0, NULL)
/*-----------------------------------------------------------------------*/
/* IAUBP00, jd, rb, rp, rbp

   returns the frame bias and precession, IAU 2000.  <jd> are the
   Julian Dates for which results are desired.  <rb> receives the
   frame bias maxtrix, <rp> the precession matrix, and <rbp> the
   bias-precession matrix. */
BIND(iauBp00, id0oC33c33c33, s, IAUBP00, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* IAUBP06, jd, rb, rp, rbp

   returns the frame bias and precession, IAU 2006.  <jd> are the
   Julian Dates for which results are desired.  <rb> receives the
   frame bias matrix, <rp> the precession matrix, and <rbp> the
   bias-precession matrix.  */
BIND(iauBp06, id0oC33c33c33, s, IAUBP06, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* IAUBPN2XY, rbpn, x, y

   extracts the x,y coordinates of the Celestial Intermediate Pole
   from the bias-precesion-nutation matrix. */
BIND(iauBpn2xy, ic33odd, s, IAUBPN2XY, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUC2I00A(jd)

 Returns the celestial-to-intermedia matrix for the Julian Dates <jd>
 using the IAU 2000A precession-nutation model. */
BIND(iauC2i00a, id0oc33, f, IAUC2I00A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUC2I00B(jd)

 Returns the celestial-to-intermedia matrix for the Julian Dates <jd>
 using the IAU 2000B precession-nutation model. */
BIND(iauC2i00b, id0oc33, f, IAUC2I00B, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUC2I06A(jd)

 Returns the celestial-to-intermedia matrix for the Julian Dates <jd>
 using the IAU 2006 precession model and IAU 2000A nutation model. */
BIND(iauC2i06a, id0oc33, f, IAUC2I06A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* rc2i = IAUC2IBPN(jd, rbpn)

   Forms the celestial-to-intermediate matrix <rc2i> for Julian Dates
   <jd>, given the bias-precession-nutation matrix <rbpn>. */
BIND(iauC2ibpn, id0c33oc33, f, IAUC2IBPN, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUC2IXY(jd, x, y)

   returns the celestial-to-intermediate-frame-of-date matrix for
   Julian Date <jd> and CIP x, y coordinates.
 */
BIND(iauC2ixy, id0ddoc33, f, IAUC2IXY, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* rc2i = IAUC2IXYS(x, y, s) 

   returns the celestial-to-intermediate-frame-of-date matrix given
   the CIP x, y and the CIO locator s.
 */
BIND(iauC2ixys, idddoc33, f, IAUC2IXYS, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* angles = IAUC2S(p)

   Translates p-vector <p> to spherical coordinates.
   <angles(0,/all)> = longitude angle (radians)
   <angles(1,/all)> = latitude angle (radians)
   */
BIND(iauC2s, ib3rb2, f, IAUC2S, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* rc2t = IAUC2T00A(jdtt, jdut, x, y)

   Return the celestial-to-terrestrial matrix given <jdtt>, <jdut>,
   and the polar coordinates <x>, <y>, using the IAU2000A nutation
   model.  <jdtt> and <jdut> should express the same instants in time,
   in the TT and UT1 timescales, respectively */
BIND(iauC2t00a, id0d0ddoc33_mod, f, IAUC2T00A, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* rc2t = IAUC2T00B(jdtt, jdut, x, y)

   Return the celestial-to-terrestrial matrix given <jdtt>, <jdut>,
   and the polar coordinates <x>, <y>, using the IAU2000B nutation
   model.  <jdtt> and <jdut> should express the same instants in time,
   in the TT and UT1 timescales, respectively */
BIND(iauC2t00b, id0d0ddoc33_mod, f, IAUC2T00B, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* rc2t = IAUC2T06A(jd, x, y)

   Return the celestial-to-terrestrial matrix given <jdtt>, <jdut>,
   and the polar coordinates <x>, <y>, using the IAU 2006 precession
   model and the IAU 2000A nutation model.  <jdtt> and <jdut> should
   express the same instants in time, in the TT and UT1 timescales,
   respectively */
BIND(iauC2t06a, id0d0ddoc33_mod, f, IAUC2T06A, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* IAUC2TCIO(rc2i, era, rpom)

   Returns the celestial-to-terrestrial matrix based on the CIO-based
   components (the celestial-to-intermediate matrix <rc2i>, the Earth
   Rotation Angle <era>, and the polar motion matrix <rpom>) */
BIND(iauC2tcio, ic33dc33oc33, f, IAUC2TCIO, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUC2TEQX(rbpn, gst, rpom)

   Returns the celestial-to-terrestrial matrix based on the
   celestial-to-true matrix <rbpn>, the Greenwich Apparent Sidereal
   Time <gst>, and the polar motion matrix <rpom>) */
BIND(iauC2teqx, ic33dc33oc33, f, IAUC2TEQX, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUC2TPE(jd, dpsi, deps, xp, yp)

   Returns the celestial-to-terrestrial matrix given the Julian Date
   <jd>, the nutation <dpsi>, <deps>, and the polar coordinates <xp>,
   <yp> */
BIND(iauC2tpe, id000ddddoc33, f, IAUC2TPE, 5, 5, NULL)
/*-----------------------------------------------------------------------*/
/* IAUC2TXY(jd, x, y, xp, yp)

   Returns the celestial-to-terrestrial matrix given the Julian Date
   <jd>, the CIP coordinates <x>, <y>, and the polar motion <xp>,
   <yp> */
BIND(iauC2txy, id000ddddoc33, f, IAUC2TXY, 5, 5, NULL)
/*-----------------------------------------------------------------------*/
/* IAUCAL2JD(<caldates>)

   returns the Julian Dates corresponding to the specified Gregorian
   calendar dates <caldates>, which must have 3 elements in its first
   dimension (year, month, day).  Unexpected results may be given if
   illegal year, month, or day numbers are specified.  Dates before
   -4799-01-01 or after 1465073-02-28 are rejected. */
int ana_iauCal2jd(int narg, int ps[])
{
  loopInfo srcinfo, tgtinfo;
  pointer src, tgt;
  double djm0, djm;
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>L3*;r>L-3*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  src = ptrs[0];
  tgt = ptrs[1];
  srcinfo = infos[0];
  tgtinfo = infos[1];

  /* iauCal2jd(int iy, int im, int id, double *djm0, double *djm) */
  switch (infos[0].type) {
  case ANA_LONG:
    do {
      if (iauCal2jd(src.l[0], src.l[1], src.l[2], &djm0, &djm))
        *tgt.l = 0;
      else
        *tgt.l = 2400000.5 + djm;
      src.l += 3;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_FLOAT:
    do {
      int day = floor(src.f[2]);
      double daypart = src.f[2] - day;
      if (iauCal2jd((int) src.f[0], (int) src.f[1], day, &djm0, &djm))
        *tgt.f = 0;
      else
        *tgt.f = 2400000.5 + djm + daypart;
      src.f += 3;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_DOUBLE:
    do {
      int day = floor(src.d[2]);
      double daypart = src.d[2] - day;
      if (iauCal2jd((int) src.d[0], (int) src.d[1], day, &djm0, &djm))
        *tgt.d = 0;
      else
        *tgt.d = 2400000.5 + djm + daypart;
      src.d += 3;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  default:
    return cerror(ILL_TYPE, ps[0]);
  }
  return iq;
}
REGISTER(iauCal2jd, f, IAUCAL2JD, 1, 1, NULL);
/*-----------------------------------------------------------------------*/
/* IAUDAT(date) returns delta(AT) = TAI - UTC for the given UTC date
   ([year, month, day]) */
int ana_iauDat(int narg, int ps[])
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if (internalMode & 1) {       /* /VALID */
    time_t t;
    int jdlo, jdhi, y, m, d, shi, slo;
    double f, dt;

    /* determine the last date of validity of the current implementation */

    jdlo = 2441318;         /* 1972-01-01 */
    t = time(NULL);
    jdhi = (double) t/86400.0 + 2440587.5 + 10000;
    iauJd2cal(jdlo, 0.0, &y, &m, &d, &f);
    slo = iauDat(y, m, d, f, &dt);
    iauJd2cal(jdhi, 0.0, &y, &m, &d, &f);
    shi = iauDat(y, m, d, f, &dt);
    do {
      int jd, s;
      jd = (jdhi + jdlo)/2;
      iauJd2cal(jd, 0.0, &y, &m, &d, &f);
      s = iauDat(y, m, d, f, &dt);
      if (s) {
        jdhi = jd;
        shi = s;
      } else {
        jdlo = jd;
        slo = s;
      }
    } while (jdhi - jdlo > 1);
    iq = scalar_scratch(ANA_LONG);
    scalar_value(iq).l = jdlo;
    return iq;
  } else {
    if (narg < 2)
      return anaerror("Need 1 argument", ps[0]);
    if ((iq = standard_args(narg, ps, "i>L3*;rD-3*", &ptrs, &infos)) < 0)
      return ANA_ERROR;
    switch (infos[0].type) {
    case ANA_LONG:
      while (infos[1].nelem--) {
        iauDat(ptrs[0].l[0], ptrs[0].l[1], ptrs[0].l[2], 0.0, ptrs[1].d++);
        ptrs[0].l += 3;
      }
      break;
    case ANA_FLOAT:
      while (infos[1].nelem--) {
        int d = (int) floor(ptrs[0].f[2]);
        double f = ptrs[0].f[2] - d;
        iauDat((int) ptrs[0].f[0], (int) ptrs[0].f[1], d, f, ptrs[1].d++);
        ptrs[0].f += 3;
      }
      break;
    case ANA_DOUBLE:
      while (infos[1].nelem--) {
        int d = (int) floor(ptrs[0].d[2]);
        double f = ptrs[0].d[2] - d;
        iauDat((int) ptrs[0].d[0], (int) ptrs[0].d[1], d, f, ptrs[1].d++);
        ptrs[0].d += 3;
      }
      break;
    }
  }
  return iq;
}
REGISTER(iauDat, f, IAUDAT, 0, 1, "1VALID");
/*-----------------------------------------------------------------------*/
/* IAUDTDB(jd, elong, u, v)

   Returns an approximation of TDB-TT in seconds, the difference
   between barycentric dynamical time and terrestrial time, for an
   observer on the Earth. <jd> is the Julian Date in TDB or TT,
   <elong> is the east longitude in radians, <u> is the distance in km
   from the Earth spin axis, and <v> is the distance in km north of
   the equatorial plane. */
BIND(iauDtdb, id00DDDrd, f, IAUDTDB, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEE00(<jd>, <epsa>, <dpsi>)

   Returns the equation of the equinoxes (IAU 2000) given the nutation
   in longitude <epsa> and the mean obliquity <dpsi> */
BIND(iauEe00, id0ddrd, f, IAUEE00, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEE00A(<jd>)

   Returns the equation of the equinoxes, compatible with IAU 2000
   resolutions */
BIND(iauEe00a, iddrd, f, IAUEE00A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEE00B(<jd>)

   Returns the equation of the equinoxes, compatible with IAU 2000
   resolutions but using the truncated nutation model IAU 2000B */
BIND(iauEe00b, iddrd, f, IAUEE00B, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEE06A(<jd>)

   Returns the equation of the equinoxes, compatible with IAU 2000
   resolutions and the IAU 2006/200A precession-nutation */
BIND(iauEe06a, iddrd, f, IAUEE06A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEECT00(<jd>)

   Returns the equation of the equinoxes complementary term,
   consistent with IAU 2000 resolutions. */
BIND(iauEect00, iddrd, f, IAUEECT00, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEFORM(<ellid>)

   Returns the equatorial radius (in meters) and the flattening of the
   Earth reference ellipsoid with the given ellipsoid identifier
   <ellid>: 1 = WGS84, 2 = GRS80, 3 = WGS72.  If an unsupported
   <ellid> is specified, then zero radius and flattening are
   returned. */
BIND(iauEform, ilob2rl, s, IAUEFORM, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEO06A(<jd>)

   Returns the equation of the origins, using IAU 2006 precession and
   IAU 2000A nutation. */
BIND(iauEo06a, iddrd, f, IAUEO06A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEORS(<rnpb>, <s>) 

   Returns the equation of the origins, given the classical NPB matrix
   <rnpb> and the CIO locator <s> */
BIND(iauEors, ic33drd, f, IAUEORS, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEPB(<jd>)

   Returns the Besselian epoch corresponding to Julian Date <jd>. */
BIND(iauEpb, iddrd, f, IAUEPB, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEPB2JD(<bepoch>)

   Returns the Julian Date corresponding to Besselian epoch <bepoch>
   (e.g., 1957.3). */
int ana_iauEpb2jd(int narg, int ps[])
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    double djm0, djm;
    
    iauEpb2jd(*ptrs[0].d++, &djm0, &djm);
    *ptrs[1].d++ = djm0 + djm;
  }
  return iq;
}
REGISTER(iauEpb2jd, f, IAUEPB2JD, 1, 1, NULL);
/*-----------------------------------------------------------------------*/
/* IAUEPJ(<jd>)

   Returns the Julian epoch corresponding to Julian Date <jd>. */
BIND(iauEpj, iddrd, f, IAUEPJ, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEPJ2JD(<bepoch>)

   Returns the Julian Date corresponding to Julian epoch <bepoch>
   (e.g., 1957.3). */
int ana_iauEpj2jd(int narg, int ps[])
{
  pointer *ptrs;
  loopInfo *infos;
  int iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD*", &ptrs, &infos)) < 0)
    return ANA_ERROR;
  while (infos[0].nelem--) {
    double djm0, djm;
    
    iauEpj2jd(*ptrs[0].d++, &djm0, &djm);
    *ptrs[1].d++ = djm0 + djm;
  }
  return iq;
}
REGISTER(iauEpj2jd, f, IAUEPJ2JD, 1, 1, NULL);
/*-----------------------------------------------------------------------*/
/* IAUEPV00, jd, pvh, pvb

   Returns the Earth position and velocity, heliocentric (pvh) and
   barycentric (pvb), with respect to the Barycentric Celestial
   Reference System.  pvh(0,*) = heliocentric position (AU), pvh(1,*)
   = heliocentric velocity (AU/d), pvb(0,*) = barycentric position
   (AU), pvb(1,*) = barycentric velocity (AU/d) */
BIND(iauEpv00, id0c23c23rl, s, IAUEPV00, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUEQEQ94(<jd>)

   Returns the equation of the equinoxes according to the IAU 1994 model.
 */
BIND(iauEqeq94, iddrd, f, IAUEQEQ94, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUERA00(<jd>)

   Returns the Earth rotation angle according to the IAU 2000 model.
*/
BIND(iauEra00, iddrd, f, IAUERA00, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAD03(<t>)

   Returns the mean elongation of the Moon from the Sun, a fundamental
   argument according to the IERS conventions of 2003, as a function
   of the number <t> of Julian centuries since J2000.0 TDB.  It makes
   no practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFad03, idrd, f, IAUFAD03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAE03(<t>)

   Returns the mean longitude of the Earth, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFae03, idrd, f, IAUFAE03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAF03(<t>)

   Returns the mean longitude of the Moon minus the mean longitude of
   the ascending node, a fundamental argument according to the IERS
   conventions of 2003, as a function of the number <t> of Julian
   centuries since J2000.0 TDB.  It makes no practical difference if
   <t> is measured since J2000.0 TT instead. */
BIND(iauFaf03, idrd, f, IAUFAF03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAJU03(<t>)

   Returns the mean longitude of Jupiter, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFaju03, idrd, f, IAUFAJU03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAL03(<t>)

   Returns the mean anomaly of the Moon, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFal03, idrd, f, IAUFAL03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFALP03(<t>)

   Returns the mean anomaly of the Sun, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFalp03, idrd, f, IAUFALP03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAMA03(<t>)

   Returns the mean longitude of Mars, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFama03, idrd, f, IAUFAMA03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAME03(<t>)

   Returns the mean longitude of Mercury, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFame03, idrd, f, IAUFAME03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFANE03(<t>)

   Returns the mean longitude of Neptune, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFane03, idrd, f, IAUFANE03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAOM03(<t>)

   Returns the mean longitude of the Moon's ascending node, a
   fundamental argument according to the IERS conventions of 2003, as
   a function of the number <t> of Julian centuries since J2000.0 TDB.
   It makes no practical difference if <t> is measured since J2000.0
   TT instead. */
BIND(iauFaom03, idrd, f, IAUFAOM03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAPA03(<t>)

   Returns the general accumulated precession in longitude, a
   fundamental argument according to the IERS conventions of 2003, as
   a function of the number <t> of Julian centuries since J2000.0 TDB.
   It makes no practical difference if <t> is measured since J2000.0
   TT instead. */
BIND(iauFapa03, idrd, f, IAUFAPA03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFASA03(<t>)

   Returns the mean longitude of Saturn, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFasa03, idrd, f, IAUFASA03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAUR03(<t>)

   Returns the mean longitude of Uranus, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFaur03, idrd, f, IAUFAUR03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFAVE03(<t>)

   Returns the mean longitude of Venus, a fundamental argument
   according to the IERS conventions of 2003, as a function of the
   number <t> of Julian centuries since J2000.0 TDB.  It makes no
   practical difference if <t> is measured since J2000.0 TT
   instead. */
BIND(iauFave03, idrd, f, IAUFAVE03, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFK52H, <ra5>, <dec5>, <dra5>, <ddec5>, <px5>, <rv5>, <rah>,
   <dech>, <drah>, <ddech>, <pxh>, <rvh>

   Transforms FK5 (J2000.0) star data into the Hipparcos system.  The
   FK5 data is: right ascension <ra5>, declination <dec5>, proper
   motion in right ascension <dra5>, proper motion in declination
   <ddec5>, parallax <px5>, radial velocity <rv5>.  The Hipparcos data
   is: right ascension <rah>, declination <dech>, proper motion in
   right ascension <drah>, proper motion in declination <ddech>,
   parallax <pxh>, radial velocity <rvh>. */
BIND(iauFk52h, iddddddodddddd, s, IAUFK52H, 12, 12, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFK5HIP, <r5h>, <s5h>

   Returns the FK5-to-Hipparcos rotation <r5h> and spin <s5h> */
BIND(iauFk5hip, oC33B3, s, IAUFK5HIP, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFK5HZ, <ra5>, <dec5>, <jd>, <rah>, <dech>

   Transforms an FK5 (J2000.0) star position (right ascension <ra5>,
   declination <dec5>) determined at Julian Date <jd> into the system
   of the Hipparcos catalog, assuming zero Hipparcos proper motion.
   The returned coordinates in the Hipparcos system are right
   ascension <rah>, declination <dech> */
BIND(iauFk5hz, iddd0odd, s, IAUFK5HZ, 5, 5, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFW2M(<gamma_bar>, <phi_bar>, <psi>, <eps>)

   Forms a rotation matrix given the Fukushima-Williams angles
   <gamma_bar>, <phi_bar>, <psi>, and <eps> */
BIND(iauFw2m, iddddoc33, f, IAUFW2M, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* IAUFW2XY, <gamma_bar>, <phi_bar>, <psi>, <eps>, <x>, <y> 

   Returns CIP <x>, <y>, given the Fukushima-Williams
   bias-precession-nutation angles <gamma_bar>, <phi_bar>, <psi>,
   <eps>. */
BIND(iauFw2xy, iddddodd, s, IAUFW2XY, 6, 6, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGC2GD(<xyz>, <ellid>)
 
   Returns geodetic coordinates (elongation, phi, height)
   corresponding to geocentric coordinates <xyz> using the specified
   reference ellipsoid <ellid> */
BIND(iauGc2gd, iLb3od3rl, f, IAUGC2GD, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGC2GDE(<xyz>, <a>, <f>)

   Returns geodetic coordinates (elongation, phi, height)
   corresponding to geocentric coordinates <xyz> using the reference
   ellipsoid with equatorial radius <a> and flattening <f> */
BIND(iauGc2gde, ib3DDod3rl, f, IAUGC2GDE, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGD2GC(<eph>, <ellid>)

   Returns geocentric coordinates (x, y, z) corresponding to geodetic
   coordinates <eph> (elongation, phi, height) for the specified
   reference ellipsoid <ellid> */
BIND(iauGd2gc, iLd3ob3rl, f, IAUGD2GC, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGD2GCE(<eph>, <a>, <f>)

   Returns geocentric coordinates (x, y, z) corresponding to geodetic
   coordinates <eph> (elongation, phi, height) for the reference
   ellipsoid with equatorial radius <a> and flattening <f> */
BIND(iauGd2gce, id3DDob3rl, f, IAUGD2GCE, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGMST06(<jdut>, <jdtt>)

   Returns Greenwich mean sidereal time according to a model that is
   consistent with IAU 2006 precession.  <jdut> is the Julian Date
   (UT1).  <jdtt> is the Julian Date (TT) of the same instant. <jdut>
   is needed to predict the Earth rotation, and TT to predict the
   effects of precession.  If <jdtt> is set equal to <jdut> then
   errors of order 100 microarcseconds result. */
BIND(iauGmst06, iddddrd_mod, f, IAUGMST06, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGMST82(<jd>)
 
   Returns Greenwich mean sidereal time for the given UT1 Julian Date
   <jd>, according to the IAU 1982 model */
BIND(iauGmst82, id0rd, f, IAUGMST82, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGST00A(<jdut>, <jdtt>)

   Returns Greenwich apparent sidereal time consisten with IAU 2000
   resolutions.  <jdut> is the Julian Date (UT1).  <jdtt> is the
   Julian Date (TT) of the same instant. <jdut> is needed to predict
   the Earth rotation, and TT to predict the effects of precession.
   If <jdtt> is set equal to <jdut> then errors of order 100
   microarcseconds result. */
BIND(iauGst00a, iddddrd_mod, f, IAUGST00A, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGST00B(<jdut>, <jdtt>)

   Returns Greenwich apparent sidereal time consistent with IAU 2000
   resolutions but using the truncated nutation model IAU 2000B.
   <jdut> is the Julian Date (UT1).  <jdtt> is the Julian Date (TT) of
   the same instant. <jdut> is needed to predict the Earth rotation,
   and TT to predict the effects of precession.  If <jdtt> is set
   equal to <jdut> then errors of order 100 microarcseconds result. */
BIND(iauGst00b, id0rd, f, IAUGST00B, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGST06(<jdut>, <jdtt>, <rnpb>)

   Returns Greenwich apparent sidereal time consistent with IAU 2006
   resolutions, given the nutation-precession-bias matrix
   <rnpb>. <jdtt> is the Julian Date (TT) of the same instant. <jdut>
   is needed to predict the Earth rotation, and TT to predict the
   effects of precession.  If <jdtt> is set equal to <jdut> then
   errors of order 100 microarcseconds result. */
BIND(iauGst06, iddddc33rd_mod, f, IAUGST06, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGST06A(<jdut>, <jdtt>)

   Returns Greenwich apparent sidereal time consistent with IAU 2000
   and IAU 2006 resolutions.  <jdut> is the Julian Date (UT1).  <jdtt>
   is the Julian Date (TT) of the same instant. <jdut> is needed to
   predict the Earth rotation, and TT to predict the effects of
   precession.  If <jdtt> is set equal to <jdut> then errors of order
   100 microarcseconds result. */
BIND(iauGst06a, iddddrd_mod, f, IAUGST06A, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUGST94(<jd>)
 
   Returns Greenwich apparent sidereal time for the given UT1 Julian
   Date <jd>, consistent with IAU 1982/94 resolutions */
BIND(iauGst94, id0rd, f, IAUGST94, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUH2FK5, <rah>, <dech>, <drah>, <ddech>, <pxh>, <rvh>, <ra5>,
   <dec5>, <dra5>, <ddec5>, <px5>, <rv5>

   Transforms Hipparcos star data into the FK5 (J2000.0) system.  The
   Hipparcos data is: right ascension <rah>, declination <dech>,
   proper motion in right ascension <drah>, proper motion in
   declination <ddech>, parallax <pxh>, radial velocity <rvh>.  The
   FK5 data is: right ascension <ra5>, declination <dec5>, proper
   motion in right ascension <dra5>, proper motion in declination
   <ddec5>, parallax <px5>, radial velocity <rv5>. */
BIND(iauH2fk5, iddddddodddddd, s, IAUH2FK5, 12, 12, NULL)
/*-----------------------------------------------------------------------*/
/* IAUHFK5Z, <rah>, <dech>, <jd>, <ra5>, <dec5>

   Transforms a Hipparcos star position (right ascension <rah>,
   declination <dech>) determined at Julian Date <jd> into the FK5
   (J2000.0) system, assuming zero Hipparcos proper motion.  The
   returned coordinates in the FK5 system are right ascension <ra5>,
   declination <dec5> */
BIND(iauHfk5z, iddd0odddd, s, IAUHFK5Z, 5, 5, NULL)
/*-----------------------------------------------------------------------*/
/* IAUIR()

   Returns an identity r-matrix */
BIND(iauIr, oC33, f, IAUIR, 0, 0, NULL)
/*-----------------------------------------------------------------------*/
/* IAUJD2CAL(<jd>) returns the Gregorian calendar dates corresponding
   to the specified Julian Dates <jd>.  The return value has the same
   dimensions as <jd> but with one extra dimension of size 3 prefixed,
   for the year, month, and day, respectively.  Zeros are returned for
   input JD outside of the supported range.  Julian day numbers before
   -68569.5 or after 1e9 are considered illegal -- actually, JDs
   greater than 536802342 yield erroneous results, but the SOFA
   routine does not indicate problems until the JD exceeds 1e9. */
BIND(iauJd2cal, id0ollldrl_cal, f, IAUJD2CAL, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUNUM00A(<jd>)

   Returns the matrix for nutation for a given Julian Date <jd>,
   according to the IAU 2000A model. */
BIND(iauNum00a, id0oc33, f, IAUNUM00A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUNUM00B(<jd>)

   Returns the matrix for nutation for a given Julian Date <jd>,
   according to the IAU 2000B model. */
BIND(iauNum00b, id0oc33, f, IAUNUM00B, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUNUM06A(<jd>)

   Returns the matrix for nutation for a given Julian Date <jd>,
   according to the IAU 2006/2000A model. */
BIND(iauNum06a, id0oc33, f, IAUNUM06A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUNUMAT(<epsa>, <dpsi>, <deps>)

   Returns the matrix of nutation based on the mean obliquity of the
   date <epsa> and the nutation components <dpsi> and <deps>. */
BIND(iauNumat, iDDoC33, f, IAUNUMAT, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUNUT00A, <jd>, <dpsi>, <deps>

   Returns the nutation for Julian Date <jd> according to the IAU
   2000A model (MHB2000 luni-solar and planetary nutation with fre
   core nutation omitted).  <dpsi> and <deps> are the returned
   nutation components (luni-solar + planetary) */
BIND(iauNut00a, id0odd, s, IAUNUT00A, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUNUT00B, <jd>, <dpsi>, <deps>

   Returns the nutation for Julian Date <jd> according to the IAU
   2000B model.  <dpsi> and <deps> are the returned nutation
   components (luni-solar + planetary) */
BIND(iauNut00b, id0odd, s, IAUNUT00B, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUNUT06A, <jd>, <dpsi>, <deps>

   Returns the nutation for Julian Date <jd> according to the IAU
   2000A model, with adjustments to match the IAU 2006 precession.
   <dpsi> and <deps> are the returned nutation components (luni-solar
   + planetary) */
BIND(iauNut06a, id0odd, s, IAUNUT06A, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUNUT80, <jd>, <dpsi>, <deps>

   Returns the nutation for Julian Date <jd> according to the IAU 1980
   model.  <dpsi> and <deps> are the returned nutation components
   (luni-solar + planetary) */
BIND(iauNut80, id0odd, s, IAUNUT80, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUNUTM80(<jd>)

   Returns the nutation matrix for Julian Date <jd>, according to the
   IAU 1980 model.
 */
BIND(iauNutm80, id0oc33, f, IAUNUTM80, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUOBL06(<jd>)

   Returns the mean obliquity of the ecliptic according to the IAU
   2006 precession model.
 */
BIND(iauObl06, id0rd, f, IAUOBL06, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUOBL80(<jd>)

   Returns the mean obliquity of the ecliptic according to the IAU
   1980 model. */
BIND(iauObl80, id0rd, f, IAUOBL80, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUP06E, <jd>, <eps0>, <psia>, <oma>, <bpa>, <bqa>, <pia>, <bpia>,
   <epsa>, <chia>, <za>, <zetaa>, <thetaa>, <pa>, <gam>, <phi>, <psi>

   Returns IAU 2006 equinox-based precession angles for Julian Date
   <jd>.  The returned precession angles are: epsilon_0, psi_A,
   omega_A, P_A, Q_A, pi_A, Pi_A, obliquity epsilon_A, chi_A, z_A,
   zeta_A, theta_A, p_A, F-W angle gamma_J2000, F-W angle phi_J2000,
   F-W angle psi_J2000.
 */
BIND(iauP06e, id0odddddddddddddddd, s, IAUP06E, 17, 17, NULL)
/*-----------------------------------------------------------------------*/
/* IAUP2S(<p>)

   Returns the spherical polar coordinates (theta, phi, r)
   corresponding to the p-vector <p> */
BIND(iauP2s, ib3od3, f, IAUP25, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPAP(<ref>, <tgt>)

   Returns the position angle of p-vector <tgt> from p-vector <ref>
*/
BIND(iauPap, ib3b3rd, f, IAUPAP, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPAS(<al>, <ap>, <bl>, <bp>)

   Returns the position angle of B with respect to A.  <al> =
   longitude of point A in radians, <ap> = latitude of point A in
   radians, <bl> = longitude of point B in radians, <bp> = latitude of
   point B in radians
*/
BIND(iauPas, iddddrd, f, IAUPAS, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPB06, <jd>, <bzeta>, <bz>, <btheta>

   Returns three Euler angles which implement general precesion from
   epoch J2000.0, using the IAU 2006 model, for Julian Date <jd>.
   Frame bias (the offset between ICRS and J2000.0) is included. */
BIND(iauPb06, id0oddd, s, IAUPB06, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPDP(<a>, <b>)

   Returns the inner product of the two p-vectors <a> and <b> */
BIND(iauPdp, ib3b3rd, f, IAUPDP, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPFW06, <jd>, <gamb>, <phib>, <psib>, <epsa>

   Returns the IAU 2006 precession angles in Fukushima-Williams
   4-angle formulation. */
BIND(iauPfw06, id0odddd, s, IAUPFW06, 5, 5, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPLAN94(<jd>, <planet>)

   Returns the approximate heliocentric position and velocity at
   Julian Date <jd> of major planet <planet> (1 = Mercury, 2 = Venus,
   3 = EMB, 4 = Mars, ..., 8 = Neptune) 
*/
BIND(iauPlan94, id0Loc23rl, f, IAUPLAN94, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPM(<p>)

   Returns the modulus of the p-vector <p>
*/
BIND(iauPm, id3rd, f, IAUPM, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPMAT00(<jd>)

   Returns the precession matrix (including frame bias) from GCRS to a
   specified Julian Date <jd> according to the IAU 2000 model.
*/
BIND(iauPmat00, id0oc33, f, IAUPMAT00, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPMAT06(<jd>)

   Returns the precession matrix (including frame bias) from GCRS to a
   specified Julian Date <jd> according to the IAU 2006 model.
*/
BIND(iauPmat06, id0oc33, f, IAUPMAT06, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPMAT76(<jd>)

   Returns the precession matrix from J2000.0 to a specified Julian
   Date <jd> according to the IAU 1976 model. */
BIND(iauPmat76, id0oc33, f, IAUPMAT76, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPN, <p>, <r>, <u>

   Converts p-vector <p> into a modulus <r> and unit vector <u> */
BIND(iauPn, ib3odb3, s, IAUPN, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPN00, <jd>, <dpsi>, <deps>, <epsa>, <rb>, <rp>, <rbp>, <rn>, <rbpn>

   Returns precession-nutation according to the IAU 2000 model for the
   specified Julian Date <jd> and nutation <dpsi>, <deps>.  The output
   parameters are the mean obliquity <epsa>, the frame bias matrix
   <rb>, the precession matrix <rp>, the bias-precession matrix <rbp>,
   the nutation matrix <rn>, and the GCRS-to-true matrix <rbpn>. */
BIND(iauPn00, id0ddodc33c33c33c33c33, s, IAUPN00, 9, 9, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPN00A, <jd>, <dpsi>, <deps>, <epsa>, <rb>, <rp>, <rbp>, <rn>, <rbpn>

   Returns precession-nutation according to the IAU 200A model for the
   specified Julian Date <jd> and nutation <dpsi>, <deps>.  The output
   parameters are the mean obliquity <epsa>, the frame bias matrix
   <rb>, the precession matrix <rp>, the bias-precession matrix <rbp>,
   the nutation matrix <rn>, and the GCRS-to-true matrix <rbpn>. */
BIND(iauPn00a, id0ddodc33c33c33c33c33, s, IAUPN00A, 9, 9, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPN00B, <jd>, <dpsi>, <deps>, <epsa>, <rb>, <rp>, <rbp>, <rn>, <rbpn>

   Returns precession-nutation according to the IAU 200A model for the
   specified Julian Date <jd> and nutation <dpsi>, <deps>.  The output
   parameters are the mean obliquity <epsa>, the frame bias matrix
   <rb>, the precession matrix <rp>, the bias-precession matrix <rbp>,
   the nutation matrix <rn>, and the GCRS-to-true matrix <rbpn>. */
BIND(iauPn00b, id0ddodc33c33c33c33c33, s, IAUPN00B, 9, 9, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPN06, <jd>, <dpsi>, <deps>, <epsa>, <rb>, <rp>, <rbp>, <rn>, <rbpn>

   Returns precession-nutation according to the IAU 2006 model for the
   specified Julian Date <jd> and nutation <dpsi>, <deps>.  The output
   parameters are the mean obliquity <epsa>, the frame bias matrix
   <rb>, the precession matrix <rp>, the bias-precession matrix <rbp>,
   the nutation matrix <rn>, and the GCRS-to-true matrix <rbpn>. */
BIND(iauPn06, id0ddodc33c33c33c33c33, s, IAUPN06, 9, 9, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPN06A, <jd>, <dpsi>, <deps>, <epsa>, <rb>, <rp>, <rbp>, <rn>, <rbpn>

   Returns precession-nutation according to the IAU 2006/2000A models
   for the specified Julian Date <jd> and nutation <dpsi>, <deps>.
   The output parameters are the mean obliquity <epsa>, the frame bias
   matrix <rb>, the precession matrix <rp>, the bias-precession matrix
   <rbp>, the nutation matrix <rn>, and the GCRS-to-true matrix
   <rbpn>. */
BIND(iauPn06a, id0ddodc33c33c33c33c33, s, IAUPN06A, 9, 9, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPNM00A(<jd>)

   Returns the matrix of precession-nutation (including frame bias)
   for Julian Date <jd>, equinox-based, IAU 2000A model. */
BIND(iauPnm00a, id0oc33, f, IAUPNM00A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPNM00B(<jd>)

   Returns the matrix of precession-nutation (including frame bias)
   for Julian Date <jd>, equinox-based, IAU 2000B model. */
BIND(iauPnm00b, id0oc33, f, IAUPNM00B, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPNM06A(<jd>)

   Returns the matrix of precession-nutation (including frame bias)
   for Julian Date <jd>, equinox-based, IAU 2006 precession and IAU
   2000A nutation models. */
BIND(iauPnm06a, id0oc33, f, IAUPNM06A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPNM80(<jd>)

   Returns the matrix of precession-nutation (including frame bias)
   for Julian Date <jd>, equinox-based, IAU 1976 precession and IAU
   1980 nutation models. */
BIND(iauPnm80, id0oc33, f, IAUPNM80, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPOM00(<xp>, <yp>, <sp>)

   Returns the matrix of polar motion, IAU 2000, given the coordinates
   <xp>, <yp> of the pole and TIO locator <sp>. */
BIND(iauPom00, idddoc33, f, IAUPOM00, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPR00, <jd>, <dpsipr>, <depspr>

   Returns the precession-rate part of the IAU 2000
   precession-nutation models (part of MHB2000). */
BIND(iauPr00, id0odd, s, IAUPR00, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPREC76, <jd1>, <jd2>, <zeta>, <z>, <theta>

   Returns the three Euler angles <zeta>, <z>, <theta> which implement
   general precession between epochs (Julian Dates) <jd1> and <jd2>,
   using the IAU 1976 model (as for the FK5 catalog).  <zeta> is the
   1st rotation angle in radians clockwise around the z axis, <z> the
   3rd rotation angle in radians clockwise around the z axis, and
   <theta> the 2nd rotation angle in radians counterclockwise around
   the y axis. */
BIND(iauPrec76, id0d0oddd, s, IAUPREC76, 5, 5, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPV2S, <pv>, <theta>, <phi>, <r>, <td>, <pd>, <rd>

   Converts position/velocity vector <pv> from Cartesian to spherical
   coordinates.  <theta> = longitude angle, <phi> = latitude angle,
   <r> = radial distance, <td> = rate of change of theta, <pd> = rate
   of change of <phi>, <rd> = rate of change of <r> */
BIND(iauPv2s, ic23odddddd, s, IAUPV25, 7, 7, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPVDPV(<a>, <b>)

   Returns the inner product of pv-vectors <a> and <b> */
BIND(iauPvdpv, ic23c23ob2, f, IAUPVDPV, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPVM, <pv>, <r>, <s>

   Returns the modulus of pv-vector <pv>.  <r> is the modulus of the
   position component, and <s> is the modulus of the velocity
   component. */
BIND(iauPvm, ic23odd, s, IAUPVM, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPVSTAR, <pv>, <ra>, <dec>, <pmr>, <pmd>, <px>, <rv>

   Converts position-velocity matrix <pv> to catalog coordinates right
   ascension <ra> (rad), declination <dec> (rad), right ascension
   proper motion <pmr> in rad/a, declination proper motion <pmd>
   (rad/a), parallax <px> (arcsec), and radial velocity <rv> (km/s
   receding) */
BIND(iauPvstar, ic23oddddddrl, s, IAUPVSTAR, 7, 7, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPVXPV(<a>, <b>)

   Returns the outer product of two pv-vectors <a> and <b> */
BIND(iauPvxpv, ic23c23oc23, f, IAUPVXPV, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUPXP(<a>, <b>)

   Returns the outer product of two p-vectors <a> and <b> */
BIND(iauPxp, ib3b3ob3, f, IAUPXP, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAURM2V(<r>)

   Returns the rotation vector corresponding to rotation matrix <r>.
   The rotation vector has the same direction as the Euler axis of
   matrix <r> and has its magnitude equal to the rotation angle in
   radians of the rotation matrix. */
BIND(iauRm2v, ic33ob3, f, IAURM2V, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAURV2M(<w>)

   Returns the rotation matrix corresponding to rotation vector <w>.
   The rotation matrix Euler axis gets the direction of <w>, and the
   rotation matrix gets a rotation angle equal to the modulus of
   <w>. */
BIND(iauRv2m, ib3oc33, f, IAURV2M, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAURX, <phi>, <r>

   Rotates r-matrix <r> about the x axis over angle <phi> */
BIND(iauRx, idc33, s, IAURX, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAURXPV(<r>, <pv>)

   Returns the matrix product of r-matrix <r> and pv-vector <pv> */
BIND(iauRxpv, ic33c23oc23, f, IAURXPV, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAURY, <phi>, <r>

   Rotates r-matrix <r> about the y axis over angle <phi> */
BIND(iauRy, idc33, s, IAURY, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAURZ, <phi>, <r>

   Rotates r-matrix <r> about the z axis over angle <phi> */
BIND(iauRz, idc33, s, IAURZ, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUS00(<jd>, <x>, <y>)

   Returns the CIO locator for the specified Julian Date <jd> given
   the CIP's <x> and <y> coordinates, using IAU 2000A
   precession-nutation */
BIND(iauS00, id0ddrd, f, IAUS00, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUS00A(<jd>)

   Returns the CIO locator for the specified Julian Date <jd>, using
   IAU 2000A precession-nutation */
BIND(iauS00a, id0rd, f, IAUS00A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUS00B(<jd>)

   Returns the CIO locator for the specified Julian Date <jd>, using
   IAU 2000B precession-nutation */
BIND(iauS00b, id0rd, f, IAUS00B, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUS06(<jd>, <x>, <y>)

   Returns the CIO locator for the specified Julian Date <jd> given
   the CIP's <x> and <y> coordinates, using IAU 2006/2000A
   precession-nutation */
BIND(iauS06, id0ddrd, f, IAUS06, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUS06A(<jd>)

   Returns the CIO locator for the specified Julian Date <jd>, using
   IAU 2006 precession and IAU 2000A nutation */
BIND(iauS06a, id0rd, f, IAUS06A, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUS2C(<theta>, <phi>)

   Returns the cartesian coordinates (direction cosines) corresponding
   to spherical coordinates longitude <theta> and latitude <phi> */
BIND(iauS2c, iddob3, f, IAUS2C, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUS2P(<theta>, <phi>, <r>)

   Converts spherical polar coordinates longitude <theta>, latitude
   <phi>, radial distance <r> to a p-vector */
BIND(iauS2p, idddob3, f, IAUS2P, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUS2PV(<theta>, <phi>, <r>, <td>, <pd>, <rd>)

   Returns the pv-vector that corresponds to the spherical coordinates
   longitude <theta>, latitude <phi>, radial distance <r>, rate of
   change of theta <td>, rate of change of phi <pd>, rate of change of
   r <rd> */
BIND(iauS2pv, iddddddoc23, f, IAUS2PV, 6, 6, NULL)
/*-----------------------------------------------------------------------*/
/* IAUSEPP(<a>, <b>)

   Returns the angular separation between two p-vectors <a> and <b> */
BIND(iauSepp, ib3b3rd, f, IAUSEPP, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUSEPS(<lon1>, <lat1>, <lon2>, <lat2>)

   Returns the angular separation between the direction with longitude
   <lon1>, latitude <lat1> and the direction with longitude <lon2>,
   latitude <lat2> */
BIND(iauSeps, iddddrd, f, IAUSEPS, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* IAUSP00(<jd>)

   Returns the TIO locator s' for Julian Date <jd>, positioning the
   Terrestrial Intermediate Origin on the equator of the Celestial
   Intermediate Pole */
BIND(iauSp00, id0rd, f, IAUSP00, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUSTARPM, <ra1>, <dec1>, <pmr1>, <pmd1>, <px1>, <rv1>, <jd1>,
   <jd2>, <ra2>, <dec2>, <pmr2>, <pmd2>, <px2>, <rv2>

   Updates star catalog data for space motion. <ra> = right ascension,
   <dec> = declination, <pmr> = right ascension proper motion, <pmd> =
   declination proper motion, <px> = parallax, <rv> = radial velocity;
   *1 = before, *2 = after */
BIND(iauStarpm, iddddddd0d0oddddddrl, s, IAUSTARPM, 14, 14, NULL)
/*-----------------------------------------------------------------------*/
/* IAUSTARPV(<ra>, <dec>, <pmr>, <pmd>, <px>, <rv>)

   Converts star catalog coordinates to a position-velocity matrix */
BIND(iauStarpv, iddddddoc23rl, f, IAUSTARPV, 6, 6, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTAITT(<jdtai>)

   Translates Julian Date <jdtai> from TAI to TT */
BIND(iauTaitt, id0oddrl_jd, f, IAUTAITT, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTAIUT1(<jdtai>, <dta>)

   Translates Julian Date <jdtai> from TAI to UT1, given <dta> = UT1 -
   TAi in seconds */
BIND(iauTaiut1, id0doddrl_jd, f, IAUTAIUT1, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTAIUTC(<jdtai>)

   Translates Julian Date <jdtai> from TAI to UTC */
BIND(iauTaiutc, id0oddrl_jd, f, IAUTAIUTC, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTCBTDB(<jdtcb>)

   Translates Julian Date <jdtcb> from TCB to TDB */
BIND(iauTcbtdb, id0oddrl_jd, f, IAUTCBTDB, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTCGTT(<jdtcg>)

   Translates Julian Date <jdtcg> from TCG to TT */
BIND(iauTcgtt, id0oddrl_jd, f, IAUTCGTT, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTDBTCB(<jdtdb>)

   Translates Julian Date <jdtdb> from TDB to TCB */
BIND(iauTdbtcb, id0oddrl_jd, f, IAUTDBTCB, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTDBTT(<jdtdb>, <dtr>)

   Translates Julian Date <jdtbd> from TDB to TT, given <dtr> = TDB -
   TT in seconds  */
BIND(iauTdbtt, id0doddrl_jd, f, IAUTDBTT, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTR(<r>)

   Transposes r-matrix <r> */
BIND(iauTr, ic33oc33, f, IAUTR, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTRXP(<r>, <p>)

   Multiply p-vector <p> by the transpose of r-matrix <r> */
BIND(iauTrxp, ic33b3ob3, f, IAUTRXP, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTRXPV(<r>, <pv>)

   Multiply pv-vector <pv> by the transpose of r-matrix <r> */
BIND(iauTrxpv, ic33c23oc23, f, IAUTRXPV, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTTTAI(<jdtt>)

   Transforms Julian Date <jdtt> from TT to TAI */
BIND(iauTttai, id0oddrl_jd, f, IAUTTTAI, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTTTCG(<jdtt>)

   Transforms Julian Date <jdtt> from TT to TCG */
BIND(iauTttcg, id0oddrl_jd, f, IAUTTTCG, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTTTDB(<jdtt>, <dt>)

   Transforms Julian Date <jdtt> from TT to TDB, given <dt> = TDB -
   TT in seconds */
BIND(iauTttdb, id0doddrl_jd, f, IAUTTTDB, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUTTUT1(<jdtt>, <dt>)

   Transforms Julian Date <jdtt> from TT to UT1, given <dt> = TT - UT1
   in seconds */
BIND(iauTtut1, id0doddrl_jd, f, IAUTTUT1, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUUT1TAI(<jdut>, <dt>)

   Transforms Julian Date <jdut> from UT1 to TAI, given <dt> = UT1 -
   TAI in seconds */
BIND(iauUt1tai, id0doddrl_jd, f, IAUUT1TAI, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUUT1TT(<jdut>, <dt>)

   Transforms Julian Date <jdut> from UT1 to TT, given <dt> = UT1 -
   TT in seconds */
BIND(iauUt1tt, id0doddrl_jd, f, IAUUT1TT, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUUT1UTC(<jdut>, <dt>)

   Transforms Julian Date <jdut> from UT1 to UTC, given <dt> = UT1 -
   UTC in seconds */
BIND(iauUt1utc, id0doddrl_jd, f, IAUUT1UTC, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUUTCTAI(<jdut>)

   Transforms Julian Date <jdut> from UTC to TAI */
BIND(iauUtctai, id0oddrl_jd, f, IAUUTCTAI, 1, 1, NULL)
/*-----------------------------------------------------------------------*/
/* IAUUTCUT1(<jdut>, <dt>)

   Transforms Julian Date <jdut> from UTC to UT1, given <dt> = UT1 -
   UTC in seconds */
BIND(iauUtcut1, id0doddrl_jd, f, IAUUTCUT1, 2, 2, NULL)
/*-----------------------------------------------------------------------*/
/* IAUXY06, <jd>, <x>, <y>

   Returns the Celestial Intermediate Pole <x>, <y> coordinates for
   the given Julian Date <jd>, based on IAU 2006 precession and IAU
   2000A nutation. */
BIND(iauXy06, id0odd, s, IAUXY06, 3, 3, NULL)
/*-----------------------------------------------------------------------*/
/* IAUXYS00A, <jd>, <x>, <y>, <s>

   Returns the Celestial Intermediate Pole <x>, <y> coordinates and
   the CIO locator <s> for Julian Date <jd>, based on the IAU 2000A
   precession-nutation model */
BIND(iauXys00a, id0oddd, s, IAUXYS00A, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* IAUXYS00B, <jd>, <x>, <y>, <s>

   Returns the Celestial Intermediate Pole <x>, <y> coordinates and
   the CIO locator <s> for Julian Date <jd>, based on the IAU 2000B
   precession-nutation model */
BIND(iauXys00b, id0oddd, s, IAUXYS00B, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
/* IAUXYS06A, <jd>, <x>, <y>, <s>

   Returns the Celestial Intermediate Pole <x>, <y> coordinates and
   the CIO locator <s> for Julian Date <jd>, based on the IAU 2006
   precession and IAU 2000A nutation models */
BIND(iauXys06a, id0oddd, s, IAUXYS06A, 4, 4, NULL)
/*-----------------------------------------------------------------------*/
