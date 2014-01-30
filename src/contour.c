/* This is file contour.c.

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
/* File contour.c */
/* LUX's contouring routines. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "action.h"

/* contour parameters and flags */
extern	Int	current_pen, autocon, contour_mode, contour_box, 
	contour_nlev, contour_border, contour_tick_pen,	contour_ticks,
	contour_style, iorder;
extern	double	wxb, wxt, wyb, wyt, contour_dash_lev, contour_tick_fac;
Int	contour_flag;
Int	contour_sym;
static	Int	nx, ny;
static	double	xa, xb, ya, yb;
static	double	xsc, ysc;
Int	tkplot(double, double, Int, Int);
/*------------------------------------------------------------------------- */
Int lux_contour(Int narg, Int ps[]) /* contour routine */		
/* call is CONTOUR, image, nlev, xa, xb, ya, yb */
{
  array	*h;
  Int	iq, i, nc, lineStyle, symStyle;
  double	*pf, *cl, xmax, xmin, *pp, yq, xspace, xq;
  Int	luxcon(double *, Int, Int, double *, Int, Int, double, double, double,
	       double, double), installString(char *), lux_replace(Int, Int),
	box(void), ticks(void), fixPlotStyle(Int *, Int *);
  extern Int	tkCoordSys;
  extern double	theDashSize;
  char	gotLevels;

  iq = ps[0];
  CK_ARR(iq, 1);
  iq = lux_double(1, &iq);
  h = (array *) sym[iq].spec.array.ptr;
  ny = 1; nx = h->dims[0]; if (h->ndim != 1) ny = h->dims[1];
  pf = (double *) ((char *) h + sizeof( array ));
  /* check if $contours array exists yet */
  if (contour_sym == 0) {
    i = installString("$CONTOURS");
    contour_sym = findVar(i, 0);
  }
  gotLevels = 0;
  /* if scalar, then = #levels.  if array, then store in $contours */
  if (narg > 1)
    switch (sym[ps[1]].class)
    { case LUX_SCALAR:
	contour_nlev = int_arg( ps[1]);
	break;
      case LUX_ARRAY:
	lux_replace(contour_sym, ps[1]);
	h = HEAD(contour_sym);
	GET_SIZE(contour_nlev, h->dims, h->ndim);
	gotLevels = 1;  break;
      case 0:  break;
      default:
	return cerror(ILL_CLASS, ps[1]); }
  if (contour_nlev <= 0 || contour_nlev > 50)
  { printf("invalid # of contour levels = %d\n",contour_nlev); return -1; }
  xa = wxb; xb = wxt; ya = wyb; yb = wyt; /*default window */
  if (narg > 2 && sym[ps[2]].class) xa = double_arg( ps[2]);
  if (narg > 3 && sym[ps[3]].class) xb = double_arg( ps[3]);
  if (narg > 4 && sym[ps[4]].class) ya = double_arg( ps[4]);
  if (narg > 5 && sym[ps[5]].class) yb = double_arg( ps[5]);
  if (narg > 6 && sym[ps[6]].class) contour_style = int_arg(ps[6]);
  if (narg > 7 && sym[ps[7]].class) theDashSize = double_arg(ps[7]);
  else theDashSize = 1.0;
						/* check for auto mode */
  if (!gotLevels && ((internalMode & 1) || (!internalMode && autocon)))
  { redef_array(contour_sym, 3, 1, &contour_nlev);
    h = (array *) sym[contour_sym].spec.array.ptr;
    cl = (double *) ((char *) h + sizeof( array ));
    pp = pf;	nc = nx * ny - 1;	xmin = xmax = *pp++;
    while (nc--) {	if (*pp > xmax)  xmax = *pp;
			if (*pp < xmin)  xmin = *pp;	pp++; }
    yq = xmax - xmin;	pp = cl;
    switch (contour_mode) {
    case 0: default:		/* linear */
      xspace = yq / contour_nlev;	*pp++ = xmin + 0.5 * xspace;  break;
    case 1:  					/* sqrt */
      xspace = sqrt(yq) / contour_nlev;	*pp++ = 0.0;  break;
    case 2:  					/* log */
      xq = MAX( xmin, 1.0);	xspace = log(yq) / contour_nlev;
      *pp++ = log(xq) + 0.5 * xspace;  break;
    }
    nc = contour_nlev - 1;
    while (nc--) { *pp = *(pp - 1) + xspace;  pp++; }
    nc = contour_nlev;	pp = cl;
    switch (contour_mode) {
    case 1:  					/* sqrt */
      while (nc--) { *pp = pow(*pp, 2.0) + xmin;  pp++; } break;
    case 2:  					/* log */
      while (nc--) { *pp =exp(*pp) + xmin;  pp++; } break;
    }
  } else {				/* not auto mode, check $contours */
    if ( sym[contour_sym].class != 4 )
      return cerror(BAD_CONTOURS, contour_sym);
    iq = lux_double(1, &contour_sym);
    h = (array *) sym[iq].spec.array.ptr;
    if (h->ndim != 1) return cerror(BAD_CONTOURS, contour_sym);
    cl = (double *) ((char *) h + sizeof( array ));
    contour_nlev = MIN(contour_nlev, h->dims[0]);  /* min of these used */
  }
  xsc = (xb - xa)/nx;	ysc = (yb - ya)/ny;
  tkCoordSys = LUX_DVI;
  /* do we want border and ticks? */
  if (contour_flag == 0)	{ /* set flag means none, set via tvcon */
    if (contour_border > 0) box();
    if (contour_ticks > 0) ticks();
  }
  /* ensure that contour_style is legal */
  lineStyle = contour_style;  symStyle = 0;
  fixPlotStyle(&symStyle, &lineStyle);
  symStyle = contour_style;	/* save for later */
  contour_style = lineStyle;
  luxcon(pf, nx, ny, cl, contour_nlev, iorder, xa, ya, xb, yb,
	contour_dash_lev);
  contour_style = symStyle;	/* restore */
  return 1;
}
/*------------------------------------------------------------------------- */
Int box(void)
{
  Int	set_pen(Int pen);

  set_pen(contour_border);
  tkplot(xa, ya, 0, 0);	tkplot(xb, ya, 1, 0);	tkplot(xb, yb, 1, 0);
  tkplot(xa, yb, 1, 0);	tkplot(xa, ya, 1, 0);
  set_pen(current_pen);
  return 1;
}
/*------------------------------------------------------------------------- */
Int ticks(void)
{
  Int	oldpen, pen;
  double	tx, ty, y, x, xs, ys;
  Int	set_pen(Int);

  if (contour_tick_pen <= 0)
    return 1;
  oldpen = current_pen;
  pen = contour_tick_pen;
  set_pen(pen);
  if (iorder%2 == 0) {		/* note: swapped x and y  LS 14jan93 */
    xs = xsc;
    ys = ysc;
  } else {
    xs = ysc;
    ys = xsc;
  }
  tx = xsc*contour_tick_fac;
  ty = ysc*contour_tick_fac;
  tx = ty = MIN(tx, ty);	/* use smallest of the 2 */
				/* do the sides */
  if (iorder < 2 || iorder > 5) {
    y = ya;
    while (y < yb) {
      tkplot(xa,y,0,0);
      tkplot(xa+tx,y,1,0);
      tkplot(xb,y,0,0);
      tkplot(xb-tx,y,1,0);
      y += ys;
    }
  } else {
    y = yb;
    while (y > ya) {
      tkplot(xa,y,0,0);
      tkplot(xa+tx,y,1,0);
      tkplot(xb,y,0,0);
      tkplot(xb-tx,y,1,0);
      y -= ys;
    }
  }
				/* top and bottom */
  if (iorder < 4) {
    x = xa;
    while (x < xb) {
      tkplot(x,ya,0,0);
      tkplot(x,ya+ty,1,0);
      tkplot(x,yb,0,0);
      tkplot(x,yb-ty,1,0);
      x += xs;
    }
  } else {
    x = xb;			/* changed from y = yb.  LS 17mar99 */
    while (y > ya) {
      tkplot(x,ya,0,0);
      tkplot(x,ya+ty,1,0);
      tkplot(x,yb,0,0);
      tkplot(x,yb-ty,1,0);
      x -= xs;
    }
  }
				/* now draw the origin box if wanted */
  if (contour_box) {
    switch (iorder/2) {
      case 0:
	tkplot(xa,ya,4,0);
	break;
      case 1:
	tkplot(xa,yb,4,0);
	break;
      case 2:
	tkplot(xb,yb,4,0);
	break;
      case 3:
	tkplot(xb,ya,4,0);
	break;
    }
  }
  set_pen(oldpen);
  return 1;
}
/*------------------------------------------------------------------------- */
Int tkdash(double *aa, double *bb, Int *ndsh, Int *ntimes)
/* actually no dashed line support yet 3/9/92 */
     /* there is now!  LS 4feb95 */
{
  Int	nc;
  double	x, y;

  nc = *ntimes;
  while (nc-- > 0) 
  { x = *aa++;	y = *bb++;	tkplot(x,y, 0, 0);
    x = *aa++;	y = *bb++;	tkplot(x,y, contour_style, 0); }
  return 1;
}
/*------------------------------------------------------------------------- */
Int lower_int(double x)
/* returns the first integer closer to -Infinity than x */
{
  Int	i;

  i = (Int) x;
  if (x < 0) i--;
  return i;
}
/*------------------------------------------------------------------------- */
Int closest_int(double x)
/* returns the integer closest to x (with integer + 0.5 -> integer + 1) */
{
  Int	i;

  i = (Int) x;
  if (x < 0) i--;
  if (x - i >= 0.5) i++;
  return i;
}
