/* This is file plots.c.

Copyright 2013 Louis Strous, Richard Shine

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
/* File plots.c */
/* LUX routines dealing with line plots. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if HAVE_LIBX11
#include <X11/Xlib.h>
#else
#define setup_x()	LUX_OK
#endif
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "action.h"

extern	int32_t	lastmin_sym, lastmax_sym;
extern	float	callig_xb, callig_yb;
		 /* some common variables */
array	*h;
pointer q1, q2, q3, q4;
int32_t	tkplot(float x, float y, int32_t lineStyle, int32_t symStyle),
	callig(char *, float, float, float, float, int32_t, int32_t),
	sform(float, float);
 /* plotting context */
int32_t	lunplt = 0, ixlow, ixhigh, iylow, iyhigh, standardGray;
float	xfac = 512.0, yfac = 512.0, xerrsize = 0.05, yerrsize = 0.05;
int32_t	numarrays, numstrings, numscalars;
static	int32_t	xsym, ysym, xtitlesym, ytitlesym, titlesym, symStyle, nx, ny,
  exsym, eysym;
static	int32_t	nelem, isym, nbreak, *qi, huge = INT32_MAX, dq3, dq4,
		lineStyle;	/* lineStyle not yet fully implemented */
char	form[20], label[25], callig_update = 1, *plotxfmt = "%1g",
  *plotyfmt = "%1g";
int32_t	landscape = 1, iorder = 0, current_pen = 3;
float	current_gray = 1, startx, stepx, starty, stepy;
 /* contents of VMS common plots follows */
