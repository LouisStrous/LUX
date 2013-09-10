/* file coord.c */
/* coordinate transformations */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h>
#include <stdio.h>
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
"$Id: coord.c,v 4.0 2001/02/07 20:36:58 strous Exp $";

extern Float	xfac, yfac, wxt, wxb, wyt, wyb, xmin, xmax, ymin, ymax;
#if HAVE_LIBX11
extern Float	tvix, tviy, tvixb, tviyb, tvscale;
#endif
extern Int	iorder, iyhigh, ipltyp;

Int	ana_replace(Int, Int);

/* A bunch of coordinate transformation routines */
/* choices: ANA_DVI, ANA_DEV (xport), ANA_IMG, ANA_PLT (plot),
   ANA_RIM, ANA_RPL */
Int fromCoordinateSystem, toCoordinateSystem;
/*---------------------------------------------------------------------*/
Int coordMap(Float *x, Float *y)
/* transforms (x,y) (in fromCoordinateSystem) to toCoordinateSystem */
{
  Int coordTrf(Float *, Float *, Int, Int);

  return coordTrf(x, y, fromCoordinateSystem, toCoordinateSystem);
}
/*---------------------------------------------------------------------*/
Int coordTrf(Float *x, Float *y, Int from, Int to)
     /* xfac,yfac are the pixel dimensions of the current window */
     /* wxb,wxt is the DVI x range of the plot window */
     /* wyb,wyt is the DVI y range of the plot window */
     /* tvix,tvixb-1 is the DEV x range of the last image */
     /* tviy,tviyb-1 is the DEV y range of the last image*/
     /* tvscale is the current magnification factor */
