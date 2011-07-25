#include "sofam.h"
#include "action.h"

/* This file provides ANA bindings for SOFA routines */

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
/* IAUCAL2JD(<caldates>) returns the Julian Day numbers corresponding
   to the specified Gregorian calendar dates <caldates>, which must
   have a multiple of 3 elements in its first dimension (year, month,
   day).  Unexpected results may be given if illegal year, month, or
   day numbers are specified.  Dates before -4799-01-01 are
   rejected. */
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
                    ANA_DOUBLE,
                    SL_AXISCOORD, /* only indicated target axis coords */
                    &target, &tgtinfo, &tgt) < 0)
    return ANA_ERROR;
  /* iauCal2jd(int iy, int im, int id, double *djm0, double *djm) */
  switch (symbol_type(ps[0])) {
  case ANA_BYTE:
    do {
      iauCal2jd(src.b[0], src.b[1], src.b[2], &djm0, &djm);
      src.b += 3;
      *tgt.d = djm0 + djm;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_WORD:
    do {
      iauCal2jd(src.w[0], src.w[1], src.w[2], &djm0, &djm);
      src.w += 3;
      *tgt.d = djm0 + djm;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_LONG:
    do {
      iauCal2jd(src.l[0], src.l[1], src.l[2], &djm0, &djm);
      src.l += 3;
      *tgt.d = djm0 + djm;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_FLOAT:
    do {
      int day = floor(src.f[2]);
      double daypart = src.f[2] - day;
      iauCal2jd((int) src.f[0], (int) src.f[1], day, &djm0, &djm);
      src.f += 3;
      *tgt.d = djm0 + djm + daypart;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_DOUBLE:
    do {
      int day = floor(src.d[2]);
      double daypart = src.d[2] - day;
      iauCal2jd((int) src.d[0], (int) src.d[1], day, &djm0, &djm);
      src.d += 3;
      *tgt.d = djm0 + djm + daypart;
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
   to the specified Julian day numbers <jd>.  The return value has the
   same dimensions as <jd> but with one extra dimension of size 3
   prefixed, for the year, month, and day, respectively.  Unexpected
   results may be given if illegal Julian day day numbers are
   specified.  Julian day numbers before -68569.5 or after 1e9 are
   considered illegal. */
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
                    ANA_DOUBLE,
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
      iauJd2cal(src.b[0], 0.0, &y, &m, &d, &fd);
      *tgt.d++ = y;
      *tgt.d++ = m;
      *tgt.d++ = d + fd;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_WORD:
    do {
      int y, m, d;
      double fd;
      iauJd2cal(src.w[0], 0.0, &y, &m, &d, &fd);
      *tgt.d++ = y;
      *tgt.d++ = m;
      *tgt.d++ = d + fd;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_LONG:
    do {
      int y, m, d;
      double fd;
      iauJd2cal(src.l[0], 0.0, &y, &m, &d, &fd);
      *tgt.d++ = y;
      *tgt.d++ = m;
      *tgt.d++ = d + fd;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_FLOAT:
    do {
      int y, m, d;
      double fd;
      iauJd2cal(src.f[0], 0.0, &y, &m, &d, &fd);
      *tgt.d++ = y;
      *tgt.d++ = m;
      *tgt.d++ = d + fd;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  case ANA_DOUBLE:
    do {
      int y, m, d;
      double fd;
      iauJd2cal(src.d[0], 0.0, &y, &m, &d, &fd);
      *tgt.d++ = y;
      *tgt.d++ = m;
      *tgt.d++ = d + fd;
    } while (advanceLoop(&tgtinfo), advanceLoop(&srcinfo) < srcinfo.rndim);
    break;
  default:
    return cerror(ILL_ARG, ps[0]);
  }
  if (!loopIsAtStart(&tgtinfo))
    return anaerror("Source loop is finished but target loop is not!", ps[0]);
  return result;
}