int32_t	ilabx = 1, ilaby = 1, irxf = 1, iryf = 1, ndx, fstepx = 0, fstepy = 0;
int32_t	nd, ipltyp, ifz = 0, ifzx = 1, ndxs, ndys, ier = 1;
int32_t	ifont = 3, ndlabx = 2, ndlaby = 2, iblank = 1, ndot = 1, ifirstflag;
float	xmin, xmax, ymin, ymax;
float	wxb = 0.15, wxt = 0.9, wyb = 0.1, wyt = 0.7;
float	ticx = 0.01, ticy = 0.01, plims[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, 
	xlimit = 0.9999, ylimit = 0.9999;
float	fsized = 1, symsize = 1.0, dashsize = 1.0, theDashSize = 1.0;
float	symratio = 1.0, ticxr = 0.5, ticyr = 0.5, dvx, dv;
 /* end of VMS  common plots */
int32_t	useProjection = 0, oldLineStyle = 0;
static float	zero = 0.0, one = 0.99999999;
float	*plotWindow[4] = { &wxb, &wxt, &wyb, &wyt }, 
	*screenWindow[4] = { &zero, &one, &zero, &one };
int32_t	tkCoordSys = LUX_DEP;
extern int32_t	projectTk;
extern float	*projectMatrix;
int32_t	createFullProjection(float *, float *, float *), 
	coordTrf(float *, float *, int32_t, int32_t);
 /* for bounding box LS 18jan95: */
float	postXBot = FLT_MAX, postXTop = -FLT_MAX, 
	postYBot = FLT_MAX, postYTop = -FLT_MAX;
int32_t	updateBoundingBox = 1;	/* if 1, then everything counts */
/* bounding box control is necessary for PLOT, XYMOV, TV */

int32_t	alternateDash = 0;	/* if nonzero, then xymov "moves" are
				   displayed as "draws" in background
				   color instead. */
float   lumpx = 0;		/* the desired resolution in the x direction */
				/* in plots (0 = full) */

void	set_cur_pen(void);

#ifdef DEVELOP
extern float	*currentOblique, *currentPerspective;
#endif
 /*------------------------------------------------------------------------- */
enum { POSSYM, NEGSYM, POSLINE, ZEROS, NONE };
int32_t fixPlotStyle(int32_t *symbol, int32_t *line)
     /* checks symbol and line style choices for legality and puts in */
     /* standard format.  LS 3feb95 */
{
  static char	types[] =
    /* -9..-1: NEGSYM, 0: ZEROS, 1: POSLINE, 2..9: POSSYM, */
    /* 10..19: POSLINE, 20..29: POSSYM */
  { NEGSYM, NEGSYM, NEGSYM, NEGSYM, NEGSYM, NEGSYM, NEGSYM, NEGSYM, NEGSYM, 
      ZEROS, POSLINE, POSSYM, POSSYM, POSSYM, POSSYM, POSSYM, POSSYM, POSSYM, 
      POSSYM, POSLINE, POSLINE, POSLINE, POSLINE, POSLINE, POSLINE, POSLINE, 
      POSLINE, POSLINE, POSLINE, POSSYM, POSSYM, POSSYM, POSSYM, POSSYM, 
      POSSYM, POSSYM, POSSYM, POSSYM, POSSYM };
  char	sym, lin, ok = 1;
  int32_t	temp;

  /* first, see what the specified types are */
  if (*symbol >= -9 && *symbol <= 29)
    sym = types[*symbol + 9];
  else if (*symbol == LUX_UNSPECIFIED)
    sym = NONE;
  else
    return luxerror("Specified symbol style %d is illegal\n", 0, *symbol);
  if (*line >= -9 && *line <= 29)
    lin = types[*line + 9];
  else if (*line == LUX_UNSPECIFIED)
    lin = NONE;
  else
    return luxerror("Specified line style %d is illegal\n", 0, *line);
  /* then, check for legal combinations and put in standard format */
  switch (sym) {
    case POSSYM:
      switch (lin) {
	case POSSYM:		/* 3, 4 not allowed; */
	  if (*line == *symbol) {
	    *line = 0;
	    break;
	  }
	case NEGSYM:
	  ok = 0;
	  break;
	case NONE:
	  *line = 1;
	  break;
      }
      break;
    case NEGSYM:
      switch (lin) {
	case NEGSYM:
	  if (*line == *symbol) {
	    *line = 0;
	    *symbol = -*symbol;
	    break;
	  }
	case POSLINE: case POSSYM:
	  ok = 0;
	  break;
	case NONE:
	  *line = 0;
	case ZEROS:
	  *symbol = -*symbol;
	  break;
      }
      break;
    case POSLINE:
      switch (lin) {
	case POSSYM:
	  temp = *line;
	  *line = *symbol;
	  *symbol = temp;
	  break;
	case POSLINE:
	  if (*line == *symbol) {
	    *symbol = 0;
	    break;
	  }
	case NEGSYM:
	  ok = 0;
	  break;
	case NONE: case ZEROS:
	  *line = *symbol; 
	  *symbol = 0;
	  break;
      }
      break;
    case ZEROS:
      switch (lin) {
	case POSSYM:
	  *symbol = *line;
	  *line = 0;
	  break;
	case NEGSYM:
	  *symbol = -*line;
	case NONE:
	  *line = 0;
	  break;
      }
      break;
    case NONE:
      switch (lin) {
	case POSSYM:
	  *symbol = *line;
	  *line = 1;
	  break;
	case NEGSYM:
	  *symbol = -*line;
	  *line = 0;
	  break;
	case NONE:
	  *line = 1;
	case POSLINE: case ZEROS:
	  *symbol = 0; 
	  break;
      }
      break;
  }
  if (!ok)
    return
      luxerror("Illegal combination of symbol type (%d) and line style (%d)\n", 
	    0, *symbol, *line);
  return 1;
}
 /*------------------------------------------------------------------------- */
int32_t lux_plot(int32_t narg, int32_t ps[]) /* plot routine */
 /* plots data */
{
  int32_t	oldIer = ier, n;
  int32_t	preplot(int32_t, int32_t []), plotxy(float [], float [], float [], float [],
				     int32_t, int32_t, int32_t), labels(void);

  ier = (internalMode & 64)? 0: ier; /* /KEEP: erase before plotting? */
  n = ((lunplt != 0 || setup_x() == LUX_OK)
       && (preplot(narg, ps) == LUX_OK)
       && (plotxy(q1.f, q2.f, q3.f, q4.f, nelem, dq3, dq4) == LUX_OK)
       //       && (labels() == LUX_OK)
    );
  ier = oldIer;
  return n? 1: LUX_ERROR; 
}
 /*------------------------------------------------------------------------- */
int32_t lux_oplot(int32_t narg, int32_t ps[]) /* oplot routine */
 /* over plot data */
{
  int32_t	preplot(int32_t, int32_t []),
    oplotx(float [], float [], float [], float [], int32_t, int32_t, int32_t);

  if ((lunplt == 0 && setup_x() != LUX_OK)
      || preplot(narg, ps) != 1
      || fixPlotStyle(&symStyle, &lineStyle) < 0
      || oplotx(q1.f, q2.f, q3.f, q4.f, nelem, dq3, dq4) != 1)
    return LUX_ERROR;
  return 1; 
}
 /*------------------------------------------------------------------------- */
int32_t preplot(int32_t narg, int32_t ps[])
 /* used by plot and oplot to setup */
{
  int32_t	i, iq, freePos = 1, xfmtsym, yfmtsym;
  int32_t	lux_indgen(int32_t, int32_t []);
  extern int32_t	curRoutineNum;
  char	*keyName(internalRoutine *, int32_t, int32_t);

  /* loop through the args and check what we have */
  xsym = ysym = ytitlesym = xtitlesym = titlesym = isym = exsym = eysym = 
    xfmtsym = yfmtsym = 0;
  symStyle = lineStyle = LUX_UNSPECIFIED;
  numarrays = numscalars = numstrings = 0;
  theDashSize = dashsize;
  for (i = 0; i < narg; i++) {
    iq = ps[i];
    if (!iq) {
      freePos = 0;		/* only keyword switches left; must be in */
				/* proper positions */
      continue; 		/* enables switch treatment */
    }
    switch (symbol_class(iq)) {
      case LUX_SCALAR:
	if (freePos)
	  switch (numscalars) {
	    case 0:
	      symStyle = int_arg(iq);
	      break;
	    case 1:
	      lineStyle = int_arg(iq);
	      break;
	    case 2:
	      theDashSize = float_arg(iq);
	      break;
	    case 3:
	      xerrsize = fabs(float_arg(iq));
	      break;
	    case 4:
	      yerrsize = fabs(float_arg(iq));
	      break;
	    default:
	      printf("too many scalars in list\n");	
	      return cerror(ILL_ARG_LIST, iq);
	  } else switch (i) {
	    case 2: 
	      symStyle = int_arg(iq);
	      break;
	    case 3:
	      lineStyle = int_arg(iq);
	      break;
	    case 7:
	      theDashSize = float_arg(iq);
	      break;
	    case 11:
	      xerrsize = fabs(float_arg(iq));
	      break;
	    case 12:
	      yerrsize = fabs(float_arg(iq));
	      break;
	    default:
	      printf("non-scalar argument expected for %s\n", 
		     keyName(subroutine, curRoutineNum, i));
	      return cerror(ILL_ARG_LIST, iq);
	  }
	numscalars++;
	break;
      case LUX_STRING:
	if (freePos)
	  switch (numstrings) {
	  case 0:
	    xtitlesym = iq;
	    break;
	  case 1:
	    ytitlesym = iq;
	    break;
	  case 2:
	    titlesym  = iq;
	    break;
	  case 3:
	    xfmtsym = iq;
	    break;
	  case 4:
	    yfmtsym = iq;
	    break;
	  default:
	    printf("too many strings in list\n");
	    return cerror(ILL_ARG_LIST, iq);
	  }
	else
	  switch (i) {
	  case 4:
	    xtitlesym = iq;
	    break;
	  case 5:
	    ytitlesym = iq;
	    break;
	  case 6:
	    titlesym = iq;
	    break;
	  case 13:
	    xfmtsym = iq;
	    break;
	  case 14:
	    yfmtsym = iq;
	    break;
	  default:
	    printf("non-string argument expected for %s\n", 
		   keyName(subroutine, curRoutineNum, i));
	    return cerror(ILL_ARG_LIST, iq);
	  }
	numstrings++;
	break;
      case LUX_ARRAY:
	if (freePos)
	  switch (numarrays) {
	    case 0:
	      xsym = iq;
	      break;
	    case 1:
	      ysym = iq;
	      break;
	    case 2:
	      exsym = iq;
	      break;
	    case 3:
	      eysym = iq;
	      break;
	    case 4:
	      isym = iq;
	      break;
	    default:
	      printf("too many arrays in list\n");
	      return cerror(ILL_ARG_LIST, iq);
	  }
	else
	  switch (i) {
	    case 0:
	      xsym = iq;
	      break;
	    case 1:
	      ysym = iq;
	      break;
	    case 8:
	      exsym = iq;
	      break;
	    case 9:
	      eysym = iq;
	      break;
	    case 10:
	      isym = iq;
	      break;
	    default:
	      printf("non-array argument expected for %s\n", 
		     keyName(subroutine, curRoutineNum, i));
	      return cerror(ILL_ARG_LIST, iq);
	  }
	numarrays++;
	break;
      default:
	printf("illegal argument in PLOT or OPLOT\n");
	return cerror(ILL_ARG, iq);
    }
  }
  if (ysym == 0) {
    if (xsym == 0) {
      printf("no data array specified\n");
      return cerror(ILL_ARG_LIST, 0);
    }
    ysym = xsym;		/* only one array, gen an x array */
    ysym = lux_float(1, &ysym);
    xsym = array_clone(ysym, sym[ysym].type);
    xsym = lux_indgen(1, &xsym); /* x specified */
  }
  xsym = lux_float(1, &xsym);
  nx = array_size(xsym);
  q1.f = (float *) array_data(xsym);
  ysym = lux_float(1, &ysym);
  ny = array_size(ysym);
  q2.f = (float *) array_data(ysym);
  nelem = MIN(nx, ny);		/* we use the smaller number of elements */
  if (exsym) {
    if (array_size(exsym) == nelem)
      dq3 = 0;
    else if (array_size(exsym) == 2*nelem)
      dq3 = nelem;
    else
      return cerror(INCMP_ARG, exsym);
    exsym = lux_float(1, &exsym);
    q3.f = (float *) array_data(exsym);
  } else
    q3.f = NULL;		/* no error bars */
  if (eysym) {
    if (array_size(eysym) == nelem)
      dq4 = 0;
    else if (array_size(eysym) == 2*nelem)
      dq4 = nelem;
    else
      return cerror(INCMP_ARG, eysym);
    eysym = lux_float(1, &eysym);
    q4.f = (float *) array_data(eysym);
  } else
    q4.f = NULL;		/* no error bars */
  if (isym) {			/* line breaks specified */
    isym = lux_long(1, &isym);
    nbreak = array_size(isym);
    qi = (int32_t *) array_data(isym);
  } else {
    qi = &huge;
    nbreak = 0;
  }
  if (internalMode & 0x400) 	/* the user specified a plot type */
    ipltyp = (internalMode & 0x300) >> 8;
  plotxfmt = xfmtsym? string_arg(xfmtsym): "%1g";
  plotyfmt = yfmtsym? string_arg(yfmtsym): "%1g";
  return 1;
}
 /*------------------------------------------------------------------------- */
int32_t mm(float *x, int32_t n, float *max, float *min)
{
 /* just return the min and max of a floating array */
  if (n <= 0)
  { printf("bad count in mm, n = %d\n", n);
    return -1; }
  *max = *min = *x++;
  n--;
  while (n--)
  { *max = MAX(*max, *x);
    *min = MIN(*min, *x);
    x++; }
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t plotxy(float xx[], float yy[], float ex[], float ey[], int32_t n, int32_t dx,
	    int32_t dy)
/* xx[]: x coordinates */
/* yy[]: y coordinates */
/* n: number of data points */
{
  int32_t	ixlog, iylog, i, j;
  float	x, y, stepxdvi, stepydvi, x2, y2, endx, endy;
  float	xlabel, f;
  extern int32_t	calligCoordSys;
  extern float	callig_ratio;
  char	*p;
  static float	logs[] = { 0.30103, 0.47712, 0.60206, 0.69897,
			   0.77815, 0.84510, 0.90309, 0.95424 };
  void	setupticks(float *min, float *max, int32_t ndlab, int32_t fstep, int32_t ilog,
		   int32_t irf, int32_t f, float wb, float wt, float plim_min,
		   float plim_max, float *start, float *step, float *end,
		   float *stepdvi);
  int32_t lux_erase(int32_t, int32_t []);
  int32_t oplotx(float [], float [], float [], float [], int32_t, int32_t, int32_t);

  iylog = ipltyp % 2;
  ixlog = (ipltyp/2) % 2;
  xmax = -FLT_MAX;
  xmin = FLT_MAX;
  ymax = -FLT_MAX;
  ymin = FLT_MAX;
  if (!ixlog && !iylog) {	/* linear-linear plot */
    if (!plims[0] || !plims[1])
      mm(xx, n, &xmax, &xmin);
    if (plims[0])
      xmin = plims[0];
    if (plims[1])
      xmax = plims[1];
    else if (lineStyle == 20 || symStyle == 20)	/* bar graph */
      xmax++;

    if (!plims[2] || !plims[3])
      mm(yy, n, &ymax, &ymin);
    if (plims[2])
      ymin = plims[2];
    if (plims[3])
      ymax = plims[3];
  } else if (ixlog && !iylog) {	/* log-linear plot */
    /* so: xmin, xmax must be greater than 0; we exclude data points
       with x <= 0 */
    for (i = 0; i < n; i++) {
      if (xx[i] > 0) {		/* acceptable */
	if (xx[i] > xmax)
	  xmax = xx[i];
	if (xx[i] < xmin)
	  xmin = xx[i];
	if (yy[i] > ymax)
	  ymax = yy[i];
	if (yy[i] < ymin)
	  ymin = yy[i];
      }
    }
    if (plims[0] > 0)
      xmin = plims[0];
    if (plims[1] > 0)
      xmax = plims[1];
    if (plims[2])
      ymin = plims[2];
    if (plims[3])
      ymax = plims[3];
  } else if (!ixlog && iylog) {	/* linear-log plot */
    /* so: ymin, ymax must be greater than 0; we exclude data points
       with y <= 0 */
    for (i = 0; i < n; i++) {
      if (yy[i] > 0) {		/* acceptable */
	if (xx[i] > xmax)
	  xmax = xx[i];
	if (xx[i] < xmin)
	  xmin = xx[i];
	if (yy[i] > ymax)
	  ymax = yy[i];
	if (yy[i] < ymin)
	  ymin = yy[i];
      }
    }
    if (plims[0])
      xmin = plims[0];
    if (plims[1])
      xmax = plims[1];
    if (plims[2] > 0)
      ymin = plims[2];
    if (plims[3] > 0)
      ymax = plims[3];
  } else {			/* log-log plot */
    /* so: xmin, ymin, xmax, ymax must be greater than 0; we exclude
       data points with x <= 0 or y <= 0 */
    for (i = 0; i < n; i++) {
      if (xx[i] > 0 && yy[i] > 0) { /* acceptable */
	if (xx[i] > xmax)
	  xmax = xx[i];
	if (xx[i] < xmin)
	  xmin = xx[i];
	if (yy[i] > ymax)
	  ymax = yy[i];
	if (yy[i] < ymin)
	  ymin = yy[i];
      }
    }
    if (plims[0] > 0)
      xmin = plims[0];
    if (plims[1] > 0)
      xmax = plims[1];
    if (plims[2] > 0)
      ymin = plims[2];
    if (plims[3] > 0)
      ymax = plims[3];
  }
  if (xmin < -FLT_MAX || xmax > FLT_MAX
      || ymin < -FLT_MAX || ymax > FLT_MAX
      || xmin > FLT_MAX || xmax < -FLT_MAX
      || ymin > FLT_MAX || ymax < -FLT_MAX)
    luxerror("Infinity in plot coordinates", 0);

  ndlabx = MAX(ndlabx, 1);	/* # minor divisions per major division */
  ndlaby = MAX(ndlaby, 1);
  setupticks(&xmin, &xmax, ndlabx, fstepx, ixlog, irxf, ifzx, wxb, wxt,
	     plims[0], plims[1], &startx, &stepx, &endx, &stepxdvi);
  setupticks(&ymin, &ymax, ndlaby, fstepy, iylog, iryf, ifz, wyb, wyt,
	     plims[2], plims[3], &starty, &stepy, &endy, &stepydvi);
  tkCoordSys = calligCoordSys = LUX_DVI;
  if (ier)
    lux_erase(0, &ilabx);	/* ilabx is dummy argument */

  /* draw the plot window */
  /* if !PLTYP >= 4, then only draw the left and bottom parts */
  tkplot(wxb, wyt, 0, 0);
  tkplot(wxb, wyb, 1, 0);
  tkplot(wxt, wyb, 1, 0);
  if (ipltyp < 4) {		/* draw complete window */
    tkplot(wxt, wyt, 1, 0);
    tkplot(wxb, wyt, 1, 0);
  }

  xlabel = 0.0;

  /* draw the tick marks and the labels along the x axis*/
  for (x = startx; x*stepx <= endx*stepx; x += stepx) {
    /* a bottom tick mark */
    tkplot(wxb + (x - xmin)*stepxdvi, wyb, 0, 0);
    tkplot(wxb + (x - xmin)*stepxdvi, wyb + ticx*fsized*callig_ratio, 1, 0);
    if (ipltyp < 4) {
      /* a top tick mark */
      tkplot(wxb + (x - xmin)*stepxdvi, wyt, 0, 0);
      tkplot(wxb + (x - xmin)*stepxdvi, wyt - ticx*fsized*callig_ratio, 1,
	     0);
    }
    /* the label */
    if (fabs(x) < 0.001*stepx)
      x = 0.0;			/* so we don't get 0 with roundoff error */
    sprintf(curScrat, ixlog? "%1g": plotxfmt, ixlog? pow(10,x): x);
    callig(curScrat, 0, 0, fsized, 0, ifont, 0);
    f = wxb + (x - xmin)*stepxdvi - 0.5*callig_xb;
    if (f > xlabel) {		/* if there is room */
      callig(curScrat, f, wyb - 0.03*fsized*callig_ratio, fsized, 0,
	     ifont, 1);
      xlabel = callig_xb;
    }
  } /* end of for (x = startx; x <= xmax; x += stepx) */

  /* now the secondary tick marks */
  if (ixlog) {
    x -= (floor(x - xmin) + 1)*stepx;
    for (; x <= xmax; x += stepx) {
      for (i = 0; i < 9; i++) {
	x2 = wxb + (x - xmin + logs[i])*stepxdvi;
	if (x2 >= wxb && x2 <= wxt) {
	  tkplot(x2, wyb, 0, 0);
	  tkplot(x2, wyb + ticx*ticxr*fsized*callig_ratio, 1, 0);
	  if (ipltyp < 4) {
	    tkplot(x2, wyt, 0, 0);
	    tkplot(x2, wyt - ticx*ticxr*fsized*callig_ratio, 1, 0);
	  }
	}
      }
    }
  } else {
    stepx /= ndlabx;
    x -= floor((x - xmin)/stepx)*stepx;
    for ( ; x*stepx <= xmax*stepx; x += stepx) {
      tkplot(wxb + (x - xmin)*stepxdvi, wyb, 0, 0);
      tkplot(wxb + (x - xmin)*stepxdvi, wyb + ticx*ticxr*fsized*callig_ratio,
	     1, 0);
      if (ipltyp < 4) {
	tkplot(wxb + (x - xmin)*stepxdvi, wyt, 0, 0);
	tkplot(wxb + (x - xmin)*stepxdvi, wyt - ticx*ticxr*fsized*callig_ratio,
	       1, 0);
      }
    }
    stepx *= ndlabx;
  }

  xlabel = 0;

  /* draw the tick marks and the labels along the y axis*/
  for (y = starty; y*stepy <= endy*stepy; y += stepy) {
    /* a left tick mark */
    tkplot(wxb, wyb + (y - ymin)*stepydvi, 0, 0);
    tkplot(wxb + ticy*fsized*callig_ratio, wyb + (y - ymin)*stepydvi, 1, 0);
    if (ipltyp < 4) {
      /* a right tick mark */
      tkplot(wxt, wyb + (y - ymin)*stepydvi, 0, 0);
      tkplot(wxt - ticy*fsized*callig_ratio, wyb + (y - ymin)*stepydvi,
	     1, 0);
    }
    /* the label */
    if (fabs(y) < 0.001*stepy)
      y = 0.0;			/* so we don't get 0 with roundoff error */
    sprintf(curScrat, iylog? "%1g": plotyfmt, iylog? pow(10,y): y);
    callig(curScrat, 0, 0, fsized, 0, ifont, 0);
    if (callig_xb > xlabel)
      xlabel = callig_xb;
    callig(curScrat, wxb - 0.005*fsized*callig_ratio - callig_xb,
	   wyb + (y - ymin)*stepydvi - 0.01*fsized*callig_ratio,
	   fsized, 0, ifont, 1);
  } /* end of for (y = starty; y <= ymay; y += stepy) */

  /* now the secondary tick marks */
  if (iylog) {
    y -= (floor(y - ymin) + 1)*stepy;
    for (; y*stepy <= ymax*stepy; y += stepy) {
      for (i = 0; i < 9; i++) {
	y2 = wyb + (y - ymin + logs[i])*stepydvi;
	if (y2 >= wyb && y2 <= wyt) {
	  tkplot(wxb, y2, 0, 0);
	  tkplot(wxb + ticy*ticyr*fsized*callig_ratio, y2, 1, 0);
	  if (ipltyp < 4) {
	    tkplot(wxt, y2, 0, 0);
	    tkplot(wxt - ticy*ticyr*fsized*callig_ratio, y2, 1, 0);
	  }
	}
      }
    }
  } else {
    stepy /= ndlaby;
    y -= floor((y - ymin)/stepy)*stepy;
    for (; y <= ymax; y += stepy) {
      tkplot(wxb, wyb + (y - ymin)*stepydvi, 0, 0);
      tkplot(wxb + ticy*ticyr*fsized*callig_ratio, wyb + (y - ymin)*stepydvi,
	     1, 0);
      if (ipltyp < 4) {
	tkplot(wxt, wyb + (y - ymin)*stepydvi, 0, 0);
	tkplot(wxt - ticy*ticyr*fsized*callig_ratio, wyb + (y - ymin)*stepydvi,
	       1, 0);
      }
    }
    stepy *= ndlaby;
  }

  if (xtitlesym) {		/* x title */
    p = string_value(xtitlesym);
    callig(p, 0, 0, fsized, 0, ifont, 0);
    callig(p, 0.5*(wxb + wxt - callig_xb), wyb - 0.07*fsized*callig_ratio,
	   fsized, 0, ifont, 1);
  }

  if (ytitlesym) {		/* y title */
    p = string_value(ytitlesym);
    callig(p, 0, 0, fsized, 90, ifont, 0);
    callig(p, wxb - xlabel - 0.02*fsized*callig_ratio,
	   0.5*(wyb + wyt - callig_yb), fsized, 90, ifont, 1);
  }

  if (titlesym) {		/* main title */
    p = string_value(titlesym);
    callig(p, 0, 0, fsized, 0, ifont, 0);
    callig(p, 0.5*(wxb + wxt - callig_xb), wyt + 0.02*fsized*callig_ratio,
	   fsized, 0, ifont, 1);
  }

  if (ixlog) {
    xmin = pow(10, xmin);
    xmax = pow(10, xmax);
  }
  if (iylog) {
    ymin = pow(10, ymin);
    ymax = pow(10, ymax);
  }

  if (n > 0) {			/* no bounding box checking required in */
    int32_t oplotx(float *, float *, float *, float *, int32_t, int32_t, int32_t);
   /* oplotx, since all data points fall within the plot window, i.e. */
   /* also within the current bounding box.  May want to check when */
   /* the axis labels and titles are drawn, so don't just set to zero */
    i = updateBoundingBox;
    updateBoundingBox = 0;
    if (fixPlotStyle(&symStyle, &lineStyle) < 0)
      return LUX_ERROR;
    j = oplotx(xx, yy, ex, ey, n, dx, dy);
    updateBoundingBox = i;
    return j;
  }
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t positionClassWindow(float x, float y, float **w)
/* returns position class of point (x, y) relative to the plot window: */
/*    0 1 2 */
/*    3 4 5 */
/*    6 7 8 */
/* with #4 the plot window itself. */
/* LS 25jun94 */
{
  int32_t	class;

  if (x < *(w[0]))
    class = 0;
  else if (x > *(w[1]))
    class = 2;
  else class = 1;
  if (y > *(w[3]))
    class += 6;
  else if (y >= *(w[2]))
    class += 3;
  return class;
}
 /*------------------------------------------------------------------------- */
#define NONE_IN		0
#define ALL_IN		1
#define FIRST_IN	2
#define SECOND_IN	3
#define PART_IN		4
int32_t clipToWindow(float *xo, float *yo, float x[2], float y[2], float **w)
/* restricts the line between (*xo, *yo) and (x[0], y[0]) to the part within 
 the window.  returns the coordinates of the moved points in 
 (x[0], y[0]) and, if necessary, (x[1], y[1]), and returns NONE_IN if no 
 part of the line lies within the window, FIRST_IN if (*xo, *yo) was 
 already in the window, SECOND_IN if (x[0], y[0]) was already in the 
 window, ALL_IN if both (*xo, *yo) and (x[0], y[0]) were inside the 
 window, and PART_IN if none of the two points was in the window, 
 but a part of the connecting line was.  The clipped line segment runs
 from (x[0], y[0]) to (x[1], y[1]).  the entry values of x[0] and y[0], 
 made finite if necessary, are stored in *xo and *yo on exit.  */
/* LS 25jun94 */
{
  float	dx, dy, *xx, *yy, savex, savey;
  int32_t	c1, c2;
  int32_t	result = NONE_IN;
  static char	action[9][9] = 
  { { 0, 0, 0, 0, 1, 2, 0, 2, 2 }, 
    { 0, 0, 0, 2, 1, 2, 2, 2, 2 }, 
    { 0, 0, 0, 2, 1, 0, 2, 2, 0 }, 
    { 0, 2, 2, 0, 1, 2, 0, 2, 2 }, 
    { 1, 1, 1, 1, 1, 1, 1, 1, 1 }, 
    { 2, 2, 0, 2, 1, 0, 2, 2, 0 }, 
    { 0, 2, 2, 0, 1, 2, 0, 0, 0 }, 
    { 2, 2, 2, 2, 1, 2, 0, 0, 0 }, 
    { 2, 2, 0, 2, 1, 0, 0, 0, 0 } };

				/* transform infinite to huge finite */
  if (*x > FLT_MAX)
    *x = PLOT_INFTY;
  else if (*x < -FLT_MAX)
    *x = -PLOT_INFTY;
  if (*y > FLT_MAX)
    *y = PLOT_INFTY;
  else if (*y < -FLT_MAX)
    *y = -PLOT_INFTY;
  dx = *x - *xo;
  if (dx == 0.0)
    dx = FLT_MIN;
  dy = *y - *yo;
  if (dy == 0.0)
    dy = FLT_MIN;
  savex = *x;
  savey = *y;
  c1 = positionClassWindow(*xo, *yo, w);
  c2 = positionClassWindow(*x, *y, w);
  switch (action[c2][c1])
  { case 0:			/* no part of the connecting line */
				/* lies in the plot window */
      result = NONE_IN;  break;
    case 1:			/* at most one of the end points lies */
				/* outside the plot window */
      /* first, see which point needs moving */
      x[1] = *x;
      y[1] = *y;
      x[0] = *xo;
      y[0] = *yo;
      if (c1 == 4 && c2 == 4)	/* both points already in plot window */
      { result = ALL_IN;  break; }
      if (c1 == 4)		/* (*xo, *yo) already in plot window */
      { xx = &x[1];  yy = &y[1]; }
      else			/* (*x, *y) already in plot window */
      { xx = x;  yy = y; }
      /* next, move the indicated point toward the plot window */
      if (*xx < *(w[0]))
      { *yy += dy*(*(w[0]) - *xx)/dx;
	*xx = *(w[0]); }
      else if (*xx > *(w[1]))
      { *yy += dy*(*(w[1]) - *xx)/dx;
	*xx = *(w[1]); }
      if (*yy < *(w[2]))
      { *xx += dx*(*(w[2]) - *yy)/dy;
	*yy = *(w[2]); }
      else if (*yy > *(w[3]))
      { *xx += dx*(*(w[3]) - *yy)/dy;
	*yy = *(w[3]); }
      result = (c1 == 4)? FIRST_IN: SECOND_IN;  break;
    case 2:			/* both end points lie outside the plot */
				/* window, but the line crosses the window */
      /* ensure proper direction of movement */
      if (*x > *xo || (*x == *xo && *y > *yo))
      { x[1] = x[0];
	y[1] = y[0];
	*x = *xo;
	*y = *yo;
	c1 = 0; }
      else
      { x[1] = *xo;
	y[1] = *yo;
	c1 = 1; }
      /* restrict lower-leftmost point */
      if (*x < *(w[0]))
      { *y += dy*(*(w[0]) - *x)/dx;
	*x = *(w[0]); }
      else if (*x > *(w[1]))
      { *y += dy*(*(w[1]) - *x)/dx;
	*x = *(w[1]); }
      if (*y < *(w[2]))
      { *x += dx*(*(w[2]) - *y)/dy;
	*y = *(w[2]); }
      else if (*y > *(w[3]))
      { *x += dx*(*(w[3]) - *y)/dy;
	*y = *(w[3]); }
      /* restrict upper-rightmost point */
      if (y[1] > *(w[3]))
      { x[1] += dx*(*(w[3]) - y[1])/dy;
	y[1] = *(w[3]); }
      else if (y[1] < *(w[2]))
      { x[1] += dx*(*(w[2]) - y[1])/dy;
	y[1] = *(w[2]); }
      if (x[1] > *(w[1]))
      { y[1] += dy*(*(w[1]) - x[1])/dx;
	x[1] = *(w[1]); }
      else if (x[1] < *(w[0]))
      { y[1] += dy*(*(w[0]) - x[1])/dx;
	x[1] = *(w[0]); }
      if (c1)			/* need to exchange places */
      { dx = x[1];
	x[1] = x[0];
	x[0] = dx;
	dy = y[1];
	y[1] = y[0];
	y[0] = dy; }
      result = PART_IN;  break; }
  *xo = savex;
  *yo = savey;
  return result;
}
 /*------------------------------------------------------------------------- */
int32_t oplotx(float x[], float y[], float ex[], float ey[], int32_t n, int32_t dx,
	   int32_t dy)
{
  float		xx[2], yy[2], xl, yl, xe, ye, xerrs, yerrs;
  int32_t	i;
  char	c, noclipbars;
  extern int32_t	fromCoordinateSystem, toCoordinateSystem, calligCoordSys;
  int32_t	coordMap(float *, float *), dashload(int32_t, int32_t *, float *, float);

  noclipbars = (internalMode & 256) == 0; /* no /CLIPBARS */
  ifirstflag = 1;		/* used by symplot if blanking enabled */
  oldLineStyle = 0;		/* force clean start for dashed lines */
  fromCoordinateSystem = (internalMode & 7);
  if (!fromCoordinateSystem)
    fromCoordinateSystem = LUX_PLT;
  toCoordinateSystem = calligCoordSys = tkCoordSys = LUX_DVI;
  /*  set_cur_pen(); */  /* interferes with lux_pencolor(). LS 1apr99 */
  /* get scale and first set of coordinates
    all plot points must lie between physical limits of plot */
  /* first transform to LUX_DVI coords (including logarithms if required) */
  /* and then clip all parts sticking beyond the plot window */
  *xx = x[0];
  *yy = y[0];
  coordMap(xx, yy);
  xl = *xx;
  yl = *yy;
  c = clipToWindow(&xl, &yl, xx, yy, plotWindow);
  if (symStyle != 20) {		/* not a bar graph */
    xerrs = 0.5*xerrsize*(wxt - wxb);
    yerrs = 0.5*yerrsize*(wyt - wyb);
    if (c != NONE_IN) {		/* the data point is in the plot window */
      tkplot(*xx, *yy, 0, 0);
    }
    for (i = 0; i < n; i++) {	/* loop over points */
      if (lumpx) {
	int32_t nlump = 1, lumpindex;
	float sumx, sumy;
	*xx = x[i];
	*yy = y[i];
	coordMap(xx, yy);
	lumpindex = *xx/lumpx;
	sumx = *xx;
	sumy = *yy;
	while (++i < n) {
	  *xx = x[i];
	  *yy = y[i];
	  coordMap(xx, yy);
	  if ((int32_t) (*xx/lumpx) == lumpindex) {
	    sumx += *xx;
	    sumy += *yy;
	    nlump++;
	  } else
	    break;
	}
	*xx = sumx/nlump;
	*yy = sumy/nlump;
	i--;
      } else {
	*xx = x[i];
	*yy = y[i];
	coordMap(xx, yy);
      }
      c = clipToWindow(&xl, &yl, xx, yy, plotWindow);
      switch (c) {
      case SECOND_IN: case PART_IN: /* need to move to 1st point */
	if (i < *qi)
	  tkplot(*xx, *yy, 0, 0); /* move there */
	/* then fall through to the drawing of the second point */
      case FIRST_IN: case ALL_IN: /* need to draw 2nd point */
	if (i < *qi && (!(internalMode & 128) || c != PART_IN)) {
	  /* /WHOLE is not set, so draw the partial line */
	  tkplot(xx[1], yy[1], lineStyle,
		 (c == PART_IN || c == FIRST_IN)? 0: symStyle);
	  if (ex && (c == ALL_IN || c == SECOND_IN)) {
	    xe = x[i] - ex[i];	/* go left first */
	    coordMap(&xe, NULL);
	    if (noclipbars || xe >= wxb) {
	      tkplot(xe, yl - xerrs, 0, 0); /* left vertical bar */
	      tkplot(xe, yl + xerrs, 1, 0);
	      tkplot(xe, yl, 0, 0);
	    } else
	      tkplot(wxb, yl, 0, 0);
	    xe = x[i] + ex[i + dy];	/* then go right */
	    coordMap(&xe, NULL);
	    if (noclipbars || xe <= wxt) {
	      tkplot(xe, yl, 1, 0);	/* draw the horizontal bar */
	      tkplot(xe, yl - xerrs, 0, 0); /* the right vertical bar */
	      tkplot(xe, yl + xerrs, 1, 0);
	    } else
	      tkplot(wxt, yl, 1, 0);
	    tkplot(*xx, *yy, 0, 0);	/* and then return to the data point */
	  }
	  if (ey && (c == ALL_IN || c == SECOND_IN)) { /* draw error bar */
	    ye = y[i] + ey[i];
	    coordMap(NULL, &ye);
	    if (noclipbars || ye <= wyt) {
	      tkplot(xl - yerrs, ye, 0, 0); /* the top horizontal bar */
	      tkplot(xl + yerrs, ye, 1, 0);
	      tkplot(xl, ye, 0, 0);
	    } else
	      tkplot(xl, wyt, 0, 0);
	    ye = y[i] - ey[i + dy];	/* then go down */
	    coordMap(NULL, &ye);
	    if (noclipbars || ye >= wyb) {
	      tkplot(xl, ye, 1, 0);
	      tkplot(xl - yerrs, ye, 0, 0); /* bottom horizontal bar */
	      tkplot(xl + yerrs, ye, 1, 0);
	    } else
	      tkplot(xl, wyb, 1, 0);
	    tkplot(xx[1], yy[1], 0, 0); /* and then return to data point */
	  }
	} else {		/* plot break:  move pen to next position */
	  tkplot(xx[1], yy[1], 0, symStyle);
	  if (nbreak > 1) {
	    nbreak--;
	    qi++;
	  } else if (nbreak == 1) {
	    nbreak = 0;
	    qi = &huge;
	  }	/* end of if (nbreak > 1) */
	} /* end of if (i < *qi && ...) */
      } /* end of switch (c) */
    }	/* end of for (i = 0; i < n; i++) */
  } else {				/* bar-type plot */
    if (c != NONE_IN)
      tkplot(*xx, *yy, 0, 0);
    if (n > 1) {
      for (i = 1; i < n; i++) {	/* loop over points */
	/* first move x */
	*xx = x[i];
	*yy = y[i - 1];
	coordMap(xx, yy);
	c = clipToWindow(&xl, &yl, xx, yy, plotWindow);
	switch (c) {
	  case SECOND_IN: case PART_IN:
	    tkplot(*xx, *yy, 0, 0);
	  case FIRST_IN: case ALL_IN:
	    tkplot(xx[1], yy[1], lineStyle, 0);
	    break;
	  }
	*xx = x[i];
	*yy = y[i];
	coordMap(xx, yy);
	c = clipToWindow(&xl, &yl, xx, yy, plotWindow);
	switch (c) {
	  case SECOND_IN: case PART_IN:
	    tkplot(*xx, *yy, 0, 0);
	  case FIRST_IN: case ALL_IN:
	    tkplot(xx[1], yy[1], lineStyle, 0);
	    break;
	} /* end of switch (c) */
      }	/* end of for (i = 1; i < n; i++) */
    } /* end of if (n > 1) */
    /* now draw the last horizontal bit */
    *xx = x[n - 1] + 1;
    *yy = y[n - 1];
    coordMap(xx, yy);
    c = clipToWindow(&xl, &yl, xx, yy, plotWindow);
    switch (c) {
      case SECOND_IN: case PART_IN:
	tkplot(*xx, *yy, 0, 0);
      case FIRST_IN: case ALL_IN:
	tkplot(xx[1], yy[1], lineStyle, 0);
	break;
    }
  }
  return 1;
}
 /*------------------------------------------------------------------------- */
int32_t dashload(int32_t id, int32_t *ndash, float *sm, float dashsize)
{
 static	int32_t	nds[] = {2, 2, 2, 4,  4,  2,  6,  6,  8,  6};
 static	int32_t	ip[]  = {0, 2, 4, 6, 10, 14, 16, 22, 28, 36};
 static	float	xlens[] = {
   .005, .005,			/* style 10: short-dashed */
   .010, .010,			/* style 11: medium-dashed */
   .020, .020,			/* style 12: long-dashed */
   .010, .005, .002, .005,	/* style 13: dot-short-dashed */
   .020, .005, .005, .005,	/* style 14: dot-medium-dashed */
   .002, .010,			/* style 15: dotted */
   .010, .005, .002, .005, .002, .005, /* style 16: dot-dot-dashed */
   .010, .005, .010, .005, .002, .005, /* style 17: dot-dash-dashed */
   .010, .005, .010, .005, .002, .005, .002, .005, /* 18: dot-dot-dash-dash */
   .005, .005, .010, .005, .020, .005 /* 19: short-medium-dash */
 };
 int32_t	i;

 *ndash = nds[id];
 for (i = 0; i < nds[id]; i++)
   *sm++ = dashsize*xlens[i + ip[id]];
 return 1;
}
 /*------------------------------------------------------------------------- */
int32_t setl(float *ymax, float *ymin, int32_t *nq, float *dv, int32_t ilog, float wyb, 
	 float wyt)
/*
 *ymin: minimum y value on input, minimum plot limit on output
 *ymax: maximum y value on input, maximum plot limit on output
 *nq: number of axis divisions on output
 *dv: DVI per axis division on output
 ilog: flags log axis
 wyb: DVI lower plot limit
 wyt: DVI upper plot limit
*/
				 /* set limits, used for "nice" plot limits */
{
 float	yn, yx, xq, yq, zq, rdx, xt, xb;
 int32_t	ilg, j1, j2;

 /* if ilog == 1 then log scaling done */
 if (ilog != 0) {
   /* 0's not allowed, but want small #'s */
   if (*ymin <= 0.0)
     *ymin = 1.e-5;
   if (*ymax <= 0.0)
     *ymax = 1.e-4;
   yn = log10((double) *ymin);
   /* negative YN's need to be bumped down 1 unless YMIN was an */
   /* exact power of 10 */
   if (yn < 0)
     if (floor(yn) > (log10((double) *ymin) + 1.e-7))
       yn = yn - 1;
   yn = pow(10, floor(yn));
   yx = pow(10, floor((log10((double) *ymax) + 1)));
   xq = log10(yx/yn);
   *nq = xq;
   /* Note, NQ is 1 cycle too big if YMAX is exactly a power of 10 */
   if (log10((double) *ymax) == floor(log10((double) *ymax)))
     (*nq)--; 
   if (*nq <= 0) {
     printf("internal error in setl,  plot may be very strange\n");
     *nq = 1;
   }
   *ymin = yn;
   *ymax = yn*pow(10, (double) *nq);
   *dv = (wyt - wyb)/ *nq;
 } else { 					/* linear case */
   xq = *ymax - *ymin;
   if (xq <= 0)
     xq = 2;
   ilg = log10(xq);
   if (xq <= 1)
     ilg--;
   yq = pow(10, (double) ilg);
   zq = xq/yq;
   rdx = 1;
   if (zq <= 6)
     rdx = .5;
   if (zq <= 2)
     rdx = .2;
   if (zq <= 1)
     rdx = .1;
   xq = yq*rdx;
   *dv = xq;
   j2 = *ymax/xq;
   j1 = *ymin/xq;
   xt = j2*xq;
   xb = j1*xq;
   if (xt < *ymax)
     j2++;
   if (xb > *ymin)
     j1--;
						 /* j1 should be even */
   if (ABS(j1) % 2 == 1)
     j1--;
   *nq = ABS(j2 - j1);
   if (*nq % 2 == 1)
     j2++;
   *ymax = xq*j2;
   *ymin = xq*j1;
   *nq = ABS(j2 - j1);
 }
 return 1;
}
/*------------------------------------------------------------------------- */
void setupticks(float *min, float *max, int32_t ndlab, int32_t fstep, int32_t ilog,
		int32_t irf, int32_t f, float wb, float wt, float plim_min,
		float plim_max, float *start, float *step, float *end,
		float *stepdvi)
/* *min: INPUT: least value in data; OUTPUT: least plot data limit */
/* *max: INPUT: greatest value in data; OUTPUT: greatest plot data limit */
/* ndlab: INPUT: number of minor divisions per major division */
/* fstep: INPUT: free step size (0 = automatic, 1 = take from <step>) */
/* ilog: INPUT: flags logarithmic scale */
/* irf: INPUT: plot limit rounding (1 = yes, 0 = no) */
/* f: INPUT: free lower limit; if == 0, then lower limit is non-positive */
/* wb: INPUT: least window limit (dvi) */
/* wt: INPUT: greatest window limit (dvi) */
/* plim_min: INPUT: explicit plot data minimum */
/* plim_max: INPUT: explicit plot data maximum */
/* *start: OUTPUT: data value corresponding to first label */
/* *step: INPUT/OUTPUT: data increment per major division (if ilog == 0) */
/* *end: OUTPUT: data value at or beyond the last label */
/* *stepdvi: OUTPUT: coordinate increment (dvi) per major division */
/* it is assumed that *max != *min, but we may have *max < *min. */
/* if <ilog> is set, then it is assumed that *min, *max > 0 */
{
  float	xq, stepvalue;

  if (ilog) {			/* logarithmic scale */
    /* we assume that *min > 0 */
    if (plim_min && plim_min > 0) {
      if (*min < plim_min)
	*min = plim_min;
      if (*max < plim_min)
	*max = plim_min;
    }
    if (plim_max && plim_max > 0) {
      if (*min > plim_max)
	*min = plim_max;
      if (*max > plim_max)
	*max = plim_max;
    }
    if (*max == *min)
      *max = *min + 1;
    *max = log10(*max);
    *min = log10(*min);
    if (irf) {
      xq = *max - *min;
      if (xq < 1.05) {
	xq = 0.5*(1.05 - xq);
	if (!plim_max)
	  *max += xq;
	if (!plim_min && f)
	  *min -= xq;
      } else {
	if (!plim_max)
	  *max += xq*0.05;
	if (!plim_min && f)
	  *min -= xq*0.05;
      }
    }
    xq = ABS(*max - *min);
    if (xq < 1) {		/* we want at least a ratio of 10 */
      *max = *min + 1;
      xq = 1.0;
    }
    *start = ceil(*min);
    *step = 1.0;
    *end = *max;
    *stepdvi = (wt - wb)/xq;
  } else {			/* linear scale */
    if (*min >= 0.0 && !f)
      *min = 0.0;
    if (plim_min) {
      if (*min < plim_min)
	*min = plim_min;
      if (*max < plim_min)
	*max = plim_min;
    }
    if (plim_max) {
      if (*min > plim_max)
	*min = plim_max;
      if (*max > plim_max)
	*max = plim_max;
    }
    if (*max == *min)
      *max = *min + 1;
    if (irf) {			/* widen the range a bit so the data does */
				/* not extend right to the plot's edge */
      xq = *max - *min;
      if (!plim_max)
	*max += xq*0.05;
      if (!plim_min && (*min < 0.0 || f))
	*min -= xq*0.05;
    }
    xq = ABS(*max - *min);
    if (fstep && *step)
      stepvalue = *step;
    else {
      stepvalue = pow(10,round(log10(xq) - 0.1))*0.1;
      xq /= stepvalue;
      stepvalue *= 2*((xq < 20 && xq > 8)) + 5*(xq >= 20) + (xq <= 8);
      *step = stepvalue;
    }
    if (stepvalue > 0) {
      *start = irf? ceil(*min/stepvalue)*stepvalue: *min;
      *end = *max;
    } else {
      *start = irf? ceil(*max/stepvalue)*stepvalue: *max;
      *end = *min;
    }
    *stepdvi = (wt - wb)/(*max - *min);
    if (stepvalue < 0) {
      *stepdvi = -*stepdvi;
      stepvalue = *min;		/* switch *min, *max */
      *min = *max;
      *max = stepvalue;
    }
  }
  if (*start == 0.0)
    *start = 0.0;		/* change -0 into regular 0 */
  if (*end == 0.0)
    *end = 0.0;
}
/*------------------------------------------------------------------------- */
int32_t tkplot(float x, float y, int32_t lineStyle, int32_t symStyle)
/* plots a line segment using style <lineStyle> and puts a symbol */
/* at the end of type <symStyle>.  Both of these arguments are */
/* nonnegative numbers.  LS 4feb95 */
/* Fixed lacking update of xLast, yLast in some cases.  LS 20jul2000 */
{
  static int32_t	ndash, depth = 0, id;
  static float	dashes[10], xLast = -0.0001, yLast = -0.0001;
  static char	penState;
  static float	s;
  float	dx, dy, s0, sd, xc, yc, xx[2], yy[2];
  int32_t	result, ix, iy;
  int32_t	symplot(float, float, int32_t, int32_t), 
  	postvec(float, float, int32_t);
#if HAVE_LIBX11
  int32_t	xwindow_plot(int32_t, int32_t, int32_t);
#endif

 /* use current context to convert to device dependent coords (DD) and
	 call routine for current device */
 /* if drawing a dashed line, then there is no need to transform coordinates */
 /* and clip the results for each individual dash.  just do that once, for */
 /* the whole line, and thereafter each dash is guaranteed to be visible */

  if (symStyle == 20)
    symStyle = 0;
  if (symStyle > 1) {
    depth++;
    result = symplot(x, y,symStyle, lineStyle);
    depth--;
    if (callig_update) {
      callig_xb = x;
      callig_yb = y;
    }
    xLast = x;			/* added LS 20jul2000 */
    yLast = y;
    return result;
  }
  if (!depth) {		     /* don't do this in recursively called tkplot */
    coordTrf(&x, &y, tkCoordSys, LUX_DVI);
#ifdef DEVELOP
    if (useProjection || projectTk) {
      int32_t	project(float, float, float);
      extern float	projected[];
      
      project(x, y, 0);
      x = projected[0];
      y = projected[1];
    }
#endif
    *xx = x;  *yy = y;
    ix = clipToWindow(&xLast, &yLast, xx, yy, screenWindow);
    depth++;
    switch (ix) {
      case SECOND_IN: case PART_IN: /* move to first point */
	tkplot(*xx, *yy, 0, 0);
	break;
      case NONE_IN:		/* just quit */
	depth--;  
	if (callig_update) {
	  callig_xb = x;
	  callig_yb = y;
	}
	return 1;
    }
  } else {
    xx[1] = x;
    yy[1] = y;
    xx[0] = xLast; 
    yy[0] = yLast;
    depth++;
  }
  if (lineStyle >= 10) {	/* dashed line */
    if (lineStyle != oldLineStyle) {
      dashload(lineStyle - 10, &ndash, dashes, theDashSize);
      oldLineStyle = lineStyle; 
      id = 0;
      s = 0;
      penState = 1;
    }
    dx = xx[1] - xx[0];
    dy = yy[1] - yy[0];
    sd = sqrt(dx*dx+dy*dy);
    dx = dx/sd; 
    dy = dy/sd;
    xc = xx[0];
    yc = yy[0];
    if (s > 0) {		/* some left from last time */
      s0 = s;
      s = 0;
    } else
      s0 = dashes[id];
    while ((s += s0) < sd) {	/* draw dashes */
      xc += s0*dx;
      yc += s0*dy;
      tkplot(xc, yc, penState, 0);
      id = (id+1) % ndash;
      s0 = dashes[id];
      penState = !penState;
    }
    tkplot(x, y, penState, 0);	/* finish last dash */
    s -= sd;
    if (depth)
      depth--;
    return 1;
  }
  if (lineStyle == 0 && symStyle == 1) {
    tkplot(xx[1], yy[1], 0, 0);	/* move to end position */
    lineStyle = 1; 		/* so dot gets plotted */
  }
  if (depth)
    depth--;
  if (callig_update) {
    callig_xb = x;
    callig_yb = y;
  }
  if (updateBoundingBox == 1
      && (alternateDash || lineStyle != 0 || symStyle != 0)) {
    /* update bounding box */
    if (xx[1] < postXBot)
      postXBot = xx[1];
    else if (xx[1] > postXTop)
      postXTop = xx[1];
    if (yy[1] < postYBot)
      postYBot = yy[1];
    else if (yy[1] > postYTop)
      postYTop = y; 
  }
  switch (lunplt) {
    case 0:			/* to screen */
#if HAVE_LIBX11
      ix = (int32_t) (xx[1]*xfac);
      iy = iyhigh - (int32_t) (yy[1]*yfac);
      xwindow_plot(ix, iy, lineStyle);
#else
      return cerror(NO_X11, 0);
#endif
      break;
    case 1:			/* to PostScript file */
      postvec(xx[1], yy[1], lineStyle);
      break;
  }
  return 1;
}
 /*------------------------------------------------------------------------*/
int32_t lux_pen(int32_t narg, int32_t ps[])
/* PEN, width, currentgray */
/* sets or displays the pen width and color */
{
  if (narg) {
    current_pen = int_arg( ps[0]);
    if (narg > 1)
      current_gray = float_arg(ps[1]);
    standardGray = internalMode & 1;
    set_cur_pen();
  } else
    printf("Current pen: width = %d; grey level = %f\n", current_pen, 
	   current_gray);
  return 1;
}
/*------------------------------------------------------------------------*/
int32_t lux_pencolor(int32_t narg, int32_t ps[])
/* sets or displays the current pen color. */
{
  int32_t	iq, nx, xflag, n, nred, ngreen, nblue;
  uint32_t	ired, igreen, iblue;
  static float	red, green, blue;
  char	*pc = NULL;
  float	*pf;
#if HAVE_LIBX11
  int32_t	getXcolor(char *colorname, XColor *color, int32_t alloc);
  Status	anaAllocNamedColor(char *, XColor **);
  extern int32_t	connect_flag;
#endif
  int32_t	postcolorpen(float red, float green, float blue);
#if HAVE_LIBX11
  XColor	color;
#endif
  
  if (lunplt == 0) {
#if HAVE_LIBX11
    if (setup_x() == LUX_ERROR)
      return LUX_ERROR;
#endif
    xflag = 1;
  } else
    xflag =0;
  if (narg) {
    switch (symbol_class(ps[0])) {
      case LUX_STRING:
	pc = string_value(ps[0]);
#if HAVE_LIBX11
	if (connect_flag) {
	  /* get rgb values for this color name, if for X, also set */
	  /* foreground */
	  if (getXcolor(pc, &color, xflag) != 1)
	    return LUX_ERROR;
	  red = color.red;
	  green = color.green;
	  blue = color.blue;
	  break;
	}
#endif
	if (!strncasecmp(pc, "rgbi:", 5)) { /* an RGBI specification */
	  if (sscanf(pc + 5, "%f/%f/%f", &red, &green, &blue) != 3)
	    return luxerror("Unrecognized color specification", ps[0]);
	  if (red < 0 || red > 1 || green < 0 || green > 1 || blue < 0
	      || blue > 1)
	    return luxerror("Illegal color specification", ps[0]);
	} else if (!strncasecmp(pc, "rgb:", 4)) { /* an RGB specification */
	  if (sscanf(pc + 4, "%x%n/%x%n/%x%n", &ired, &nred, &igreen, &ngreen,
		     &iblue, &nblue) != 3
	      || nred < 1 || ngreen - nred < 2 || nblue - ngreen < 2
	      || nred > 4 || ngreen - nred > 5 || nblue - ngreen > 5)
	    return luxerror("Unrecognized color specification", ps[0]);
	  red = (ired << ((4 - nred)*4))/65535.0;
	  green = (igreen << ((5 + nred - ngreen)*4))/65535.0;
	  blue = (iblue << ((5 + ngreen - nblue)*4))/65535.0;
	} else
	  return luxerror("Unrecognized color specification", ps[0]);
	break;
      case LUX_ARRAY:
	iq = lux_float(1, ps);	/* ensure that it is FLOAT */
	nx = array_dims(iq)[0];
	if (nx < 3)
	  return luxerror("PENCOLOR requires a string naming a color\nor an array of 3 RGB values between 0.0 and 1.0", ps[0]);
	pf = array_data(iq);
	/* check first */
	n = 3;
	while (n--) {
	  if (*pf > 1.0 || *pf < 0.0)
	    return luxerror("PENCOLOR requires a string naming a color\nor an array of 3 RGB values between 0.0 and 1.0", ps[0]);
	 pf++;
	}
	/* now set the values */
	pf = array_data(iq);
	red = *pf++;
	green = *pf++;
	blue = *pf++;
	break;
    }
    if (lunplt == 1)
      return postcolorpen(red, green, blue);
  } else {
    printf("current pen color setting (RGB) = %f %f %f\n", red, green, blue);
  }
  return 1;
}
/*------------------------------------------------------------------------*/
void set_cur_pen(void)
/* set pen according to current_pen and current_gray */
{
#if HAVE_LIBX11
  int32_t lux_xpen(int32_t, float);
#endif
  int32_t postpen(int32_t, float);
  
  switch (lunplt) {
#if HAVE_LIBX11 
    case 0:
      lux_xpen(current_pen, current_gray);
      return;
#endif
    default:
      postpen(current_pen, current_gray);
  }
}
 /*------------------------------------------------------------------------*/
int32_t set_pen(int32_t pen)
{
#if HAVE_LIBX11
  int32_t lux_xpen(int32_t, float);
#endif
  int32_t postpen(int32_t, float);
  
  switch (lunplt) {
    case 0:
#if HAVE_LIBX11 
      return lux_xpen(pen, current_gray);
#else
      return cerror(NO_X11, 0);
#endif
    case 1:
      return postpen(pen, current_gray);
  }
  return luxerror("Illegal plot device (%1d)!", 0, lunplt);
}
 /*------------------------------------------------------------------------*/
int32_t lux_erase(int32_t narg, int32_t ps[])
{
  int32_t	postcopy(void), toscreen;
#if HAVE_LIBX11
  int32_t	lux_xerase(int32_t, int32_t []);
#endif

  toscreen = (internalMode & 192);
  switch (toscreen) {
     case 64:			/* /SCREEN: display on screen */
      toscreen = 1;
      break;
    case 128:			/* /POSTSCRIPT: send to postscript file */
      toscreen = 0;
      break;
    default:    		/* default: according to !PDEV value */
      toscreen = (lunplt? 0: 1);
      break;
  }

  /* reset bounding box */
  postXBot = FLT_MAX;
  postXTop = -FLT_MAX;
  postYBot = FLT_MAX; 
  postYTop = -FLT_MAX;
  switch (lunplt) 
  { case 0:
#if HAVE_LIBX11
      return lux_xerase(narg, ps);
#else
      return cerror(NO_X11, *ps);
#endif
    case 1:
      return postcopy();
    }
  return luxerror("Illegal plot device (%1d)!", 0, lunplt);
}
 /*------------------------------------------------------------------------- */
int32_t lux_limits(int32_t narg, int32_t ps[]) /*set or examine limits */
{
  if (narg == 0)
  { printf("current plot range x axis %f to %f\n", plims[0], plims[1]);
    printf("                   y axis %f to %f\n", plims[2], plims[3]);
    printf("                   z axis %f to %f\n", plims[4], plims[5]);
    return 1; }
  if (narg > 0)
    plims[0] = float_arg(ps[0]);
  if (narg > 1)
    plims[1] = float_arg(ps[1]);
  if (narg > 2)
    plims[2] = float_arg(ps[2]);
  if (narg > 3)
    plims[3] = float_arg(ps[3]);
  if (narg > 4)
    plims[4] = float_arg(ps[4]);
  if (narg > 5)
    plims[5] = float_arg(ps[5]);
 return 1;
}
 /*------------------------------------------------------------------------- */
int32_t lux_window(int32_t narg, int32_t ps[]) /*set or examine window */
{
  float	tmp;
#ifdef DEVELOP
  extern float	wzb, wzt;
#endif

  if (narg == 0)
  { printf("current window x axis %f to %f\n", wxb, wxt);
    printf("               y axis %f to %f\n", wyb, wyt);
#ifdef DEVELOP
    printf("              (z axis %f to %f)\n", wzb, wzt);
#endif
    return 1; }
  if (narg > 0) 
    wxb = float_arg(ps[0]);
  if (narg > 1)
    wxt = float_arg(ps[1]);
  if (narg > 2) 
    wyb = float_arg(ps[2]);
  if (narg > 3) 
    wyt = float_arg(ps[3]);
#ifdef DEVELOP
  if (narg > 4)
    wzb = float_arg(ps[4]);
  if (narg > 5)
    wzt = float_arg(ps[5]);
#endif
  if (wxb > wxt)
  { tmp = wxb;
    wxb = wxt;
    wxt = tmp; }
  if (wyb > wyt)
  { tmp = wyb;
    wyb = wyt;
    wyt = tmp; }
#ifdef DEVELOP
  if (wzb > wzt)
  { tmp = wzb; 
    wzb = wzt;
    wzt = tmp; }
#endif
  return 1;
 }
 /*------------------------------------------------------------------------- */
int32_t lux_pdev(int32_t narg, int32_t ps[])
{
  int32_t	postreset(int32_t);
  extern float	postXBot, postYBot, postXTop, postYTop;

  if (narg > 0) {
    lunplt = int_arg( ps[0]);
    /* we reset the bounding box */
    postXBot = postYBot = FLT_MAX;
    postXTop = postYTop = -FLT_MAX;
  } else {
    printf(" 0   X window terminal\n 1   postscript file (%s)\n", 
	   (landscape)? "landscape": "portrait");
    printf("current setting = %d\n", lunplt);
  }
  return 1;
}
 /*------------------------------------------------------------------------- */
int32_t sform(float xb, float xt)
				 /* make a nice format for plot labels */
{
 /* set default form */
  int32_t	ie, il;
  float	xq, xc;

  xc = MAX(ABS(xb), ABS(xt));	
  sprintf(form, "%s", "%8.1e");	/*default*/
  if (xc >= 1.e6 || xc < 1.e-5)
    return 1;
				 /* other cases (more common actually) */
  sprintf(form, "%s", "%8.0f");
  xq = (xt - xb)/6;
  if (xq <= 0)
  { printf("equal limits in sform, xb, xt = %e, %e\n", xb, xt);
    return -1; }
  if (xq < 1)
  { xq = -log10(xq);
    ie = xq;
    if (xq > ie)
      ie++;
					 /* how many digits on lhs ? */
    xq = log10(xc);
    il = (int32_t) xq;
    if (xq > il)
      il++;
    il = MAX(il, 0);
				 /* we have a max of 7 available */
    ie = MIN(ie, 7 - il);
    form[3] = (char) (ie + 48); }
  return 1;
}
 /*------------------------------------------------------------------------- */
int32_t empty(void)
 /* flush the plot channel (whatever it is) */
{
  return 1;
}
 /*------------------------------------------------------------------------- */
int32_t symplot(float x, float y, int32_t symStyle, int32_t lineStyle)
{
  static int32_t	is[] =
  { 1, 5,9, 14, 18, 23, 27, 35, 76, 76};
  static float	poly[] =
  { 0.0085, .01, 0,0, 0,0, 0.01, 0.0075, 0,0 };
  static uint8_t	mode[] = 
  { 0, 1,0, 1,0, 1,0, 1,0, 1,1, 1,1, 0,1, 1,1, 0,1, 1,1, 1,0, 1,1, 1,0, 1,0, 1,0, 1,0, 
      1, 0,1, 1,1, 1,1, 1,1, 1,1, 1,1, 1,1, 1,1, 1,1, 1,1, 1,1, 1,1, 1,1, 1,1, 1,1, 1,
      1, 1,1, 1,1, 1,1, 1,1, 1};
  static float	dx[] =
  {0, 0,.005, -.005, .005, -.005, -.005, .005, .005, .005, -.005, 
     -.005, .005, -.007, 0,.007, -.007, -.007, 0,.007, 0,-.007, 
     -.007, 0,.007, -.007, 0,0, .005, -.005, -.007, .007, -.005, .005, 
     0.00000, 0.00110, 0.00216, 0.00318, 0.00411, 0.00495, 0.00566, 0.00624, 
     0.00666, 0.00691, 0.00700, 0.00691, 0.00666, 0.00624, 0.00566, 0.00495, 
     0.00411, 0.00318, 0.00216, 0.00110, 0.00000, -.00110, -.00216, -.00318, 
     -.00411, -.00495, -.00566, -.00624, -.00666, -.00691, -.00700, -.00691, 
     -.00666, -.00624, -.00566, -.00495, -.00411, -.00318, -.00216, -.00110, 
     0 };
  static float	dy[] =
  {.005, -.005, 0,0, .005, -.005, .005, -.005, .005, -.005, -.005, 
     .005, .005, -.005, .007, -.005, -.005, 0,.007, 0,-.007, 0,
     .005, -.007, .005, .005, .007, -.007, .005, -.005, 0,0, .005, -.005, 
     0.00700, 0.00691, 0.00666, 0.00624, 0.00566, 0.00495, 0.00411, 0.00318, 
     0.00216, 0.00110, 0.00000, -.00110, -.00216, -.00318, -.00411, -.00495, 
     -.00566, -.00624, -.00666, -.00691, -.00700, -.00691, -.00666, -.00624, 
     -.00566, -.00495, -.00411, -.00318, -.00216, -.00110, 0.00000, 0.00110, 
     0.00216, 0.00318, 0.00411, 0.00495, 0.00566, 0.00624, 0.00666, 0.00691, 
     .007 };
  static float	xp, yp;
  int32_t	nsym = 8, ns, nsm, ia, ib, icept, i, iq;
  float	tol = 1.e-5, delx, xq, yq, zq, xq2, yq2, slope, x2, y2, sq, pr;
  float	x1, y1;
  char	thing[2];
 
  xp = callig_xb;
  yp = callig_yb; /* addition LS 7oct93 */
  coordTrf(&x, &y, tkCoordSys, LUX_DVI); /* transform to LUX_DVI  LS 28jul94 */
  ns = symStyle;
  if (symStyle == 0)         /* the -1 dot case or an error */
  { tkplot(x, y, 0, 0);
    tkplot(x, y, 1, 0);
    return 1; }
  nsm = ns - 1;
  if (lineStyle)                 /* draw line and symbol, first the line */
  { if (iblank == 0)
      tkplot(x, y, lineStyle, 0); /* easy if no blanking */
    else                                 /* blanking case */
    { if (ifirstflag != 1) 
      { if (nsm <= nsym || ns > 20)
	{ if (nsm <= nsym)
	    pr = poly[nsm-1]*symsize;
	  else pr = symsize*0.01;
	 /*  we find the two endpoints of the line, 2 special cases are
	     infinite slope and zero slope */
	  delx = x - xp;
	  if (delx)
	    slope = (y - yp)/delx;
	  if (pr > 0)         /* elliptical zone case */
         { if (delx == 0)
           { xq = xp;
	     xq2 = x;
	     zq = pr*symratio;
             if (y > yp)
	     { yq = yp + zq;
	       yq2 = y - zq; }
	     else
	     { yq = yp - zq;
	       yq2 = y + zq;}
           }
	   else
           { zq = pr/sqrt((slope*slope)/(symratio*symratio) + 1.0);
             if (x > xp)
	     { xq = xp + zq;
	       xq2 = x - zq; }
	     else
	     { xq = xp - zq;
	       xq2 = x + zq; }
  	     zq = ABS(zq*slope);
	     if (y > yp)
	     { yq = yp + zq;
	       yq2 = y - zq; }
	     else 
	     { yq = yp - zq;
	       yq2 = y + zq;}
	   }
         }
	 else				/* a polygon symbol, more work */
	 { 
	   ia = is[nsm - 1];
	   ib = is[ns - 1] - 1;
		 /* we assume that this is a connected series of lines */  
           icept = 0;
           for (i = ia; i < ib; i++) 
			 /* compute intercepts and exit after finding both */
           { x1 = dx[i - 1]*symsize;
	     x2 = dx[i]*symsize;
             y1 = dy[i - 1]*symsize;
	     y2 = dy[i]*symsize;
	     sq = x2 - x1;
             if (sq == 0) 			/* infinite slope case */
	     { if (delx == 0)
		 xq = 1.0e20;
	       else
		 xq = x1; }
             else
             { sq = (y2 - y1)/sq;
               if (sq == 0) 
	       { if (delx == 0)
		 { xq = 0.0;
		   yq = y1; }
	         else
		 { if (slope == 0)
		     xq = 1.0e20;
		   else
		     xq = y1/slope; }
               }
               else				/* not a special case for sq */
               { if (delx == 0)
		 { xq = 0;
		   yq = y1 - sq*x1; }
	         else 
                 { zq = slope - sq;
                   if (zq)
		     xq = (y1 - sq*x1)/zq;
		   else
		     xq = 1.0e20;}
               }
             }
				 /* now check out this "intercept" */
             if ((xq - MAX(x1, x2)) < tol
		 && (MIN(x1, x2) - xq) < tol )
	       /* at least x checks out, now compute YQ */
             { if (delx)
		 yq = slope*xq; 
               if ((yq - MAX(y1, y2)) < tol
		   && (MIN(y1, y2) - yq) < tol)
			 /* we have an intercept, still some more work */
               { if (icept)	/* second one, must be opposite sides */
	         { if ((xq >= 0 && xq2 <= 0) || (xq <= 0 && xq2 >= 0))
	           if ((yq >= 0 && yq2 <= 0) || (yq <= 0 && yq2 >= 0))
                   { icept = 2; }
                   x1 = MIN(xq, xq2);
		   x2 = MAX(xq, xq2);
                   y1 = MIN(yq, yq2);
		   y2 = MAX(yq, yq2);
                   if (x > xp)
		   { xq = xp + x2;
		     xq2 = x + x1; }
		   else
		   { xq2 = x + x2;
		     xq = xp + x1; }
                   if (y > yp)
		   { yq = yp + y2;
		     yq2 = y+ y1; }
		   else
		   { yq2 = y + y2;
		     yq = yp + y1; }
                   break; }
                 icept = 1;		/* must have been first one */
                 xq2 = xq;
		 yq2 = yq; }
             }
           }				/* end of for loop */
           if (icept != 2)
           { printf("internal error in SYMPLOT, icept = %d\n", icept);
             return -1; }
         }  
     /*	ready to plot the line
	but we need to check if these points are really within the line
	(i.e., the symbols could be bigger than the spacing) */
	  if (ABS(xq - xp) <= ABS(xq - x)
	      && ABS(xq2 - x) <= ABS(xq2 - xp))
	    if (ABS(yq - yp) <= ABS(yq - y)
		&& ABS(yq2 - y) <= ABS(yq2 - yp))
	    { tkplot(xq, yq, 0, 0);
	      tkplot(xq2, yq2, lineStyle, 0); }
	}
      }
      xp = x;	yp = y; }
  }
					 /* draw the symbol if in range */
  if (nsm <= nsym) 
  { 
    ia = is[nsm - 1] - 1; 
    ib = is[ns - 1] - 1;
    for (i = ia; i < ib; i++)
    { xq = dx[i]*symsize;
      yq = dy[i]*symsize;
      iq = mode[i];
      tkplot(xq + x, yq + y, iq, 0); }
  }
  else				/* could be a number thing */
  { if (ns > 20 && ns < 30)
    { thing[0] = (char) (ns + 48);
      thing[1] = 0;
      if (callig(thing, x - 0.005, y - 0.005, fsized, 0.0, 3, 1) < 0)
	return LUX_ERROR;
    }
  }
  ifirstflag = 0;
 /*	always go back to center, put a dot there if
	 !dot is on and it is a closed polygon */
  iq = 0;
  if (ndot == 1 && (poly[ns - 2] == 0 || ns == 9))
    iq = -1;
  tkplot(x, y, 0, 1);
  return 1;
}
 /*------------------------------------------------------------------------- */
int32_t lux_xymov(int32_t narg, int32_t ps[])			/* xymov routine */
 /* subroutine, call is xymov(x, y, [mode, BREAKS=breaks], /boundingbox) */
{
 /* the x, y, and mode arguments can be scalars or vectors in various
	 combinations, if no mode, then assume a pen up to first point
	 and then pen down */
  int32_t	mode, nx, ny, nm, *mp, dx, dy, dm, iq, line, one = 1, nc,
    altDash, i;
  int32_t	fixPlotStyle(int32_t *, int32_t *);
  float	*x, *y, xc, yc;
  extern int32_t	tkCoordSys, ifirstflag;
  char	moveFirst;

  mode = LUX_UNSPECIFIED;
  mp = &mode;
  dm = 0;
  nm = 1;
  theDashSize = dashsize;

		 /* x, could be scalar or array */
  iq = ps[0];
  ifirstflag = 1;
  switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case LUX_SCALAR:
      nx = 1;
      iq = lux_float(1, &iq); 
      dx = 0;
      x = &scalar_value(iq).f;
      break;
    case LUX_ARRAY:
      iq = lux_float(1, &iq);
      nelem = array_size(iq);
      nd = array_num_dims(iq);
      nx = nelem;
      if (nx)
	dx = 1;
      else
	dx = 0;
      x = (float *) array_data(iq);
      break;
    default:
      return cerror(ILL_CLASS, iq);
  }
		 /* y, could be scalar or array */
  iq = ps[1];
  switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case LUX_SCALAR:
      ny = 1; 
      iq = lux_float(1, &iq);
      dy = 0;
      y = &scalar_value(iq).f;
      break;
    case LUX_ARRAY:
      iq = lux_float(1, &iq);
      nd = array_num_dims(iq);
      nelem = array_size(iq);
      ny = nelem;
      if (ny)
	dy = 1;
      else
	dy = 0;
      y = (float *) array_data(iq);
      break;
    default:
      return cerror(ILL_CLASS, iq);
  }
  if (nx == 1 || ny == 1)
    nelem = MAX(nx, ny);
  else
    nelem = MIN(nx, ny);
		 /* now check out the mode */
  if (narg > 2 && ps[2]) {
    iq = ps[2];
    switch (symbol_class(iq)) {
      case LUX_SCAL_PTR:
	iq = dereferenceScalPointer(iq);
      case LUX_SCALAR:
	nm = 1; 
	iq = lux_long(1, &iq);
	dm = 0;
	mp = &scalar_value(iq).l;
	break;
      case LUX_ARRAY:
	iq = lux_long(1, &iq);
	nd = array_num_dims(iq);
	nm = array_size(iq);
	if (nm)
	  dm = 1;
	else
	  dm = 0;
	mp = array_data(iq);
	break;
      default:
	return cerror(ILL_CLASS, iq);
    }
  } else {
    mp = &one;
    nm = 1;
    dm = 0;
  }
  if (narg > 3 && ps[3]) {	/* line breaks */
    iq = ps[3];
    switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case LUX_SCALAR:
      nbreak = 1;
      iq = lux_long(1, &iq);
      qi = &scalar_value(iq).l;
      break;
    case LUX_ARRAY:
      iq = lux_long(1, &iq);
      nbreak = array_size(iq);
      qi = array_data(iq);
      break;
    default:
      return cerror(ILL_CLASS, iq);
    }
  } else {
    nbreak = 0;
    qi = &huge;
  }

  if (internalMode & 64) {		/* update bounding box */
    updateBoundingBox = 1;
    internalMode &= ~64;
  }
  if (internalMode & 256) {	/* alternating dash colors */
    altDash = 1;
    internalMode &= ~256;
  } else
    altDash = 0;
  moveFirst = (internalMode & 128 && nelem > 1)? 1: 0;
  tkCoordSys = (internalMode & 7);
  /* set_cur_pen(); */           /* interferes with pencolor */
#ifdef DEVELOP
  if (projectTk)
    createFullProjection(projectMatrix, currentPerspective, currentOblique);
#endif
  /* ready for series (or just 1) of tkplot calls */
  ifirstflag = 1;	/* added 1oct93  LS */
  xc = *x; 
  yc = *y;
  mode = *mp;
  line = LUX_UNSPECIFIED;
  fixPlotStyle(&mode, &line);
  if (moveFirst && line > 0) {
    tkplot(xc, yc, 0, 0);
    alternateDash = altDash;
  } else {
    alternateDash = altDash;
    tkplot(xc, yc, line, mode);
  }
  ifirstflag = 0;
  x += dx; 
  y += dy;
  mp += dm;
  nc = nm - dm;
  for (i = 1; i < nelem; i++) {
    int32_t isbreak;

    xc = *x;
    yc = *y;
    if (dm) {
      line = LUX_UNSPECIFIED;
      mode = *mp;
      if (!nelem &&		/* last point */
	  (!mode || !mp[-dm]))	/* only draw a line if both edges ask for
				   drawing */
	mode = 0;
      fixPlotStyle(&mode, &line);
      if (!--nc) {
	mp -= nm;
	nc = nm;
      }
    }
    if (i == *qi) {		/* found a line break */
      isbreak = 1;
      if (nbreak) {
	if (--nbreak)
	  qi++;
	else
	  qi = &huge;
      }
    } else
      isbreak = 0;
    tkplot(xc, yc, isbreak? 0: line, isbreak? 0: mode); 
    x += dx;
    y += dy;
    mp += dm;
  }
  alternateDash = 0;
  return 1;
}
 /*------------------------------------------------------------------------- */
