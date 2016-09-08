/* This is file orientation.cc.

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
/* File orientation.c */
/* LUX routines for determining 2D and 3D orientation. */
#include <math.h>
#include <string.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "action.hh"

int32_t	lux_replace(int32_t, int32_t);
/*--------------------------------------------------------------------*/
#define SQRT3	1.7320508075688772935
int32_t lux_orientation(int32_t narg, int32_t ps[])
/* determine local orientation in a two- or three-dimensional data array */
/* Syntax: ORIENTATION,data,widths[,orientation,values,wavenumber, */
/*                                  grid,order] */
/* <data>   = 2D or 3D data array (input) */
/* <widths> = Gaussian smoothing witdths for all dimensions (input) */
/* <orientation> = orientation angle (2D) or x and y components of the */
/*            orientation (with the z component set to unity) (3D) (output) */
/* <values> = the eigenvalues of the "inertia" tensor (output) */
/* <wavenumber> = wavenumber indicating characteristic scale (output) */
/* <grid>   = dimensions of output cube.  If unspecified, then the */
/*            output cube has the same dimensions as the input cube. */
/* <order>  = approximation order of the discrete derivative (1 - 4) */
/* <aspect> = aspect ratio of the pixels (2D) or the dimensions of a */
/*            pixel (3D) */
/* LS 24mar95 */
/* Method:  local Fourier transforms yield local direction in least */
/*          squares fashion.  Calculations can be performed totally */
/*          in the spatial domain by calculating symmetric "inertia */
/*          tensor" J and finding the eigenvector corresponding to */
/*          the largest eigenvalue.  B. Jaehne "Digital Image */
/*          Processing; Concepts, Algorithms, and Scientific */
/*          Applications", 2nd edition, Springer Verlag, 1993 */
/* see at end of file for more info on the method */
{
  double	j11, j22, j33, j12, j13, j23, gg;
  float	*out, *comp, *widths, *data, *wave;
  float	*smooth1, *smooth2, *smooth3;
  float	x, sum, *ptr, *ptr2, y, d1, d2, d3, *ptr3, *ptr0, *aspect;
  double	c0, c1, c2;
  double	q, r, ang, c, s, w;
  int32_t	values, vector, d, *dims, i, iq, n1, n2, n3, odims[4], j, t, *grid;
  int32_t	step[4], w1, w2, w3, m1, m2, m3, i2, j2, t2, grid2[3], i3, j3, t3;
  int32_t	order, wavenum, ndim, xdims[3];
  char	vocal, getVal, getVec, getJ, getWave, parallel;

  /* First, treat input data */
  if (narg > 2 && ps[2])	/* ORIENTATION */
    getVec = 1;
  else
    getVec = 0;
  if (narg > 3 && ps[3])	/* VALUES */
    getVal = 1;
  else
    getVal = 0;
  if (narg > 4 && ps[4])	/* WAVENUMBER */
    getWave = 1;
  else
    getWave = 0;
  if (!getVec && !getVal && !getWave)
    return luxerror("No output symbols specified - nothing to calculate", 0);
  if (symbol_class(*ps) != LUX_ARRAY) /* DATA */
    return cerror(NEED_ARR, *ps);
  ndim = array_num_dims(*ps);
  if (ndim != 2 && ndim != 3)
    return luxerror("Need 2D or 3D array", *ps);
  dims = array_dims(*ps);
  iq = lux_float(1, ps);
  data = (float *) array_data(iq);
  if (symbol_class(ps[1]) != LUX_ARRAY) /* WIDTHS */
    return cerror(NEED_ARR, ps[1]);
  d = array_size(ps[1]);
  if (d != ndim)
    return luxerror("Need %1d smooth widths", ps[1], ndim);
  iq = lux_float(1, &ps[1]);
  widths = (float *) array_data(iq);
  for (i = 0; i < ndim; i++)
    if (widths[i] < 0)
      return luxerror("Need nonnegative smoothing widths", ps[1]);
  if (narg > 5 && ps[5]) {	/* GRID */
    iq = lux_long(1, &ps[5]);
    if (array_size(iq) != ndim)
      return luxerror("Need %1d output cube dimensions", ps[5], ndim);
    grid = (int32_t *) array_data(iq);
    memcpy(grid2, grid, ndim*sizeof(float));
    if (grid[0] == 0)
      grid2[0] = dims[0];
    else if (grid[0] < 0 || grid[0] > dims[0])
      return luxerror("Illegal output cube x dimension: %d", ps[5], grid[0]);
    if (grid[1] == 0)
      grid2[1] = dims[1];
    else if (grid[1] < 0 || grid[1] > dims[1])
      return luxerror("Illegal output cube y dimension: %d", ps[5], grid[1]);
    if (ndim == 3) {
      if (grid[2] == 0)
	grid2[2] = dims[2];
      else if (grid[2] < 0 || grid[2] > dims[2])
	return luxerror("Illegal output cube y dimension: %d", ps[5], grid[2]);
    }
  } else
    memcpy(grid2, dims, ndim*sizeof(float));
  if (narg > 6 && ps[6]) {	/* ASPECT */
    if (symbol_class(ps[6]) != LUX_ARRAY)
      return cerror(NEED_ARR, ps[6]);
    if (array_size(ps[6]) != ndim)
      return luxerror("Need %1d aspect sizes", ps[6], ndim);
    aspect = (float *) array_data(lux_float(1, ps + 6));
  } else
    aspect = NULL;
  if (narg > 7 && ps[7]) {	/* ORDER: discrete derivative accuracy */
    order = int_arg(ps[7]);
    if (order < 1 || order > 4)
      return luxerror("Approximation order must be between 1 and 4", ps[7]);
  } else
    order = 4;
  if (dims[0] < order || dims[1] < order
      || (ndim == 3 && dims[2] < order))
    return luxerror("Array dimensions must exceed approximation order", ps[0]);

  parallel = !(internalMode & 4);

  /* Create Gaussian smoothing kernels */
				/* X kernel */
  w = widths[0]*0.6005612;
  n1 = 4*((int32_t) w) + 1;
  if (n1 > dims[0]) {
    n1 = dims[0];
    if (n1 % 2 == 0)
      n1--;
  }
  ALLOCATE(smooth1, n1, float);
  y = w? 1./w: 0;
  x = -((int32_t) n1/2)*y;
  ptr = smooth1;
  sum = 0;
  for (i = 0; i < n1; i++) {
    *ptr = exp(-(x*x));
    x += y;
    sum += *ptr++;
  }
  ptr = smooth1;
  for (i = 0; i < n1; i++) {	/* normalize kernel so sum is one */
    *ptr = *ptr/sum;
    ptr++;
  }
				/* Y kernel */
  w = widths[1]*0.6005612;
  n2 = 4*((int32_t) w) + 1;
  if (n2 > dims[1]) {
    n2 = dims[1];
    if (n2 % 2 == 0)
      n2--;
  }
  ALLOCATE(smooth2, n2, float);
  y = w? 1./w: 0;
  x = -(n2/2)*y;
  ptr = smooth2;
  sum = 0;
  for (i = 0; i < n2; i++) {
    *ptr = exp(-(x*x));
    x += y;
    sum += *ptr++;
  }
  ptr = smooth2;
  for (i = 0; i < n2; i++) {
    *ptr = *ptr/sum;
    ptr++;
  }
				/* T kernel */
  if (ndim == 3) {
    w = widths[2]*0.6005612;
    n3 = 4*((int32_t) w) + 1;
    if (n3 > dims[2]) {
      n3 = dims[2];
      if (n3 % 2 == 0)
	n3--;
    }
    ALLOCATE(smooth3, n3, float);
    y = w? 1./w: 0;
    x = -(n3/2)*y;
    ptr = smooth3;
    sum = 0;
    for (i = 0; i < n3; i++) {
      *ptr = exp(-(x*x));
      x += y;
      sum += *ptr++;
    }
    ptr = smooth3;
    for (i = 0; i < n3; i++) {
      *ptr = *ptr/sum;
      ptr++;
    }
    n3 /= 2;
  }
  n1 /= 2;
  n2 /= 2;
				/* Create the output symbols */
  memcpy(odims + 1, grid2, ndim*sizeof(int32_t));
  if (getVal) {
    odims[0] = ndim;
    if ((values = array_scratch(LUX_FLOAT, ndim + 1, odims)) < 0) {
      free(smooth1);
      free(smooth2);
      if (ndim == 3)
	free(smooth3);
      return LUX_ERROR;
    }
    out = (float *) array_data(values);
  }
  if (getVec) {
    if (ndim == 3) {
      odims[0] = ndim;
      if ((vector = array_scratch(LUX_FLOAT, ndim + 1, odims)) < 0) {
	free(smooth1);
	free(smooth2);
	free(smooth3);
	return LUX_ERROR;
      }
    } else {
      if ((vector = array_scratch(LUX_FLOAT, ndim, odims + 1)) < 0) {
	free(smooth1);
	free(smooth2);
	return LUX_ERROR;
      }
    }
    comp = (float *) array_data(vector);
  }
  if (getWave) {
    if ((wavenum = array_scratch(LUX_FLOAT, ndim, odims + 1)) < 0) {
      free(smooth1);
      free(smooth2);
      if (ndim == 3)
	free(smooth3);
      return LUX_ERROR;
    }
    wave = (float *) array_data(wavenum);
  }
  vocal = (internalMode & 1);
  getJ = (internalMode & 2)? 1: 0;
  /* Now start the real work */
  memcpy(xdims, dims, ndim*sizeof(int32_t));
  if (ndim == 2) {
    xdims[2] = 1;
    grid2[2] = 1;
  }
  step[0] = 1;
  step[1] = step[0]*xdims[0];
  step[2] = step[1]*xdims[1];
  ptr2 = out;
  ptr3 = comp;
  for (t3 = xdims[2] - grid2[2]; t3 < 2*xdims[2]*grid2[2];
       t3 += 2*xdims[2]) {		/* all time frames */
    if (ndim == 3) {
      t = t3/grid2[2]/2;
      w3 = n3 + order - t;	/* first time frame in integration volume */
      if (w3 < 0)		/* extends before first time frame */
	w3 = 0;			/* adjust */
      m3 = xdims[2] + n3 - t - order; /* last time frame in volume */
      if (m3 > 2*n3 + 1)	/* extends after last time frame */
	m3 = 2*n3 + 1;		/* adjust */
      m3 -= w3;			/* number of time frames in volume */
    } else			/* 2D case */
      t = w3 = n3 = 0;
    for (j3 = xdims[1] - grid2[1]; j3 < 2*xdims[1]*grid2[1];
	 j3 += 2*xdims[1]) {	/* all rows */
      j = j3/grid2[1]/2;
      if (vocal) {
	if (ndim == 3)
	  printf("\rORIENTATION - working on time %1d of %1d, y %1d of %1d  ",
		 t, xdims[2] - 1, j, xdims[1] - 1);
	else
	  printf("\rORIENTATION - working on y %1d of %1d  ",
		 j, xdims[1] - 1);	  
	fflush(stdout);
      }
      w2 = n2 + order - j;	/* first row of integration volume */
      if (w2 < 0)		/* extends beyond left edge */
	w2 = 0;			/* adjust */
      m2 = xdims[1] + n2 - j - order; /* last row of volume */
      if (m2 > 2*n2 + 1)	/* extends beyond right edge */
	m2 = 2*n2 + 1;		/* adjust */
      m2 -= w2;			/* number of rows */
      ptr0 = data + (w2 - n2 + j)*step[1] + (w3 - n3 + t)*step[2];
      for (i3 = xdims[0] - grid2[0]; i3 < 2*xdims[0]*grid2[0];
	   i3 += 2*xdims[0]) {	/* all columns */
	i = i3/grid2[0]/2;
	w1 = n1 + order - i;	/* first column of integration volume */
	if (w1 < 0)		/* extends beyond edge of data cube */
	  w1 = 0;		/* adjust */
	m1 = xdims[0] + n1 - i - order; /* last column of volume */
	if (m1 > 2*n1 + 1)	/* extends beyond edge of data */
	  m1 = 2*n1 + 1;	/* adjust */
	m1 -= w1;		/* number of columns */

	if (ndim == 3) {	/* 3D case */
	  j11 = j22 = j33 = j12 = j13 = j23 = gg = 0; /* tensor element init */
	  ptr = ptr0 + w1 - n1 + i;
	  for (t2 = w3; t2 < w3 + m3; t2++) { /* integration volume times */
	    for (j2 = w2; j2 < w2 + m2; j2++) { /* all rows */
	      for (i2 = w1; i2 < w1 + m1; i2++) { /* all columns */
		switch (order) { /* discrete derivative order */
		  case 1:
		    d1 = ptr[1] - ptr[-1]; /* X derivative */
		    d2 = ptr[step[1]] - ptr[-step[1]]; /* Y derivative */
		    d3 = ptr[step[2]] - ptr[-step[2]]; /* T derivative */
		    break;
		  case 2:
		    d1 = (-ptr[2] + 8*ptr[1] - 8*ptr[-1] + ptr[-2])/12;
		    d2 = (-ptr[2*step[1]] + 8*ptr[step[1]] - 8*ptr[-step[1]]
			  + ptr[-2*step[1]])/12;
		    d3 = (-ptr[2*step[2]] + 8*ptr[step[2]] - 8*ptr[-step[2]]
			  + ptr[-2*step[2]])/12;
		    break;
		  case 3:
		    d1 = (ptr[3] - 9*ptr[2] + 45*ptr[1] - 45*ptr[-1]
			  + 9*ptr[-2] - ptr[-3])/60;
		    d2 = (ptr[3*step[1]] - 9*ptr[2*step[1]] + 45*ptr[step[1]]
			  - 45*ptr[-step[1]] + 9*ptr[-2*step[1]]
			  - ptr[-3*step[1]])/60;
		    d3 = (ptr[3*step[2]] - 9*ptr[2*step[2]] + 45*ptr[step[2]]
			  - 45*ptr[-step[2]] + 9*ptr[-2*step[2]]
			  - ptr[-3*step[2]])/60;
		    break;
		  case 4:
		    d1 = (-3*ptr[4] + 32*ptr[3] - 168*ptr[2] + 672*ptr[1]
			  -672*ptr[-1] + 168*ptr[-2] - 32*ptr[-3]
			  + 3*ptr[-4])/840;
		    d2 = (-3*ptr[4*step[1]] + 32*ptr[3*step[1]]
			  - 168*ptr[2*step[1]] + 672*ptr[step[1]]
			  - 672*ptr[-step[1]] + 168*ptr[-2*step[1]]
			  - 32*ptr[-3*step[1]] + 3*ptr[-4*step[1]])/840;
		    d3 = (-3*ptr[4*step[2]] + 32*ptr[3*step[2]]
			  - 168*ptr[2*step[2]] + 672*ptr[step[2]]
			  - 672*ptr[-step[2]] + 168*ptr[-2*step[2]]
			  - 32*ptr[-3*step[2]] + 3*ptr[-4*step[2]])/840;
		    break;
		} /* end of switch (order) */
		if (aspect) {
		  d1 *= aspect[0];
		  d2 *= aspect[1];
		  d3 *= aspect[2];
		}
		s = smooth1[i2]*smooth2[j2]*smooth3[t2]; /* element weight */
		j11 += d1*d1*s;	/* update tensor elements */
		j22 += d2*d2*s;
		j33 += d3*d3*s;
		j12 += d1*d2*s;
		j13 += d1*d3*s;
		j23 += d2*d3*s;
		if (getWave)
		  gg += *ptr * *ptr * s;
		ptr++;
	      }	/* end of for (i2 = w1; ... ) */
	      ptr += step[1] - m1;
	    } /* end of for (j2 = w2; ... ) */
	    ptr += step[2] - step[1]*m2;
	  } /* end of for (t2 = w3; ... ) */
	  if (getVec || getVal || getJ || getWave) {
				/* adjust tensor elements on main diagonal */
	    q = j22 + j33;
	    r = j11 + j33;
	    s = j11 + j22;
	    if (getJ) {		/* user wants tensor elements */
	      if (getVal) {	/* space for the main diagonal */
		*ptr2++ = j11;
		*ptr2++ = j22;
		*ptr2++ = j33;
	      }
	      if (getVec) {	/* space for the off-diagonal ones */
		*ptr3++ = j12;
		*ptr3++ = j13;
		*ptr3++ = j23;
	      }
	      if (getWave)
		*wave++ = gg;
	    } else {		/* user wants orientation and/or eigenvalues */
	      /* calculate coefficients of characteristic polynomial */
	      j11 = q;
	      j22 = r;
	      j33 = s;
	      c0 = j11*j23*j23 + j22*j13*j13 + j33*j12*j12 - j11*j22*j33
		+ 2*j12*j13*j23;
	      c1 = j11*j22 + j11*j33 + j22*j33 - j12*j12 - j13*j13 - j23*j23;
	      c2 = -j11 - j22 - j33;
				/* find roots of polynomial: eigenvalues */
	      q = c2*c2/9 - c1/3;
	      r = (c1*c2 - 3*c0)/6 - c2*c2*c2/27;
	      s = q*q*q - r*r;
	      if (s < 0)
		s = 0;		/* the symmetrical matrix has only real */
				/* eigenvalues; if s < 0, then this must */
				/* be due to roundoff errors */
	      ang = atan2(sqrt(s),r)/3;
	      q = sqrt(q);
	      r = c2/3;
	      s = sin(ang);
	      c = sqrt(1 - s*s); /* cosine, always > 0 */
	      c0 = 2*q*c - r;
	      r = -q*c - r;
	      q = q*s*SQRT3;
	      c1 = r + q;
	      c2 = r - q;
				/* now c0, c1, c2 are the eigenvalues */
				/* put them in decending order */
	      if (c1 > c0) {
		q = c1;
		c1 = c0;
		c0 = q;
	      }
	      if (c2 > c0) {
		q = c2;
		c2 = c1;
		c1 = c0;
		c0 = q;
	      } else if (c2 > c1) {
		q = c2;
		c2 = c1;
		c1 = q;
	      }
	      if (getWave) {
		if (gg) {
		  q = 0.5*(j11 + j22 + j33) - c2; /* really c2 and not c0? */
		  if (q < 0)
		    q = 0;	/* must be a roundoff error */
		  gg = sqrt(q/gg);
		} else
		  gg = 0.0;
		*wave++ = gg;
	      }
	      if (getVec) {	/* user wants orientation */
		/* OK, now find eigenvector for largest eigenvalue */
		j11 -= c0;
		j22 -= c0;
		j33 -= c0;
		w = c2*c2*0.0001;
		s = j11*j22 - j12*j12;
		if (ABS(s) > w) {
		  q = j12*j23 + j22*j13;
		  r = j11*j23 + j12*j13;
		} else {
		  r = j11*j33 - j13*j13;
		  if (ABS(r) > w) {
		    q = j13*j23 + j33*j12;
		    s = j13*j12 + j11*j23;
		  } else {
		    q = j22*j33 - j23*j23;
		    r = j23*j13 + j33*j12;
		    s = j23*j12 + j22*j13;
		  }
		}
		*ptr3++ = q;	/* return components of eigenvector */
		*ptr3++ = r;
		*ptr3++ = s;
	      }	/* end of if (getVec) */
              if (getVal) {	/* user wants eigenvalues */
		q = (c1 + c2 - c0)/2; /* adjust eigenvalues */
                r = (c0 + c2 - c1)/2;
                s = (c0 + c1 - c2)/2;
                *ptr2++ = q;    /* return eigenvalues */
                *ptr2++ = r;
                *ptr2++ = s;
	      }
	    } /* end of if (getJ) else */
	  } /* end of if (getVec || getVal || getJ) else */
	} else { /* 2D case */
	  j11 = j22 = j12 = gg = 0; /* tensor element start values */
	  ptr = ptr0 + w1 - n1 + i; /* first integration element */
	  for (j2 = w2; j2 < w2 + m2; j2++) { /* all rows for integration */
	    for (i2 = w1; i2 < w1 + m1; i2++) { /* columns for integration */
	      switch (order) {	/* discrete derivative order */
		case 1:
		  d1 = ptr[1] - ptr[-1]; /* derivative in X */
		  d2 = ptr[step[1]] - ptr[-step[1]]; /* derivative in Y */
		  break;
		case 2:
		  d1 = (-ptr[2] + 8*ptr[1] - 8*ptr[-1] + ptr[-2])/12;
		  d2 = (-ptr[2*step[1]] + 8*ptr[step[1]] - 8*ptr[-step[1]]
			+ ptr[-2*step[1]])/12;
		  break;
		case 3:
		  d1 = (ptr[3] - 9*ptr[2] + 45*ptr[1] - 45*ptr[-1]
			+ 9*ptr[-2] - ptr[-3])/60;
		  d2 = (ptr[3*step[1]] - 9*ptr[2*step[1]] + 45*ptr[step[1]]
			- 45*ptr[-step[1]] + 9*ptr[-2*step[1]]
			- ptr[-3*step[1]])/60;
		  break;
		case 4:
		  d1 = (-3*ptr[4] + 32*ptr[3] - 168*ptr[2] + 672*ptr[1]
			-672*ptr[-1] + 168*ptr[-2] - 32*ptr[-3]
			+ 3*ptr[-4])/840;
		  d2 = (-3*ptr[4*step[1]] + 32*ptr[3*step[1]]
			- 168*ptr[2*step[1]] + 672*ptr[step[1]]
			- 672*ptr[-step[1]] + 168*ptr[-2*step[1]]
			- 32*ptr[-3*step[1]] + 3*ptr[-4*step[1]])/840;
		  break;
	      } /* end of switch (order) */
	      if (aspect) {
		d1 *= aspect[0];
		d2 *= aspect[1];
	      }
	      s = smooth1[i2]*smooth2[j2]; /* weight factor of this element */
	      j11 += d2*d2*s;	/* update tensor elements */
	      j22 += d1*d1*s;
	      j12 += d1*d2*s;
	      if (getWave)
		gg += *ptr * *ptr * s;
	      ptr++;
	    }
	    ptr += step[1] - m1;
	  } /* end of for (i2 = w1; ... ) */
	  if (getVec || getVal || getJ || getWave) {
	    if (getJ) {
	      if (getVal) {
		*ptr2++ = j11;
		*ptr2++ = j22;
	      }
	      if (getVec)
		*ptr3++ = j12;
	      if (getWave)
		*wave++ = gg;
	    } else { /* user wants orientation and/or eigenvalues */
	    /* we must determine the eigenvalues of the matrix */
	    /*   j11 j12 */
	    /*   j12 j22 */
	      /* calculate coefficients of characteristic polynomial */
	      c0 = j11*j22 - j12*j12;
	      c1 = -j11 - j22;
				/* find roots of polynomial: eigenvalues */
	      q = c1*c1 - 4*c0;
	      if (q < 0)
		q = 0;		/* the symmetrical matrix has only real */
				/* roots, so q < 0 must be due to */
				/* roundoff errors */
	      q = sqrt(q);
	      if (c1 < 0)
		q = -q;
	      q = -(c1 + q)/2;	/* first solution */
	      r = q? c0/q: 0.0;	/* second solution */
	      /* put them in descending order */
	      if (r > q) {
		s = r;
		r = q;
		q = s;
	      }
	      if (getVal) {	/* the user wants the eigenvalues */
		*ptr2++ = q;
		*ptr2++ = r;
	      }
	      /* OK, now find angle */
	      if (getVec)	/* the user wants the orientation */
		*ptr3++ = 0.5*atan2(-2*j12, j11 - j22);
	      if (getWave) {	/* user wants wavenumber */
		if (gg) {
		  q = j11 + j22 - (parallel? q: r);
		  if (q < 0)
		    q = 0;	/* must be a roundoff error */
		  gg = sqrt(q/gg);
		} else
		  gg = 0.0;
		*wave++ = gg;
	      }
	    } /* end of if (getJ) else */
	  } /* end of if (getVec || getVal || getJ) */
	} /* end of if (ndim == 3) else */
      }	/* end of for (i3 = xdims[0] - grid2[0]; ... ) */
    } /* end of for (j3 = xdims[1] - grid2[1]; ... ) */
  } /* end of for (t3 = xdims[2] - grid2[2]; ... ) */
  if (vocal)
    printf("\n");
  if (getVec)
    lux_replace(ps[2], vector);
  if (getVal)
    lux_replace(ps[3], values);
  if (getWave)
    lux_replace(ps[4], wavenum);
  free(smooth1);
  free(smooth2);
  if (ndim == 3)
    free(smooth3);
  return 1;
}

