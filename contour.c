/* File contour.c */
/* ANA's contouring routines. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: contour.c,v 4.0 2001/02/07 20:36:58 strous Exp $";

/* contour parameters and flags */
extern	int	current_pen, autocon, contour_mode, contour_box, 
	contour_nlev, contour_border, contour_tick_pen,	contour_ticks,
	contour_style, iorder;
extern	float	wxb, wxt, wyb, wyt, contour_dash_lev, contour_tick_fac;
int	contour_flag;
int	contour_sym;
static	int	nx, ny;
static	float	xa, xb, ya, yb;
static	float	xsc, ysc;
int	tkplot(float, float, int, int);
/*------------------------------------------------------------------------- */
int ana_contour(int narg, int ps[]) /* contour routine */		
/* call is CONTOUR, image, nlev, xa, xb, ya, yb */
{
  array	*h;
  int	iq, i, nc, lineStyle, symStyle;
  float	*pf, *cl, xmax, xmin, *pp, yq, xspace, xq;
  int	anacon(float *, int, int, float *, int, int, float, float, float,
	       float, float), installString(char *), ana_replace(int, int),
	box(void), ticks(void), fixPlotStyle(int *, int *);
  extern int	tkCoordSys;
  extern float	theDashSize;
  char	gotLevels;

  iq = ps[0];
  CK_ARR(iq, 1);
  iq = ana_float(1, &iq);
  h = (array *) sym[iq].spec.array.ptr;
  ny = 1; nx = h->dims[0]; if (h->ndim != 1) ny = h->dims[1];
  pf = (float *) ((char *) h + sizeof( array ));
  /* check if $contours array exists yet */
  if (contour_sym == 0) {
    i = installString("$CONTOURS");
    contour_sym = findVar(i, 0);
  }
  gotLevels = 0;
  /* if scalar, then = #levels.  if array, then store in $contours */
  if (narg > 1)
    switch (sym[ps[1]].class)
    { case ANA_SCALAR:
	contour_nlev = int_arg( ps[1]);
	break;
      case ANA_ARRAY:
	ana_replace(contour_sym, ps[1]);
	h = HEAD(contour_sym);
	GET_SIZE(contour_nlev, h->dims, h->ndim);
	gotLevels = 1;  break;
      case 0:  break;
      default:
	return cerror(ILL_CLASS, ps[1]); }
  if (contour_nlev <= 0 || contour_nlev > 50)
  { printf("invalid # of contour levels = %d\n",contour_nlev); return -1; }
  xa = wxb; xb = wxt; ya = wyb; yb = wyt; /*default window */
  if (narg > 2 && sym[ps[2]].class) xa = float_arg( ps[2]);
  if (narg > 3 && sym[ps[3]].class) xb = float_arg( ps[3]);
  if (narg > 4 && sym[ps[4]].class) ya = float_arg( ps[4]);
  if (narg > 5 && sym[ps[5]].class) yb = float_arg( ps[5]);
  if (narg > 6 && sym[ps[6]].class) contour_style = int_arg(ps[6]);
  if (narg > 7 && sym[ps[7]].class) theDashSize = float_arg(ps[7]);
  else theDashSize = 1.0;
						/* check for auto mode */
  if (!gotLevels && ((internalMode & 1) || (!internalMode && autocon)))
  { redef_array(contour_sym, 3, 1, &contour_nlev);
    h = (array *) sym[contour_sym].spec.array.ptr;
    cl = (float *) ((char *) h + sizeof( array ));
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
    iq = ana_float(1, &contour_sym);
    h = (array *) sym[iq].spec.array.ptr;
    if (h->ndim != 1) return cerror(BAD_CONTOURS, contour_sym);
    cl = (float *) ((char *) h + sizeof( array ));
    contour_nlev = MIN(contour_nlev, h->dims[0]);  /* min of these used */
  }
  xsc = (xb - xa)/nx;	ysc = (yb - ya)/ny;
  tkCoordSys = ANA_DVI;
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
  anacon(pf, nx, ny, cl, contour_nlev, iorder, xa, ya, xb, yb,
	contour_dash_lev);
  contour_style = symStyle;	/* restore */
  return 1;
}
/*------------------------------------------------------------------------- */
int box(void)
{
  int	set_pen(int pen);

  set_pen(contour_border);
  tkplot(xa, ya, 0, 0);	tkplot(xb, ya, 1, 0);	tkplot(xb, yb, 1, 0);
  tkplot(xa, yb, 1, 0);	tkplot(xa, ya, 1, 0);
  set_pen(current_pen);
  return 1;
}
/*------------------------------------------------------------------------- */
int ticks(void)
{
  int	oldpen, pen;
  float	tx, ty, y, x, xs, ys;
  int	set_pen(int);

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
int tkdash(float *aa, float *bb, int *ndsh, int *ntimes)
/* actually no dashed line support yet 3/9/92 */
     /* there is now!  LS 4feb95 */
{
  int	nc;
  float	x, y;

  nc = *ntimes;
  while (nc-- > 0) 
  { x = *aa++;	y = *bb++;	tkplot(x,y, 0, 0);
    x = *aa++;	y = *bb++;	tkplot(x,y, contour_style, 0); }
  return 1;
}
/*------------------------------------------------------------------------- */
int lower_int(float x)
/* returns the first integer closer to -Infinity than x */
{
  int	i;

  i = (int) x;
  if (x < 0) i--;
  return i;
}
/*------------------------------------------------------------------------- */
int closest_int(float x)
/* returns the integer closest to x (with integer + 0.5 -> integer + 1) */
{
  int	i;

  i = (int) x;
  if (x < 0) i--;
  if (x - i >= 0.5) i++;
  return i;
}
