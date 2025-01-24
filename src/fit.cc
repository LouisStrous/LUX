/* This is file fit.cc.

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
// File fit.c
// General fitting procedure
// Started 22 October 1995.  Louis Strous
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>             // for LONG_MAX
#include <time.h>               // for difftime
#include "action.hh"
#include "bindings.hh"
#include "lux_func_if.hh"
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>
#include <limits.h>

char    PoissonChiSq;           // flag: Poisson chi-square fitting?
uint32_t random_bits(void);
void indexxr_d(int32_t, double *, int32_t *);

//-----------------------------------------------------------------------
double gaussians(double *par, int32_t nPar, double *x, double *y, double *w, int32_t nData)
// Returns the rms error (if PoissonChiSq is zero) or chi-square for
// Poisson distributions (if PoissonChiSq is nonzero) in fitting y vs x
// with a set of gaussians.
// The fit profile is  par[0]+par[1]*exp(-((x - par[2])/par[3])^2)
// [ + par[4]*exp(-((x - par[5])/par[6])^2) ... ].  par[] must have
// at least 4 elements and for each three more another gaussian is
// added.   LS 26oct95  31oct95
{
  int32_t   j, wstep;
  double        rms, fit, arg, one = 1.0;

  if (w)
    wstep = 1;
  else
  { w = &one;
    wstep = 0; }
  rms = 0.0;
  if (PoissonChiSq)
  { while (nData--)
    { fit = par[0];
      for (j = 1; j < nPar - 2; j += 3)
      { arg = (*x - par[j + 1])/par[j + 2];
        fit += par[j]*exp(-arg*arg); }
      arg = *y - fit;
      if (fit)
        rms += (arg*arg/fit)**w;
      x++;  y++;  w += wstep; }
    return rms; }
  else
  { while (nData--)
    { fit = *y - par[0];
      for (j = 1; j < nPar - 2; j += 3)
      { arg = (*x - par[j + 1])/par[j + 2];
        fit -= par[j]*exp(-arg*arg); }
      rms += fit*fit**w;
      x++;  y++;  w += wstep; }
    return sqrt(rms); }
}
//-----------------------------------------------------------------------
double powerfunc(double *par, int32_t nPar, double *x, double *y, double *w, int32_t nData)
// returns the error in fitting y vs x with a general power function
// superposed on a linear trend.
// the fit profile is  par[0] + par[1]*x + par[2]*(x - par[3])^par[4]
// LS 13nov95
{
  int32_t   wstep;
  double        rms, dfit, one;

  if (w)
    wstep = 1;
  else
  { w = &one;
    wstep = 0; }
  rms = 0.0;
  while (nData--)
  { dfit = *y++ - par[0] - par[1]* *x - par[2]*pow(*x - par[3], par[4]);
    rms += dfit*dfit* *w;
    x++;  w += wstep; }
  return sqrt(rms);
}
//-----------------------------------------------------------------------
void enforce_bounds(double *par, double *lowbound, double *hibound, int32_t nPar)
{
  if (lowbound || hibound) {
    int i;
    for (i = 0; i < nPar; i++) {
      if (lowbound && par[i] < lowbound[i])
        par[i] = lowbound[i];
      else if (hibound && par[i] > hibound[i])
        par[i] = hibound[i];
    }
  }
}
//-----------------------------------------------------------------------
int32_t lux_generalfit(ArgumentCount narg, Symbol ps[])
/* FIT([X,]Y,START,STEP[,LOWBOUND,HIGHBOUND][,WEIGHTS][,QTHRESH,PTHRESH,
   ITHRESH,DTHRESH][,FAC,NITER,NSAME][,ERR][,FIT][,TTHRESH][,/VOCAL,/DOWN,
   /PCHI][/GAUSSIANS,/POWERFUNC]) */
