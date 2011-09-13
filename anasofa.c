#include "sofam.h"
#include "action.h"
#include <string.h>

/* This file provides ANA bindings for SOFA routines */

/* NOT INCLUDED:

   iauA2af - decompose radians into degrees, arcminutes, arcseconds, fraction
   iauA2tf - decompose radians into hours, minutes, seconds, fraction
   iauAf2a - convert degrees, arcminutes, arcseconds to radians
   iauAnp  - normalize angle into the range 0 <= a < 2pi
   iauAnpm - normalize angle into the range -pi <= a < +pi
 */

/*-----------------------------------------------------------------------*/
/* IAUBI00() returns the frame bias components of the IAU 2000
   precession-nutation models (part of MHB2000 with additions), in a
   3-element DOUBLE array.  The elements are: (0) the longitude
   correction, (1) the obliquity correction, (2) the ICRS RA of the
   J2000.0 mean equinox. */
int ana_iauBi00(int narg, int ps[])
{
  /* iauBi00(double *dpsibi, double *depsbi, double *dra) */
  int result;
  int dim = 3;
  double *tgt;

  result = array_scratch(ANA_DOUBLE, 1, &dim);
  tgt = array_data(result);
  iauBi00(&tgt[0], &tgt[1], &tgt[2]);
  return result;
}
/*-----------------------------------------------------------------------*/
int ana_iauJD_333(int narg, int ps[], void (*f)(double, double, double (*)[3], double (*)[3], double (*)[3]))
{
  int nelem, dorb, dorp, dorbp, iq;
  pointer jd;
  double *rbtgt, *rptgt, *rbptgt;
  double rb[3][3], rp[3][3], rbp[3][3];

  iq = ps[0];
  if (symbol_type(iq) < ANA_FLOAT)
    iq = ana_float(1, &iq);
  if (numerical(iq, NULL, NULL, &nelem, &jd) < 0)
    return ANA_ERROR;
  dorb = (narg > 1 && ps[1]);
  dorp = (narg > 2 && ps[2]);
  dorbp = (narg > 3 && ps[3]);
  if (!dorb && !dorp && !dorbp)
    return anaerror("Nothing to do", ps[0]);
  {
    int extradims[2] = { 3, 3 };

    if (dorb) {
      if (redef_array_extra_dims(ps[1], ps[0], ANA_DOUBLE, 2, extradims) != ANA_OK)
        return ANA_ERROR;
      rbtgt = array_data(ps[1]);
    }
    if (dorp) {
      if (redef_array_extra_dims(ps[2], ps[0], ANA_DOUBLE, 2, extradims) != ANA_OK)
        return ANA_ERROR;
      rptgt = array_data(ps[2]);
    }
    if (dorbp) {
      if (redef_array_extra_dims(ps[3], ps[0], ANA_DOUBLE, 2, extradims) != ANA_OK)
        return ANA_ERROR;
      rbptgt = array_data(ps[3]);
    }
  }
  /* f(double date1, double date2, double rb[3][3], double rbp[3][3]) */ 
  switch (symbol_type(iq)) {
  case ANA_FLOAT:
    while (nelem--) {
      f((double) *jd.f++, 0.0, rb, rp, rbp);
      if (dorb) {
        memcpy(rbtgt, rb, 9*sizeof(*rb));
        rbtgt += 9;
      }
      if (dorp) {
        memcpy(rptgt, rp, 9*sizeof(*rp));
        rptgt += 9;
      }
      if (dorbp) {
        memcpy(rbptgt, rbp, 9*sizeof(*rbp));
        rbptgt += 9;
      }
    }
    break;
  case ANA_DOUBLE:
    while (nelem--) {
      f(*jd.d++, 0.0, rb, rp, rbp);
      if (dorb) {
        memcpy(rbtgt, rb, 9*sizeof(*rbtgt));
        rbtgt += 9;
      }
      if (dorp) {
        memcpy(rptgt, rp, 9*sizeof(*rptgt));
        rptgt += 9;
      }
      if (dorbp) {
        memcpy(rbptgt, rbp, 9*sizeof(*rbptgt));
        rbptgt += 9;
      }
    }
    break;
  }
  return ANA_OK;
}
/*-----------------------------------------------------------------------*/
/* IAUBP00, jd, rb, rp, rbp

   returns the frame bias and precession, IAU 2000.  <jd> are the
   Julian Dates for which results are desired.  <rb> receives the
   frame bias maxtrix, <rp> the precession matrix, and <rbp> the
   bias-precession matrix. */