/* if <x> or <y> are equal to NULL, then the corresponding coordinate */
/* is not treated. */
{
  extern Int	setup;

  if (from == to)
    return 1;
  /* first, transform to ANA_DEV coordinate system */
  switch (from) {
    case ANA_DEV:
      break;
    case ANA_DEP:
      if (!((x && *x < 1.0 && *x != 0.0) || (y && *y < 1.0 && *y != 0.0)))
	break;
      /* else fall-thru */
    case ANA_DVI:
      if (x) 
	*x *= xfac;
      if (y)
	*y *= yfac;
      break;
#if HAVE_LIBX11
    case ANA_RIM:
      if (x) 
	*x = tvix + *x * (tvixb - tvix);
      if (y) {
	if (setup & 8)
	  *y = tviyb - *y * (tviyb - tviy);
	else
	  *y = tviy + *y * (tviyb - tviy);
      }
      break;
    case ANA_X11:
      if (y)
	*y = yfac - 1 - *y;
      break;
    case ANA_IMG:
      if (x)
	*x = tvix + *x*tvscale;
      if (y) {
	if (setup & 8)
	  *y = tviyb - *y * tvscale;
	else
	  *y = tviy + *y * tvscale;
      }
      break;
#endif
    case ANA_PLT:
      if (x) {
	if (ipltyp/2 % 2) {
	  if (xmin && xmin != xmax)
	    *x = log(*x/xmin)/log(xmax/xmin);
	  else
	    *x = 0.0;
	} else {
	  if (xmin != xmax)
	    *x = (*x - xmin)/(xmax - xmin);
	  else
	    *x = 0.0;
	}
      }
      if (y) {
	if (ipltyp % 2) {
	  if (ymin && ymin != ymax)
	    *y = log(*y/ymin)/log(ymax/ymin);
	  else
	    *y = 0;
	} else {
	  if (ymin != ymax) 
	    *y = (*y - ymin)/(ymax - ymin);
	  else
	    *y = 0.0;
	}
      }
      /* fall-thru */
    case ANA_RPL:
      if (x)
	*x = (*x * (wxt - wxb) + wxb)*xfac;
      if (y)
	*y = (*y * (wyt - wyb) + wyb)*yfac;
      break;
    default:
      return cerror(ILL_COORD_SYS, from);
  }
  /* then, transform to desired coordinate system */
  switch (to) {
    case ANA_DEV:
      return 1;
    case ANA_DVI: case 0:
      if (x)
	*x = xfac? *x/xfac: 0.0;
      if (y)
	*y = yfac? *y/yfac: 0.0;
      break;
#if HAVE_LIBX11
    case ANA_RIM:
      if (x)
	*x = (tvixb != tvix)? (*x - tvix)/(tvixb - tvix): 0.0;
      if (y) {
	if (setup & 8)
	  *y = (tviyb != tviy)? (tviyb - *y)/(tviyb - tviy): 0.0;
	else
	  *y = (tviyb != tviy)? (*y - tviy)/(tviyb - tviy): 0.0;
      }
      break;
    case ANA_IMG:
      if (tvscale) {
	if (x)
	  *x = (*x - tvix)/tvscale;
	if (y) {
	  if (setup & 8)
	    *y = (tviyb - *y)/tvscale;
	  else
	    *y = (*y - tviy)/tvscale;
	}
      } else {
	if (x)
	  *x = 0.0;
	if (y)
	  *y = 0.0;
      }
      break;
    case ANA_X11:
      if (y)
	*y = yfac - 1 - *y;
      break;
#endif
    case ANA_PLT:  case ANA_RPL:
      if (x) {
	if (xfac && wxt != wxb)
	  *x = (*x/xfac - wxb)/(wxt - wxb);
	else
	  *x = 0.0;
      }
      if (y) {
	if (yfac && wyt != wyb)
	  *y = (*y/yfac - wyb)/(wyt - wyb);
	else
	  *y = 0.0;
      }
      if (to == ANA_RPL)
	break;
      if (x) {
	if (ipltyp/2 % 2)
	  *x = (xmin != xmax)? xmin*exp(log(xmax/xmin)* *x): 0.0;
	else
	  *x = *x * (xmax - xmin) + xmin;
      }
      if (y) {
	if (ipltyp % 2)
	  *y = (ymin != ymax)? ymin*exp(log(ymax/ymin)* *y): 0.0;
	else
	  *y = *y * (ymax - ymin) + ymin;
      }
      break;
    default:
      return cerror(ILL_COORD_SYS, to);
  }
  return 1;
}
/*---------------------------------------------------------------------*/
Int ana_coordtrf(Int narg, Int ps[])
/* transform coordinates between various coordinate systems */
/* syntax:  COORDTRF,xold,yold[,xnew,ynew] */
/* specify the coordinate systems with keywords: */
/* /DVI, /DEV, /IMG, /PLT, /RIM, /RPL, /DEP, /X11 */
/* /TODVI, /TODEV, /TOIMG, /TOPLT, /TORIM, /TORPL, /TOX11 */
{
  Int	iq, n, n2, from, to;
  Float	*xold, *yold, *xnew, *ynew, x, y;

  from = (internalMode & 7);
  to = (internalMode/8 & 7);
  iq = ana_float(1, ps);	/* Float xold */
  switch (symbol_class(iq)) {
    case ANA_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case ANA_SCALAR:
      xold = &scalar_value(iq).f;
      n = 1;
      break;
    case ANA_ARRAY:
      n = array_size(iq);
      xold = (Float *) array_data(iq);
      break;
    default:
      return cerror(ILL_CLASS, iq);
  }
  iq = ana_float(1, &ps[1]);	/* Float yold */
  switch (symbol_class(iq)) {
    case ANA_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case ANA_SCALAR:
      yold = &scalar_value(iq).f;
      n2 = 1;
      break;
    case ANA_ARRAY:
      n2 = array_size(iq);
      yold = (Float *) array_data(iq);
      break;
    default:
      return cerror(ILL_CLASS, iq);
  }
  if (n != n2)
    return cerror(INCMP_DIMS, iq);
  if (narg >= 3) {		/* xnew */
    ana_replace(ps[2], iq);
    iq = ps[2];
    if (symbol_class(iq) == ANA_SCALAR)
      xnew = &scalar_value(iq).f;
    else
      xnew = (Float *) array_data(iq);
  } else
    xnew = xold;
  if (narg >= 4) {		/* ynew */
    ana_replace(ps[3], iq);
    iq = ps[3];
    if (symbol_class(iq) == ANA_SCALAR)
      ynew = &scalar_value(iq).f;
    else
      ynew = (Float *) array_data(iq);
  } else
    ynew = yold;
  while (n--) {
    x = *xold++;
    y = *yold++;
    coordTrf(&x, &y, from, to);
    *xnew++ = x;
    *ynew++ = y;
  }
  return 1;
}
/*---------------------------------------------------------------------*/
