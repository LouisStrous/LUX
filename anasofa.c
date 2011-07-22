#include "sofam.h"
#include "action.h"

/* This file provides ANA bindings for SOFA routines */

/* IAUCAL2JD(<caldates>) returns the Julian Day number corresponding
   to the specified calendar dates <caldates>, which must have a
   multiple of 3 elements in its first dimension (year, month, day) */
int ana_iauCal2jd(int narg, int ps[])
{
  loopInfo srcinfo, tgtinfo;
  pointer src, tgt;
  int result;
  double djm0, djm;

  if (standardLoop(ps[0], ANA_ZERO, 
                   SL_EXACT     /* exactly the output type */
                   | SL_COMPRESS /* omit selected axis from result */
                   | SL_AXISCOORD /* only indicated axis coords */
                   | SL_EACHROW,  /* user advances along indicates axis */
                   ANA_DOUBLE, &srcinfo, &src, &result, &tgtinfo, &tgt) < 0)
    return ANA_ERROR;
  if (srcinfo.rdims[0] % 3 != 0)
    return anaerror("Need multiple of 3 elements in 1st dimension!", ps[0]);
  switch (symbol_type(ps[0])) {
  case ANA_LONG:
    do {
      for (int i = 0; i < srcinfo.rdims[0]; i += 3) {
        /* iauCal2jd(int iy, int im, int id, double *djm0, double *djm) */
        iauCal2jd(src.l[0], src.l[1], src.l[2], &djm0, &djm);
        src.l += 3;
        *tgt.d++ = djm0 + djm;
      }
    } while (advanceLoops(&srcinfo, &tgtinfo) < srcinfo.rndim);
    break;
  }
  return result;
}