int ana_iauBp00(int narg, int ps[])
{
  return ana_iauJD_333(narg, ps, iauBp00);
}
/*-----------------------------------------------------------------------*/
/* IAUBP06, jd, rb, rp, rbp

   returns the frame bias and precession, IAU 2006.  <jd> are the
   Julian Dates for which results are desired.  <rb> receives the
   frame bias matrix, <rp> the precession matrix, and <rbp> the
   bias-precession matrix.  */
int ana_iauBp06(int narg, int ps[])
{
  return ana_iauJD_333(narg, ps, iauBp06);
}
/*-----------------------------------------------------------------------*/
/* IAUBPN2XY, rbpn, x, y

   extracts the x,y coordinates of the Celestial Intermediate Pole
   from the bias-precesion-nutation matrix. */
int ana_iauBpn2xy(int narg, int ps[])
{
  int ndim, *dims, n, iq;
  pointer rbpn, x, y;
  double (*r)[3];

  iq = ana_double(1, &ps[0]);
  if (numerical(iq, &dims, &ndim, NULL, &rbpn) < 0)
    return ANA_ERROR;
  if (ndim < 2 || dims[0] != 3 || dims[1] != 3)
    return anaerror("The first two dimensions should have size 3", ps[0]);
  if (redef_array(ps[1], ANA_DOUBLE, ndim - 2, &dims[2]) != ANA_OK)
    return ANA_ERROR;
  numerical(ps[1], NULL, NULL, &n, &x);
  if (redef_array(ps[2], ANA_DOUBLE, ndim - 2, &dims[2]) != ANA_OK)
    return ANA_ERROR;
  numerical(ps[2], NULL, NULL, NULL, &y);
  r = (double (*)[3]) rbpn.d;
  while (n--) {
    iauBpn2xy(r, x.d, y.d);
    r += 3;
    x.d++;
    y.d++;
  }
  return ANA_OK;
}
/*-----------------------------------------------------------------------*/
int ana_iauC2i(int narg, int ps[], void (*f)(double, double, double (*)[3]))
{
  int iq, nelem, *dims, ndim, tgtdims[MAX_DIMS];
  pointer jd, tgt;

  iq = ana_double(1, &ps[0]);
  if (numerical(iq, &dims, &ndim, &nelem, &jd) < 0)
    return ANA_ERROR;
  if (ndim + 2 > MAX_DIMS)
    return cerror(N_DIMS_OVR, 0);
  tgtdims[0] = tgtdims[1] = 3;
  memcpy(tgtdims + 2, dims, ndim*sizeof(*dims));
  ndim += 2;
  iq = array_scratch(ANA_DOUBLE, ndim, tgtdims);
  tgt.d = array_data(iq);
  while (nelem--) {
    double rc2i[3][3];

    f(*jd.d++, 0.0, rc2i);
    memcpy(tgt.d, rc2i, 9*sizeof(double));
    tgt.d += 9;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
/* IAUC2I00A(jd)

 Returns the celestial-to-intermedia matrix for the Julian Dates <jd>
 using the IAU 2000A precession-nutation model. */
int ana_iauC2i00a(int narg, int ps[])
{
  return ana_iauC2i(narg, ps, iauC2i00a);
}
/*-----------------------------------------------------------------------*/
/* IAUC2I00B(jd)

 Returns the celestial-to-intermedia matrix for the Julian Dates <jd>
 using the IAU 2000B precession-nutation model. */
int ana_iauC2i00b(int narg, int ps[])
{
  return ana_iauC2i(narg, ps, iauC2i00b);
}
/*-----------------------------------------------------------------------*/
/* IAUC2I06A(jd)

 Returns the celestial-to-intermedia matrix for the Julian Dates <jd>
 using the IAU 2006 precession model and IAU 2000A nutation model. */
int ana_iauC2i06a(int narg, int ps[])
{
  return ana_iauC2i(narg, ps, iauC2i06a);
}
/*-----------------------------------------------------------------------*/
/* rc2i = IAUC2IBPN(jd, rbpn)

   Forms the celestial-to-intermediate matrix <rc2i> for Julian Dates
   <jd>, given the bias-precession-nutation matrix <rbpn>. */
int ana_iauC2ibpn(int narg, int ps[])
{
  int iq, *jddims, jdndim, *rbpndims, rbpnndim, nelem, rbpnstep, result;
  pointer jd, rbpn, tgt;

  iq = ps[0];
  if (symbol_type(iq) < ANA_FLOAT)
    iq = ana_float(1, &iq);
  if (numerical(iq, &jddims, &jdndim, &nelem, &jd) < 0)
    return ANA_ERROR;
  if (narg > 1 && ps[1]) {
    iq = ana_double(1, &ps[1]);
    if (iq == ANA_ERROR)
      return ANA_ERROR;
    if (numerical(ps[1], &rbpndims, &rbpnndim, NULL, &rbpn) < 0)
      return ANA_ERROR;
    if (rbpnndim < 2 || rbpndims[0] != 3 || rbpndims[1] != 3)
      return anaerror("1st and 2nd dimensions should be equal to 3", ps[1]);
    int n = array_size(ps[1])/9;
    if (n != nelem && n != 1)
      return cerror(INCMP_DIMS, ps[1]);
    rbpnstep = (n == 1? 0: 9);
  } else
    return anaerror("Not supported yet", 0);
  {
    int tgtdims[MAX_DIMS] = { 3, 3 };

    memcpy(tgtdims + 2, jddims, jdndim*sizeof(int));
    result = array_scratch(ANA_DOUBLE, jdndim + 2, tgtdims);
    tgt.d = array_data(result);
  }
   
  double (*r)[3] = (double (*)[3]) rbpn.d;
  double (*rc2i)[3] = (double (*)[3]) tgt.d;

  switch (symbol_type(iq)) {
  case ANA_FLOAT:
    while (nelem--) {
      iauC2ibpn((double) *jd.f++, 0.0, r, rc2i);
      rc2i += 3;
      if (r)
        r += 3;
    }
    break;
  case ANA_DOUBLE:
    while (nelem--) {
      iauC2ibpn(*jd.d++, 0.0, r, rc2i);
      rc2i += 3;
      if (r)
        r += 3;
    }
    break;
  }
  return result;
}
/*-----------------------------------------------------------------------*/
/* IAUC2IXY(jd, x, y)

   returns the celestial-to-intermediate-frame-of-date matrix for
   Julian Date <jd> and CIP x, y coordinates.
 */
int ana_iauC2ixy(int narg, int ps[])
{
  int nelem, n, result, iq, *jddims, jdndim;
  pointer jd, x, y;
  double (*tgt)[3];

  iq = ana_double(1, &ps[0]);
  if (iq < 0)
    return ANA_ERROR;
  if (numerical(iq, &jddims, &jdndim, &nelem, &jd) < 0)
    return ANA_ERROR;
  iq = ana_double(1, &ps[1]);
  if (numerical(iq, NULL, NULL, &n, &x) < 0)
    return ANA_ERROR;
  if (n != nelem)
    return cerror(INCMP_ARG, ps[1]);
  iq = ana_double(2, &ps[2]);
  if (numerical(iq, NULL, NULL, &n, &y) < 0)
    return ANA_ERROR;
  if (n != nelem)
    return cerror(INCMP_ARG, ps[2]);
  {
    int dims[MAX_DIMS] = { 3, 3 };

    memcpy(dims + 2, jddims, jdndim*sizeof(int));
    result = array_scratch(ANA_DOUBLE, 2 + jdndim, dims);
    tgt = (double (*)[3]) array_data(result);
  }
  while (nelem--) {
    iauC2ixy(*jd.d++, 0.0, *x.d++, *y.d++, tgt);
    tgt += 3;
  }
  return result;
}
/*-----------------------------------------------------------------------*/
/* rc2i = IAUC2IXYS(x, y, s) 

   returns the celestial-to-intermediate-frame-of-date matrix given
   the CIP x, y and the CIO locator s.
 */
int ana_iauC2ixys(int narg, int ps[])
{
  int iq, *dims, ndim, nelem, nelem2, tgtdims[MAX_DIMS] = { 3, 3 };
  pointer x, y, s;
  double (*rc2i)[3];

  iq = ana_double(1, &ps[0]);
  if (numerical(iq, &dims, &ndim, &nelem, &x) < 0)
    return ANA_ERROR;
  if (ndim > MAX_DIMS - 2)
    return cerror(N_DIMS_OVR, ps[0]);
  iq = ana_double(1, &ps[1]);
  if (numerical(iq, NULL, NULL, &nelem2, &y) < 0)
    return ANA_ERROR;
  if (nelem2 != nelem)
    return cerror(INCMP_ARG, ps[1]);
  iq = ana_double(1, &ps[2]);
  if (numerical(iq, NULL, NULL, &nelem2, &s) < 0)
    return ANA_ERROR;
  if (nelem2 != nelem)
    return cerror(INCMP_ARG, ps[2]);
  memcpy(tgtdims + 2, dims, ndim*sizeof(*dims));
  iq = array_scratch(ANA_DOUBLE, ndim + 2, tgtdims);
  rc2i = array_data(iq);
  while (nelem--) {
    iauC2ixys(*x.d++, *y.d++, *s.d++, rc2i);
    rc2i += 3;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
/* angles = IAUC2S(p)

   Translates p-vector <p> to spherical coordinates.
   <angles(0,/all)> = longitude angle (radians)
   <angles(1,/all)> = latitude angle (radians)
   */
int ana_iauC2s(int narg, int ps[]) {
  loopInfo srcinfo, tgtinfo;
  pointer src, tgt;
  int target;
  int more[1] = { 2 };

  if (standardLoopX(ps[0], ANA_ZERO,
                    SL_EACHROW
                    | SL_AXISCOORD,
                    &srcinfo, &src,
                    1, more,    /* one extra dimension */
                    0, NULL,    /* no removed dimensions */
                    ANA_DOUBLE,
                    SL_EXACT | SL_EACHROW
                    | SL_AXISCOORD,
                    &target, &tgtinfo, &tgt) < 0)
    return ANA_ERROR;
  if (srcinfo.rdims[0] != 3)
    return anaerror("Need 3 elements in first dimension", ps[0]);
  do {
    iauC2s(src.d, tgt.d, tgt.d + 1);
    src.d += 3;
    tgt.d += 2;
  } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
  return target;
}
/*-----------------------------------------------------------------------*/
/* rc2t = IAUC2T00A(jd, x, y)

   Return the celestial-to-terrestrial matrix for <jd> given the polar
   coordinates <x>, <y>, using the IAU2000A nutation model
*/
int ana_iauC2t00a(int narg, int ps[])
{
  loopInfo jdinfo, tgtinfo;
  pointer jd, tgt, x, y;
  int target, iq, nelem;
  int more[2] = { 3, 3 };
  double (*rc2t)[3];

  if (standardLoopX(ps[0], ANA_ZERO,
                    SL_AXISCOORD,
                    &jdinfo, &jd,
                    2, more,
                    0, NULL,
                    ANA_DOUBLE,
                    SL_EXACT | SL_EACHROW | SL_AXISCOORD,
                    &target, &tgtinfo, &tgt) < 0)
    return ANA_ERROR;
  iq = ana_double(1, &ps[1]);
  if (numerical(iq, NULL, NULL, &nelem, &x) < 0)
    return ANA_ERROR;
  if (nelem != jdinfo.nelem)
    return cerror(INCMP_ARG, ps[1]);
  iq = ana_double(1, &ps[2]);
  if (numerical(iq, NULL, NULL, &nelem, &y) < 0)
    return ANA_ERROR;
  if (nelem != jdinfo.nelem)
    return cerror(INCMP_ARG, ps[2]);
  do {
    rc2t = (double (*)[3]) tgt.d;
    iauC2t00a(*jd.d, 0.0, 0.0, 0.0, *x.d++, *y.d++, rc2t);
  } while (advanceLoop(&tgtinfo), advanceLoop(&jdinfo) < jdinfo.rndim);
  return target;
}
/*-----------------------------------------------------------------------*/
/* IAUCAL2JD(<caldates>)

   returns the Chronological Julian Day Numbers corresponding to the
   specified Gregorian calendar dates <caldates>, which must have a
   multiple of 3 elements in its first dimension (year, month, day).
   Unexpected results may be given if illegal year, month, or day
   numbers are specified.  Dates before -4799-01-01 or after
   1465073-02-28 are rejected. */
int ana_iauCal2jd(int narg, int ps[])
{
  loopInfo srcinfo, tgtinfo;
  pointer src, tgt;
  int target, i;
  double djm0, djm;
  int less[1] = { 3 };

  if (standardLoopX(ps[0], ANA_ZERO,
                    SL_EACHROW      /* we handle advancing along axis 0 */
                    | SL_AXISCOORD, /* only indicated source axis coords */
                    &srcinfo, &src,
                    0, NULL,    /* no extra dimensions */
                    1, less,    /* reduce one dimension */
                    ANA_LONG,
                    SL_UPGRADE | /* upgrade target type if source type > LONG */
                    SL_AXISCOORD, /* only indicated target axis coords */
                    &target, &tgtinfo, &tgt) < 0)
    return ANA_ERROR;
  /* iauCal2jd(int iy, int im, int id, double *djm0, double *djm) */
  switch (symbol_type(ps[0])) {
  case ANA_BYTE:
    do {
      if (iauCal2jd(src.b[0], src.b[1], src.b[2], &djm0, &djm))
        *tgt.l = 0;
      else
        *tgt.l = 2400001 + djm;
      src.b += 3;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_WORD:
    do {
      if (iauCal2jd(src.w[0], src.w[1], src.w[2], &djm0, &djm))
        *tgt.l = 0;
      else
        *tgt.l = 2400001 + djm;
      src.w += 3;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_LONG:
    do {
      if (iauCal2jd(src.l[0], src.l[1], src.l[2], &djm0, &djm))
        *tgt.l = 0;
      else
        *tgt.l = 2400001 + djm;
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
        *tgt.f = 2400001 + djm + daypart;
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
        *tgt.d = 2400001 + djm + daypart;
      src.d += 3;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  default:
    return cerror(ILL_TYPE, ps[0]);
  }
  if (!loopIsAtStart(&tgtinfo))
    return anaerror("Source loop is finished but target loop is not!", ps[0]);
  return target;
}
/*-----------------------------------------------------------------------*/
/* IAUJD2CAL(<jd>) returns the Gregorian calendar dates corresponding
   to the specified Chronological Julian Day Numbers <jd>.  The return
   value has the same dimensions as <jd> but with one extra dimension
   of size 3 prefixed, for the year, month, and day, respectively.
   Zeros are returned for input CJDN outside of the supported range.
   Julian day numbers before -68569.5 or after 1e9 are considered
   illegal -- actually, CJDNs greater than 536802342 yield erroneous
   results, but the SOFA routine does not indicate problems until the
   CJDN exceeds 1e9. */
int ana_iauJd2cal(int narg, int ps[])
{
  loopInfo srcinfo, tgtinfo;
  pointer src, tgt;
  int result, more[1] = { 3 };
  double djm0, djm;

  if (standardLoopX(ps[0], ANA_ZERO, 
                    SL_AXISCOORD, /* only indicated axis coords */
                    &srcinfo, &src,
                    1, more,    /* one extra dimension */
                    0, NULL,    /* no reduced dimensions */
                    ANA_LONG,
                    SL_UPGRADE |
                    SL_EACHROW, /* we handle the new dimension ourselves */
                    &result,
                    &tgtinfo, &tgt) < 0)
    return ANA_ERROR;
  
  /* iauJd2cal(double dj1, double dj2, int *iy, int *im, int *id, double *fd) */
  switch (symbol_type(ps[0])) {
  case ANA_BYTE:
    do {
      int y, m, d;
      double fd;
      if (iauJd2cal(src.b[0], -0.5, &y, &m, &d, &fd)) {
        *tgt.l++ = 0;
        *tgt.l++ = 0;
        *tgt.l++ = 0;
      } else {
        *tgt.l++ = y;
        *tgt.l++ = m;
        *tgt.l++ = d + fd;
      }
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_WORD:
    do {
      int y, m, d;
      double fd;
      if (iauJd2cal(src.w[0], -0.5, &y, &m, &d, &fd)) {
        *tgt.l++ = 0;
        *tgt.l++ = 0;
        *tgt.l++ = 0;
      } else {
        *tgt.l++ = y;
        *tgt.l++ = m;
        *tgt.l++ = d + fd;
      }
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_LONG:
    do {
      int y, m, d;
      double fd;
      if (iauJd2cal(src.l[0], -0.5, &y, &m, &d, &fd)) {
        *tgt.l++ = 0;
        *tgt.l++ = 0;
        *tgt.l++ = 0;
      } else {
        *tgt.l++ = y;
        *tgt.l++ = m;
        *tgt.l++ = d + fd;
      }
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_FLOAT:
    do {
      int y, m, d;
      double fd;
      if (iauJd2cal(src.f[0], -0.5, &y, &m, &d, &fd)) {
        *tgt.f++ = 0;
        *tgt.f++ = 0;
        *tgt.f++ = 0;
      } else {
        *tgt.f++ = y;
        *tgt.f++ = m;
        *tgt.f++ = d + fd;
      }
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_DOUBLE:
    do {
      int y, m, d;
      double fd;
      if (iauJd2cal(src.d[0], -0.5, &y, &m, &d, &fd)) {
        *tgt.d++ = 0;
        *tgt.d++ = 0;
        *tgt.d++ = 0;
      } else {
        *tgt.d++ = y;
        *tgt.d++ = m;
        *tgt.d++ = d + fd;
      }
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  default:
    return cerror(ILL_ARG, ps[0]);
  }
  if (!loopIsAtStart(&tgtinfo))
    return anaerror("Source loop is finished but target loop is not!", ps[0]);
  return result;
}
