/* File fit.c */
/* General fitting procedure */
/* Started 22 October 1995.  Louis Strous */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>             /* for LONG_MAX */
#include <time.h>               /* for difftime */
#include "action.h"
#include "ana_func_if.h"
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>
#include <limits.h>
static char rcsid[] __attribute__ ((unused)) =
"$Id: fit.c,v 4.0 2001/02/07 20:37:00 strous Exp $";

char    PoissonChiSq;           /* flag: Poisson chi-square fitting? */
uint32_t random_bits(void);
void indexxr_d(Int, Double *, Int *);

/*-----------------------------------------------------------------------*/
Double gaussians(Double *par, Int nPar, Double *x, Double *y, Double *w, Int nData)
/* Returns the rms error (if PoissonChiSq is zero) or chi-square for */
/* Poisson distributions (if PoissonChiSq is nonzero) in fitting y vs x */
/* with a set of gaussians. */
/* The fit profile is  par[0]+par[1]*exp(-((x - par[2])/par[3])^2) */
/* [ + par[4]*exp(-((x - par[5])/par[6])^2) ... ].  par[] must have */
/* at least 4 elements and for each three more another gaussian is */
/* added.   LS 26oct95  31oct95 */
{
  Int   j, wstep;
  Double        rms, fit, arg, one = 1.0;

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
/*-----------------------------------------------------------------------*/
Double powerfunc(Double *par, Int nPar, Double *x, Double *y, Double *w, Int nData)
/* returns the error in fitting y vs x with a general power function */
/* superposed on a linear trend. */
/* the fit profile is  par[0] + par[1]*x + par[2]*(x - par[3])^par[4] */
/* LS 13nov95 */
{
  Int   wstep;
  Double        rms, dfit, one;

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
/*-----------------------------------------------------------------------*/
void enforce_bounds(Double *par, Double *lowbound, Double *hibound, Int nPar)
{
  if (lowbound || hibound) {
    int i;
    for (i = 0; i < nPar; i++) {
      if (lowbound && hibound
	  && lowbound[i] > -INFTY && hibound[i] < INFTY) {
	if (par[i] < lowbound[i] || par[i] > hibound[i])
	  par[i] = fmod(par[i] - lowbound[i], hibound[i] - lowbound[i])
	    + lowbound[i];
      } else if (lowbound && lowbound[i] > -INFTY && par[i] < lowbound[i])
	par[i] = lowbound[i];
      else if (hibound && hibound[i] < INFTY && par[i] > hibound[i])
	par[i] = hibound[i];
    }
  }
}
/*-----------------------------------------------------------------------*/
Int ana_generalfit(Int narg, Int ps[])
/* FIT([X,]Y,START,STEP[,LOWBOUND,HIGHBOUND][,WEIGHTS][,QTHRESH,PTHRESH,
   ITHRESH,DTHRESH][,FAC,NITER,NSAME][,ERR][,FIT][,TTHRESH][,/VOCAL,/DOWN,
   /PCHI][/GAUSSIANS,/POWERFUNC]) */
/* This routine is intended to enable the user to fit iteratively */
/* to any profile specification. */
/* LS 24oct95 */
{
  Int   ySym, xSym, nPoints, n, nPar, nIter, iThresh, nSame, size,
    iq, i, j, iter, same, fitSym, fitTemp, xTemp, nn, wSym;
  Double *yp, *xp, *start, *step, qThresh, *pThresh, fac, *lowbound, *hibound,
        *err, *par, qBest1, qBest2, *parBest1, *parBest2, *ran, qual, temp,
    dir, dThresh, qLast, mu, *meanShift, *weights, tThresh;
  char  vocal, onebyone, vocal_err;
  void  randome(Double *output, Int number);
  Double (*fitProfiles[2])(Double *, Int, Double *, Double *, Double *, Int) =
  { gaussians, powerfunc };
  Double (*fitFunc)(Double *, Int, Double *, Double *, Double *, Int);
  extern Int    nFixed;
  Word  fitPar, fitArg[4];
  Int   ana_indgen(Int, Int []), eval(Int);
  void  zap(Int);
  time_t starttime;
  
  /* check input variables */
  if (narg >= 15 && (iq = ps[15])) { /* FIT */
    if (symbol_class(iq) != ANA_STRING)
      return cerror(NEED_STR, iq);
    n = SP_USER_FUNC;
    fitSym = stringpointer(string_value(iq), n);
    if (fitSym < 0)             /* internal routine rather than user-defined */
      return
        anaerror("Sorry, fitting to internal routines is not yet implemented",
              iq);
  } else
    fitSym = 0;

  i = (internalMode/16 & 3);    /* fit function */
  if (i) {
    if (fitSym)
      return anaerror("Multiple fit functions specified", fitSym);
    i--;
  }
  fitFunc = fitProfiles[i];     /* fit function */

  if (ps[1]) {                  /* X & Y */
    ySym = ps[1];
    xSym = ps[0];
  } else {                              /* only Y */
    ySym = ps[0];
    xSym = 0;
  }
  if (symbol_class(ySym) != ANA_ARRAY)
    return cerror(NEED_ARR, ySym);
  ySym = ana_double(1, &ySym);  /* ensure ANA_DOUBLE */
  nPoints = array_size(ySym);   /* number of data points */
  yp = (Double *) array_data(ySym); /* pointer to data */

  if (xSym)                     /* X */
    switch (symbol_class(xSym)) {
      case ANA_SCALAR:
        if (!fitSym)
          return anaerror("Need array in standard fit", xSym);
        /* fall-thru to next case */
      case ANA_ARRAY:
        if (!fitSym) {
          n = array_size(xSym);
          if (n != nPoints)
            return cerror(INCMP_ARG, ySym);
        }
        xSym = ana_double(1, &xSym);
        xTemp = xSym;
        if (symbol_class(xSym) == ANA_ARRAY)
          xp = (Double *) array_data(xSym);
        break;
      default:
        return cerror(ILL_CLASS, xSym);
    }
  else {
    xTemp = ana_indgen(1, &ySym);
    xp = (Double *) array_data(xTemp);
  }

  iq = ps[2];                   /* START: initial parameter values */
  if (symbol_class(iq) != ANA_ARRAY)
    return cerror(NEED_ARR, iq);
  iq = ana_double(1, &iq);
  nPar = array_size(iq);
  start = (Double *) array_data(iq);
  
  iq = ps[3];                   /* STEP: initial parameter step sizes */
  if (symbol_class(iq) != ANA_ARRAY)
    return cerror(NEED_ARR, iq);
  iq = ana_double(1, &iq);
  n = array_size(iq);
  if (n == nPar - 1)            /* assume last element of START is quality */
    nPar--;
  else if (n != nPar)
    return cerror(INCMP_ARG, ps[3]);
  if (!fitSym)                  /* standard fit */
    switch (i) {                /* check for proper number of arguments */
      case 0:                   /* gaussians */
        if ((nPar - 1) % 3 != 0)
          return anaerror("Need one plus a multiple of three parameters for gaussian fits", ps[3]);
        break;
      case 1:                   /* power function */
        if (nPar != 5)
          return anaerror("Need five parameters for power-function fits", ps[3]);
        break;
    }
  step = (Double *) array_data(iq);

  if (narg >= 5 && (iq = ps[4])) { /* LOWBOUND: lower bound on parameters */
    if (symbol_class(iq) != ANA_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = ana_double(1, &iq);
    n = array_size(iq);
    if (n != nPar)
      return cerror(INCMP_ARG, ps[4]);
    lowbound = (Double *) array_data(iq);
  } else
    lowbound = NULL;

  if (narg >= 6 && (iq = ps[5])) { /* HIGHBOUND: upper bound on parameters */
    if (symbol_class(iq) != ANA_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = ana_double(1, &iq);
    n = array_size(iq);
    if (n != nPar)
      return cerror(INCMP_ARG, ps[5]);
    hibound = (Double *) array_data(iq);
  } else
    hibound = NULL;

  if (lowbound && hibound) {
    for (i = 0; i < nPar; i++)
      if (hibound[i] <= lowbound[i])
	return anaerror("High bound %g of parameter %d is not greater than low bound %g", ps[5], hibound[i], i + 1, lowbound[i]);
  }

  if (narg >= 7 && (iq = ps[6])) { /* WEIGHTS */
    if (symbol_class(iq) != ANA_ARRAY)
      return cerror(NEED_ARR, iq);
    wSym = ana_double(1, &iq);
    n = array_size(wSym);
    if (n != nPoints)
      return cerror(INCMP_ARG, ps[6]);
    weights = (Double *) array_data(wSym);
  } else
    weights = NULL;

  if (narg >= 8 && ps[7])       /* QTHRESH: quality value threshold */
    qThresh = double_arg(ps[7]);
  else
    qThresh = 0.0;

  if (narg >= 9 && (iq = ps[8])) { /* PTHRESH: parameter change threshold */
    if (symbol_class(iq) != ANA_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = ana_double(1, &iq);
    n = array_size(iq);
    if (n != nPar)
      return cerror(INCMP_ARG, ps[8]);
    pThresh = (Double *) array_data(iq);
  } else
    pThresh = NULL;

  if (narg >= 10 && ps[9])      /* ITHRESH: iterations threshold */
    iThresh = int_arg(ps[9]);
  else
    iThresh = 1000;
  
  if (narg >= 11 && ps[10])     /* DTHRESH */
    dThresh = double_arg(ps[10]);
  else
    dThresh = 0.0;

  if (narg >= 12 && ps[11]) {   /* FAC: step size reduction factor */
    fac = double_arg(ps[11]);
    if (fac < 0)
      fac = -fac;
    if (fac > 1)
      fac = 1.0/fac;
  } else
    fac = 0.9;

  if (narg >= 13 && ps[12])     /* NITER: number of iterations per cycle */
    nIter = int_arg(ps[12]);
  else
    nIter = 10;
  
  if (narg >= 14 && ps[13])     /* NSAME: threshold on constant parameters */
    nSame = int_arg(ps[13]);
  else
    nSame = 5;

  if (narg >= 15 && ps[14]) {   /* ERR: output variable for error estimates */
    if (ps[14] < nFixed || ps[14] >= NAMED_END) {
      if (xSym)
        free(xp);
      return anaerror("Need named output variable", ps[14]);
    }
    i = nPar;
    redef_array(ps[14], ANA_DOUBLE, 1, &i);
    err = (Double *) array_data(ps[14]);
  } else {
    err = malloc(nPar*sizeof(Double));
    if (!err)
      return cerror(ALLOC_ERR, 0);
  }

  if (narg >= 17 && ps[16])    /* TTHRESH: threshold on CPU time */
    tThresh = double_arg(ps[16]);
  else
    tThresh = 3600;

  vocal = (internalMode & 1)? 1: 0; /* /VOCAL */
  vocal_err = ((internalMode & 129) == 129)? 1: 0; /* /VERR */
  if (internalMode & 4)         /* /DOWN */
    dir = 0.8;
  else
    dir = 1./0.8;

  PoissonChiSq = internalMode & 8; /* get Poisson chi-squares fit (if */
                                   /* applicable) */

  i = nPar + 1;
  fitPar = array_scratch(ANA_DOUBLE, 1, &i); /* fit parameters */
  par = (Double *) array_data(fitPar);

  size = nPar*sizeof(Double);

  /* copy start values into par */
  memcpy(par, start, size);
    
  /* enforce upper and lower bounds */
  enforce_bounds(par, lowbound, hibound, nPar);

  if (fitSym) {                 /* prepare executable for user-defined */
                                /* fit function */
    if (isFreeTemp(fitPar))
      symbol_context(fitPar) = 1; /* so they won't be deleted */
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
    symbol_class(fitTemp) = ANA_USR_FUNC;
    usr_func_arguments(fitTemp) = fitArg;
    symbol_memory(fitTemp) = (weights? 4: 3)*sizeof(Word);
    usr_func_number(fitTemp) = fitSym;
  }

  if (tThresh)
    starttime = time(0);
  else
    starttime = 0;

  onebyone = (internalMode & 64? 1: 0);
  /* internalMode is likely to change due to calling the user function */

  /* get initial fit quality */
  if (fitSym) {
    i = eval(fitTemp);
    if (i < 0) {                /* some error */
      if (narg <= 14 || !ps[14])
        free(err);
      if (!xSym)
        zap(xTemp);
      if (symbol_context(ySym) == 1)
        symbol_context(ySym) = -compileLevel; /* so they'll be deleted */
                                              /* when appropriate */
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
  allocate(parBest1, nPar, Double);
  allocate(parBest2, nPar, Double);
  allocate(ran, nPar, Double);
  memcpy(parBest1, par, size);
  memcpy(parBest2, par, size);
  memcpy(err, step, size);
  allocate(meanShift, nPar, Double);
  for (i = 0; i < nPar; i++)
    meanShift[i] = 0.0;

  iter = 0;
  same = 0;
  mu = 0;
  Double qLag = 1.2*qBest2;
  if (!qLag)
    qLag = 1;
  do {                          /* iterate */
    qLast = qBest2;
    nn = onebyone? nPar: nIter;
    if (onebyone)
      randome(ran, nPar);
    for (i = 0; i < nn; i++) {
      if (!onebyone)
        randome(ran, nPar);       /* get random numbers */
      mu = 2 - mu/qLast;
      if (mu < 1)
        mu = 1;
      if (onebyone) {
        par[i] = parBest1[i] + mu*meanShift[i] + err[i]*ran[i];
	enforce_bounds(par, lowbound, hibound, nPar);
      } else {
        for (j = 0; j < nPar; j++) /* update parameters */
          par[j] = parBest1[j] + mu*meanShift[j] + err[j]*ran[j];
	enforce_bounds(par, lowbound, hibound, nPar);
      }
      if (fitSym) {
        j = eval(fitTemp);
        if (j < 0)                      /* some error */
          goto generalfit_1;
        qual = double_arg(j);
        zapTemp(j);
      } else
        qual = fitFunc(par, nPar, xp, yp, weights, nPoints); /* fit quality */
      if (qual < qBest1) {      /* this one is better */
        qBest1 = qual;
        memcpy(parBest1, par, size);
      } else {			/* restore parameter */
	if (onebyone)
	  par[i] = parBest1[i];
      }
    } /* end for (i = 0; i < nn; i++) */
    if (qBest1 < qBest2) {      /* this cycle yielded a better one */
      same = 0;
      for (j = 0; j < nPar; j++) {
	temp = parBest1[j] - parBest2[j];
	if (temp) {		/* some improvement due to this parameter */
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
          if (n >= 14/(vocal_err? 2: 1)) /* we show at most twenty */
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
  memcpy(par, parBest2, size);
  par[nPar] = qBest2;

  /* presumably we're close to a local minimum now; home in */
  Double qualplus, qualmin;
  for (i = 0; i < nPar; i++) {
    if (step[i]) {
      Double h = fabs(step[i]);
      Double r;
      do {
	par[i] = parBest2[i] + h;
	if (fitSym) {
	  j = eval(fitTemp);
	  if (j < 0)
	    goto generalfit_1;
	  qualplus = double_arg(j);
	  zapTemp(j);
	} else
	  qualplus = fitFunc(par, nPar, xp, yp, weights, nPoints);
	par[i] = parBest2[i] - h;
	if (fitSym) {
	  j = eval(fitTemp);
	  if (j < 0)
	    goto generalfit_1;
	  qualmin = double_arg(j);
	  zapTemp(j);
	} else
	  qualmin = fitFunc(par, nPar, xp, yp, weights, nPoints);
	r = qBest2? (qualplus + qualmin)/qBest2: 3.5;
	h *= 12/(r*r - 4);
      } while ((r > 5 || r < 3) && h < err[i]*1e6);
      /* q² = a²(1 + ((x - b)/c)²) */
      Double psi = (qualplus*qualplus - qualmin*qualmin)/4;
      Double beta = qBest2*qBest2;
      Double delta = (qualplus*qualplus + qualmin*qualmin)/2 - beta;
      Double a2, b, c;
      if (delta) {
	a2 = beta - psi*psi/delta; /* a² */
	if (a2 < 0) {
	  a2 = 0;
	  b = 0;
	} else
	  b = -h*psi/delta;
	c = h*sqrt(a2/delta);
      } else {
	b = 0;
	c = INFTY;
      }
      par[i] = parBest2[i] + b;
      Double q;
      if (fitSym) {
	j = eval(fitTemp);
	if (j < 0)
	  goto generalfit_1;
	q = double_arg(j);
	zapTemp(j);
      } else
	q = fitFunc(par, nPar, xp, yp, weights, nPoints);
      if (q < qBest2) {
	parBest2[i] = par[i];
	qBest2 = q;
      } else {
	par[i] = parBest2[i];	/* restore */
      }
      err[i] = c*sqrt(3);
    } else
      err[i] = 0;
  }
  par[nPar] = qBest2;

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
          if (n >= 14/(vocal_err? 2: 1)) /* we show at most fourteen */
            break;
        }
    }
    printf(" \n");
  }

  if (!xSym)
    zap(xTemp);
  if (fitSym) {
    symbol_class(fitTemp) = ANA_SCALAR;
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

  generalfit_1:
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
/*------------------------------------------------------------*/
gsl_vector *gsl_vector_from_ana_symbol(Int iq, Int axis)
{
  gsl_vector *v;
  Int ndim, *dims, nelem, i;
  pointer data;

  iq = ana_double(1, &iq);
  if (numerical(iq, &dims, &ndim, &nelem, &data) < 0
      || axis >= ndim) {
    errno = EDOM;
    return NULL;
  }

  v = calloc(1, sizeof(gsl_vector));
  /* v->owner = 0 is essential! */
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
/*------------------------------------------------------------*/
Double fit2_func(const gsl_vector *a, void *p)
{
  ana_func_if *afif = (ana_func_if *) p;
  pointer par = ana_func_if_get_param_data(afif, 0);
  memcpy(par.d, a->data, a->size*sizeof(Double));
  Double result = ana_func_if_call(afif);
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
  function Double generalfit2_func(const gsl_vector * X, void *
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
Int ana_generalfit2(Int narg, Int ps[])
{
  Int nPar, nPoints, result, ithresh = 0, nithresh;
  gsl_multimin_fminimizer *minimizer = NULL;
  ana_func_if *afif = NULL;
  gsl_vector *par_v = NULL, *step_v = NULL;
  Int d_step_sym, d_par_sym, d_x_sym, d_y_sym;
  Double *errors = NULL, sthresh = 0;

  Int vocal = (internalMode & 1);

  {
    Int nx, *dims, ndim, nStep;

    d_x_sym = ana_double(1, &ps[0]);	/* X */
    if (isFreeTemp(d_x_sym))
      symbol_context(d_x_sym) = 1;     /* avoid premature deletion */
    d_y_sym = ana_double(1, &ps[1]);	/* Y */
    if (isFreeTemp(d_y_sym))
      symbol_context(d_y_sym) = 1;
    d_par_sym = ana_double(1, &ps[2]); /* PAR */
    if (isFreeTemp(d_par_sym))
      symbol_context(d_par_sym) = 1;	/* avoid premature deletion */
    d_step_sym = ana_double(1, &ps[3]); /* STEP */
    if (isFreeTemp(d_step_sym))
      symbol_context(d_step_sym) = 1;
    if (numerical(d_x_sym, &dims, &ndim, &nx, NULL) < 0 /* X */
	|| numerical(d_y_sym, NULL, NULL, &nPoints, NULL) < 0 /* Y */
	|| numerical(d_par_sym, NULL, NULL, &nPar, NULL) < 0
	|| numerical(d_step_sym, NULL, NULL, &nStep, NULL) < 0)
      return ANA_ERROR;
    if (nStep == nPar - 1)
      --nPar;			/* assume last element of START is quality */
    if (nStep != nPar) {
      result = anaerror("Number of elements (%d) in step argument is unequal to number of elements (%d) in parameters argument", ps[3], nStep, nPar);
      goto end;
    }
  }
  if (!symbolIsString(ps[4])) {	/* FUNCNAME */
    result = cerror(NEED_STR, ps[4]);
    goto end;
  }
  if (narg > 5 && ps[5]) {	/* ERR */
    redef_array(ps[5], ANA_DOUBLE, 1, &nPar);
    errors = (Double *) array_data(ps[5]);
  }
  if (narg > 6 && ps[6])	/* ITHRESH */
    ithresh = int_arg(ps[6]);
  if (ithresh <= 0)
    ithresh = 10000;
  if (narg > 7 && ps[7])	/* STHRESH */
    sthresh = double_arg(ps[7]);
  if (narg > 8 && ps[8])	/* NITHRESH */
    nithresh = int_arg(ps[8]);
  else
    nithresh = sqrt(nPar)*10;
  if (nithresh <= 0)
    nithresh = INT32_MAX;

  par_v = gsl_vector_from_ana_symbol(d_par_sym, -1);
  step_v = gsl_vector_from_ana_symbol(d_step_sym, -1);

  afif = ana_func_if_alloc(string_value(ps[4]), 3);
  if (!afif) {
    switch (errno) {
    case EDOM:
      result = anaerror("Cannot fit to internal routine", ps[4]);
      break;
    case ENOMEM:
      result = cerror(ALLOC_ERR, 0);
      break;
    default:
      result = anaerror("Unhandled error %d", 0, errno);
      break;
    }
    goto end;
  }

  ana_func_if_set_param(afif, 0, d_par_sym);		 /* par */
  ana_func_if_set_param(afif, 1, d_x_sym);		 /* x */
  ana_func_if_set_param(afif, 2, d_y_sym);		 /* y */

  minimizer = gsl_multimin_fminimizer_alloc(gsl_multimin_fminimizer_nmsimplex2,
					    nPar);
  if (!minimizer) {
    result = anaerror("Unable to allocate memory for minimizer", 0);
    goto end;
  }

  gsl_multimin_function m_func;
  m_func.n = nPar;
  m_func.f = &fit2_func;
  m_func.params = (void *) afif;

  par_v->size = nPar; /* GSL requires the par size to match the number
			 of parameters, but we want to keep room for
			 storing the quality */

  if (gsl_multimin_fminimizer_set(minimizer, &m_func, par_v, step_v)) {
    result = ANA_ERROR;
    goto end;
  }

  Int status;
  Int iter = 0;
  Int show = nPar;
  if (nPar > 9)
    show = 9;
  time_t report_after;
  if (vocal)
    report_after = time(NULL);
  Double oldqual, newqual = 0, vocal_oldqual = 0, size, vocal_oldsize = 0;
  Int no_improvement_niter = 0;
  do {
    ++iter;
    status = gsl_multimin_fminimizer_iterate(minimizer);
    if (status)
      break;
    size = gsl_multimin_fminimizer_size(minimizer);
    oldqual = newqual;
    newqual = sqrt(minimizer->fval/nPoints);
    Double improvement = newqual - oldqual;
    if (improvement)
      no_improvement_niter = 0;
    else
      ++no_improvement_niter;
    if (vocal && time(NULL) > report_after) {
      Int i, j = 0;
      Double vocal_improvement = newqual - vocal_oldqual;
      vocal_oldqual = newqual;
      Double size_improvement = size - vocal_oldsize;
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
    Int n = best_par->size + 1;
    result = array_scratch(ANA_DOUBLE, 1, &n);
  }
  Double *tgt = array_data(result);
  memcpy(tgt, best_par->data, best_par->size*sizeof(Double));
  Double best_min = ana_func_if_call(afif);
  tgt[best_par->size] = best_min;

  if (errors) {
    Int i;
    for (i = 0; i < nPar; i++) {
      if (step_v->data[i]) {
	Double h;
	memcpy(par_v->data, best_par->data, nPar*sizeof(Double));
	h = step_v->data[i];
	Double qualplus, qualmin, r;
	do {
	  par_v->data[i] = best_par->data[i] + h;
	  qualplus = ana_func_if_call(afif);
	  par_v->data[i] = best_par->data[i] - h;
	  qualmin = ana_func_if_call(afif);
	  r = (qualplus + qualmin)/best_min;
	  if (r > 4)
	    h /= sqrt(r);
	  else if (r < 3)
	    h *= sqrt(r);
	} while ((r > 4 || r < 3) && h < errors[i]*1e6);
	Double v = sqrt((qualplus + qualmin)/(2*best_min) - 1);
	if (v)
	  errors[i] = h/v;
	else
	  errors[i] = INFTY;
	par_v->data[i] = best_par->data[i];
      } else
	errors[i] = 0.0;
    }
  }

 end:
  gsl_vector_free(par_v);
  gsl_vector_free(step_v);
  gsl_multimin_fminimizer_free(minimizer);
  ana_func_if_free(afif);
  if (symbol_context(d_par_sym) == 1)
    symbol_context(d_par_sym) = -compileLevel; /* so it is deleted when appropriate */
  if (symbol_context(d_x_sym) == 1)
    symbol_context(d_x_sym) = -compileLevel; /* so it is deleted when appropriate */
  if (symbol_context(d_y_sym) == 1)
    symbol_context(d_y_sym) = -compileLevel; /* so it is deleted when appropriate */
  if (symbol_context(d_step_sym) == 1)
    symbol_context(d_step_sym) = -compileLevel; /* so it is deleted when appropriate */
  return result;
}
REGISTER(generalfit2, f, FIT3, 5, 7, "X:Y:START:STEP:F:ERR:ITHRESH:1VOCAL");
/*------------------------------------------------------------*/
typedef union {
  Byte  *b;
  unsigned short        *w;
  uint32_t  *l;
  Float *f;
  Double        *d;
} uscalar;

/* replace NaN values by (other) random values */
void denan(Byte *data, Int size, Int partype)
/* <data> = start of data
   <size> = size of data, in bytes
   <partpe> = data type (ANA_BYTE ... ANA_FLOAT) */
{
  uscalar p;
  Int i;

  switch (partype) {
  case ANA_FLOAT:
    p.f = (Float *) data;
    for (i = 0; i < size/sizeof(Float); i++) {
      while (isnan(*p.f)) {
        uint32_t *r;
        for (r = (uint32_t *) p.b; (Byte *) r < p.b + sizeof(Float); r++)
          *r = random_bits();
      }
      p.f++;
    }
    break;
  case ANA_DOUBLE:
    p.d = (Double *) data;
    for (i = 0; i < size/sizeof(Double); i++) {
      while (isnan(*p.d)) {
        uint32_t *r;
        for (r = (uint32_t *) p.b; (Byte *) r < p.b + sizeof(Double); r++) {
          *r = random_bits();
        }
      }
      p.d++;
    }
    break;
  }
}

void printgene(Byte *gene, Int nPar, Int partype, Int showbits, 
               Double *quality) {
  Int j;
  uscalar p;

  p.b = gene;
  putchar('[');
  for (j = 0; j < nPar; j++) {
    if (j)
      putchar(' ');
    switch (partype) {
    case ANA_BYTE:
      printf("%u", p.b[j]);
      break;
    case ANA_WORD:
      printf("%u", p.w[j]);
      break;
    case ANA_LONG:
      printf("%u", p.l[j]);
      break;
    case ANA_FLOAT:
      printf("%g", p.f[j]);
      break;
    case ANA_DOUBLE:
      printf("%g", p.d[j]);
      break;
    }
    if (showbits) {
      Int i;
      Byte *b = gene + ana_type_size[partype]*j;
      printf(" {");
      for (i = ana_type_size[partype] - 1; i >= 0; i--)
        printf("%02x", b[i]);
      printf("}");
    }
  }
  if (quality)
    printf(": %16.12g", *quality);
  putchar(']');
}

void printgenenl(Byte *gene, Int nPar, Int partype, Int showbits, 
                 Double *quality) {
  printgene(gene, nPar, partype, showbits, quality);
  putchar('\n');
}

Int hasnan(Byte *gene, Int nPar, Int partype) {
  Int i;
  uscalar p;

  switch (partype) {
  case ANA_FLOAT:
    p.f = (Float *) gene;
    for (i = 0; i < nPar; i++) {
      if (isnan(*p.f++))
        return 1;
    }
    break;
  case ANA_DOUBLE:
    p.d = (Double *) gene;
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
void calculate_distribution(Double *distr, Double *deviation, Int *rtoi,
                            Int nPopulation, Double mu)
{
  Double sum = 0.0;
  Int i;

  for (i = 0; i < nPopulation; i++)
    sum += 1/deviation[i];
  Double a = 1, b = 0;
  if (sum > 0) {
    Double avg_fitness = sum/nPopulation;
    Double best_fitness = 1/deviation[rtoi[nPopulation - 1]];
    if (avg_fitness != best_fitness) {
      a = (mu - 1)/(best_fitness - avg_fitness);
      b = (best_fitness - avg_fitness*mu)/(best_fitness - avg_fitness);
    }
  }
  sum = 0.0;
  distr[0] = 0.0;
  for (i = 1; i <= nPopulation; i++) {
    Double x = 1/deviation[rtoi[i - 1]];
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

Int ana_geneticfit(Int narg, Int ps[])
/* FIT2(x,y,START,fit [,mu,ngenerations,population,pcross,pmutate,vocal]
        [,/ELITE,/BYTE,/WORD,/LONG,/FLOAT,/DOUBLE]) */
{
  Int   fitSym, iq, nPoints, nPopulation, nPar, fitTemp, i, j, size, result,
    *rtoi, pair, k, w, generation, i1, i2, ibit,
    iter = 0, vocal, typesize, nGeneration;
  uscalar p;
  Word  *par, fitPar, xSym, ySym, fitArg[4], wSym;
  Byte  *genes, *parent1, *parent2, *genes2, t1, t2;
  Double *deviation, mu, *distr, random_one(void), pcross, *deviation2,
    crossmark, mutatemark, pmutate, sum;
  Double *weights, *start;
  void  invertPermutation(Int *data, Int n),
    indexxr_f(Int n, Float ra[], Int indx[]);
  Int   random_distributed(Int modulus, Double *distr);
  Byte  changed, elite, partype;
  static uint16_t mask1[] = {
    0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01
  }, mask2[] = {
    0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe,
  }, mask3[] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  };
  Int *crossoversites, *mutationsites;
  void indexxr_d(Int, Double *, Int *);
 
  iq = ps[3];                   /* fit */
  if (!symbolIsStringScalar(iq))
    return cerror(NEED_STR, ps[3]);
  fitSym = stringpointer(string_value(iq), SP_USER_FUNC);
  if (fitSym < 0)
    return
      anaerror("Sorry, fitting to interal functions is not yet implemented",
            iq);

  if (symbolIsScalar(ps[2])) {
    nPar = int_arg(ps[2]);      /* nPar */
    if (nPar < 1)
      return anaerror("A positive value is required for the number of parameters",
                      ps[2]);
    start = NULL;
    switch (internalMode/2 & 7) {
    case 1:
      partype = ANA_BYTE;
      break;
    case 2: default:
      partype = ANA_WORD;
      break;
    case 3:
      partype = ANA_LONG;
      break;
    case 4:
      partype = ANA_FLOAT;
      break;
    case 5:
      partype = ANA_DOUBLE;
      break;
    }
  } else if (symbolIsArray(ps[2])) { /* start values */
    Int iq = ana_double(1, &ps[2]);
    nPar = array_size(iq);
    start = (Double *) array_data(iq);
    partype = symbol_type(iq);
  }

  if (narg > 5 && (iq = ps[5])) { /* mu: selection pressure */
    mu = double_arg(iq);
    if (mu < 1.0)
      mu = 1.0;
  } else
    mu = 1.5;

  if (narg > 6 && (iq = ps[6])) { /* generations: number of generations */
    nGeneration = int_arg(iq);
    if (nGeneration < 1)
      nGeneration = 1;
  } else
    nGeneration = 50;

  if (narg > 8 && (iq = ps[8])) { /* pcross: cross-over probability */
    pcross = double_arg(iq);
    if (pcross < 0.0)
      pcross = 0.0;
    else if (pcross > 1.0)
      pcross = 1.0;
  } else
    pcross = 0.1;
  pcross /= 2;

  if (narg > 7 && (iq = ps[7])) { /* population */
    nPopulation = int_arg(iq);
    if (nPopulation < 1)
      nPopulation = 1;
    /* we make sure that the population fills a number of ints exactly */
    if (partype < ANA_LONG)
      nPopulation += (nPopulation % (sizeof(Int)/ana_type_size[partype]));
  } else
    nPopulation = 100;

  if (narg > 9 && (iq = ps[9])) { /* pmutate: mutation probability */
    pmutate = double_arg(iq);
    if (pmutate < 0.0)
      pmutate = 0.0;
    else if (pmutate > 1.0)
      pmutate = 1.0;
  } else
    pmutate = 1.0/nPopulation;
  pmutate /= 2;

  if (!symbolIsNumericalArray(ps[0])) /* x */
    return cerror(NEED_NUM_ARR, ps[0]);
  xSym = ana_double(1, ps);
  if (isFreeTemp(xSym))
    symbol_context(xSym) = 1;   /* so it doesn't get prematurely deleted */
  nPoints = array_size(xSym);

  if (!symbolIsNumericalArray(ps[1])) /* y */
    return cerror(NEED_NUM_ARR, ps[1]);
  if (array_size(ps[1]) != nPoints)
    return cerror(INCMP_ARG, ps[1]);
  ySym = ana_double(1, ps + 1);
  if (isFreeTemp(ySym))
    symbol_context(ySym) = 1;

  if (narg > 4 && (iq = ps[4])) { /* weights */
    Int n;
    if (symbol_class(iq) != ANA_ARRAY)
      return cerror(NEED_ARR, iq);
    wSym = ana_double(1, &iq);
    n = array_size(wSym);
    if (n != nPoints)
      return cerror(INCMP_ARG, iq);
    weights = (Double *) array_data(wSym);
    if (isFreeTemp(wSym))
      symbol_context(wSym) = 1;
  } else
    weights = NULL;

  if (narg > 10 && (iq = ps[10])) /* VOCAL */
    vocal = int_arg(iq);
  else
    vocal = 0;

  elite = (internalMode & 1);

  fitPar = array_scratch(partype, 1, &nPar);
  symbol_context(fitPar) = 1;   /* so it doesn't get prematurely deleted */
  par = array_data(fitPar);
  typesize = ana_type_size[partype];
  size = nPar*typesize;

  fitArg[0] = fitPar;
  fitArg[1] = xSym;
  fitArg[2] = ySym;
  if (weights)
    fitArg[3] = wSym;
  fitTemp = nextFreeTempExecutable();
  symbol_class(fitTemp) = ANA_USR_FUNC;
  usr_func_arguments(fitTemp) = fitArg;
  symbol_memory(fitTemp) = (weights? 4: 3)*sizeof(Word);
  usr_func_number(fitTemp) = fitSym;

  /* create initial population */
  genes = malloc(nPopulation*size);
  genes2 = malloc(nPopulation*size);
  deviation = malloc(nPopulation*sizeof(Double));
  deviation2 = malloc(nPopulation*sizeof(Double));
  rtoi = malloc(nPopulation*sizeof(Int));
  /* itor = malloc(nPopulation*sizeof(Int)); */

  /* we fill <genes> with random bits.  <genes> exactly spans a number */
  /* of ints, so we don't need to worry about the exact type */
  p.l = (uint32_t *) genes;
  if (start) {                  /* fill first member with start values */
    memcpy(p.d, start, nPar*sizeof(Double));
    p.d += nPar;
    i = size/sizeof(Int);
  } else
    i = 0;
  for ( ; i < nPopulation*size/sizeof(Int); i++)
    *p.l++ = random_bits();
  /* if the parameter type is FLOAT or DOUBLE, then some of the bit patterns */
  /* may not correspond to representable values, but rather to NaNs. */
  /* We must fix those elements. We use a single scheme for both FLOAT */
  /* and DOUBLE values, assuming that when the bit patterns of a DOUBLE */
  /* NaN are interpreted as the bit patterns of some FLOAT values instead, */
  /* then at least one of those FLOATs is a FLOAT NaN, and replacing that */
  /* FLOAT NaN with a representable FLOAT value generates a representable */
  /* DOUBLE value. */
  denan(genes, nPopulation*size, partype);

  /* now calculate the fitness of all members of the population.
     Less is better. */
  for (i = 0; i < nPopulation; i++) {
    memcpy(par, genes + i*size, size);
    j = eval(fitTemp);          /* get deviation ("distance from goal") */
    if (j == ANA_ERROR)         /* some error occurred */
      goto geneticfit_1;
    deviation[i] = fabs(double_arg(j)); /* distance from goal */
    if (vocal) {
      printf("%d/%d: ", i, nPopulation);
      printgene(genes + i*size, nPar, partype, 0, &deviation[i]);
      putchar('\n');
    }
    zapTemp(j);
  }
  internalMode = 0;             /* or we may get unexpected results */
  indexxr_d(nPopulation, deviation, rtoi); /* get ranking */
  /* now deviation[rtoi[i]] is the i-th largest distance from goal;
     rtoi[0] is the worst fit, rtoi[nPopulation - 1] is the best fit */

  /* calculate the distribution function for selecting members.
     member i gets a reproduction probability equal to
     distr[i+1] - distr[i] */
  distr = malloc(nPopulation*sizeof(Double));
  calculate_distribution(distr, deviation, rtoi, nPopulation, mu);

  if (vocal > 1) {
    p.b = genes;
    for (i = 0; i < nPopulation; i++) {
      printf("%d/%d ", i, nPopulation);
      printgene(p.b + rtoi[i]*size, nPar, partype, 0, &deviation[rtoi[i]]);
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
  crossmark = pcross? random_one()/pcross: LONG_MAX; /* bytes */
  mutatemark = pmutate? random_one()/pmutate: LONG_MAX; /* bits */

  Byte *child1 = malloc(size);
  Byte *child2 = malloc(size);
  crossoversites = calloc(size*8, sizeof(Int));
  mutationsites = calloc(size*8, sizeof(Int));

  Int mutatecount = 0;
  Int crossovercount = 0;
  Int offspringcount = 0;

  /* TODO: implement RESOLUTION argument, which says how many bytes
     per parameter to fit, beginning with the most significant Byte.
     TODO: implement additional stop criteria other than just
     "number of generations"; at least something like "stop if
     quality unchanged for a fixed number of iterations" */
  
  generation = nGeneration;
  /* iterate over the desired number of generations */
  while (generation--) {        /* all generations */
    if (elite) {                /* always keep the best two */
      memcpy(genes2, genes + size*rtoi[nPopulation - 1], size);
      genes2 += size;
      *deviation2++ = deviation[rtoi[nPopulation - 1]];
      memcpy(genes2, genes + size*rtoi[nPopulation - 2], size);
      genes2 += size;
      *deviation2++ = deviation[rtoi[nPopulation - 2]];
    }
    for (pair = elite; pair < nPopulation/2; pair++) { /* remaining pairs */
      /* pick parents */
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
      while (crossmark < size*8) { /* do cross-over */
        ibit = crossmark;
        crossmark += random_one()/pcross;
        crossoversites[ibit]++;
        w = ibit/8;             /* Byte index */
        k = ibit - 8*w;         /* bit index */
        for (i = 0; i < w; i++) { /* swap before cross-over Word */
          t1 = child1[i];
          child1[i] = child2[i];
          child2[i] = t1;
        } /* end of for (i = 0; ...) */
        if (k) {                /* cross-over site is not on Word boundary */
          t1 = ((child1[w] & mask1[k]) | (child2[w] & mask2[k]));
          t2 = ((child2[w] & mask1[k]) | (child1[w] & mask2[k]));
          child1[w] = t1;
          child2[w] = t2;
        } /* end of if (k) */
        if (hasnan(child1, nPar, partype)
            || hasnan(child2, nPar, partype)) {
          /* we generated one or two NaNs from regular numbers: */
          /* undo the crossover */
          for (i = 0; i < w; i++) { /* swap before cross-over Word */
            t1 = child1[i];
            child1[i] = child2[i];
            child2[i] = t1;
          } /* end of for (i = 0; ...) */
          if (k) {              /* cross-over site is not on Word boundary */
            t1 = ((child1[w] & mask1[k]) | (child2[w] & mask2[k]));
            t2 = ((child2[w] & mask1[k]) | (child1[w] & mask2[k]));
            child1[w] = t1;
            child2[w] = t2;
          } /* end of if (k) */
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

      /* do mutations */
      while (mutatemark < size*8) {
        ibit = mutatemark;
        mutatemark += random_one()/pmutate;
        mutationsites[ibit]++;
        w = ibit/8;             /* Byte index */
        k = ibit - 8*w;         /* bit index */
        child1[w] ^= mask3[k]; /* flip bit */
        if (hasnan(child1, nPar, partype))
          /* the mutation generated a NaN; undo */
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
        w = ibit/8;             /* Byte index */
        k = ibit - 8*w;         /* bit index */
        child2[w] ^= mask3[k]; /* flip bit */
        if (hasnan(child2, nPar, partype))
          /* the mutation generated a NaN; undo */
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

      /* store processed genes in new population */
      memcpy(genes2, child1, size);
      genes2 += size;
      memcpy(genes2, child2, size);
      genes2 += size;
      if (changed) { /* TODO: only reevaluate for the changed child, not both */
        memcpy(par, child1, size);
        j = eval(fitTemp);
        if (j == ANA_ERROR)
          goto geneticfit_3;
        *deviation2++ = double_arg(j);
        zapTemp(j);
        memcpy(par, child2, size);
        j = eval(fitTemp);
        if (j == ANA_ERROR)
          goto geneticfit_3;
        *deviation2++ = double_arg(j);
        zapTemp(j);
      } else {
        *deviation2++ = deviation[rtoi[i1]];
        *deviation2++ = deviation[rtoi[i2]];
      }
    } /* end of for (pair = 0; ...) */
    genes2 -= nPopulation*size;
    deviation2 -= nPopulation;
    
    memcpy(genes, genes2, nPopulation*size);
    memcpy(deviation, deviation2, nPopulation*sizeof(Double));

    indexxr_d(nPopulation, deviation, rtoi); /* get ranking */
    calculate_distribution(distr, deviation, rtoi, nPopulation, mu);

    if (vocal > 1) {
      p.b = genes;
      for (i = 0; i < nPopulation; i++) {
        printf("%d/%d ", i, nPopulation);
        printgene(p.b + rtoi[i]*size, nPar, partype, 0, &deviation[rtoi[i]]);
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
    /* memcpy(itor, rtoi, nPopulation*sizeof(Int));
       invertPermutation(itor, nPopulation); */
  } /* end of while (generation--) */
  
  result = array_scratch(partype, 1, &nPar);
  memcpy(array_data(result), genes + nPar*rtoi[nPopulation - 1]*typesize,
         size);
  if (vocal) {
    printf("%4d offspring (probability %g)\n%4d cross-overs (probability %g per gene pair)\n%4d mutations (probability %g per bit)\n",
           offspringcount, ((Double) offspringcount)/(nPopulation*nGeneration),
           crossovercount, ((Double) crossovercount)/(nPopulation*nGeneration/2),
           mutatecount, ((Double) mutatecount)/(nPopulation*nGeneration*size*8));
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

  geneticfit_2:
  zap(fitPar);
  symbol_class(fitTemp) = symbol_type(fitTemp) = 0;
  if (symbol_context(xSym) == 1)
    zap(xSym);
  if (symbol_context(ySym) == 1)
    zap(ySym);
  free(distr);
  free(rtoi);
  /* free(itor); */
  free(genes);
  free(deviation);
  free(genes2);
  free(deviation2);
  free(child1);
  free(child2);
  free(crossoversites);
  free(mutationsites);
  return result;

  geneticfit_3:
  genes2 -= pair*2*nPar*typesize;
  deviation2 -= pair*2;

  geneticfit_1:
  if (symbol_context(ySym) == 1)
    symbol_context(ySym) = -compileLevel; /* so they'll be deleted */
                                /* when appropriate */
  if (symbol_context(fitPar) == 1)
    symbol_context(fitPar) = -compileLevel;
  if (xSym && symbol_context(xSym) == 1)
    symbol_context(xSym) = -compileLevel;
  if (weights && symbol_context(wSym) == 1)
    symbol_context(wSym) = -compileLevel;
  result = ANA_ERROR;
  goto geneticfit_2;
}