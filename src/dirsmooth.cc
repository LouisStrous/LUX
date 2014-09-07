/* This is file dirsmooth.cc.

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
/* File dirsmooth.c */
/* LUX routines for directional smoothing */

#include "action.hh"
#include <math.h>

#define LEFT	0
#define DOWN	1
#define RIGHT	2
#define UP	3
#define CENTER	4
#define DONE	5

int32_t sign(float x)
{
  if (x > 0)
    return 1;
  else if (x < 0)
    return -1;
  else
    return 0;
}
/*--------------------------------------------------------------------*/
int32_t sgnclass(float x)
{
  if (x > 0)
    return 2;
  else if (x < 0)
    return 0;
  else
    return 1;
}
/*--------------------------------------------------------------------*/
int32_t traverseElement(float xin, float yin, float vx, float vy,
		    float *xout, float *yout)
/* if you start at position (<xin>,<yin>), with 0 <= <xin>,<yin> <= 1,
   and move in the direction given by (<vx>,<vy>), then this routine
   determines which pixel boundary you cross (UP, DOWN, LEFT, RIGHT,
   or CENTER if <vx> = <vy> = 0) and what the coordinates are.  The
   new coordinates are returned in <*xout> and <*yout> and the pixel
   boundary code is the return value of the routine.  LS 17oct98 */
{
  if (vx > 0) {			/* to the right */
    /* first we check for UP.  The vector that separates UP from RIGHT
       has coordinates (1 - xin, 1 - yin); rotated counterclockwise
       over 90 degrees this becomes (yin - 1, xin): if the inner
       product of the velocity vector (vx,vy) with this vector is
       positive, then we are going UP.  The vector that separates
       RIGHT from DOWN is (1-xin,-yin), which leads to (yin, 1-xin). */
    if (vx*(yin - 1) + vy*(1 - xin) >= 0) { /* we're going UP */
      if (yin == 1) {		/* we're already at the top */
	/* go to the right edge */
	*xout = 1;
	*yout = 1;
	return RIGHT;
      }
      *xout = vy? xin + (1 - yin)/vy*vx: 1;
      *yout = 1;
      return UP;
    } else if (vx*yin + vy*(1 - xin) >= 0) { /* we're going RIGHT */
      if (xin == 1) {		/* we're already at the right edge */
	*xout = 1;
	if (vy >= 0) {
	  *yout = 1;
	  return UP;
	}
	*yout = 0;
	return DOWN;
      }
      *xout = 1;
      *yout = vx? yin + (1 - xin)/vx*vy: 0;
      return RIGHT;
    } else {			/* we're going DOWN */
      if (yin == 0) {		/* we're already at the bottom edge */
	*xout = 1;
	*yout = 0;
	return RIGHT;
      }
      *xout = vy? xin - yin/vy*vx: 1;
      *yout = 0;
      return DOWN;
    }
  }

  if (vx < 0) {		/* to the left */
    if (vx*yin - vy*xin >= 0) {	/* DOWN */
      if (yin == 0) {		/* we're already at the bottom */
	*xout = 0;
	*yout = 0;
	return LEFT;
      }
      *xout = vy? xin - yin/vy*vx: 0;
      *yout = 0;
      return DOWN;
    } else if (vx*(yin - 1) - vy*xin >= 0) { /* LEFT */
      if (xin == 0) {		/* we're already at the left edge */
	*xout = 0;
	if (vy >= 0) {
	  *yout = 1;
	  return UP;
	}
	*yout = 0;
	return DOWN;
      }
      *xout = 0;
      *yout = vx? yin - xin/vx*vy: 1;
      return LEFT;
    } else {			/* UP */
      if (yin == 1) {
	*xout = 0;
	*yout = 1;
	return LEFT;
      }
      *xout = vy? xin + (1 - yin)/vy*vx: 0;
      *yout = 1;
      return UP;
    }
  }

  if (vy > 0) {		/* straight up */
    *xout = xin;
    *yout = 1;
    return UP;
  }

  if (vy < 0) {		/* straight down */
    *xout = xin;
    *yout = 0;
    return DOWN;
  }
  /* no movement at all */
  *xout = xin;
  *yout = yin;
  return CENTER;
}
/*--------------------------------------------------------------------*/
#define FACTOR	(0.886226925)	/* 0.5*sqrt(pi) */
int32_t lux_lic(int32_t narg, int32_t ps[])
/* Y = LIC(<data>,<vx>,<vy>)
   smooths 2D image <data> in the direction indicated by the
   angle <vx> and <vy>, over a distance indicated by the magnitude of vector
   <v>. */
{
  int32_t	iq, nx, ny, ix, iy, c, index, rindex, count, twosided, normalize,
    gaussian, iq0, di;
  float	x1, y1, x2, y2, *vx0, *vy0, value, vx, vy, s, s0, ds, dslimit,
    weight, ws;
  pointer	src, trgt, src0;
  loopInfo	srcinfo, trgtinfo;

  iq0 = ps[0];			/* data */
  if (symbol_class(iq0) != LUX_ARRAY /* not an array */
      || array_num_dims(iq0) != 2) /* or doesn't have 2 dimensions */
    return cerror(NEED_2D_ARR, iq0);
  iq0 = lux_float(1, &iq0);
  nx = array_dims(iq0)[0];
  ny = array_dims(iq0)[1];

  iq = ps[1];			/* vx */
  if (symbol_class(iq) != LUX_ARRAY
      || array_num_dims(iq) != 2
      || array_dims(iq)[0] != nx
      || array_dims(iq)[1] != ny)
    return cerror(INCMP_ARG, iq);
  iq = lux_float(1, &iq);
  vx0 = array_data(iq);

  iq = ps[2];			/* vy */
  if (symbol_class(iq) != LUX_ARRAY
      || array_num_dims(iq) != 2
      || array_dims(iq)[0] != nx
      || array_dims(iq)[1] != ny)
    return cerror(INCMP_ARG, iq);
  iq = lux_float(1, &iq);
  vy0 = array_data(iq);

  if (standardLoop(iq0, 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT | SL_EACHCOORD,
		   LUX_FLOAT, &srcinfo, &src, &iq, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;
  src0.f = src.f;

  twosided = ((internalMode & 1) == 0); /* /TWOSIDED */
  normalize = (internalMode & 4)? 1: 0;	/* /NORMALIZE */
  gaussian = (internalMode & 2)? 1: 0; /* /GAUSSIAN */

  if (!gaussian)		/* boxcar */
    do {
      count = twosided + 1;
      /* we work up to the desired smoothing width by measuring the path
	 lengths in each pixel and adding them up until we reach the
	 desired path length.  This scheme backfires when the velocities
	 at some point circle the common corner of four pixels: then the
	 algorithm can keep circling that point in very small steps for
	 a very long time.  If many small steps follow one another in
	 succession, then we are probably in such a situation and it is
	 then unlikely that we'll get out of it any time soon.  We guard
	 against this by maintaining an exponentially weighted average
	 of the past steps and quitting if the weighted average gets too
	 small.  We use an exponential decay scale of 2 steps and
	 a limit value of 0.2. */
      dslimit = 1.0;		/* current weighted average of step sizes */
      value = 0.0;
      *trgt.f = 0.0;
      while (count--) {
	rindex = 0;		/* index relative to current start location */
	ix = srcinfo.coords[0];	/* x pixel coordinate */
	iy = srcinfo.coords[1];	/* y pixel coordinate */
	index = src.f - src0.f;	/* index relative to data start */

	x1 = 0.5;		/* x coordinate in pixel (between 0 and 1) */
	y1 = 0.5;		/* y coordinate in pixel (between 0 and 1) */
	vx = vx0[index];	/* x velocity */
	vy = vy0[index];	/* y velocity */
	if (count) {
	  vx = -vx;
	  vy = -vy;
	}

	s0 = sqrt(vx*vx + vy*vy); /* length indicates smoothing width */
	s = 0.0;
	
	while (s < s0) {
	  c = traverseElement(x1, y1, vx, vy, &x2, &y2);
	  /* calculate distance inside the current pixel */
	  x1 -= x2;
	  y1 -= y2;
	  ds = sqrt(x1*x1 + y1*y1);
	  if (s + ds > s0)
	    ds = s0 - s;
	  dslimit = 0.5*(dslimit + ds);
	  if (dslimit < 0.2) {	/* we fear a semi-infinite loop here */
	    value += src.f[rindex]*s;
	    s = s0;		/* we break it off */
	    continue;
	  }
	  switch (c) {
	    case UP:
	      if (iy == ny - 1) { /* already at top */
		value += src.f[rindex]*s;
		s = s0;
		continue;
	      }
	      di = nx;
	      x1 = x2;
	      y1 = 0.0;
	      iy++;
	      break;
	    case RIGHT:
	      if (ix == nx - 1) { /* already at right edge */
		value += src.f[rindex]*s;
		s = s0;
		continue;
	      }
	      di = 1;
	      x1 = 0.0;
	      y1 = y2;
	      ix++;
	      break;
	    case DOWN:
	      if (iy == 0) {	/* already at bottom */
		value += src.f[rindex]*s;
		s = s0;
		continue;
	      }
	      di = -nx;
	      x1 = x2;
	      y1 = 1.0;
	      iy--;
	      break;
	    case LEFT:
	      if (ix == 0) {	/* already at left edge */
		value += src.f[rindex]*s;
		s = s0;
		continue;
	      }
	      di = -1;
	      x1 = 1.0;
	      y1 = y2;
	      ix--;
	      break;
	    case CENTER:
	      value += src.f[rindex]*s;
	      s = s0;
	      continue;
	  } /* end of switch (c) */
	  vx = vx0[index];
	  vy = vy0[index];
	  if (count) {
	    vx = -vx;
	    vy = -vy;
	  }
	  value += src.f[rindex]*ds;
	  index += di;
	  rindex += di;
	  s += ds;
	} /* end of while (s < s0) */
      } /* end of while (count--) */
      if (normalize && !count)
	value /= s0;
      *trgt.f = value;
    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
  else				/* gaussian smoothing */
    do {
      count = twosided + 1;
      /* we work up to the desired smoothing width by measuring the path
	 lengths in each pixel and adding them up until we reach the
	 desired path length.  This scheme backfires when the velocities
	 at some point circle the common corner of four pixels: then the
	 algorithm can keep circling that point in very small steps for
	 a very long time.  If many small steps follow one another in
	 succession, then we are probably in such a situation and it is
	 then unlikely that we'll get out of it any time soon.  We guard
	 against this by maintaining an exponentially weighted average
	 of the past steps and quitting if the weighted average gets too
	 small.  We use an exponential decay scale of 2 steps and
	 a limit value of 0.2. */
      dslimit = 1.0;		/* current weighted average of step sizes */
      value = 0.0;
      while (count--) {
	rindex = 0;		/* index relative to current start location */
	ix = srcinfo.coords[0];	/* x pixel coordinate */
	iy = srcinfo.coords[1];	/* y pixel coordinate */
	index = src.f - src0.f;	/* index relative to data start */

	x1 = 0.5;		/* x coordinate in pixel (between 0 and 1) */
	y1 = 0.5;		/* y coordinate in pixel (between 0 and 1) */
	vx = vx0[index];	/* x velocity */
	vy = vy0[index];	/* y velocity */
	if (count) {
	  vx = -vx;
	  vy = -vy;
	}

	s0 = sqrt(vx*vx + vy*vy); /* length indicates smoothing width */
	s = 0.0;
	ws = 0.0;
	
	while (s < s0) {
	  c = traverseElement(x1, y1, vx, vy, &x2, &y2);
	  /* calculate distance inside the current pixel */
	  x1 -= x2;
	  y1 -= y2;
	  ds = sqrt(x1*x1 + y1*y1);
	  if (s + ds > s0)
	    ds = s0 - s;
	  weight = s/s0;
	  weight = exp(-weight*weight);
	  dslimit = 0.5*(dslimit + ds);
	  if (dslimit < 0.2) {	/* we fear a semi-infinite loop here */
	    ds = 0.5;
	    if (s + ds > s0)
	      ds = s0 - s;
	  }
	  switch (c) {
	    case UP:
	      if (iy == ny - 1) { /* already at top */
		weight = s/s0 + 0.5;
		weight = exp(-weight*weight + 0.25)*FACTOR*(s0 - s);
		value += src.f[rindex]*weight;
		ws += weight;
		s = s0;
		continue;
	      }
	      di = nx;
	      x1 = x2;
	      y1 = 0.0;
	      iy++;
	      break;
	    case RIGHT:
	      if (ix == nx - 1) { /* already at right edge */
		weight = s/s0 + 0.5;
		weight = exp(-weight*weight + 0.25)*FACTOR*(s0 - s);
		value += src.f[rindex]*weight;
		ws += weight;
		s = s0;
		continue;
	      }
	      di = 1;
	      x1 = 0.0;
	      y1 = y2;
	      ix++;
	      break;
	    case DOWN:
	      if (iy == 0) {	/* already at bottom */
		weight = s/s0 + 0.5;
		weight = exp(-weight*weight + 0.25)*FACTOR*(s0 - s);
		value += src.f[rindex]*weight;
		ws += weight;
		s = s0;
		continue;
	      }
	      di = -nx;
	      x1 = x2;
	      y1 = 1.0;
	      iy--;
	      break;
	    case LEFT:
	      if (ix == 0) {	/* already at left edge */
		weight = s/s0 + 0.5;
		weight = exp(-weight*weight + 0.25)*FACTOR*(s0 - s);
		value += src.f[rindex]*weight;
		ws += weight;
		s = s0;
		continue;
	      }
	      di = -1;
	      x1 = 1.0;
	      y1 = y2;
	      ix--;
	      break;
	    case CENTER:
	      weight = s/s0 + 0.5;
	      weight = exp(-weight*weight + 0.25)*FACTOR*(s0 - s);
	      value += src.f[rindex]*weight;
	      s = s0;
	      continue;
	  } /* end of switch (c) */
	  vx = vx0[index];
	  vy = vy0[index];
	  if (count) {
	    vx = -vx;
	    vy = -vy;
	  }
	  value += src.f[rindex]*weight*ds;
	  rindex += di;
	  index += di;
	  ws += weight*ds;
	  s += ds;
	} /* end of while (d < DONE) */
      } /* end of while (count--) */
      if (normalize)
	value /= ws;
      *trgt.f = value;
    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
  return iq;
}
/*--------------------------------------------------------------------*/
