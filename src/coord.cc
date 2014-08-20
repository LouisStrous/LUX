/* This is file coord.c.

Copyright 2013 Louis Strous

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
/* file coord.c */
/* coordinate transformations */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h>
#include <stdio.h>
#include "action.h"

extern float	xfac, yfac, wxt, wxb, wyt, wyb, xmin, xmax, ymin, ymax;
#if HAVE_LIBX11
extern float	tvix, tviy, tvixb, tviyb, tvscale;
#endif
extern int32_t	iorder, iyhigh, ipltyp;

int32_t	lux_replace(int32_t, int32_t);

/* A bunch of coordinate transformation routines */
/* choices: LUX_DVI, LUX_DEV (xport), LUX_IMG, LUX_PLT (plot),
   LUX_RIM, LUX_RPL */
int32_t fromCoordinateSystem, toCoordinateSystem;
/*---------------------------------------------------------------------*/
int32_t coordMap(float *x, float *y)
/* transforms (x,y) (in fromCoordinateSystem) to toCoordinateSystem */
{
  int32_t coordTrf(float *, float *, int32_t, int32_t);

  return coordTrf(x, y, fromCoordinateSystem, toCoordinateSystem);
}
/*---------------------------------------------------------------------*/
int32_t coordTrf(float *x, float *y, int32_t from, int32_t to)
     /* xfac,yfac are the pixel dimensions of the current window */
     /* wxb,wxt is the DVI x range of the plot window */
     /* wyb,wyt is the DVI y range of the plot window */
     /* tvix,tvixb-1 is the DEV x range of the last image */
     /* tviy,tviyb-1 is the DEV y range of the last image*/
     /* tvscale is the current magnification factor */
/* if <x> or <y> are equal to NULL, then the corresponding coordinate */
/* is not treated. */
{
  extern int32_t	setup;

  if (from == to)
    return 1;
  /* first, transform to LUX_DEV coordinate system */
  switch (from) {
    case LUX_DEV:
      break;
    case LUX_DEP:
      if (!((x && *x < 1.0 && *x != 0.0) || (y && *y < 1.0 && *y != 0.0)))
	break;
      /* else fall-thru */
    case LUX_DVI:
      if (x) 
	*x *= xfac;
      if (y)
	*y *= yfac;
      break;
#if HAVE_LIBX11
    case LUX_RIM:
      if (x) 
	*x = tvix + *x * (tvixb - tvix);
      if (y) {
	if (setup & 8)
	  *y = tviyb - *y * (tviyb - tviy);
	else
	  *y = tviy + *y * (tviyb - tviy);
      }
      break;
    case LUX_X11:
      if (y)
	*y = yfac - 1 - *y;
      break;
    case LUX_IMG:
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
    case LUX_PLT:
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
    case LUX_RPL:
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
    case LUX_DEV:
      return 1;
    case LUX_DVI: case 0:
      if (x)
	*x = xfac? *x/xfac: 0.0;
      if (y)
	*y = yfac? *y/yfac: 0.0;
      break;
#if HAVE_LIBX11
    case LUX_RIM:
      if (x)
	*x = (tvixb != tvix)? (*x - tvix)/(tvixb - tvix): 0.0;
      if (y) {
	if (setup & 8)
	  *y = (tviyb != tviy)? (tviyb - *y)/(tviyb - tviy): 0.0;
	else
	  *y = (tviyb != tviy)? (*y - tviy)/(tviyb - tviy): 0.0;
      }
      break;
    case LUX_IMG:
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
    case LUX_X11:
      if (y)
	*y = yfac - 1 - *y;
      break;
#endif
    case LUX_PLT:  case LUX_RPL:
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
      if (to == LUX_RPL)
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
int32_t lux_coordtrf(int32_t narg, int32_t ps[])
/* transform coordinates between various coordinate systems */
/* syntax:  COORDTRF,xold,yold[,xnew,ynew] */
/* specify the coordinate systems with keywords: */
/* /DVI, /DEV, /IMG, /PLT, /RIM, /RPL, /DEP, /X11 */
/* /TODVI, /TODEV, /TOIMG, /TOPLT, /TORIM, /TORPL, /TOX11 */
{
  int32_t	iq, n, n2, from, to;
  float	*xold, *yold, *xnew, *ynew, x, y;

  from = (internalMode & 7);
  to = (internalMode/8 & 7);
  iq = lux_float(1, ps);	/* float xold */
  switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case LUX_SCALAR:
      xold = &scalar_value(iq).f;
      n = 1;
      break;
    case LUX_ARRAY:
      n = array_size(iq);
      xold = (float *) array_data(iq);
      break;
    default:
      return cerror(ILL_CLASS, iq);
  }
  iq = lux_float(1, &ps[1]);	/* float yold */
  switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case LUX_SCALAR:
      yold = &scalar_value(iq).f;
      n2 = 1;
      break;
    case LUX_ARRAY:
      n2 = array_size(iq);
      yold = (float *) array_data(iq);
      break;
    default:
      return cerror(ILL_CLASS, iq);
  }
  if (n != n2)
    return cerror(INCMP_DIMS, iq);
  if (narg >= 3) {		/* xnew */
    lux_replace(ps[2], iq);
    iq = ps[2];
    if (symbol_class(iq) == LUX_SCALAR)
      xnew = &scalar_value(iq).f;
    else
      xnew = (float *) array_data(iq);
  } else
    xnew = xold;
  if (narg >= 4) {		/* ynew */
    lux_replace(ps[3], iq);
    iq = ps[3];
    if (symbol_class(iq) == LUX_SCALAR)
      ynew = &scalar_value(iq).f;
    else
      ynew = (float *) array_data(iq);
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