int32_t lux_postimage(int32_t narg, int32_t ps[])			/* postimage routine */
 /* subroutine, call is postimage(image, x0, x1, y0, y1) */
{
  extern	int32_t	scalemax, scalemin;
  int32_t	lux_scale(int32_t, int32_t *), postgray(char *, int32_t, int32_t, float, float, float, 
					float, int32_t);
  float	x0, x1, y0, y1;
  int32_t	iq, nd, nx, ny, s1, s2;
  char	*ptr;
  array	*h;

  x0 = wxb;
  x1 = wxt;
  y0 = wyb;
  y1 = wyt;
  iq = ps[0];
  CK_ARR(iq, 0);
  if (sym[iq].type != LUX_BYTE )
 /* scale using (0, 255) for scalemax and scalemin */
  { s1 = scalemin;
    s2 = scalemax;
    scalemin = 0;
    scalemax = 255;
    iq = lux_scale(1, &ps[0]);
    scalemin = s1;
    scalemax = s2;	}
  h = (array *) sym[iq].spec.array.ptr;
  nd = h->ndim;
  if (nd != 2) 
  { printf("xtv - array must be 2-D\n"); 
    return -1; }
  nx = h->dims[0];
  ny = h->dims[1];
  ptr = (char *) ((char *) h + sizeof(array));
  if (narg > 1)
    x0 = float_arg(ps[1]);
  if (narg > 2)
    x1 = float_arg(ps[2]);
  if (narg > 3)
    y0 = float_arg(ps[3]);
  if (narg > 4) 
    y1 = float_arg(ps[4]);
  printf("postimage mode: %d\n", landscape);
  return postgray(ptr, nx, ny, x0, x1, y0, y1, iorder);
}
 /*------------------------------------------------------------------------- */
int32_t lux_postraw(int32_t narg, int32_t ps[])			/* postraw routine */
 /* subroutine, call is postraw(string) */
{
  int32_t	iq;
  char	*s;
  int32_t	postrawout(char *);

  iq = ps[0];
  if ( sym[iq].class != 2 ) return cerror(NEED_STR, 0);
  s = (char *) sym[iq].spec.array.ptr;
  return postrawout(s);
}
 /*------------------------------------------------------------------------- */