// This routine is intended to enable the user to fit iteratively
// to any profile specification.
// LS 24oct95
{
  int32_t ySym, xSym, nPoints, n, nPar, nIter, iThresh, nSame, size,
    iq, i, j, iter, same, fitSym, fitTemp, xTemp, nn, wSym;

  double *yp, *xp, *start, *step, qThresh, *pThresh, fac, *lowbound,
    *hibound, *err, *par, qBest1, qBest2, *parBest1, *parBest2, *ran,
    qual, temp, dir, dThresh, qLast, mu, *meanShift, *weights, tThresh;

  char  vocal, onebyone, vocal_err;
  void  randome(void *output, int32_t number, double limit);
  double (*fitProfiles[2])(double *, int32_t, double *, double *, double *, int32_t) =
  { gaussians, powerfunc };
  double (*fitFunc)(double *, int32_t, double *, double *, double *, int32_t);
  extern int32_t    nFixed;
  int16_t  fitPar, fitArg[4];
  int32_t   lux_indgen(int32_t, int32_t []), eval(int32_t);
  void  zap(int32_t);
  time_t starttime;

  // check input variables
  if (narg >= 15 && (iq = ps[15])) { // FIT
    if (symbol_class(iq) != LUX_STRING)
      return cerror(NEED_STR, iq);
    n = SP_USER_FUNC;
    fitSym = stringpointer(string_value(iq), n);
    if (fitSym < 0)    // internal routine rather than user-defined
      return
        luxerror("Sorry, fitting to internal routines is not yet implemented",
              iq);
  } else
    fitSym = 0;

  i = (internalMode/16 & 3);    // fit function
  if (i) {
    if (fitSym)
      return luxerror("Multiple fit functions specified", fitSym);
    i--;
  }
  fitFunc = fitProfiles[i];     // fit function

  if (ps[1]) {                  // X & Y
    ySym = ps[1];
    xSym = ps[0];
  } else {                              // only Y
    ySym = ps[0];
    xSym = 0;
  }
  if (symbol_class(ySym) != LUX_ARRAY)
    return cerror(NEED_ARR, ySym);
  ySym = lux_double(1, &ySym);  // ensure LUX_DOUBLE
  nPoints = array_size(ySym);   // number of data points
  yp = (double *) array_data(ySym); // pointer to data

  if (xSym)                     // X
    switch (symbol_class(xSym)) {
      case LUX_SCALAR:
        if (!fitSym)
          return luxerror("Need array in standard fit", xSym);
        // fall-thru to next case
      case LUX_ARRAY:
        if (!fitSym) {
          n = array_size(xSym);
          if (n != nPoints)
            return cerror(INCMP_ARG, ySym);
        }
        xSym = lux_double(1, &xSym);
        xTemp = xSym;
        if (symbol_class(xSym) == LUX_ARRAY)
          xp = (double *) array_data(xSym);
        break;
      default:
        return cerror(ILL_CLASS, xSym);
    }
  else {
    xTemp = lux_indgen(1, &ySym);
    xp = (double *) array_data(xTemp);
  }

  iq = ps[2];                   // START: initial parameter values
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  iq = lux_double(1, &iq);
  nPar = array_size(iq);
  start = (double *) array_data(iq);

  iq = ps[3];                   // STEP: initial parameter step sizes
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  iq = lux_double(1, &iq);
  n = array_size(iq);
  if (n == nPar - 1)            // assume last element of START is quality
    nPar--;
  else if (n != nPar)
    return cerror(INCMP_ARG, ps[3]);
  if (!fitSym)                  // standard fit
    switch (i) {                // check for proper number of arguments
      case 0:                   // gaussians
        if ((nPar - 1) % 3 != 0)
          return luxerror("Need one plus a multiple of three parameters for gaussian fits", ps[3]);
        break;
      case 1:                   // power function
        if (nPar != 5)
          return luxerror("Need five parameters for power-function fits", ps[3]);
        break;
    }
  step = (double *) array_data(iq);

  if (narg >= 5 && (iq = ps[4])) { // LOWBOUND: lower bound on parameters
    if (symbol_class(iq) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = lux_double(1, &iq);
    n = array_size(iq);
    if (n != nPar)
      return cerror(INCMP_ARG, ps[4]);
    lowbound = (double *) array_data(iq);
  } else
    lowbound = NULL;

  if (narg >= 6 && (iq = ps[5])) { // HIGHBOUND: upper bound on parameters
    if (symbol_class(iq) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = lux_double(1, &iq);
    n = array_size(iq);
    if (n != nPar)
      return cerror(INCMP_ARG, ps[5]);
    hibound = (double *) array_data(iq);
  } else
    hibound = NULL;

  if (lowbound && hibound) {
    for (i = 0; i < nPar; i++)
      if (hibound[i] <= lowbound[i])
        return luxerror("High bound %g of parameter %d is not greater than low bound %g", ps[5], hibound[i], i + 1, lowbound[i]);
  }

  if (narg >= 7 && (iq = ps[6])) { // WEIGHTS
    if (symbol_class(iq) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    wSym = lux_double(1, &iq);
    n = array_size(wSym);
    if (n != nPoints)
      return cerror(INCMP_ARG, ps[6]);
    weights = (double *) array_data(wSym);
  } else
    weights = NULL;

  if (narg >= 8 && ps[7])       // QTHRESH: quality value threshold
    qThresh = double_arg(ps[7]);
  else
    qThresh = 0.0;

  if (narg >= 9 && (iq = ps[8])) { // PTHRESH: parameter change threshold
    if (symbol_class(iq) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = lux_double(1, &iq);
    n = array_size(iq);
    if (n != nPar)
      return cerror(INCMP_ARG, ps[8]);
    pThresh = (double *) array_data(iq);
  } else
    pThresh = NULL;

  if (narg >= 10 && ps[9])      // ITHRESH: iterations threshold
    iThresh = int_arg(ps[9]);
  else
    iThresh = 1000;

  if (narg >= 11 && ps[10])     // DTHRESH
    dThresh = double_arg(ps[10]);
  else
    dThresh = 0.0;

  if (narg >= 12 && ps[11]) {   // FAC: step size reduction factor
    fac = double_arg(ps[11]);
    if (fac < 0)
      fac = -fac;
    if (fac > 1)
      fac = 1.0/fac;
  } else
    fac = 0.9;

  if (narg >= 13 && ps[12])     // NITER: number of iterations per cycle
    nIter = int_arg(ps[12]);
  else
    nIter = 10;

  if (narg >= 14 && ps[13])     // NSAME: threshold on constant parameters
    nSame = int_arg(ps[13]);
  else
    nSame = 5;

  if (narg >= 15 && ps[14]) {   // ERR: output variable for error estimates
    if (ps[14] < nFixed || ps[14] >= NAMED_END) {
      if (xSym)
        free(xp);
      return luxerror("Need named output variable", ps[14]);
    }
    i = nPar;
    redef_array(ps[14], LUX_DOUBLE, 1, &i);
    err = (double *) array_data(ps[14]);
  } else {
    err = (double*) malloc(nPar*sizeof(double));
    if (!err)
      return cerror(ALLOC_ERR, 0);
  }

  if (narg >= 17 && ps[16])    // TTHRESH: threshold on CPU time
    tThresh = double_arg(ps[16]);
  else
    tThresh = 3600;

  vocal = (internalMode & 1)? 1: 0; // /VOCAL
  vocal_err = ((internalMode & 129) == 129)? 1: 0; // /VERR
  if (internalMode & 4)         // /DOWN
    dir = 0.8;
  else
    dir = 1./0.8;

  PoissonChiSq = internalMode & 8; // get Poisson chi-squares fit (if
                                   // applicable)

  i = nPar + 1;
  fitPar = array_scratch(LUX_DOUBLE, 1, &i); // fit parameters
  par = (double *) array_data(fitPar);

  size = nPar*sizeof(double);

  // copy start values into par
  memcpy(par, start, size);

  // enforce upper and lower bounds
  enforce_bounds(par, lowbound, hibound, nPar);

  if (fitSym) {                 // prepare executable for user-defined
                                // fit function
    if (isFreeTemp(fitPar))
      symbol_context(fitPar) = 1; // so they won't be deleted
    if (isFreeTemp(xTemp))
      symbol_context(xTemp) = 1;
    if (isFreeTemp(ySym))
      symbol_context(ySym) = 1;
    if (weights && isFreeTemp(wSym))
      symbol_context(wSym) = 1;
    fitArg[0] = fitPar;
    fitArg[1] = xTemp;
    fitArg[2] = ySym;
    if (weights)
      fitArg[3] = wSym;
    fitTemp = nextFreeTempExecutable();
    symbol_class(fitTemp) = LUX_USR_FUNC;
    usr_func_arguments(fitTemp) = fitArg;
    symbol_memory(fitTemp) = (weights? 4: 3)*sizeof(int16_t);
    usr_func_number(fitTemp) = fitSym;
  }

  if (tThresh)
    starttime = time(0);
  else
    starttime = 0;

  onebyone = (internalMode & 64? 1: 0);
  // internalMode is likely to change due to calling the user function

  // get initial fit quality
  if (fitSym) {
    i = eval(fitTemp);
    if (i < 0) {                // some error
      if (narg <= 14 || !ps[14])
        free(err);
      if (!xSym)
        zap(xTemp);
      if (symbol_context(ySym) == 1)
        symbol_context(ySym) = -compileLevel; // so they'll be deleted
                                              // when appropriate
      if (symbol_context(fitPar) == 1)
        symbol_context(fitPar) = -compileLevel;
      if (xSym && symbol_context(xSym) == 1)
        symbol_context(xSym) = -compileLevel;
      if (weights && symbol_context(wSym) == 1)
        symbol_context(wSym) = -compileLevel;
      return i;
    }
    qBest2 = qBest1 = double_arg(i);
    zapTemp(i);
  } else
    qBest2 = qBest1 = fitFunc(par, nPar, xp, yp, weights, nPoints);
  ALLOCATE(parBest1, nPar, double);
  ALLOCATE(parBest2, nPar, double);
  ALLOCATE(ran, nPar, double);
  memcpy(parBest1, par, size);
  memcpy(parBest2, par, size);
  memcpy(err, step, size);
  ALLOCATE(meanShift, nPar, double);
  for (i = 0; i < nPar; i++)
    meanShift[i] = 0.0;

  int bad = 0;
  iter = 0;
  same = 0;
  mu = 0;
  double qLag = 1.2*qBest2;
  if (!qLag)
    qLag = 1;
  do {                          // iterate
    qLast = qBest2;
    nn = onebyone? nPar: nIter;
    if (onebyone)
      randome(ran, nPar, 0);
    for (i = 0; i < nn; i++) {
      if (!onebyone)
        randome(ran, nPar, 0);  // get random numbers
      mu = 2 - mu/qLast;
      if (mu < 1)
        mu = 1;
      if (onebyone) {
        par[i] = parBest1[i] + mu*meanShift[i] + err[i]*ran[i];
        enforce_bounds(par, lowbound, hibound, nPar);
      } else {
        for (j = 0; j < nPar; j++) // update parameters
          par[j] = parBest1[j] + mu*meanShift[j] + err[j]*ran[j];
        enforce_bounds(par, lowbound, hibound, nPar);
      }
      if (fitSym) {
        j = eval(fitTemp);
        if (j < 0) {            // some error
          bad = 1;
          break;
        }
        qual = double_arg(j);
        zapTemp(j);
      } else
        qual = fitFunc(par, nPar, xp, yp, weights, nPoints); // fit quality
      if (qual < qBest1) {      // this one is better
        qBest1 = qual;
        memcpy(parBest1, par, size);
      } else {                  // restore parameter
        if (onebyone)
          par[i] = parBest1[i];
      }
    } // end for (i = 0; i < nn; i++)

    if (bad)
      break;

    if (qBest1 < qBest2) {      // this cycle yielded a better one
      same = 0;
      for (j = 0; j < nPar; j++) {
        temp = parBest1[j] - parBest2[j];
        if (temp) {             // some improvement due to this parameter
          meanShift[j] = meanShift[j]*fac + temp*(1 - fac);
          err[j] = (err[j]*fac + 2*fabs(temp)*(1 - fac))/dir;
        } else {
          meanShift[j] = 0;
        }
      }
      qBest2 = qBest1;
      memcpy(parBest2, parBest1, size);
    } else {
      for (j = 0; j < nPar; j++) {
        meanShift[j] = 0;
        err[j] *= dir;
      }
    }
    iter++;
    same++;
    qLag = qLag*0.98 + qBest2*0.02;
    if (vocal) {
      printf("\r%3d %#9.4g(%#6.2f):", iter, qBest2, qBest2? qLag*0.99/qBest2 - 1: 0);
      n = 0;
      for (j = 0; j < nPar; j++)
        if (step[j]) {
          printf("%#10.4g", parBest2[j]);
          if (vocal_err)
            printf("(%#8.2g)", err[j]);
          n++;
          if (n >= 14/(vocal_err? 2: 1)) // we show at most twenty
            break;
        }
    }
    n = 0;
    if (pThresh) {
      for (j = 0; j < nPar; j++)
        if (err[j] < pThresh[j])
          n++;
    }
    mu = fabs(qLast - qBest2);
    /* NOTE: difftime() doesn't work correctly in glibc-2.12.2; it
       often seems to return its first argument, rather than the
       difference between its arguments. So, subtract time_t values
       instead. */
  } while (qBest2 >= qThresh
           && (mu >= dThresh*fabs(qBest2) || !mu)
           && qBest2 < qLag*0.99
           && n < nPar
           && (!iThresh || iter < iThresh)
           && same <= nSame
           && (!tThresh || time(NULL) - starttime < tThresh));
  if (!bad) {
    memcpy(par, parBest2, size);
    par[nPar] = qBest2;

    if (qBest2) {
      // presumably we're close to a local minimum now; home in
      double qualplus, qualmin;
      for (i = 0; i < nPar; i++) {
        if (step[i]) {
          double h = fabs(step[i]);
          double chi2;
          int count = 0;
          do {
            ++count;
            par[i] = parBest2[i] + h;
            if (fitSym) {
              j = eval(fitTemp);
              if (j < 0) {
                bad = 1;
                break;
              }
              qualplus = double_arg(j);
              zapTemp(j);
            } else
              qualplus = fitFunc(par, nPar, xp, yp, weights, nPoints);
            par[i] = parBest2[i] - h;
            if (fitSym) {
              j = eval(fitTemp);
              if (j < 0) {
                bad = 1;
                break;
              }
              qualmin = double_arg(j);
              zapTemp(j);
            } else
              qualmin = fitFunc(par, nPar, xp, yp, weights, nPoints);

            double z = (qualplus - qualmin)/qBest2;
            chi2 = (qualplus + qualmin)/qBest2 - 0.5*z*z - 2;
            double q;
            if (chi2 > 0) {
              par[i] = parBest2[i] - 0.5*h*z/chi2;
              if (fitSym) {
                j = eval(fitTemp);
                if (j < 0) {
                  bad = 1;
                  break;
                }
                q = double_arg(j);
                zapTemp(j);
              } else
                q = fitFunc(par, nPar, xp, yp, weights, nPoints);
              if (q < qBest2) {
                qBest2 = q;
                parBest2[i] = par[i];
              }
              h *= sqrt(2/chi2);
            }
            else                // step is way too small
              h *= 10;
          } while ((chi2 > 3 || chi2 < 1) && h < err[i]*1e6 && count < 10);

          if (bad)
            break;

          par[i] = parBest2[i];         // restore
          err[i] = h;
        } else
          err[i] = 0;
      }
    } else {                    // perfect fit
      for (i = 0; i < nPar; i++) {
        err[i] = 0;
      }
    }
    par[nPar] = qBest2;

    if (!bad) {
      if (vocal) {
        iter++;
        if (vocal) {
          printf("\r%3d %#9.4g(%#6.2f):", iter, qBest2, qBest2? qLag*0.99/qBest2 - 1: 0);
          n = 0;
          for (j = 0; j < nPar; j++)
            if (step[j]) {
              printf("%#10.4g", parBest2[j]);
              if (vocal_err)
                printf("(%#8.2g)", err[j]);
              n++;
              if (n >= 14/(vocal_err? 2: 1)) // we show at most fourteen
                break;
            }
        }
        printf(" \n");
      }

      if (!xSym)
        zap(xTemp);
      if (fitSym) {
        symbol_class(fitTemp) = LUX_SCALAR;
        zap(fitTemp);
      }
      free(parBest1);
      free(parBest2);
      free(ran);
      free(meanShift);
      if (symbol_context(ySym) == 1)
        symbol_context(ySym) = -compileLevel;
      if (symbol_context(fitPar) == 1)
        symbol_context(fitPar) = -compileLevel;
      if (xSym && symbol_context(xSym) == 1)
        symbol_context(xSym) = -compileLevel;
      if (weights && symbol_context(wSym) == 1)
        symbol_context(wSym) = -compileLevel;
      if (narg <= 14 || !ps[14])
        free(err);
      return fitPar;
    }
  }

  if (narg <= 14 || !ps[14])
    free(err);
  if (!xSym)
    zap(xTemp);
  free(parBest1);
  free(parBest2);
  free(ran);
  free(meanShift);
  return j;
}
//------------------------------------------------------------
gsl_vector *gsl_vector_from_lux_symbol(int32_t iq, int32_t axis)
{
  gsl_vector *v;
  int32_t ndim, *dims, nelem, i;
  Pointer data;

  iq = lux_double(1, &iq);
  if (numerical(iq, &dims, &ndim, &nelem, &data) < 0
      || axis >= ndim) {
    errno = EDOM;
    return NULL;
  }

  v = (gsl_vector*) calloc(1, sizeof(gsl_vector));
  // v->owner = 0 is essential!
  if (!v) {
    errno = ENOMEM;
    return NULL;
  }
  v->stride = 1;
  if (axis < 0) {
    v->size = nelem;
  } else {
    v->size = dims[axis];
    for (i = 0; i < axis; i++)
      v->stride *= dims[i];
  }
  v->data = data.d;
  return v;
}
//------------------------------------------------------------
double fit2_func(const gsl_vector *a, void *p)
{
  lux_func_if *afif = (lux_func_if *) p;
  Pointer par = lux_func_if_get_param_data(afif, 0);
  memcpy(par.d, a->data, a->size*sizeof(double));
  double result = lux_func_if_call(afif);
  if (isnan(result))
    return GSL_NAN;
  return result;
}

/*

  par = FIT3(x, y, start, step, funcname [, err, ithresh, sthresh,
  nithresh] [, /VOCAL])

  Seeks parameters <par> such that <funcname>(<par>, <x>, <y>) is as
  small as possible.  The last element of <par> contains the standard
  deviation of the difference <y> and <funcname>(<par>, <x>, <y>).

  We use the GSL framework to seek X that minimizes the value of
  function double generalfit2_func(const gsl_vector * X, void *
  PARAMS).  Their X corresponds to our <par>, and their PARAMS
  corresponds to our <x> and <y> -- confusing!

  <err> receives estimates of the uncertainty in the elements of
  <par>.

  <ithresh> is the maximum number of iterations.  It defaults to 0,
  which means "no limit".

  <stresh> is the threshold on the linear size of the parameter space
  volume that is being searched for the minimum by the algorithm.  If
  the size becomes less than the threshold, then the search stops.  It
  defaults to 0, which means "no limit".

  <nithresh> is the maximum allowed number of successive iterations
  during which no better fit is found.  If no improvement is found
  after more than that many iterations (once the search is well on its
  way), then the search is stopped.  This parameter defaults to 10
  times the number of parameters.  0 means "no limit".

*/
int32_t lux_generalfit2(ArgumentCount narg, Symbol ps[])
{
#if GSL_INCLUDE
  int32_t nPar, nPoints, result, ithresh = 0, nithresh;
  gsl_multimin_fminimizer *minimizer = NULL;
  lux_func_if *afif = NULL;
  gsl_vector *par_v = NULL, *step_v = NULL;
  int32_t d_step_sym, d_par_sym, d_x_sym, d_y_sym;
  double *errors = NULL, sthresh = 0;

  int32_t vocal = (internalMode & 1);
  result = 0;

  {
    int32_t nx, *dims, ndim, nStep;

    d_x_sym = lux_double(1, &ps[0]);    // X
    if (isFreeTemp(d_x_sym))
      symbol_context(d_x_sym) = 1;     // avoid premature deletion
    d_y_sym = lux_double(1, &ps[1]);    // Y
    if (isFreeTemp(d_y_sym))
      symbol_context(d_y_sym) = 1;
    d_par_sym = lux_double(1, &ps[2]); // PAR
    if (isFreeTemp(d_par_sym))
      symbol_context(d_par_sym) = 1;    // avoid premature deletion
    d_step_sym = lux_double(1, &ps[3]); // STEP
    if (isFreeTemp(d_step_sym))
      symbol_context(d_step_sym) = 1;
    if (numerical(d_x_sym, &dims, &ndim, &nx, NULL) < 0 // X
        || numerical(d_y_sym, NULL, NULL, &nPoints, NULL) < 0 // Y
        || numerical(d_par_sym, NULL, NULL, &nPar, NULL) < 0
        || numerical(d_step_sym, NULL, NULL, &nStep, NULL) < 0)
      return LUX_ERROR;
    if (nStep == nPar - 1)
      --nPar;                   // assume last element of START is quality
    if (nStep != nPar) {
      result = luxerror("Number of elements (%d) in step argument is unequal to number of elements (%d) in parameters argument", ps[3], nStep, nPar);
    }
  }
  if (!result) {
    if (!symbolIsString(ps[4])) {       // FUNCNAME
      result = cerror(NEED_STR, ps[4]);
    }
  }
  if (!result) {
    if (narg > 5 && ps[5]) {    // ERR
      redef_array(ps[5], LUX_DOUBLE, 1, &nPar);
      errors = (double *) array_data(ps[5]);
    }
    if (narg > 6 && ps[6])      // ITHRESH
      ithresh = int_arg(ps[6]);
    if (ithresh <= 0)
      ithresh = 10000;
    if (narg > 7 && ps[7])      // STHRESH
      sthresh = double_arg(ps[7]);
    if (narg > 8 && ps[8])      // NITHRESH
      nithresh = int_arg(ps[8]);
    else
      nithresh = sqrt(nPar)*10;
    if (nithresh <= 0)
      nithresh = INT32_MAX;

    par_v = gsl_vector_from_lux_symbol(d_par_sym, -1);
    step_v = gsl_vector_from_lux_symbol(d_step_sym, -1);

    afif = lux_func_if_alloc(string_value(ps[4]), 3);
    if (!afif) {
      switch (errno) {
      case EDOM:
        result = luxerror("Cannot fit to internal routine", ps[4]);
        break;
      case ENOMEM:
        result = cerror(ALLOC_ERR, 0);
        break;
      default:
        result = luxerror("Unhandled error %d", 0, errno);
        break;
      }
    }
  }

  if (!result) {
    lux_func_if_set_param(afif, 0, d_par_sym);           // par
    lux_func_if_set_param(afif, 1, d_x_sym);             // x
    lux_func_if_set_param(afif, 2, d_y_sym);             // y

    minimizer = gsl_multimin_fminimizer_alloc(gsl_multimin_fminimizer_nmsimplex2,
                                              nPar);
    if (!minimizer) {
      result = luxerror("Unable to allocate memory for minimizer", 0);
    }
  }

  gsl_multimin_function m_func;
  if (!result) {
    m_func.n = nPar;
    m_func.f = &fit2_func;
    m_func.params = (void *) afif;

    par_v->size = nPar; /* GSL requires the par size to match the number
                           of parameters, but we want to keep room for
                           storing the quality */

    if (gsl_multimin_fminimizer_set(minimizer, &m_func, par_v, step_v)) {
      result = LUX_ERROR;
    }
  }

  if (!result) {
    int32_t status;
    int32_t iter = 0;
    int32_t show = nPar;
    if (nPar > 9)
      show = 9;
    time_t report_after;
    if (vocal)
      report_after = time(NULL);
    double oldqual, newqual = 0, vocal_oldqual = 0, size, vocal_oldsize = 0;
    int32_t no_improvement_niter = 0;
    do {
      ++iter;
      status = gsl_multimin_fminimizer_iterate(minimizer);
      if (status)
        break;
      size = gsl_multimin_fminimizer_size(minimizer);
      oldqual = newqual;
      newqual = sqrt(minimizer->fval/nPoints);
      double improvement = newqual - oldqual;
      if (improvement)
        no_improvement_niter = 0;
      else
        ++no_improvement_niter;
      if (vocal && time(NULL) > report_after) {
        int32_t i, j = 0;
        double vocal_improvement = newqual - vocal_oldqual;
        vocal_oldqual = newqual;
        double size_improvement = size - vocal_oldsize;
        vocal_oldsize = size;
        printf("%d %g (%.3g) %.3g (%+.3g):", iter, newqual, vocal_improvement,
               size, size_improvement);
        for (i = 0; i < nPar && j < show; i++) {
          if (step_v->data[i]) {
            printf(" %g", gsl_vector_get(minimizer->x, i));
            j++;
          }
        }
        putchar('\n');
        report_after = time(NULL) + 2;
      }
    } while (iter < ithresh
             && gsl_multimin_test_size(size, sthresh) == GSL_CONTINUE
             && (no_improvement_niter < nithresh
                 || no_improvement_niter == iter - 1));
    gsl_vector *best_par = gsl_multimin_fminimizer_x(minimizer);

    {
      int32_t n = best_par->size + 1;
      result = array_scratch(LUX_DOUBLE, 1, &n);
    }
    double *tgt = (double*) array_data(result);
    memcpy(tgt, best_par->data, best_par->size*sizeof(double));
    double best_min = lux_func_if_call(afif);
    tgt[best_par->size] = best_min;

    if (errors) {
      int32_t i;
      for (i = 0; i < nPar; i++) {
        if (step_v->data[i]) {
          double h;
          memcpy(par_v->data, best_par->data, nPar*sizeof(double));
          h = step_v->data[i];
          double qualplus, qualmin, r;
          do {
            par_v->data[i] = best_par->data[i] + h;
            qualplus = lux_func_if_call(afif);
            par_v->data[i] = best_par->data[i] - h;
            qualmin = lux_func_if_call(afif);
            r = (qualplus + qualmin)/best_min;
            if (r > 4)
              h /= sqrt(r);
            else if (r < 3)
              h *= sqrt(r);
          } while ((r > 4 || r < 3) && h < errors[i]*1e6);
          double v = sqrt((qualplus + qualmin)/(2*best_min) - 1);
          if (v)
            errors[i] = h/v;
          else
            errors[i] = INFTY;
          par_v->data[i] = best_par->data[i];
        } else
          errors[i] = 0.0;
      }
    }
  }
  gsl_vector_free(par_v);
  gsl_vector_free(step_v);
  gsl_multimin_fminimizer_free(minimizer);
  lux_func_if_free(afif);
  if (symbol_context(d_par_sym) == 1)
    symbol_context(d_par_sym) = -compileLevel; // so it is deleted when appropriate
  if (symbol_context(d_x_sym) == 1)
    symbol_context(d_x_sym) = -compileLevel; // so it is deleted when appropriate
  if (symbol_context(d_y_sym) == 1)
    symbol_context(d_y_sym) = -compileLevel; // so it is deleted when appropriate
  if (symbol_context(d_step_sym) == 1)
    symbol_context(d_step_sym) = -compileLevel; // so it is deleted when appropriate
  return result;
#else
  return cerror(NOSUPPORT, 0, "FIT3", "libgsl");
#endif
}
REGISTER(generalfit2, f, fit3, 5, 7, "x:y:start:step:f:err:ithresh:1vocal");
//------------------------------------------------------------ This
// This union differs from Pointer in that all integer members are
// unsigned.
union uscalar {
  uint8_t* ui8;
  uint16_t* i16;
  uint32_t* i32;
  float *f;
  double *d;
};

// replace NaN values by (other) random values
void denan(uint8_t *data, int32_t size, int32_t partype)
/* <data> = start of data
   <size> = size of data, in bytes
   <partpe> = data type (LUX_INT8 ... LUX_FLOAT) */
{
  uscalar p;
  int32_t i;

  switch (partype) {
  case LUX_FLOAT:
    p.f = (float *) data;
    for (i = 0; i < size/sizeof(float); i++) {
      while (isnan(*p.f)) {
        uint32_t *r;
        for (r = (uint32_t *) p.ui8; (uint8_t *) r < p.ui8 + sizeof(float); r++)
          *r = random_bits();
      }
      p.f++;
    }
    break;
  case LUX_DOUBLE:
    p.d = (double *) data;
    for (i = 0; i < size/sizeof(double); i++) {
      while (isnan(*p.d)) {
        uint32_t *r;
        for (r = (uint32_t *) p.ui8; (uint8_t *) r < p.ui8 + sizeof(double); r++) {
          *r = random_bits();
        }
      }
      p.d++;
    }
    break;
  }
}

void printgene(uint8_t *gene, int32_t nPar, int32_t partype, int32_t showbits,
               double *quality) {
  int32_t j;
  uscalar p;

  p.ui8 = gene;
  putchar('[');
  for (j = 0; j < nPar; j++) {
    if (j)
      putchar(' ');
    switch (partype) {
    case LUX_INT8:
      printf("%u", p.ui8[j]);
      break;
    case LUX_INT16:
      printf("%u", p.i16[j]);
      break;
    case LUX_INT32:
      printf("%u", p.i32[j]);
      break;
    case LUX_FLOAT:
      printf("%g", p.f[j]);
      break;
    case LUX_DOUBLE:
      printf("%g", p.d[j]);
      break;
    }
    if (showbits) {
      int32_t i;
      uint8_t *b = gene + lux_type_size[partype]*j;
      printf(" {");
      for (i = lux_type_size[partype] - 1; i >= 0; i--)
        printf("%02x", b[i]);
      printf("}");
    }
  }
  if (quality)
    printf(": %16.12g", *quality);
  putchar(']');
}

void printgenenl(uint8_t *gene, int32_t nPar, int32_t partype, int32_t showbits,
                 double *quality) {
  printgene(gene, nPar, partype, showbits, quality);
  putchar('\n');
}

int32_t hasnan(uint8_t *gene, int32_t nPar, int32_t partype) {
  int32_t i;
  uscalar p;

  switch (partype) {
  case LUX_FLOAT:
    p.f = (float *) gene;
    for (i = 0; i < nPar; i++) {
      if (isnan(*p.f++))
        return 1;
    }
    break;
  case LUX_DOUBLE:
    p.d = (double *) gene;
    for (i = 0; i < nPar; i++) {
      if (isnan(*p.d++))
        return 1;
    }
    break;
  default:
    break;
  }
  return 0;
}

/*
   fitness = 1/deviation
   avg fitness => reproduction probability = 1/nPopulation
   greatest fitness => reproduction probability = mu/nPopulation < 1
   linear relation; P < 0 => 0; P > 1 => 1
   p(avg) = 1/nPopulation
   p(best) = mu/nPopulation
   p ~ (fitness - avg)/(best - avg) * (mu - 1) + 1
     = (fitness*mu - fitness - avg*mu + avg + best - avg)/(best - avg)
     = (fitness*mu - fitness - avg*mu + best)/(best - avg)
     = (fitness*(mu - 1) + best - avg*mu)/(best - avg)
     = fitness*((mu - 1)/(best - avg)) + (best - avg*mu)/(best - avg)
*/
void calculate_distribution(double *distr, double *deviation, int32_t *rtoi,
                            int32_t nPopulation, double mu)
{
  double sum = 0.0;
  int32_t i;

  for (i = 0; i < nPopulation; i++)
    sum += 1/deviation[i];
  double a = 1, b = 0;
  if (sum > 0) {
    double avg_fitness = sum/nPopulation;
    double best_fitness = 1/deviation[rtoi[nPopulation - 1]];
    if (avg_fitness != best_fitness) {
      a = (mu - 1)/(best_fitness - avg_fitness);
      b = (best_fitness - avg_fitness*mu)/(best_fitness - avg_fitness);
    }
  }
  sum = 0.0;
  distr[0] = 0.0;
  for (i = 1; i <= nPopulation; i++) {
    double x = 1/deviation[rtoi[i - 1]];
    if (x) {
      x = a*x + b;
      if (x < 0)
        x = 0;
    }
    sum += x;
    if (i < nPopulation)
      distr[i] = x;
  }

  if (sum) {
    for (i = 1; i < nPopulation; i++)
      distr[i] = distr[i - 1] + distr[i]/sum;
  } else {
    for (i = 1; i < nPopulation; i++)
      distr[i] = 1;
  }
}

int32_t lux_geneticfit(ArgumentCount narg, Symbol ps[])
/* FIT2(x,y,START,fit [,mu,ngenerations,population,pcross,pmutate,vocal]
        [,/ELITE,/BYTE,/WORD,/LONG,/FLOAT,/DOUBLE]) */
{
  int32_t   fitSym, iq, nPoints, nPopulation, nPar, fitTemp, i, j, size, result,
    *rtoi, pair, k, w, generation, i1, i2, ibit,
    iter = 0, vocal, typesize, nGeneration;
  uscalar p;
  int16_t  *par, fitPar, xSym, ySym, fitArg[4], wSym;
  uint8_t  *genes, *parent1, *parent2, *genes2, t1, t2;
  double *deviation, mu, *distr, random_one(void), pcross, *deviation2,
    crossmark, mutatemark, pmutate, sum;
  double *weights, *start;
  void  invertPermutation(int32_t *data, int32_t n),
    indexxr_f(int32_t n, float ra[], int32_t indx[]);
  int32_t   random_distributed(int32_t modulus, double *distr);
  uint8_t  changed, elite;
  Symboltype partype;
  static uint16_t mask1[] = {
    0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01
  }, mask2[] = {
    0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe,
  }, mask3[] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  };
  int32_t *crossoversites, *mutationsites;
  void indexxr_d(int32_t, double *, int32_t *);

  iq = ps[3];                   // fit
  if (!symbolIsStringScalar(iq))
    return cerror(NEED_STR, ps[3]);
  fitSym = stringpointer(string_value(iq), SP_USER_FUNC);
  if (fitSym < 0)
    return
      luxerror("Sorry, fitting to interal functions is not yet implemented",
            iq);

  if (symbolIsScalar(ps[2])) {
    nPar = int_arg(ps[2]);      // nPar
    if (nPar < 1)
      return luxerror("A positive value is required for the number of parameters",
                      ps[2]);
    start = NULL;
    switch (internalMode/2 & 7) {
    case 1:
      partype = LUX_INT8;
      break;
    case 2: default:
      partype = LUX_INT16;
      break;
    case 3:
      partype = LUX_INT32;
      break;
    case 4:
      partype = LUX_FLOAT;
      break;
    case 5:
      partype = LUX_DOUBLE;
      break;
    }
  } else if (symbolIsArray(ps[2])) { // start values
    int32_t iq = lux_double(1, &ps[2]);
    nPar = array_size(iq);
    start = (double *) array_data(iq);
    partype = symbol_type(iq);
  }

  if (narg > 5 && (iq = ps[5])) { // mu: selection pressure
    mu = double_arg(iq);
    if (mu < 1.0)
      mu = 1.0;
  } else
    mu = 1.5;

  if (narg > 6 && (iq = ps[6])) { // generations: number of generations
    nGeneration = int_arg(iq);
    if (nGeneration < 1)
      nGeneration = 1;
  } else
    nGeneration = 50;

  if (narg > 8 && (iq = ps[8])) { // pcross: cross-over probability
    pcross = double_arg(iq);
    if (pcross < 0.0)
      pcross = 0.0;
    else if (pcross > 1.0)
      pcross = 1.0;
  } else
    pcross = 0.1;
  pcross /= 2;

  if (narg > 7 && (iq = ps[7])) { // population
    nPopulation = int_arg(iq);
    if (nPopulation < 1)
      nPopulation = 1;
    // we make sure that the population fills a number of ints exactly
    if (partype < LUX_INT32)
      nPopulation += (nPopulation % (sizeof(int32_t)/lux_type_size[partype]));
  } else
    nPopulation = 100;

  if (narg > 9 && (iq = ps[9])) { // pmutate: mutation probability
    pmutate = double_arg(iq);
    if (pmutate < 0.0)
      pmutate = 0.0;
    else if (pmutate > 1.0)
      pmutate = 1.0;
  } else
    pmutate = 1.0/nPopulation;
  pmutate /= 2;

  if (!symbolIsNumericalArray(ps[0])) // x
    return cerror(NEED_NUM_ARR, ps[0]);
  xSym = lux_double(1, ps);
  if (isFreeTemp(xSym))
    symbol_context(xSym) = 1;   // so it doesn't get prematurely deleted
  nPoints = array_size(xSym);

  if (!symbolIsNumericalArray(ps[1])) // y
    return cerror(NEED_NUM_ARR, ps[1]);
  if (array_size(ps[1]) != nPoints)
    return cerror(INCMP_ARG, ps[1]);
  ySym = lux_double(1, ps + 1);
  if (isFreeTemp(ySym))
    symbol_context(ySym) = 1;

  if (narg > 4 && (iq = ps[4])) { // weights
    int32_t n;
    if (symbol_class(iq) != LUX_ARRAY)
      return cerror(NEED_ARR, iq);
    wSym = lux_double(1, &iq);
    n = array_size(wSym);
    if (n != nPoints)
      return cerror(INCMP_ARG, iq);
    weights = (double *) array_data(wSym);
    if (isFreeTemp(wSym))
      symbol_context(wSym) = 1;
  } else
    weights = NULL;

  if (narg > 10 && (iq = ps[10])) // VOCAL
    vocal = int_arg(iq);
  else
    vocal = 0;

  elite = (internalMode & 1);

  fitPar = array_scratch(partype, 1, &nPar);
  symbol_context(fitPar) = 1;   // so it doesn't get prematurely deleted
  par = (int16_t*) array_data(fitPar);
  typesize = lux_type_size[partype];
  size = nPar*typesize;

  fitArg[0] = fitPar;
  fitArg[1] = xSym;
  fitArg[2] = ySym;
  if (weights)
    fitArg[3] = wSym;
  fitTemp = nextFreeTempExecutable();
  symbol_class(fitTemp) = LUX_USR_FUNC;
  usr_func_arguments(fitTemp) = fitArg;
  symbol_memory(fitTemp) = (weights? 4: 3)*sizeof(int16_t);
  usr_func_number(fitTemp) = fitSym;

  // create initial population
  genes = (uint8_t*) malloc(nPopulation*size);
  genes2 = (uint8_t*) malloc(nPopulation*size);
  deviation = (double*) malloc(nPopulation*sizeof(double));
  deviation2 = (double*) malloc(nPopulation*sizeof(double));
  rtoi = (int32_t*) malloc(nPopulation*sizeof(int32_t));
  // itor = malloc(nPopulation*sizeof(int32_t));

  // we fill <genes> with random bits.  <genes> exactly spans a number
  // of ints, so we don't need to worry about the exact type
  p.i32 = (uint32_t *) genes;
  if (start) {                  // fill first member with start values
    memcpy(p.d, start, nPar*sizeof(double));
    p.d += nPar;
    i = size/sizeof(int32_t);
  } else
    i = 0;
  for ( ; i < nPopulation*size/sizeof(int32_t); i++)
    *p.i32++ = random_bits();
  // if the parameter type is FLOAT or DOUBLE, then some of the bit patterns
  // may not correspond to representable values, but rather to NaNs.
  // We must fix those elements. We use a single scheme for both FLOAT
  // and DOUBLE values, assuming that when the bit patterns of a DOUBLE
  // NaN are interpreted as the bit patterns of some FLOAT values instead,
  // then at least one of those FLOATs is a FLOAT NaN, and replacing that
  // FLOAT NaN with a representable FLOAT value generates a representable
  // DOUBLE value.
  denan(genes, nPopulation*size, partype);

  int bad = 0;

  /* now calculate the fitness of all members of the population.
     Less is better. */
  for (i = 0; i < nPopulation; i++) {
    memcpy(par, genes + i*size, size);
    j = eval(fitTemp);          // get deviation ("distance from goal")
    if (j == LUX_ERROR) {       // some error occurred
      bad = 1;
      break;
    }
    deviation[i] = fabs(double_arg(j)); // distance from goal
    if (vocal) {
      printf("%d/%d: ", i, nPopulation);
      printgene(genes + i*size, nPar, partype, 0, &deviation[i]);
      putchar('\n');
    }
    zapTemp(j);
  }

  uint8_t *child1 = NULL;
  uint8_t *child2 = NULL;

  if (!bad) {
    internalMode = 0;             // or we may get unexpected results
    indexxr_d(nPopulation, deviation, rtoi); // get ranking
    /* now deviation[rtoi[i]] is the i-th largest distance from goal;
       rtoi[0] is the worst fit, rtoi[nPopulation - 1] is the best fit */

    /* calculate the distribution function for selecting members.
       member i gets a reproduction probability equal to
       distr[i+1] - distr[i] */
    distr = (double*) malloc(nPopulation*sizeof(double));
    calculate_distribution(distr, deviation, rtoi, nPopulation, mu);

    if (vocal > 1) {
      p.ui8 = genes;
      for (i = 0; i < nPopulation; i++) {
        printf("%d/%d ", i, nPopulation);
        printgene(p.ui8 + rtoi[i]*size, nPar, partype, 0, &deviation[rtoi[i]]);
        putchar('\n');
      }
    }

    if (vocal) {
      sum = 0.0;
      for (i = 0; i < nPopulation; i++)
        sum += deviation[i];
      printf("%3d: %8.5g %8.5g %8.5g %8.5g\n", ++iter, deviation[rtoi[0]],
             deviation[rtoi[nPopulation/2]], deviation[rtoi[nPopulation - 1]],
             sum/nPopulation);
    }

    /* we want crossover probability equal to <pcross>
       and mutation probability equal to <pmutate>.
       We do crossover after <crossmark> pairs, and mutation
       after <mutatemark> pairs */
    crossmark = pcross? random_one()/pcross: LONG_MAX; // bytes
    mutatemark = pmutate? random_one()/pmutate: LONG_MAX; // bits

    child1 = (uint8_t*) malloc(size);
    child2 = (uint8_t*) malloc(size);
    crossoversites = (int32_t*) calloc(size*8, sizeof(int32_t));
    mutationsites = (int32_t*) calloc(size*8, sizeof(int32_t));

    int32_t mutatecount = 0;
    int32_t crossovercount = 0;
    int32_t offspringcount = 0;

    /* TODO: implement RESOLUTION argument, which says how many bytes
       per parameter to fit, beginning with the most significant Byte.
       TODO: implement additional stop criteria other than just
       "number of generations"; at least something like "stop if
       quality unchanged for a fixed number of iterations" */

    generation = nGeneration;
    // iterate over the desired number of generations
    while (generation--) {        // all generations
      if (elite) {                // always keep the best two
        memcpy(genes2, genes + size*rtoi[nPopulation - 1], size);
        genes2 += size;
        *deviation2++ = deviation[rtoi[nPopulation - 1]];
        memcpy(genes2, genes + size*rtoi[nPopulation - 2], size);
        genes2 += size;
        *deviation2++ = deviation[rtoi[nPopulation - 2]];
      }
      for (pair = elite; pair < nPopulation/2; pair++) { // remaining pairs
        // pick parents
        i1 = random_distributed(nPopulation, distr);
        do {
          i2 = random_distributed(nPopulation, distr);
        } while (i1 == i2);
        if (vocal > 1)
          printf("parents %d (%d) %d (%d)\n", i1, rtoi[i1], i2, rtoi[i2]);
        parent1 = genes + size*rtoi[i1];
        parent2 = genes + size*rtoi[i2];
        memcpy(child1, parent1, size);
        memcpy(child2, parent2, size);
        offspringcount += 2;

        changed = 0;
        while (crossmark < size*8) { // do cross-over
          ibit = crossmark;
          crossmark += random_one()/pcross;
          crossoversites[ibit]++;
          w = ibit/8;             // Byte index
          k = ibit - 8*w;         // bit index
          for (i = 0; i < w; i++) { // swap before cross-over int16_t
            t1 = child1[i];
            child1[i] = child2[i];
            child2[i] = t1;
          } // end of for (i = 0; ...)
          if (k) {                // cross-over site is not on int16_t boundary
            t1 = ((child1[w] & mask1[k]) | (child2[w] & mask2[k]));
            t2 = ((child2[w] & mask1[k]) | (child1[w] & mask2[k]));
            child1[w] = t1;
            child2[w] = t2;
          } // end of if (k)
          if (hasnan(child1, nPar, partype)
              || hasnan(child2, nPar, partype)) {
            // we generated one or two NaNs from regular numbers:
            // undo the crossover
            for (i = 0; i < w; i++) { // swap before cross-over int16_t
              t1 = child1[i];
              child1[i] = child2[i];
              child2[i] = t1;
            } // end of for (i = 0; ...)
            if (k) {              // cross-over site is not on int16_t boundary
              t1 = ((child1[w] & mask1[k]) | (child2[w] & mask2[k]));
              t2 = ((child2[w] & mask1[k]) | (child1[w] & mask2[k]));
              child1[w] = t1;
              child2[w] = t2;
            } // end of if (k)
          } else {
            changed++;
            crossovercount++;
            if (vocal > 2) {
              printf("cross-over %d %d: before = ", i1, i2);
              printgene(parent1, nPar, partype, 1, NULL);
              printf(" | ");
              printgene(parent2, nPar, partype, 1, NULL);
              printf("\ncross-over Byte %d bit %d\n", w, k);
              printf("           %d %d after   = ", i1, i2);
              printgene(child1, nPar, partype, 1, NULL);
              printf(" | ");
              printgene(child2, nPar, partype, 1, NULL);
              putchar('\n');
            }
          }
        }
        crossmark -= size*8;

        // do mutations
        while (mutatemark < size*8) {
          ibit = mutatemark;
          mutatemark += random_one()/pmutate;
          mutationsites[ibit]++;
          w = ibit/8;             // Byte index
          k = ibit - 8*w;         // bit index
          child1[w] ^= mask3[k]; // flip bit
          if (hasnan(child1, nPar, partype))
            // the mutation generated a NaN; undo
            child1[w] ^= mask3[k];
          else {
            changed++;
            mutatecount++;
            if (vocal > 2) {
              printf("mutation #1 %d: before = ", i1);
              printgene(parent1, nPar, partype, 1, NULL);
              printf("\nmutate Byte %d bit %d\n", w, k);
              printf("mutation #1 %d: after  = ", i1);
              printgene(child1, nPar, partype, 1, NULL);
              putchar('\n');
            }
          }
        }
        mutatemark -= size*8;
        while (mutatemark < size*8) {
          ibit = mutatemark;
          mutatemark += random_one()/pmutate;
          mutationsites[ibit]++;
          w = ibit/8;             // Byte index
          k = ibit - 8*w;         // bit index
          child2[w] ^= mask3[k]; // flip bit
          if (hasnan(child2, nPar, partype))
            // the mutation generated a NaN; undo
            child2[w] ^= mask3[k];
          else {
            changed++;
            mutatecount++;
            if (vocal > 2) {
              printf("mutation #2 %d: before = ", i2);
              printgene(parent2, nPar, partype, 1, NULL);
              printf("\nmutate Byte %d bit %d\n", w, k);
              printf("mutation #2 %d: after  = ", i2);
              printgene(child2, nPar, partype, 1, NULL);
              putchar('\n');
            }
          }
        }
        mutatemark -= size*8;

        // store processed genes in new population
        memcpy(genes2, child1, size);
        genes2 += size;
        memcpy(genes2, child2, size);
        genes2 += size;
        if (changed) { // TODO: only reevaluate for the changed child, not both
          memcpy(par, child1, size);
          j = eval(fitTemp);
          if (j == LUX_ERROR) {
            bad = 3;
            break;
          }
          *deviation2++ = double_arg(j);
          zapTemp(j);
          memcpy(par, child2, size);
          j = eval(fitTemp);
          if (j == LUX_ERROR) {
            bad = 3;
            break;
          }
          *deviation2++ = double_arg(j);
          zapTemp(j);
        } else {
          *deviation2++ = deviation[rtoi[i1]];
          *deviation2++ = deviation[rtoi[i2]];
        }
      } // end of for (pair = 0; ...)

      if (bad)
        break;

      genes2 -= nPopulation*size;
      deviation2 -= nPopulation;

      memcpy(genes, genes2, nPopulation*size);
      memcpy(deviation, deviation2, nPopulation*sizeof(double));

      indexxr_d(nPopulation, deviation, rtoi); // get ranking
      calculate_distribution(distr, deviation, rtoi, nPopulation, mu);

      if (vocal > 1) {
        p.ui8 = genes;
        for (i = 0; i < nPopulation; i++) {
          printf("%d/%d ", i, nPopulation);
          printgene(p.ui8 + rtoi[i]*size, nPar, partype, 0, &deviation[rtoi[i]]);
          putchar('\n');
        }
      }

      if (vocal) {
        sum = 0.0;
        for (i = 0; i < nPopulation; i++)
          sum += 1/deviation[i];
        printf("%3d: %8.5g %8.5g %16.12g %8.5g\n", ++iter, deviation[rtoi[0]],
               deviation[rtoi[nPopulation/2]], deviation[rtoi[nPopulation - 1]],
               sum/nPopulation);
      }
      /* memcpy(itor, rtoi, nPopulation*sizeof(int32_t));
         invertPermutation(itor, nPopulation); */
    } // end of while (generation--)

    if (!bad) {
      result = array_scratch(partype, 1, &nPar);
      memcpy((char*) array_data(result), genes + nPar*rtoi[nPopulation - 1]*typesize,
             size);
      if (vocal) {
        printf("%4d offspring (probability %g)\n%4d cross-overs (probability %g per gene pair)\n%4d mutations (probability %g per bit)\n",
               offspringcount, ((double) offspringcount)/(nPopulation*nGeneration),
               crossovercount, ((double) crossovercount)/(nPopulation*nGeneration/2),
               mutatecount, ((double) mutatecount)/(nPopulation*nGeneration*size*8));
        if (vocal > 1) {
          printf("cross-over site frequencies:\n");
          for (i = 0; i < size*8; i++)
            printf("%d ", crossoversites[i]);
          putchar('\n');
          printf("mutation site frequencies:\n");
          for (i = 0; i < size*8; i++)
            printf("%d ", mutationsites[i]);
          putchar('\n');
        }
      }
    }
  }

  if (bad == 3) {
    genes2 -= pair*2*nPar*typesize;
    deviation2 -= pair*2;

    if (symbol_context(ySym) == 1)
      symbol_context(ySym) = -compileLevel; // so they'll be deleted
    // when appropriate
    if (symbol_context(fitPar) == 1)
      symbol_context(fitPar) = -compileLevel;
    if (xSym && symbol_context(xSym) == 1)
      symbol_context(xSym) = -compileLevel;
    if (weights && symbol_context(wSym) == 1)
      symbol_context(wSym) = -compileLevel;
    result = LUX_ERROR;
  }

  zap(fitPar);
  symbol_class(fitTemp) = (Symbolclass) 0;
  symbol_type(fitTemp) = (Symboltype) 0;
  if (symbol_context(xSym) == 1)
    zap(xSym);
  if (symbol_context(ySym) == 1)
    zap(ySym);
  free(distr);
  free(rtoi);
  // free(itor);
  free(genes);
  free(deviation);
  free(genes2);
  free(deviation2);
  free(child1);
  free(child2);
  free(crossoversites);
  free(mutationsites);
  return result;
}
