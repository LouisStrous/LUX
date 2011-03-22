/* File fit.c */
/* General fitting procedure */
/* Started 22 October 1995.  Louis Strous */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>             /* for LONG_MAX */
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
"$Id: fit.c,v 4.0 2001/02/07 20:37:00 strous Exp $";

char    PoissonChiSq;           /* flag: Poisson chi-square fitting? */

/*-----------------------------------------------------------------------*/
double gaussians(double *par, int nPar, double *x, double *y, double *w, int nData)
/* Returns the rms error (if PoissonChiSq is zero) or chi-square for */
/* Poisson distributions (if PoissonChiSq is nonzero) in fitting y vs x */
/* with a set of gaussians. */
/* The fit profile is  par[0]+par[1]*exp(-((x - par[2])/par[3])^2) */
/* [ + par[4]*exp(-((x - par[5])/par[6])^2) ... ].  par[] must have */
/* at least 4 elements and for each three more another gaussian is */
/* added.   LS 26oct95  31oct95 */
{
  int   j, wstep;
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
/*-----------------------------------------------------------------------*/
double powerfunc(double *par, int nPar, double *x, double *y, double *w, int nData)
/* returns the error in fitting y vs x with a general power function */
/* superposed on a linear trend. */
/* the fit profile is  par[0] + par[1]*x + par[2]*(x - par[3])^par[4] */
/* LS 13nov95 */
{
  int   wstep;
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
/*-----------------------------------------------------------------------*/
int ana_generalfit(int narg, int ps[])
/* FIT([X,]Y,START,STEP[,LOWBOUND,HIGHBOUND][,WEIGHTS][,QTHRESH,PTHRESH,
   ITHRESH,DTHRESH][,FAC,NITER,NSAME][,ERR][,FIT][,/VOCAL,/DOWN,
   /PCHI][/GAUSSIANS,/POWERFUNC]) */
/* This routine is intended to enable the user to fit iteratively */
/* to any profile specification. */
/* IN DEVELOPMENT  LS 24oct95 */
{
  int   ySym, xSym, nPoints, n, nPar, nIter, iThresh, nSame, size,
    iq, i, j, iter, same, fitSym, fitTemp, xTemp, nn, wSym;
  double *yp, *xp, *start, *step, qThresh, *pThresh, fac, *lowbound, *hibound,
	dir, dThresh, qLast, mu, *meanShift, *weights;
        *err, *par, qBest1, qBest2, *parBest1, *parBest2, *ran, qual, temp,
  char  vocal, onebyone;
  void  randomn(int seed, double *output, int number, char hasUniform);
  double (*fitProfiles[2])(double *, int, double *, double *, double *, int) =
  { gaussians, powerfunc };
  double (*fitFunc)(double *, int, double *, double *, double *, int);
  extern int    nFixed;
  word  fitPar, fitArg[4];
  int   ana_indgen(int, int []), eval(int);
  void  zap(int);
  
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
  yp = (double *) array_data(ySym); /* pointer to data */

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
          xp = (double *) array_data(xSym);
        break;
      default:
        return cerror(ILL_CLASS, xSym);
    }
  else {
    xTemp = ana_indgen(1, &ySym);
    xp = (double *) array_data(xTemp);
  }

  iq = ps[2];                   /* START: initial parameter values */
  if (symbol_class(iq) != ANA_ARRAY)
    return cerror(NEED_ARR, iq);
  iq = ana_double(1, &iq);
  nPar = array_size(iq);
  start = (double *) array_data(iq);
  
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
  step = (double *) array_data(iq);

  if (narg >= 5 && (iq = ps[4])) { /* LOWBOUND: lower bound on parameters */
    if (symbol_class(iq) != ANA_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = ana_double(1, &iq);
    n = array_size(iq);
    if (n != nPar)
      return cerror(INCMP_ARG, ps[4]);
    lowbound = (double *) array_data(iq);
  } else
    lowbound = NULL;

  if (narg >= 6 && (iq = ps[5])) { /* HIGHBOUND: upper bound on parameters */
    if (symbol_class(iq) != ANA_ARRAY)
      return cerror(NEED_ARR, iq);
    iq = ana_double(1, &iq);
    n = array_size(iq);
    if (n != nPar)
      return cerror(INCMP_ARG, ps[5]);
    hibound = (double *) array_data(iq);
  } else
    hibound = NULL;

  if (narg >= 7 && (iq = ps[6])) { /* WEIGHTS */
    if (symbol_class(iq) != ANA_ARRAY)
      return cerror(NEED_ARR, iq);
    wSym = ana_double(1, &iq);
    n = array_size(wSym);
    if (n != nPoints)
      return cerror(INCMP_ARG, ps[6]);
    weights = (double *) array_data(wSym);
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
    pThresh = (double *) array_data(iq);
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
    err = (double *) array_data(ps[14]);
  } else {
    err = malloc(nPar*sizeof(double));
    if (!err)
      return cerror(ALLOC_ERR, 0);
  }


  vocal = (internalMode & 1)? 1: 0; /* /VOCAL */
  if (internalMode & 4)         /* /DOWN */
    dir = 0.8;
  else
    dir = 1./0.8;

  PoissonChiSq = internalMode & 8; /* get Poisson chi-squares fit (if */
                                   /* applicable) */

  i = nPar + 1;
  fitPar = array_scratch(ANA_DOUBLE, 1, &i); /* fit parameters */
  par = (double *) array_data(fitPar);

  size = nPar*sizeof(double);

  /* copy start values into par */
  memcpy(par, start, size);
    
  /* enforce upper and lower bounds */
  if (lowbound) {
    for (i = 0; i < nPar; i++)
      if (par[i] < lowbound[i])
        par[i] = lowbound[i]; }
  if (hibound) {
    for (i = 0; i < nPar; i++)
      if (par[i] > hibound[i])
        par[i] = hibound[i];
  }

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
    symbol_memory(fitTemp) = (weights? 4: 3)*sizeof(word);
    usr_func_number(fitTemp) = fitSym;
  }

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
  allocate(parBest1, nPar, double);
  allocate(parBest2, nPar, double);
  allocate(ran, nPar, double);
  memcpy(parBest1, par, size);
  memcpy(parBest2, par, size);
  memcpy(err, step, size);
  allocate(meanShift, nPar, double);
  for (i = 0; i < nPar; i++)
    meanShift[i] = 0.0;

  iter = 0;
  same = 0;
  mu = 0;
  onebyone = (internalMode & 64? 1: 0);
  nn = (onebyone? nPar: nIter);
  do {                          /* iterate */
    qLast = qBest2;
    if (onebyone)
      randomn(0, ran, nPar, 0);
    for (i = 0; i < nn; i++) {
      if (!onebyone)
        randomn(0, ran, nPar, 0);       /* get random numbers */
      mu = 2 - mu/qLast;
      if (mu < 1)
        mu = 1;
      if (onebyone) {
        par[i] = parBest2[i] + mu*meanShift[i] + err[i]*ran[i];
        if (lowbound && par[i] < lowbound[i])
          par[i] = lowbound[i];
        if (hibound && par[i] > hibound[i])
          par[i] = hibound[i];
      } else {
        for (j = 0; j < nPar; j++) /* update parameters */
          par[j] = parBest2[j] + mu*meanShift[j] + err[j]*ran[j];
        if (lowbound)           /* enforce lower bounds */
          for (j = 0; j < nPar; j++)
            if (par[j] < lowbound[j])
              par[j] = lowbound[j];
        if (hibound)            /* enforce higher bounds */
          for (j = 0; j < nPar; j++)
            if (par[j] > hibound[j])
              par[j] = hibound[j];
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
      }
    } /* end for (i = 0; i < nn; i++) */
    if (qBest1 < qBest2) {      /* this cycle yielded a better one */
      same = 0;
      for (j = 0; j < nPar; j++) {
        temp = parBest1[j] - parBest2[j];
        meanShift[j] = (meanShift[j]*fac + temp*(1 - fac));
        err[j] = (err[j]*fac + 2*fabs(temp)*(1 - fac))/dir;
      }
      qBest2 = qBest1;
      memcpy(parBest2, parBest1, size);
    } else
      for (j = 0; j < nPar; j++)
        meanShift[j] *= fac;
    for (j = 0; j < nPar; j++)
      err[j] *= dir;
    iter++;
    same++;
    if (same == nSame - 1) {
      memcpy(err, step, size);
      for (j = 0; j < nPar; j++)
        meanShift[j] = 0.0;
    }
    if (vocal) {
      printf("%3d %10.5g: ", iter, qBest2);
      n = 0;
      for (j = 0; j < nPar; j++)
        if (step[j]) {
          printf("%10.4g", parBest2[j]);
          n++;
          if (n == 20)          /* we show at most twenty */
            break;
        }
      putchar('\n');
    }
    n = 0;
    if (pThresh) {
      for (j = 0; j < nPar; j++)
        if (err[j] < pThresh[j])
          n++;
    }
    mu = fabs(qLast - qBest2);
  } while (qBest2 >= qThresh
           && (mu >= dThresh*fabs(qBest2) || !mu)
           && n < nPar
           && iter < iThresh
	   && same <= nSame); 
  temp = (1 << nSame);
  for (j = 0; j < nPar; j++)
    err[j] /= temp;
  memcpy(par, parBest2, size);
  par[nPar] = qBest2;

  if (narg > 14 && ps[14]) {    /* have explicity ERROR; calculate */
                                /* "standard" errors */
    if (!qBest2) {              /* the fit is perfect */
      for (j = 0; j < nPar; j++)
        err[j] = 0;             /* so all errors are zero */
    } else for (i = 0; i < nPar; i++) {/* check all parameters */
      double    h, h0, hmax, hmin, qmin, qmax;

      if (!step[i]) {           /* this parameter was kept fixed, */
        err[i] = 0;             /* so we have no error estimate */
        continue;               /* and can start with the next one */
      }
      /* we define the "error" in a parameter as the average distance */
      /* of that parameter from its "best" value at which the fit quality */
      /* has doubled.  We find the parameter values at which the double */
      /* of the minimum error occurs using bisecting.  First, we start */
      /* from the best parameter value and go to higher values until we */
      /* find a parameter value at which the error value is high enough. */
      /* Then we use bisecting to find the exact value of the parameter */
      /* at which double the error occurs.  Then we do the same thing */
      /* going from the best value to lower values, and then the resulting */
      /* error estimate for the parameter is the average of the distances */
      /* going up and going down. */
      h = step[i];		/* first estimate: the step size */
      do {
        par[i] = parBest2[i] + h; /* best parameter value + error estimate */
        /* calculate the quality */
        if (fitSym) {
          j = eval(fitTemp);
          if (j < 0)            /* some error */
            goto generalfit_1;
          qual = double_arg(j);
          zapTemp(j);
        } else
          qual = fitFunc(par, nPar, xp, yp, weights, nPoints);
        qual /= qBest2;
        if (qual < 2)           /* not yet high enough */
          h *= 2;
      } while (qual < 2 && h < FLT_MAX);
      if (h >= FLT_MAX) {
        err[i] = FLT_MAX;
        par[i] = parBest2[i];
        continue;
      }
      h0 = h;                   /* remember this one for going down */
      /* now get ready for bisecting */
      hmax = h;                 /* set the initial bisection bounds */
      hmin = 0;
      qmin = -1;                /* minimum error/minimum error - 2 */
      /* calculate the error offset at the upper bound */
      par[i] = parBest2[i] + hmax;/* current upper bound */
      if (fitSym) {
        j = eval(fitTemp);
        if (j < 0)                      /* some error */
          goto generalfit_1;
        qmax = double_arg(j);
        zapTemp(j);
      } else
        qmax = fitFunc(par, nPar, xp, yp, weights, nPoints);
      qmax = qmax/qBest2 - 2;
      /* OK, now do the bisecting */
      do {
        h = 0.5*(hmin + hmax);  /* middle of current range */
        par[i] = parBest2[i] + h;
        /* calculate quality */
        if (fitSym) {
          j = eval(fitTemp);
          if (j < 0)                    /* some error */
            goto generalfit_1;
          qual = double_arg(j);
          zapTemp(j);
        } else
          qual = fitFunc(par, nPar, xp, yp, weights, nPoints);
        qual = qual/qBest2 - 2;
        if (qual > 0) {         /* we're too high */
          hmax = h;
          qmax = qual;
        } else {
          hmin = h;
          qmin = qual;
        }
      } while (fabs(qual) > 1e-4);
      err[i] = h;
      /* now do the same thing going from the best parameter value */
      /* to lower values */
      h = h0;                   /* value we got from going up is probably */
                                /* pretty good */
      /* we bracket the location with double the minimum error */
      do {
        par[i] = parBest2[i] - h; /* best parameter value - error estimate */
        /* calculate the quality */
        if (fitSym) {
          j = eval(fitTemp);
          if (j < 0)            /* some error */
            goto generalfit_1;
          qual = double_arg(j);
          zapTemp(j);
        } else
          qual = fitFunc(par, nPar, xp, yp, weights, nPoints);
        qual /= qBest2;
        if (qual < 2)           /* not yet high enough */
          h *= 2;
      } while (qual < 2 && h < FLT_MAX);
      if (h >= FLT_MAX) {
        err[i] = FLT_MAX;
        par[i] = parBest2[i];
        continue;
      }
      /* now get ready for bisecting */
      hmax = h;                 /* set the initial bisection bounds */
      hmin = 0;
      qmin = -1;                /* minimum error/minimum error - 2 */
      /* calculate the error offset at the upper bound */
      par[i] = parBest2[i] - hmax;/* current lower bound */
      if (fitSym) {
        j = eval(fitTemp);
        if (j < 0)                      /* some error */
          goto generalfit_1;
        qmax = double_arg(j);
        zapTemp(j);
      } else
        qmax = fitFunc(par, nPar, xp, yp, weights, nPoints);
      qmax = qmax/qBest2 - 2;
      /* OK, now do the bisecting */
      do {
        h = 0.5*(hmin + hmax);  /* middle of current range */
        par[i] = parBest2[i] - h;
        /* calculate quality */
        if (fitSym) {
          j = eval(fitTemp);
          if (j < 0)                    /* some error */
            goto generalfit_1;
          qual = double_arg(j);
          zapTemp(j);
        } else
          qual = fitFunc(par, nPar, xp, yp, weights, nPoints);
        qual = qual/qBest2 - 2;
        if (qual > 0) {         /* we're too high */
          hmax = h;
          qmax = qual;
        } else {
          hmin = h;
          qmin = qual;
        }
      } while (fabs(qual) > 1e-4);
      err[i] = 0.5*(err[i] + h)/sqrt(2*qBest2);
      par[i] = parBest2[i];     /* restore */
    } /* end of for (i = 0; i < nPar; i++) */
  } else
    free(err);
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
int ana_geneticfit(int narg, int ps[])
/* FIT2(x,y,nPar,fit [,mu,ngenerations,population,pcross,pmutate,vocal]
        [,/ELITE,/BYTE,/WORD,/LONG,/FLOAT,/DOUBLE]) */
{
  int   fitSym, iq, nPoints, nPopulation, nPar, fitTemp, i, j, size, result,
    *rtoi, *itor, pair, k, w, generation, i1, i2, icross, imutate, ibit,
    iter = 0, vocal, typesize;
  union {
    byte	*b;
    unsigned short	*w;
    unsigned int	*l;
    float	*f;
    double	*d;
  } p;
  word	*par, fitPar, xSym, ySym, fitArg[3];
  float	*quality, mu, *distr, random_one(void), pcross, *quality2,
  byte  *genes, *parent1, *parent2, *genes2, t1, t2;
    crossmark, mutatemark, pmutate, sum;
  void  invertPermutation(int *data, int n),
    indexxr_f(int n, float ra[], int indx[]);
  int   random_distributed(int modulus, double *distr);
  byte  changed, elite, partype;
  static unsigned short int mask1[] = {
    0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01
  }, mask2[] = {
    0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe,
  }, mask3[] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  };
 
  iq = ps[3];                   /* fit */
  if (!symbolIsString(iq))
    return cerror(NEED_STR, ps[3]);
  fitSym = stringpointer(string_value(iq), SP_USER_FUNC);
  if (fitSym < 0)
    return
      anaerror("Sorry, fitting to interal functions is not yet implemented",
	    iq);

  nPar = int_arg(ps[2]);	/* nPar */
  if (nPar < 1)
    return anaerror("A positive value is required for the number of parameters",
		 ps[2]);

  if (narg > 4 && ps[4]) {	/* mu: selection pressure */
    mu = float_arg(ps[4]);
    if (mu < 0.0)
      mu = 0.0;
    else if (mu > 1.0)
      mu = 1.0;
  } else
    mu = 0.1;

  if (narg > 5 && ps[5]) {	/* generations: number of generations */
    generation = int_arg(ps[5]);
    if (generation < 1)
      generation = 1;
  } else
    generation = 50;

  if (narg > 7 && ps[7]) {	/* pcross: cross-over probability */
    pcross = float_arg(ps[7]);
    if (pcross < 0.0)
      pcross = 0.0;
    else if (pcross > 1.0)
      pcross = 1.0;
  } else
    pcross = 0.3;

  if (narg > 8 && ps[8]) {	/* pmutate: mutation probability */
    pmutate = float_arg(ps[8]);
    if (pmutate < 0.0)
      pmutate = 0.0;
    else if (pmutate > 1.0)
      pmutate = 1.0;
  } else
    pmutate = 0.01;

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

  if (narg > 6 && ps[6]) {	/* population */
    nPopulation = int_arg(ps[6]);
    if (nPopulation < 1)
      nPopulation = 1;
    /* we make sure that the population fills a number of ints exactly */
    if (partype < ANA_LONG)
      nPopulation += (nPopulation % (sizeof(int)/ana_type_size[partype]));
  } else
    nPopulation = 100;

  if (!symbolIsNumericalArray(ps[0])) /* x */
    return cerror(NEED_NUM_ARR, ps[0]);
  xSym = ana_float(1, ps);
  if (isFreeTemp(xSym))
    symbol_context(xSym) = 1;   /* so it doesn't get prematurely deleted */
  nPoints = array_size(xSym);

  if (!symbolIsNumericalArray(ps[1])) /* y */
    return cerror(NEED_NUM_ARR, ps[1]);
  if (array_size(ps[1]) != nPoints)
    return cerror(INCMP_ARG, ps[1]);
  ySym = ana_float(1, ps + 1);
  if (isFreeTemp(ySym))
    symbol_context(ySym) = 1;

  if (narg > 9 && ps[9])	/* VOCAL */
    vocal = int_arg(ps[9]);
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
  fitTemp = nextFreeTempExecutable();
  symbol_class(fitTemp) = ANA_USR_FUNC;
  usr_func_arguments(fitTemp) = fitArg;
  symbol_memory(fitTemp) = 3*sizeof(word);
  usr_func_number(fitTemp) = fitSym;

  /* create initial population */
  genes = malloc(nPopulation*size);
  genes2 = malloc(nPopulation*size);
  quality = malloc(nPopulation*sizeof(float));
  quality2 = malloc(nPopulation*sizeof(float));
  rtoi = malloc(nPopulation*sizeof(int));
  /* itor = malloc(nPopulation*sizeof(int)); */

  /* we fill <genes> with random bits.  <genes> exactly spans a number */
  /* of ints, so we don't need to worry about the exact type */
  p.l = (unsigned int *) genes;
  for (i = 0; i < nPopulation*size/sizeof(int); i++)
    *p.l++ = random_bits();
  /* if the parameter type is FLOAT or DOUBLE, then some of the bit patterns */
  /* may not correspond to representable values, but rather to NaNs. */
  /* We must fix those elements. We use a single scheme for both FLOAT */
  /* and DOUBLE values, assuming that when the bit patterns of a DOUBLE */
  /* NaN are interpreted as the bit patterns of some FLOAT values instead, */
  /* then at least one of those FLOATs is a FLOAT NaN, and replacing that */
  /* FLOAT NaN with a representable FLOAT value generates a representable */
  /* DOUBLE value. */
  if (partype >= ANA_FLOAT) {	/* FLOAT or DOUBLE */
    p.f = (float *) genes;
    for (i = 0; i < nPopulation*size/sizeof(float); i++) {
      while (isnan(*p.f))
	*p.l = random_bits();
      p.f++;
    }
  }

  /* now calculate the quality of all members of the population */
  for (i = 0; i < nPopulation; i++) {
    memcpy(par, genes + i*nPar*typesize, size);
    j = eval(fitTemp);		/* get quality */
    if (j == ANA_ERROR)		/* some error occurred */
      goto geneticfit_1;
    quality[i] = float_arg(j);
    zapTemp(j);
  }
  internalMode = 0;		/* or we may get unexpected results */
  indexxr_f(nPopulation, quality, rtoi); /* get ranking */
  /* now rtoi[i] is the index of the i-th smallest element of quality[] */
  /* calculate the inverse ranking */
  /* memcpy(itor, rtoi, nPopulation*sizeof(int));
     invertPermutation(itor, nPopulation); */
  /* now itor[i] is the rank of the i-th element of quality[], in ascending */
  /* order */
  /* now calculate the distribution function for selecting members */
  distr = malloc(nPopulation*sizeof(float));
  for (i = 1; i <= nPopulation; i++)
    distr[i - 1] = i*(1 + (mu*(i - nPopulation))/(nPopulation-1))/nPopulation;

  if (vocal > 1) {
    p.b = genes;
    for (i = 0; i < nPopulation; i++) {
      putchar('[');
      for (j = 0; j < nPar; j++) {
	if (j)
	  putchar(' ');
	switch (partype) {
	  case ANA_BYTE:
	    printf("%u", p.b[rtoi[i]*nPar+j]);
	    break;
	  case ANA_WORD:
	    printf("%u", p.w[rtoi[i]*nPar+j]);
	    break;
	  case ANA_LONG:
	    printf("%u", p.l[rtoi[i]*nPar+j]);
	    break;
	  case ANA_FLOAT:
	    printf("%g", p.f[rtoi[i]*nPar+j]);
	    break;
	  case ANA_DOUBLE:
	    printf("%g", p.d[rtoi[i]*nPar+j]);
	    break;
	}
      }
      printf(":%g]", quality[rtoi[i]]);
    }
  }

  if (vocal) {
    sum = 0.0;
    for (i = 0; i < nPopulation; i++)
      sum += quality[i];
    printf("%3d: %8.5g %8.5g %8.5g %8.5g\n", ++iter, quality[rtoi[0]],
	   quality[rtoi[nPopulation/2]], quality[rtoi[nPopulation - 1]],
	   sum/nPopulation);
  }

  /* iterate over the desired number of generations */
  crossmark = pcross? random_one()/pcross: LONG_MAX;
  mutatemark = pmutate? random_one()/pmutate: LONG_MAX;
  imutate = icross = 0;

  while (generation--) {	/* all generations */
    if (elite) {
      memcpy(genes2, genes + nPar*rtoi[nPopulation - 1]*typesize, size);
      genes2 += nPar*typesize;
      memcpy(genes2, genes + nPar*rtoi[nPopulation - 2]*typesize, size);
      genes2 += nPar*typesize;
      *quality2++ = quality[rtoi[nPopulation - 1]];
      *quality2++ = quality[rtoi[nPopulation - 2]];
    }
    for (pair = elite; pair < nPopulation/2; pair++) { /* remaining pairs */
      /* pick parents */
      i1 = random_distributed(nPopulation, distr);
      parent1 = genes + nPar*rtoi[i1]*typesize;
      do {
	i2 = random_distributed(nPopulation, distr);
	parent2 = genes + nPar*rtoi[i2]*typesize;
      } while (parent1 == parent2);

      if (++icross >= crossmark) { /* do cross-over */
	crossmark += random_one()/pcross - icross;
	icross = 0;
	k = (int) (random_one()*(nPar*8 - 1)) + 1; /* pick cross-over site */
	w = k/8;		/* byte index */
	k -= w*8;		/* bit index */
	for (i = 0; i < w; i++) { /* swap before cross-over word */
	  t1 = parent1[i];
	  parent1[i] = parent2[i];
	  parent2[i] = t1;
	} /* end of for (i = 0; ...) */
	if (k) {		/* cross-over site is not on word boundary */
	  t1 = ((parent1[w] & mask1[k]) | (parent2[w] & mask2[k]));
	  t2 = ((parent2[w] & mask1[k]) | (parent1[w] & mask2[k]));
	  parent1[w] = t1;
	  parent2[w] = t2;
	} /* end of if (k) */
	switch (partype) {
	  default:
	    changed = 1;
	    break;
	  case ANA_FLOAT:
	    changed = !(isnan(*(float *) parent1)
			|| isnan(*(float *) parent2));
	    break;
	  case ANA_DOUBLE:
	    changed = !(isnan(*(double *) parent1)
			|| isnan(*(double *) parent2));
	    break;
	}
	if (!changed) {
	  /* we generated one or two NaNs from regular numbers: */
	  /* undo the crossover */
	  for (i = 0; i < w; i++) { /* swap before cross-over word */
	    t1 = parent1[i];
	    parent1[i] = parent2[i];
	    parent2[i] = t1;
	  } /* end of for (i = 0; ...) */
	  if (k) {		/* cross-over site is not on word boundary */
	    t1 = ((parent1[w] & mask1[k]) | (parent2[w] & mask2[k]));
	    t2 = ((parent2[w] & mask1[k]) | (parent1[w] & mask2[k]));
	    parent1[w] = t1;
	    parent2[w] = t2;
	  } /* end of if (k) */
	}
      } else
	changed = 0; 
      /* end of if (random_one() < pcross) */

      /* do mutations */
      while (mutatemark < nPar*8) {
	ibit = mutatemark;
	mutatemark += random_one()/pmutate;
	w = ibit/8;		/* byte index */
	k = ibit - 8*w;		/* bit index */
	parent1[w] ^= mask3[k]; /* flip bit */
	switch (partype) {
	  default:
	    changed = 1;
	    break;
	  case ANA_FLOAT:
	    changed = !(isnan(*(float *) parent1));
	    break;
	  case ANA_DOUBLE:
	    changed = !(isnan(*(double *) parent1));
	    break;
	}
	if (!changed)
	  /* the mutation generated a NaN; undo */
	  parent1[w] ^= mask3[k];
      }
      mutatemark -= nPar*8;
      while (mutatemark < nPar*8) {
	ibit = mutatemark;
	mutatemark += random_one()/pmutate;
	w = ibit/8;		/* byte index */
	k = ibit - 8*w;		/* bit index */
	parent2[w] ^= mask3[k]; /* flip bit */
	switch (partype) {
	  default:
	    changed = 1;
	    break;
	  case ANA_FLOAT:
	    changed = !(isnan(*(float *) parent2));
	    break;
	  case ANA_DOUBLE:
	    changed = !(isnan(*(double *) parent2));
	    break;
	}
	if (!changed)
	  /* the mutation generated a NaN; undo */
	  parent2[w] ^= mask3[k];
      }
      mutatemark -= nPar*8;

      /* store processed genes in new population */
      memcpy(genes2, parent1, size);
      genes2 += nPar*typesize;
      memcpy(genes2, parent2, size);
      genes2 += nPar*typesize;
      if (changed) {
	memcpy(par, parent1, size);
	j = eval(fitTemp);
	if (j == ANA_ERROR)
	  goto geneticfit_3;
	*quality2++ = float_arg(j);
	zapTemp(j);
	memcpy(par, parent2, size);
	j = eval(fitTemp);
	if (j == ANA_ERROR)
	  goto geneticfit_3;
	*quality2++ = float_arg(j);
	zapTemp(j);
      } else {
	*quality2++ = quality[rtoi[i1]];
	*quality2++ = quality[rtoi[i2]];
      }
    } /* end of for (pair = 0; ...) */
    genes2 -= nPopulation*nPar*typesize;
    quality2 -= nPopulation;
    
    memcpy(genes, genes2, nPopulation*size);
    memcpy(quality, quality2, nPopulation*sizeof(float));

    indexxr_f(nPopulation, quality, rtoi); /* get ranking */

    if (vocal > 1) {
      p.b = genes;
      for (i = 0; i < nPopulation; i++) {
	putchar('[');
	for (j = 0; j < nPar; j++) {
	  if (j)
	    putchar(' ');
	  switch (partype) {
	    case ANA_BYTE:
	      printf("%u", p.b[rtoi[i]*nPar+j]);
	      break;
	    case ANA_WORD:
	      printf("%u", p.w[rtoi[i]*nPar+j]);
	      break;
	    case ANA_LONG:
	      printf("%u", p.l[rtoi[i]*nPar+j]);
	      break;
	    case ANA_FLOAT:
	      printf("%g", p.f[rtoi[i]*nPar+j]);
	      break;
	    case ANA_DOUBLE:
	      printf("%g", p.d[rtoi[i]*nPar+j]);
	      break;
	  }
	}
	printf(":%g]", quality[rtoi[i]]);
      }
    }

    if (vocal) {
      sum = 0.0;
      for (i = 0; i < nPopulation; i++)
	sum += quality[i];
      printf("%3d: %8.5g %8.5g %8.5g %8.5g\n", ++iter, quality[rtoi[0]],
	     quality[rtoi[nPopulation/2]], quality[rtoi[nPopulation - 1]],
	     sum/nPopulation);
    }
    /* memcpy(itor, rtoi, nPopulation*sizeof(int));
       invertPermutation(itor, nPopulation); */
  } /* end of while (generation--) */
  
  result = array_scratch(partype, 1, &nPar);
  memcpy(array_data(result), genes + nPar*rtoi[nPopulation - 1]*typesize,
         size);

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
  free(quality);
  free(genes2);
  free(quality2);
  return result;

  geneticfit_3:
  genes2 -= pair*2*nPar*typesize;
  quality2 -= pair*2;

  geneticfit_1:
  result = ANA_ERROR;
  goto geneticfit_2;
}