int32_t lux_root3(int32_t narg, int32_t ps[])
{
  float	c0, c1, c2, q, r, s, c, ang, *p;
  int32_t	iq;

  c0 = float_arg(*ps++);
  c1 = float_arg(*ps++);
  c2 = float_arg(*ps);
  r = (c1*c2 - 3*c0)/6 - c2*c2*c2/27;
  q = c2*c2/9 - c1/3;
  s = q*q*q - r*r;
  if (s < 0)
  { puts("orientation3d - complex conjugate roots from a symmetric real matrix??");
    printf("s = %g\n", s);
    puts("setting s to 0.");
    s = 0; }
  ang = atan2(sqrt(s),r)/3;
  q = sqrt(q);
  r = c2/3;
  s = sin(ang);
  c = sqrt(1 - s*s);		/* cosine, always > 0 */
  c0 = 2*q*c - r;
  r = -q*c - r;  q = q*s*SQRT3;
  c1 = r + q;
  c2 = r - q;
  iq = 3;
  iq = array_scratch(LUX_FLOAT, 1, &iq);
  p = (float *) LPTR(HEAD(iq));
  *p++ = c0;  *p++ = c1;  *p++ = c2;
  return iq;
}
/*--------------------------------------------------------------------*/
/* The method:

I. The Direction

  Assume we have a Fourier transform g(k) of data G(x), with k the
  wavevector and x the position vector.  For notational convenience
  we define

   (1)  [f] = integral f(k) g(k)^2 dk,

  write the names of all unit vectors with capital letters, and
  indicate transposition with a prime.  We wish to fit a straight line
  to the power spectrum g(k)^2.  We do that by minimizing

   (2)  Q(K0) = [r_perp(k,K0)^2]

  where K0 is a unit vector, and r_perp is the perpendicular distance
  to a line through the origin with the direction equal to K0:

   (3)  r_perp(k,K0)^2 = |k - (k'K0)K0|^2 = K0'(Ik'k-kk')K0'

  with I a matrix with elements I_ij = delta_ij, with delta_ij the
  Kronecker delta symbol which is equal to 1 if i = j and equal to 0
  otherwise.  With (3) we rewrite (2) as

   (4)  Q(K0) = K0'JK0

  where J is a symmetric matrix with elements

   (5)  J_lm = [|k|^2 delta_lm - k_l k_m].

  J is symmetric so there is a coordinate transformation that renders
  it diagonal, with its eigenvalues along the diagonal.  The smallest
  eigenvalue is the smallest attainable value for Q, and the
  corresponding eigenvector is the associated direction.

  Terms of the form [k_l k_m] are inner products in Fourier space.
  Because inner products are conserved under unitary transformations
  such as the Fourier transform, we can also calculate the inner
  products in the spatial domain:

   (6)  [k_l k_m] = integrate k_l k_m g(k)^2 dk
                  = integrate (i k_l g(k))^* (i k_m g(k)) dk
                  = integrate (dG/dx_l)^* (dG/dx_m) dx
                  = s_lm

  where a superscript * indicates the complex conjugate, and s_lm is
  defined by the equation and is supposed to hide explicit reference
  to whether it was calculated in the spatial or the Fourier domain.
  We can now rewrite Eq. (5) to

   (7)  J_lm = sum_i s_ii delta_lm - s_lm

  which can be evaluated completely in the spatial domain.  One can
  find the eigenvalues of this matrix in the usual way, by setting the
  determinant |J - I lambda| = 0 and solving for lambda.  The
  eigenvalues are measures for the data gradient in the corresponding
  eigendirections.

II. The Wavenumber

  We calculate the local wavenumber w(S) in direction S from

   (8)  w(s)^2 = integrate (k'S)^2 g(k)^2 dk / integrate g(k)^2 dk,

  i.e., it is the root-mean-square value of the distance from the
  origin measured parallel to the chosen direction, and weighted with
  the power density.  This can be rewritten

   (9)  w(s)^2 = S'WS / N

  where

   (10)  W = [kk']
   (11)  N = integrate g(k)^2 dk

  W is similar to J except that J involves the distance perpendicular
  to the chosen direction, whereas W involves the distance parallel to
  the chosen direction.  The components of W are equal to

   (12)  W_lm = [k_l k_m] = s_lm

  J and W are related through

   (13)  J = I[k'k] - W = I sum_i s_ii - W

  With the non-zero eigenvectors U_i (normalized to unit length) we
  can construct arbitrary vectors

   (14)  V = sum_i a_i U_i

  for suitable a_i, with

   (15)  sum_i a_i^2 = 1

  For such a vector we have

   (16)  w(V)^2 = (sum_i s_ii - sum_j a_j^2 lambda_j)/N

  where lambda_j is the eigenvalue corresponding to U_j.  For a
  specific eigenvector, e.g., U_1, we find

   (17)  w(U_1)^2 = (sum_i s_ii - lambda_1)/N.
  
   */
