/* File strous3.c */
/* Various ANA routines by L. Strous */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <errno.h>              /* for errno */
#include "Bytestack.h"
#include "action.h"

int	to_scratch_array(int, int, int, int []);
/*---------------------------------------------------------------------*/
int ana_bisect(int narg, int ps[])
/* y = BISECT([x,] y, values [, AXIS=axis, POS=pos, WIDTH=width]) */
/* calculates bisector positions */
/* <axis> may only have a single dimension. */
/* The first dimension of <y> traces profiles to be checked for */
/* bisectors. */
/* <values> contains the levels at which bisector positions are to be
   returned. 
   <pos> contains the position (index to each profile) at which
   the search for each bisector should start.  (From such a position,
   the local minimum is sought, and bisectors are determined from there.)
   If <pos> is not defined, then bisectors are determined starting at
   the position of the absolute minimum in each profile.  <pos> must be
   a scalar.
   If <width> is specified, then the profile widths corresponding to the
   bisector positions are returned in it. */
/* LS 7may98 */
{
  int	result, iq, pos, nLev, outDims[MAX_DIMS], step,
    lev, xSym, ySym, vSym, il, ir;
  double	xl, xr, min, minpos, max, maxpos, x1l, x2l, x1r, x2r;
  pointer	src, trgt, level, ptr, rightedge, left, width, x;
  csplineInfo	cspl;
  loopInfo	srcinfo;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_NUM_ARR, ps[0]);

  cspl = empty_cubic_spline();

  if (narg > 2 && ps[2]) {
    xSym = ps[0];
    ySym = ps[1];
    vSym = ps[2];
  } else {
    xSym = 0;
    ySym = ps[0];
    vSym = ps[1];
  }

  if (standardLoop(ySym, (narg > 3 && ps[3])? ps[3]: 0,
		   SL_COMPRESS | SL_ONEAXIS | SL_NEGONED | SL_SRCUPGRADE
		   | SL_EACHROW | SL_AXISCOORD, ANA_FLOAT,
		   &srcinfo, &src, NULL, NULL, NULL) < 0) /* <data>, <axis> */
    return ANA_ERROR;		/* some error */

  if (xSym) {
    if (!symbolIsNumericalArray(xSym))
      return cerror(NEED_NUM_ARR, xSym);
    if (array_size(xSym) != srcinfo.rdims[0])
      return cerror(INCMP_ARG, ySym);
    iq = ana_converts[srcinfo.type](1, &xSym);
    x.f = array_data(iq);
  } else
    x.f = NULL;

  if (!symbolIsNumerical(vSym)) /* <levels> */
    return ANA_ERROR;
  iq = ana_converts[srcinfo.type](1, &vSym); /* ensure proper type */
  numerical(iq, NULL, NULL, &nLev, &level);

  if (narg > 4 && ps[4]) { 	/* have <pos> */
    pos = int_arg(ps[4]);
    if (pos < 0 || pos >= srcinfo.rdims[0])
      return anaerror("Index out of range", ps[4]);
  } else				/* no <pos> */
    pos = -1;

  /* create output symbol */
  if (nLev > 1) {
    outDims[0] = nLev;
    memcpy(outDims + 1, srcinfo.dims, srcinfo.raxes[0]*sizeof(int));
    memcpy(outDims + srcinfo.raxes[0] + 1, srcinfo.dims + srcinfo.raxes[0] + 1,
	   (srcinfo.ndim - srcinfo.raxes[0] - 1)*sizeof(int));
    result = array_scratch(srcinfo.type, srcinfo.ndim, outDims);
    if (narg > 5 && ps[5])	/* have <width> */
      if (to_scratch_array(ps[5], srcinfo.type, srcinfo.ndim, outDims)
	  == ANA_ERROR)
	return ANA_ERROR;
  } else {
    if (srcinfo.ndim > 1) {
      memcpy(outDims, srcinfo.dims, srcinfo.raxes[0]*sizeof(int));
      memcpy(outDims + srcinfo.raxes[0], srcinfo.dims + srcinfo.raxes[0] + 1,
	     (srcinfo.ndim - srcinfo.raxes[0] - 1)*sizeof(int));
      result = array_scratch(srcinfo.type, srcinfo.ndim - 1, outDims);
      if (narg > 5 && ps[5])	/* have <width> */
	if (to_scratch_array(ps[5], srcinfo.type, srcinfo.ndim, outDims)
	    == ANA_ERROR)
	  return ANA_ERROR;
    } else {			/* only one return value */
      result = scalar_scratch(srcinfo.type);
      if (narg > 5 && ps[5]) {	/* have <width> */
	undefine(ps[5]);
	symbol_class(ps[5]) = ANA_SCALAR;
	scalar_type(ps[5]) = srcinfo.type;
      }	
    }
  }
  if (result < 0)
    return ANA_ERROR;
  switch (symbol_class(result)) {
    case ANA_ARRAY:
      trgt.f = (float *) array_data(result);
      if (narg > 5 && ps[5])
	width.f = (float *) array_data(ps[5]);
      else
	width.f = NULL;
      break;
    case ANA_SCALAR:
      trgt.f = &scalar_value(result).f;
      if (narg > 5 && ps[5])
	width.f = &scalar_value(ps[5]).f;
      else
	width.f = NULL;
      break;
  }

  step = srcinfo.step[0];
  
  /* now do the work */
  switch (srcinfo.type) {
    case ANA_FLOAT:
      do {
	rightedge.f = src.f + step*(srcinfo.rdims[0] - 1);
	if (pos >= 0) {
	  ptr.f = src.f + pos*step; /* start position */
	  /* now seek the local minimum */
	  if (ptr.f > src.f && ptr.f[-step] < *ptr.f)
	    while (ptr.f > src.f && ptr.f[-step] < *ptr.f)
	      ptr.f -= step;
	  else
	    while (ptr.f < rightedge.f && ptr.f[step] < *ptr.f)
	      ptr.f += step;
	} else {		/* find absolute minimum */
	  ptr.f = left.f = src.f;
	  do {
	    left.f += step;
	    if (*left.f < *ptr.f)
	      ptr.f = left.f;
	  } while (left.f < rightedge.f);
	}

	/* install table for cubic spline interpolation */
	cubic_spline_tables(x.f, srcinfo.type, 1,
			    src.f, srcinfo.type, step,
			    srcinfo.rdims[0], 0,
			    &cspl);

	/* the cubic spline may dip below the lower of the surrounding
	 specified data points: we must find the local minimum in
	 the cubic spline. */
	ir = (ptr.f - src.f)/step;
	il = ir - 1;
	if (x.f) {
	  x1l = x.f[ir - 1];
	  x1r = x2l = x.f[ir];
	  x2r = x.f[ir + 1];
	} else {
	  x1l = ir - 1;
	  x1r = x2l = ir;
	  x2r = ir + 1;
	}
	find_cspline_extremes(x1l, x2r, &minpos, &min, NULL, NULL, &cspl);

	/* the levels are assumed to be sorted in ascending order */
	for (lev = 0; lev < nLev; lev++) {
	  if (min > level.f[lev]) {
	    *trgt.f++ = -1.0;
	    if (width.f != NULL)
	      *width.f++ = 0.0;
	  } else {
	    if (*ptr.f > level.f[lev]) { /* the current level is above */
	      /* the minimum of the spline fit, but below the local minimum of
	       the tabular points */
	      xl = find_cspline_value(level.f[lev], x1l, minpos, &cspl);
	      xr = find_cspline_value(level.f[lev], minpos, x2r, &cspl);
	      *trgt.f = (xl + xr)/2;
	      if (width.f)
		*width.f++ = xr - xl;
	    } else {
	      if (il >= 0) {
		do {
		  find_cspline_extremes(x1l, x2l, NULL, NULL, &maxpos,
					&max, &cspl);
		  if (max < level.f[lev]) {
		    il--;
		    if (x.f) {
		      x1l = x.f[il];
		      x2l = x.f[il + 1];
		    } else {
		      x1l = il;
		      x2l = il + 1;
		    }
		  } else
		    break;
		} while (il >= 0);
		if (il >= 0)
		  xl = find_cspline_value(level.f[lev], maxpos, x2l, &cspl);
		else
		  xl = -DBL_MAX;
	      } else
		xl = -DBL_MAX;

	      if (ir < srcinfo.rdims[0]) { /* not yet at edge */
		do {
		  find_cspline_extremes(x1r, x2r, NULL, NULL, &maxpos,
					&max, &cspl);
		  if (max < level.f[lev]) {
		    ir++;
		    if (x.f) {
		      x1r = x.f[ir];
		      x2r = x.f[ir + 1];
		    } else {
		      x1r = ir;
		      x2r = ir + 1;
		    }
		  } else
		    break;
		} while (ir < srcinfo.rdims[0]);
		if (ir < srcinfo.rdims[0])
		  xr = find_cspline_value(level.f[lev], x1r, maxpos, &cspl);
		else
		  xr = -DBL_MAX;	/* flag: at edge */
	      } else
		xr = -DBL_MAX;

	      if (xl > -DBL_MAX && xr > -DBL_MAX) { /* not at edge */
		*trgt.f = (xl + xr)/2;
		if (width.f)
		  *width.f++ = xr - xl;
	      } else {
		if (width.f)
		  *width.f++ = 0;
		*trgt.f = -1;	/* not found */
	      }
	    }
	    trgt.f++;
	  } /* end of if (*ptr.f > level.f[lev]) else */
	} /* end of for (lev = 0; ...) */
	src.f += step*srcinfo.rdims[0];
      } while (advanceLoop(&srcinfo) < srcinfo.rndim);
      break;
    case ANA_DOUBLE:
      do {
	rightedge.d = src.d + step*(srcinfo.rdims[0] - 1);
	if (pos >= 0) {
	  ptr.d = src.d + pos*step; /* start position */
	  /* now seek the local minimum */
	  if (ptr.d > src.d && ptr.d[-step] < *ptr.d)
	    while (ptr.d > src.d && ptr.d[-step] < *ptr.d)
	      ptr.d -= step;
	  else
	    while (ptr.d < rightedge.d && ptr.d[step] < *ptr.d)
	      ptr.d += step;
	} else {		/* find absolute minimum */
	  ptr.d = left.d = src.d;
	  do {
	    left.d += step;
	    if (*left.d < *ptr.d)
	      ptr.d = left.d;
	  } while (left.d < rightedge.d);
	}

	/* install table for cubic spline interpolation */
	cubic_spline_tables(x.d, srcinfo.type, 1,
			    src.d, srcinfo.type, step,
			    srcinfo.rdims[0], 0,
			    &cspl);

	/* the cubic spline may dip below the lower of the surrounding
	 specified data points: we must find the local minimum in
	 the cubic spline. */
	ir = (ptr.d - src.d)/step;
	il = ir - 1;
	if (x.d) {
	  x1l = x.d[ir - 1];
	  x1r = x2l = x.d[ir];
	  x2r = x.d[ir + 1];
	} else {
	  x1l = ir - 1;
	  x1r = x2l = ir;
	  x2r = ir + 1;
	}
	find_cspline_extremes(x1l, x2r, &minpos, &min, NULL, NULL, &cspl);

	/* the levels are assumed to be sorted in ascending order */
	for (lev = 0; lev < nLev; lev++) {
	  if (min > level.d[lev]) {
	    *trgt.d++ = -1.0;
	    if (width.d != NULL)
	      *width.d++ = 0.0;
	  } else {
	    if (*ptr.d > level.d[lev]) { /* the current level is above */
	      /* the minimum of the spline fit, but below the local minimum of
	       the tabular points */
	      xl = find_cspline_value(level.d[lev], x1l, minpos, &cspl);
	      xr = find_cspline_value(level.d[lev], minpos, x2r, &cspl);
	      *trgt.d = (xl + xr)/2;
	      if (width.d)
		*width.d++ = xr - xl;
	    } else {
	      if (il >= 0) {
		do {
		  find_cspline_extremes(x1l, x2l, NULL, NULL, &maxpos,
					&max, &cspl);
		  if (max < level.d[lev]) {
		    il--;
		    if (x.d) {
		      x1l = x.d[il];
		      x2l = x.d[il + 1];
		    } else {
		      x1l = il;
		      x2l = il + 1;
		    }
		  } else
		    break;
		} while (il >= 0);
		if (il >= 0)
		  xl = find_cspline_value(level.d[lev], maxpos, x2l, &cspl);
		else
		  xl = -DBL_MAX;
	      } else
		xl = -DBL_MAX;

	      if (ir < srcinfo.rdims[0]) { /* not yet at edge */
		do {
		  find_cspline_extremes(x1r, x2r, NULL, NULL, &maxpos,
					&max, &cspl);
		  if (max < level.d[lev]) {
		    ir++;
		    if (x.d) {
		      x1r = x.d[ir];
		      x2r = x.d[ir + 1];
		    } else {
		      x1r = ir;
		      x2r = ir + 1;
		    }
		  } else
		    break;
		} while (ir < srcinfo.rdims[0]);
		if (ir < srcinfo.rdims[0])
		  xr = find_cspline_value(level.d[lev], x1r, maxpos, &cspl);
		else
		  xr = -DBL_MAX;	/* flag: at edge */
	      } else
		xr = -DBL_MAX;

	      if (xl > -DBL_MAX && xr > -DBL_MAX) { /* not at edge */
		*trgt.d = (xl + xr)/2;
		if (width.d)
		  *width.d++ = xr - xl;
	      } else {
		if (width.d)
		  *width.d++ = 0;
		*trgt.d = -1;	/* not found */
	      }
	    }
	    trgt.d++;
	  } /* end of if (*ptr.d > level.d[lev]) else */
	} /* end of for (lev = 0; ...) */
	src.d += step*srcinfo.rdims[0];
      } while (advanceLoop(&srcinfo) < srcinfo.rndim);
      break;
  default:
    break;
  }
  cleanup_cubic_spline_tables(&cspl);
  return result;
}
/*---------------------------------------------------------------------*/
static int cmp0(const void *a, const void *b)
{
  struct c { double v; int l; } aa, bb;
  int d;
  
  aa = *(struct c *) a;
  bb = *(struct c *) b;
  d = aa.l - bb.l;
  if (d)
    return d;
  else {
    double dl;
    dl = aa.v - bb.v;
    if (dl < 0)
      return -1;
    if (dl > 0)
      return +1;
  }
  return 0;
}
/*---------------------------------------------------------------------*/
int ana_cspline_find(int narg, int ps[])
/* z = CSPLINE_FIND(y, levels [, AXIS=axis, INDEX=index]) */
/* locates positions where a certain value gets attained, using cubic
   splines
   <y> = data values
   <levels> = the values to look for in the y coordinate
   <axis> = the single axis (of <y>) along which to look
   <index> = the indices into <z> where the data for each level begins

   The result <z> has its 1st dimension equal to the number of
   dimensions in <y> and its 2nd dimension equal to the number of
   found locations.  The locations are sorted by the level and secondarily
   by the coordinates.  The locations for level <level(i)> begin at
   index <index(i)> and run up to but not including index <index(i+1)>. */
/* LS 2009-08-09 */
{
  int	result, iq, nLev, lev, ySym, vSym, i, step, *index, j;
  pointer	src, level;
  csplineInfo	cspl;
  loopInfo	srcinfo;
  Bytestack b;
  struct c { double v; int l; int c; } *c;
  int csize;

  ySym = ps[0];		/* <y> */
  vSym = ps[1];		/* <values> */

  if (!symbolIsNumericalArray(ySym))
    return cerror(NEED_NUM_ARR, ySym);
  
  cspl = empty_cubic_spline();

  if (standardLoop(ySym, (narg > 2 && ps[2])? ps[2]: ANA_ZERO,
		   SL_NEGONED | SL_ONEAXIS | SL_SRCUPGRADE
		   | SL_AXISCOORD, ANA_FLOAT,
		   &srcinfo, &src, NULL, NULL, NULL) < 0) /* <data>, <axis> */
    return ANA_ERROR;		/* some error */
  
  if (!symbolIsNumerical(vSym)) /* <levels> */
    return ANA_ERROR;
  iq = ana_converts[srcinfo.type](1, &vSym); /* ensure proper type */

  numerical(iq, NULL, NULL, &nLev, &level);

  if (narg > 3 && ps[3]) {	/* <index> */
    if (to_scratch_array(ps[3], ANA_LONG, 1, &nLev) == ANA_ERROR)
      return ANA_ERROR;
    index = (int *) array_data(ps[3]);
    memset(index, 0, srcinfo.ndim*sizeof(int));
  } else
    index = NULL;
    
  /* we don't know beforehand how many output values there will be */
  b = Bytestack_create();	/* so store them on a byte stack */

  step = srcinfo.rsinglestep[0];

  /* we'll store the data as follows on the byte stack:
     1. the found location in the target dimension (double)
     2. the index of the level of which this is the location (int)
     3. the (one or more) coordinates of the location (int) */
  {
    struct c cc;
    csize = (byte *) &cc.c - (byte *) &cc.v + srcinfo.ndim*sizeof(int);
  }
  c = malloc(csize);

  /* now do the work */
  switch (srcinfo.type) {
    case ANA_FLOAT:
      do {
	/* install table for cubic spline interpolation */
	cubic_spline_tables(NULL, srcinfo.type, 1,
			    src.f, srcinfo.type, step,
			    srcinfo.rdims[0], 0,
			    &cspl);
	/* the levels are assumed to be sorted in ascending order */
	do {
	  if (!srcinfo.coords[0]) {
	    for (lev = 0; lev < nLev && *src.f > level.f[lev]; lev++) ;
	    /* now level.f[lev - 1] <= *src.f < level.f[lev] */
	  } else {
	    double z;

	    if (lev > 0 && *src.f < level.f[lev - 1]) {
	      /* passed a target level going down: determine &
		 remember detailed location */
	      z = find_cspline_value(level.f[lev - 1],
				     srcinfo.coords[0] - 1,
				     srcinfo.coords[0],
				     &cspl);
	      c->v = z;		/* store the found location */
	      c->l = --lev;	/* and the level index */
	      /* and after that the coordinates */
	      for (j = 0; j < srcinfo.ndim; j++)
		(&c->c)[srcinfo.raxes[j]] = srcinfo.coords[j];
	      Bytestack_push_data(b, c, (byte *) c + csize);
	    } else if (lev < nLev && *src.f >= level.f[lev]) {
	      /* passed a target level going up: determine &
		 remember detailed location */
	      z = find_cspline_value(level.f[lev],
				     srcinfo.coords[0] - 1,
				     srcinfo.coords[0],
				     &cspl);
	      c->v = z;
	      c->l = lev++;
	      for (j = 0; j < srcinfo.ndim; j++)
		(&c->c)[srcinfo.raxes[j]] = srcinfo.coords[j];
	      Bytestack_push_data(b, c, (byte *) c + csize);
	    }
	  }
	} while ((i = advanceLoop(&srcinfo)) == 0);
      } while (i < srcinfo.rndim);
      break;
  case ANA_DOUBLE:
      do {
	/* install table for cubic spline interpolation */
	cubic_spline_tables(NULL, srcinfo.type, 1,
			    src.d, srcinfo.type, step,
			    srcinfo.rdims[0], 0,
			    &cspl);
	/* the levels are assumed to be sorted in ascending order */
	do {
	  if (!srcinfo.coords[0]) {
	    for (lev = 0; lev < nLev && *src.d > level.d[lev]; lev++) ;
	    /* now level.d[lev - 1] <= *src.d < level.d[lev] */
	  } else {
	    double z;

	    if (lev > 0 && *src.d < level.d[lev - 1]) {
	      /* passed a target level going down: determine &
		 remember detailed location */
	      z = find_cspline_value(level.d[lev - 1],
				     srcinfo.coords[0] - 1,
				     srcinfo.coords[0],
				     &cspl);
	      c->v = z;
	      c->l = --lev;
	      for (j = 0; j < srcinfo.ndim; j++)
		(&c->c)[srcinfo.raxes[j]] = srcinfo.coords[j];
	      Bytestack_push_data(b, c, (byte *) c + csize);
	    } else if (lev < nLev && *src.d >= level.d[lev]) {
	      /* passed a target level going up: determine &
		 remember detailed location */
	      z = find_cspline_value(level.d[lev],
				     srcinfo.coords[0] - 1,
				     srcinfo.coords[0],
				     &cspl);
	      c->v = z;
	      c->l = lev++;
	      for (j = 0; j < srcinfo.ndim; j++)
		(&c->c)[srcinfo.raxes[j]] = srcinfo.coords[j];
	      Bytestack_push_data(b, c, (byte *) c + csize);
	    }
	  }
	} while ((i = advanceLoop(&srcinfo)) == 0);
      } while (i < srcinfo.rndim);
    break;
  default:
    break;
  }
  cleanup_cubic_spline_tables(&cspl);

  /* now we have <n> positions in the order in which we found them.
     We want to sort them first by the level and then by the position,
     so that we first have the positions for the first level, then
     those for the second level, and so on. */
  {
    int n;
    struct c *d;
    
    n = Bytestack_bytes(b, 0)/csize; /* number of found data points */
    if (n > 0) {
      Bytestack_index bi;
      union {
	struct c *c; 
	byte *b; 
      } q;

      d = (struct c *) Bytestack_peek(b, 0); /* beginning of data */
      qsort(d, n, csize, cmp0);
      
      /* create output symbol */
      bi = Bytestack_top(NULL);
      if (srcinfo.ndim > 1)
	Bytestack_push_var(NULL, srcinfo.ndim);
      Bytestack_push_var(NULL, n);
      result = array_scratch(ANA_DOUBLE, (srcinfo.ndim > 1? 2: 1),
			     (int *) Bytestack_pop(NULL, bi));
      src.d = array_data(result);
      q.c = d;
      for (i = 0; i < n; i++) {
	int j;
	for (j = 0; j < srcinfo.ndim; j++) {
	  if (j == srcinfo.raxes[0])
	    *src.d++ = q.c->v;
	  else
	    *src.d++ = ((&q.c->c)[j]);
	}
	q.b += csize;
      }
      if (index) {
	index[0] = 0;
	if (nLev > 1) {
	  lev = 0;
	  for (i = 0; i < n; i++) {
	    while (d->l != lev)
	      index[++lev] = i;
	    d = (struct c *) ((byte *)d + csize);
	  }
	}
      }
    } else
      result = ANA_MINUS_ONE;
  }
  
  Bytestack_delete(b);
  free(c);
  
  return result;
}
/*--------------------------------------------------------------------------*/
#ifdef WORDS_BIGENDIAN
#define SYNCH_OK	0xaaaa5555
#define SYNCH_REVERSE	0x5555aaaa
#else
#define SYNCH_OK	0x5555aaaa
#define SYNCH_REVERSE	0xaaaa5555
#endif
int ana_fitskey(int narg, int ps[])
/* FITSKEY(file, key) returns the value associated with the string <key> */
/* in FITS file <file>.  If <file> is a string, then it is taken as the
   name of the FITS file.  If <file> is a scalar, then its (integer) value
   is taken as the lun on which the FITS file is assumed to be opened.
   If a lun is specified, then the file pointer is left in the same position
   it was found in.
   Returns 0 if (1) the file cannot be opened */
/* for reading, (2) the file is not a FITS file (does not have "SIMPLE " */
/* as its first keyword), (3) the key is not a string, or (4) the key has */
/* more than 8 characters.  If the keyword is not found, then a 0 is */
/* returned.  The key is transformed to all uppercase before it is sought */
/* in the file.  LS 4jun98 */
{
  char	*file, *key, *key2, *scr, mustclose, ok;
  int	n, n2, i, evalString(char *, int), ptr, iq, i0, type;
  pointer	p;
  scalar	value;
  FILE	*fp;
  void	read_a_number(char **buf, scalar *value, int *type);

  switch (symbol_class(ps[0])) {
    case ANA_STRING:
      file = expand_name(string_value(ps[0]), NULL); /* full file name */
      fp = fopen(file, "r");
      if (!fp)
	return ANA_ZERO;
      mustclose = 1;		/* must close the file again when done */
      break;
    case ANA_SCALAR:
      i = int_arg(ps[0]);
      if (i < 0 || i >= MAXFILES)
	return ANA_ZERO;
      fp = ana_file[i];
      if (!fp)
	return ANA_ZERO;
      ptr = ftell(fp);		/* current file pointer position */
      fseek(fp, 0, SEEK_SET);
      mustclose = 0;		/* leave file open when done */
      break;
    default:
      return ANA_ZERO;
  }
  if (symbol_class(ps[1]) != ANA_STRING) { /* <key> must be a string */
    if (mustclose)
      fclose(fp);
    else
      fseek(fp, ptr, SEEK_SET);
    return ANA_ZERO;
  }
  n = string_size(ps[1]);
  if (n > 8) {
    if (mustclose)
      fclose(fp);
    else
      fseek(fp, ptr, SEEK_SET);
    return ANA_ZERO;
  }

  key = string_value(ps[1]);
  n2 = (n < 8)? n + 1: 8;	/* because we add a space if it fits */
  key2 = malloc(n2 + 1);
  strcpy(key2, key);
  if (n2 != n) {		/* add a space at the end to prevent */
				/* partial matches */
    key2[n] = ' ';
    key2[n + 1] = '\0';
  }
  for (key = key2; *key; key++)	/* make all uppercase */
    *key = toupper(*key);
  
  scr = curScrat;
  scr = fgets(scr, 9, fp);

  ok = 1;
  i0 = 0;			/* default offset is 0 */
  if (!scr)
    ok = 0;
  else if (strncmp(scr, "SIMPLE  ", 8)) { /* no expected FITS key */
    /* we'll accept an FZ file if its has a FITS header */
    p.s = scr;
    if (*p.l == SYNCH_OK) {	/* have an FZ file.  OK header? */
      fseek(fp, 256, SEEK_SET); /* to start of header */
      scr = fgets(scr, 9, fp);
      if (!scr || strncmp(scr, "SIMPLE  ", 8)) /* not OK */
	ok = 0;
      else			/* we have an FZ FITS header: adjust offset */
	i0 = 256;
    }
  }
  if (!ok) {			/* something was wrong */
    if (mustclose)
      fclose(fp);
    else
      fseek(fp, ptr, SEEK_SET);
    free(key2);
    return ANA_ZERO;
  }
  i = 0;

  do {
    if (fseek(fp, i*80 + i0, SEEK_SET)) {
      if (mustclose)
	fclose(fp);
      else
	fseek(fp, ptr, SEEK_SET);
      free(key2);
      return ANA_ZERO;
    }
    i++;
    scr = fgets(scr, 80, fp);
    if (!scr) {
      if (mustclose)
	fclose(fp);
      else
	fseek(fp, ptr, SEEK_SET);
      free(key2);
      return ANA_ZERO;
    }
  } while (strncmp(scr, "END ", 4) && strncmp(scr, key2, n2));

  if (mustclose)
    fclose(fp);
  else
    fseek(fp, ptr, SEEK_SET);
  free(key2);
  if (!strncmp(scr, "END ", 4))	/* found end of header but not the keyword */
    return ANA_ZERO;

  /* The FITS rules say that a comment is introduced by a forward slash.
     String constants are enclosed in single quotes ('' inside a string refers
     to a literal single quote), so we must look for the first forward slash
     outside of a string.  LS 14jan99 */
  /* if /COMMENT is specified, then we must return the comment value; */
  /* otherwise the data value.  Return comments always as strings. */
  /* LS 26may99 */
  scr += 9;			/* beginning of data value */
  n = 0;
  while (isspace((int) *scr))
    scr++;
  key = scr;
  while (*key) {
    switch (*key) {
      case '\'':		/* literal text string */
	n = !n;			/* toggle string status */
	break;
      case '/':			/* maybe FITS comment */
	if (!n) { 		/* not in a literal text string */
	  if (internalMode & 1) { /* /COMMENT */
	    scr = key + 1;	/* start reading here */
	    while (isspace((int) *scr))
	      scr++;		/* skip initial whitespace */
	    key = scr + strlen(scr) - 1; /* skip final whitespace */
	    while (key > scr && isspace((int) *key))
	      key--;
	    key[1] = '\0';
	    iq = string_scratch(strlen(scr));
	    strcpy(string_value(iq), scr);
	    return iq;
	  } else {
	    while (key > scr
		   && isspace((int) key[-1])) /* skip trailing spaces */
	      key--;
	    *key-- = '\0';	/* terminate data value */
	  }
	}
	break;
    }
    key++;
  }
  /* the data may be numerical, or a quote-enclosed string, or the values */
  /* T for true or F for false.  We return non-quoted non-numerical text */
  /* as a string */
  key = scr;
  switch (*key) {
    case '\'':			/* a string */
      scr = ++key;
      while (*key && *key != '\'') /* find the closing quote */
	key++;
      *key = '\0';		/* terminate */
      break;
    case '-': case '+':		/* may be a number */
      scr = key++;
      while (isspace((int) *key)) /* skip whitespace */
	key++;
      if (!isdigit((int) *key)) { /* treat it as a string */
	key += strlen(key);	/* go to the end of the string */
	while (key > scr
	       && isspace((int) key[-1])) /* skip trailing whitespace */
	  key--;
	break;
      }
      /* else we treat it as a number; fall through to the next case */
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
      read_a_number(&scr, &value, &type);
      iq = scalar_scratch(type);
      switch (type) {
	case ANA_BYTE:
	  scalar_value(iq).b = (byte) value.l;
	  break;
	case ANA_WORD:
	  scalar_value(iq).w = (word) value.l;
	  break;
	case ANA_LONG:
	  scalar_value(iq).l = value.l;
	  break;
	case ANA_FLOAT:
	  scalar_value(iq).f = (float) value.d;
	  break;
	case ANA_CFLOAT:
	  complex_scalar_data(iq).cf->real = 0.0;
	  complex_scalar_data(iq).cf->imaginary = (float) value.d;
	  break;
	case ANA_CDOUBLE:
	  complex_scalar_data(iq).cd->real = 0.0;
	  complex_scalar_data(iq).cd->imaginary = value.d;
	  break;
      }
      return iq;
      break;
  }
  
  iq = string_scratch(strlen(scr));
  strcpy(string_value(iq), scr); /* copy string */
  return iq;
}
/*--------------------------------------------------------------------------*/
#define LEFT	0
#define DOWN	1
#define RIGHT	2
#define UP	3
#define CENTER	4
#define DONE	5

int sign(float x)
{
  if (x > 0)
    return 1;
  else if (x < 0)
    return -1;
  else
    return 0;
}
/*--------------------------------------------------------------------*/
int sgnclass(float x)
{
  if (x > 0)
    return 2;
  else if (x < 0)
    return 0;
  else
    return 1;
}
/*--------------------------------------------------------------------*/
int traverseElement(float xin, float yin, float vx, float vy,
		    float *xout, float *yout)
/* if you start at position (<xin>,<yin>), with 0 <= <xin>,<yin> <= 1,
   and move in the direction given by (<vx>,<vy>), then this routine
   determines which pixel boundary you cross (UP, DOWN, LEFT, RIGHT,
   or CENTER if <vx> = <vy> = 0) and what the coordinates are.  The
   new coordinates are returned in <*xout> and <*yout> and the pixel
   boundary code is the return value of the routine.  LS 17oct98 */
/* We must make sure that the edges are treated properly, too. */
/* We do that by moving the edges outward a tiny bit so that our */
/* data point <xin>,<yin> is never actually one an edge. */
{
  if (vx > 0) {			/* to the right */
    /* first we check for UP.  The vector that separates UP from RIGHT
       has coordinates (1 - xin, 1 - yin); rotated counterclockwise
       over 90 degrees this becomes (yin - 1, 1 - xin): if the inner
       product of the velocity vector (vx,vy) with this vector is
       positive, then we are going UP.  The vector that separates
       RIGHT from DOWN is (1-xin,-yin), which leads to (yin, 1-xin). */
    if (vx*(yin - 1 - FLT_EPSILON) + vy*(1 + FLT_EPSILON - xin) > 0) {
      /* going UP */
      *xout = vy? xin + (1 - yin)/vy*vx: 1;
      *yout = 1;
      return UP;
    } else if (vx*(yin + FLT_EPSILON) + vy*(1 + FLT_EPSILON - xin) > 0) {
      /* we're going RIGHT */
      *xout = 1;
      *yout = vx? yin + (1 - xin)/vx*vy: 0;
      return RIGHT;
    } else {
      *xout = vy? xin - yin/vy*vx: 1;
      *yout = 0;
      return DOWN;
    }
  }

  if (vx < 0) {		/* to the left */
    /* first we check for DOWN.  The vector that separates DOWN from LEFT */
    /* has coordinates (-xin,-yin); rotated counterclockwise over 90 */
    /* degrees this becomes (yin,-xin): if the inner product of the velocity */
    /* vector (vx,vy) with this vector is positive, then we are going DOWN. */
    /* The vector that separates LEFT from UP is (-xin,1-yin), which leads */
    /* to (yin-1,-xin). */
    if (vx*(yin + FLT_EPSILON) - vy*(xin + FLT_EPSILON) > 0) {	/* DOWN */
      *xout = vy? xin - yin/vy*vx: 0;
      *yout = 0;
      return DOWN;
    } else if (vx*(yin + FLT_EPSILON - 1) - vy*(xin + FLT_EPSILON) > 0) {
      /* LEFT */
      *xout = 0;
      *yout = vx? yin - xin/vx*vy: 1;
      return LEFT;
    } else {			/* UP */
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
int ana_dir_smooth(int narg, int ps[])
/* Y = DSMOOTH(<data>,<vx>,<vy> [, /TWOSIDED, /ONESIDED, /BOXCAR, /GAUSSIAN,
               /NORMALIZE])
   smooths 2D image <data> in the direction indicated by the
   angle <vx> and <vy>, over a distance indicated by the magnitude of vector
   <v>.  The vector <vx>,<vy> indicates a flow field.  Data along the
   flow line through the data point under consideration is weighted into
   a result for the data point under consideration.  /TWOSIDED (the default)
   indicates that data on both sides of the data point under consideration
   must be weighted into the result.  /ONESIDED means that only data on
   the side indicated by the direction of the vector field at the point
   under consideration must be included.  /BOXCAR (default) indicates that
   boxcar smoothing is to be used; /GAUSSIAN selects gaussian smoothing
   instead.

LS 9nov98 */
{
  int	iq, nx, ny, ix, iy, c, index, rindex, count, twosided, total,
    gaussian, iq0, di, straight;
  float	x1, y1, x2, y2, *vx0, *vy0, value, vx, vy, s, s0, ds, dslimit,
    weight, ws, s1;
  pointer	src, trgt, src0;
  loopInfo	srcinfo, trgtinfo;

  iq0 = ps[0];			/* data */
  if (symbol_class(iq0) != ANA_ARRAY /* not an array */
      || array_num_dims(iq0) != 2) /* or doesn't have 2 dimensions */
    return cerror(NEED_2D_ARR, iq0);
  iq0 = ana_float(1, &iq0);
  nx = array_dims(iq0)[0];
  ny = array_dims(iq0)[1];

  iq = ps[1];			/* vx */
  if (symbol_class(iq) != ANA_ARRAY
      || array_num_dims(iq) != 2
      || array_dims(iq)[0] != nx
      || array_dims(iq)[1] != ny)
    return cerror(INCMP_ARG, iq);
  iq = ana_float(1, &iq);
  vx0 = array_data(iq);

  iq = ps[2];			/* vy */
  if (symbol_class(iq) != ANA_ARRAY
      || array_num_dims(iq) != 2
      || array_dims(iq)[0] != nx
      || array_dims(iq)[1] != ny)
    return cerror(INCMP_ARG, iq);
  iq = ana_float(1, &iq);
  vy0 = array_data(iq);

  if (standardLoop(iq0, 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT | SL_EACHCOORD,
		   ANA_FLOAT, &srcinfo, &src, &iq, &trgtinfo, &trgt) < 0)
    return ANA_ERROR;
  src0.f = src.f;

  twosided = ((internalMode & 1) == 0); /* /TWOSIDED */
  total = (internalMode & 4);	/* /TOTAL */
  gaussian = (internalMode & 2); /* /GAUSSIAN */
  straight = (internalMode & 16); /* /STRAIGHT */

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

	s0 = 0.5*hypot(vx, vy); /* length indicates smoothing width */
	s1 = s0;
	s = 0.0;
	
	while (s < s1) {
	  c = traverseElement(x1, y1, vx, vy, &x2, &y2);
	  /* calculate distance inside the current pixel */
	  x1 -= x2;
	  y1 -= y2;
	  ds = hypot(x1, y1);
	  if (s + ds > s1)
	    ds = s0 - s;
	  dslimit = 0.5*(dslimit + ds);
	  if (dslimit < 0.2) {	/* we fear a semi-infinite loop here */
	    value += src.f[rindex]*(s1 - s);
	    s = s1;		/* we break it off */
	    continue;
	  }
	  switch (c) {
	    case UP:
	      if (iy == ny - 1) { /* already at top */
		value += src.f[rindex]*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = nx;
	      x1 = x2;
	      y1 = 0.0;
	      iy++;
	      break;
	    case RIGHT:
	      if (ix == nx - 1) { /* already at right edge */
		value += src.f[rindex]*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = 1;
	      x1 = 0.0;
	      y1 = y2;
	      ix++;
	      break;
	    case DOWN:
	      if (iy == 0) {	/* already at bottom */
		value += src.f[rindex]*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = -nx;
	      x1 = x2;
	      y1 = 1.0;
	      iy--;
	      break;
	    case LEFT:
	      if (ix == 0) { 	/* already at left edge */
		value += src.f[rindex]*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = -1;
	      x1 = 1.0;
	      y1 = y2;
	      ix--;
	      break;
	    case CENTER:
	      value += src.f[rindex]*(s1 - s);
	      s1 = s;
	      continue;
	  } /* end of switch (c) */
	  value += src.f[rindex]*ds;
	  index += di;
	  rindex += di;
	  s += ds;
	  if (!straight) {
	    vx = vx0[index];
	    vy = vy0[index];
	    if (count) {
	      vx = -vx;
	      vy = -vy;
	    }
	  } /* end of if (straight) */
	} /* end of while (s < s0) */
      } /* end of while (count--) */
      if (!total) {
	value /= s1;
	if (twosided)
	  value *= 0.5;
      }
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
      ws = 0.0;
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

	s0 = 0.6005612*hypot(vx, vy); /* smoothing width */
	s = 0.0;
	s1 = 4*s0;
	
	while (s < s1) {
	  c = traverseElement(x1, y1, vx, vy, &x2, &y2);
	  /* calculate distance inside the current pixel */
	  x1 -= x2;
	  y1 -= y2;
	  ds = hypot(x1, y1);
	  if (s + ds > s1)
	    ds = s1 - s;
	/* we must determine the weight to assign to the contributions.
	   if gsmooth is used in one dimension, then the weights of the
	   contributions are equal to exp(-(x/s0)^2) where x is the
	   integer index of the data point relative to the point under
	   consideration.  However, if this routine is used to mimic gsmooth,
	   then s is only equal to 0.5 when x = +1 is reached, so
	   using exp(-(s/s0)^2) with s the total distance from the central
	   point does not yield good results.  We can't just add 0.5 to
	   s from the beginning either, because we do want a weight of 1
	   for the first included data. */
	  weight = s? (s + 0.5)/s0: 0.0;
	  weight = exp(-weight*weight);
	  dslimit = 0.5*(dslimit + ds);
	  if (dslimit < 0.2) {	/* we fear a semi-infinite loop here */
	    ds = 0.5;
	    if (s + ds > s1)
	      ds = s1 - s;
	  }
	  switch (c) {
	    case UP:
	      if (iy == ny - 1) { /* already at top */
		value += src.f[rindex]*weight*ds;
		ws += weight*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = nx;
	      x1 = x2;
	      y1 = 0.0;
	      iy++;
	      break;
	    case RIGHT:
	      if (ix == nx - 1) { /* already at right edge */
		value += src.f[rindex]*weight*ds;
		ws += weight*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = 1;
	      x1 = 0.0;
	      y1 = y2;
	      ix++;
	      break;
	    case DOWN:
	      if (iy == 0) {	/* already at bottom */
		value += src.f[rindex]*weight*ds;
		ws += weight*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = -nx;
	      x1 = x2;
	      y1 = 1.0;
	      iy--;
	      break;
	    case LEFT:
	      if (ix == 0) {	/* already at left edge */
		value += src.f[rindex]*weight*ds;
		ws += weight*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = -1;
	      x1 = 1.0;
	      y1 = y2;
	      ix--;
	      break;
	    case CENTER:
	      di = 0;
	      break;
	  } /* end of switch (c) */
	  value += src.f[rindex]*weight*ds;
	  rindex += di;
	  index += di;
	  ws += weight*ds;
	  s += ds;
	  if (!straight) {
	    vx = vx0[index];
	    vy = vy0[index];
	    if (count) {
	      vx = -vx;
	      vy = -vy;
	    } /* end of if (count) */
	  } /* end of if (straight) */
	} /* end of while (d < DONE) */
      } /* end of while (count--) */
      if (!total)
	value /= ws;
      *trgt.f = value;
    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
  return iq;
}
/*--------------------------------------------------------------------*/
int ana_dir_smooth2(int narg, int ps[])
/* Y = LSMOOTH(<data>,<vx>,<vy>)
   smooths 2D image <data> in the direction indicated by the
   angle <vx> and <vy>, over a distance indicated by the magnitude of vector
   <v>. */
{
  int	iq, nx, ny, ix, iy, c, index, rindex, count, twosided, normalize,
    gaussian, iq0, di, straight;
  float	x1, y1, x2, y2, *vx0, *vy0, vx, vy, s, s0, ds, dslimit,
    weight, ws, s1, norm;
  pointer	src, trgt, src0;
  loopInfo	srcinfo, trgtinfo;

  iq0 = ps[0];			/* data */
  if (symbol_class(iq0) != ANA_ARRAY /* not an array */
      || array_num_dims(iq0) != 2) /* or doesn't have 2 dimensions */
    return cerror(NEED_2D_ARR, iq0);
  iq0 = ana_float(1, &iq0);
  nx = array_dims(iq0)[0];
  ny = array_dims(iq0)[1];

  iq = ps[1];			/* vx */
  if (symbol_class(iq) != ANA_ARRAY
      || array_num_dims(iq) != 2
      || array_dims(iq)[0] != nx
      || array_dims(iq)[1] != ny)
    return cerror(INCMP_ARG, iq);
  iq = ana_float(1, &iq);
  vx0 = array_data(iq);

  iq = ps[2];			/* vy */
  if (symbol_class(iq) != ANA_ARRAY
      || array_num_dims(iq) != 2
      || array_dims(iq)[0] != nx
      || array_dims(iq)[1] != ny)
    return cerror(INCMP_ARG, iq);
  iq = ana_float(1, &iq);
  vy0 = array_data(iq);

  if (standardLoop(iq0, 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT | SL_EACHCOORD,
		   ANA_FLOAT, &srcinfo, &src, &iq, &trgtinfo, &trgt) < 0)
    return ANA_ERROR;
  src0.f = src.f;

  twosided = ((internalMode & 1) == 0); /* /TWOSIDED */
  normalize = (internalMode & 4)? 1: 0;	/* /NORMALIZE */
  gaussian = (internalMode & 2)? 1: 0; /* /GAUSSIAN */
  straight = (internalMode & 8);

  zerobytes(trgt.f, array_size(iq)*sizeof(float)); /* set to zero */

  if (!gaussian) {		/* boxcar */
    if (!normalize)
      norm = 1.0;
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

	s0 = 0.5*hypot(vx, vy); /* smoothing width */
	s1 = s0;
	s = 0.0;
	if (normalize)
	  norm = s0? 0.5/s0: 1.0;
	
	while (s < s1) {
	  c = traverseElement(x1, y1, vx, vy, &x2, &y2);
	  /* calculate distance inside the current pixel */
	  x1 -= x2;
	  y1 -= y2;
	  ds = hypot(x1, y1);
	  if (s + ds > s1)
	    ds = s0 - s;
	  dslimit = 0.5*(dslimit + ds);
	  if (dslimit < 0.2) {	/* we fear a semi-infinite loop here */
	    trgt.f[rindex] += *src.f*(s1 - s)*norm;
	    s = s1;		/* we break it off */
	    continue;
	  }
	  switch (c) {
	    case UP:
	      if (iy == ny - 1) { /* already at top */
		trgt.f[rindex] += *src.f*ds*norm;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = nx;
	      x1 = x2;
	      y1 = 0.0;
	      iy++;
	      break;
	    case RIGHT:
	      if (ix == nx - 1) { /* already at right edge */
		trgt.f[rindex] += *src.f*ds*norm;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = 1;
	      x1 = 0.0;
	      y1 = y2;
	      ix++;
	      break;
	    case DOWN:
	      if (iy == 0) {	/* already at bottom */
		trgt.f[rindex] += *src.f*ds*norm;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = -nx;
	      x1 = x2;
	      y1 = 1.0;
	      iy--;
	      break;
	    case LEFT:
	      if (ix == 0) { 	/* already at left edge */
		trgt.f[rindex] += *src.f*ds*norm;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = -1;
	      x1 = 1.0;
	      y1 = y2;
	      ix--;
	      break;
	    case CENTER:
	      trgt.f[rindex] += *src.f*(s1 - s)*norm;
	      s1 = s;
	      continue;
	  } /* end of switch (c) */
	  trgt.f[rindex] += *src.f*ds*norm;
	  index += di;
	  rindex += di;
	  s += ds;
	  if (!straight) {
	    vx = vx0[index];
	    vy = vy0[index];
	    if (count) {
	      vx = -vx;
	      vy = -vy;
	    }
	  }
	} /* end of while (s < s0) */
      } /* end of while (count--) */
    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
  } else {			/* gaussian smoothing */
    norm = 0.5*M_2_SQRTPI;
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
      ws = 0.0;
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

	s0 = 0.6005612*hypot(vx, vy);	/* smoothing width */
	s = 0.0;
	s1 = 4*s0;

	if (normalize)
	  norm = s0? (0.5*M_2_SQRTPI)/s0: (0.5*M_2_SQRTPI);
	
	while (s < s1) {
	  c = traverseElement(x1, y1, vx, vy, &x2, &y2);
	  /* calculate distance inside the current pixel */
	  x1 -= x2;
	  y1 -= y2;
	  ds = hypot(x1, y1);
	  if (s + ds > s1)
	    ds = s1 - s;
	/* we must determine the weight to assign to the contributions.
	   if gsmooth is used in one dimension, then the weights of the
	   contributions are equal to exp(-(x/s0)^2) where x is the
	   integer index of the data point relative to the point under
	   consideration.  However, if this routine is used to mimic gsmooth,
	   then s is only equal to 0.5 when x = +1 is reached, so
	   using exp(-(s/s0)^2) with s the total distance from the central
	   point does not yield good results.  We can't just add 0.5 to
	   s from the beginning either, because we do want a weight of 1
	   for the first included data. */
	  weight = s? (s + 0.5)/s0: 0.0;
	  weight = exp(-weight*weight);
	  dslimit = 0.5*(dslimit + ds);
	  if (dslimit < 0.2) {	/* we fear a semi-infinite loop here */
	    ds = 0.5;
	    if (s + ds > s1)
	      ds = s1 - s;
	  }
	  switch (c) {
	    case UP:
	      if (iy == ny - 1) { /* already at top */
		trgt.f[rindex] += *src.f*ds*weight*norm;
		ws += weight*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = nx;
	      x1 = x2;
	      y1 = 0.0;
	      iy++;
	      break;
	    case RIGHT:
	      if (ix == nx - 1) { /* already at right edge */
		trgt.f[rindex] += *src.f*ds*weight*norm;
		ws += weight*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = 1;
	      x1 = 0.0;
	      y1 = y2;
	      ix++;
	      break;
	    case DOWN:
	      if (iy == 0) {	/* already at bottom */
		trgt.f[rindex] += *src.f*ds*weight*norm;
		ws += weight*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = -nx;
	      x1 = x2;
	      y1 = 1.0;
	      iy--;
	      break;
	    case LEFT:
	      if (ix == 0) {	/* already at left edge */
		trgt.f[rindex] += *src.f*ds*weight*norm;
		ws += weight*ds;
		s += ds;
		s1 = s;
		continue;
	      }
	      di = -1;
	      x1 = 1.0;
	      y1 = y2;
	      ix--;
	      break;
	    case CENTER:
	      di = 0;
	      break;
	  } /* end of switch (c) */
	  trgt.f[rindex] += *src.f*weight*ds*norm;
	  rindex += di;
	  index += di;
	  ws += weight*ds;
	  s += ds;
	  if (!straight) {
	    vx = vx0[index];
	    vy = vy0[index];
	    if (count) {
	      vx = -vx;
	      vy = -vy;
	    }
	  }
	} /* end of while (d < DONE) */
      } /* end of while (count--) */
    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
  }
  return iq;
}
/*--------------------------------------------------------------------*/
int ana_trajectory(int narg, int ps[])
/* TRAJECTORY,<gx>,<gy>,<vx>,<vy>[,<n>][,<ox>,<oy>])
  Takes the positions indicated by <gx>,<gy> and advances them for <n>
  time steps according to the velocity field in <vx>,<vy>.  The first
  four arguments must be real arrays, and <n> must be a scalar and
  defaults to 1.  <gx> and <gy> must have the same number of elements.
  <vx> must have two dimensions.  <vy> must have the same dimensions
  as <vx>.  The first element of <vx> is assumed to reside at <gx>,<gy>
  coordinates equal to (0,0); the second element at (1,0), etcetera.
  The results are returned in <ox> and <oy> and get the
  dimensions of <gx> plus one extra dimension equal to <n> at the beginning
  (except if <n> is equal to 1).  <n> must be positive (after
  transformation to a LONG scalar).  If <n> exceeds 1, then <ox> and <oy>
  must be specified.  Otherwise, if <ox> and <oy> are not specified,
  then the results are stored in <gx> and <gy>.  LS 15-16dec99
*/
 /* the following argument lists are valid:
     gx,gy, v
     gx,gy,vx,vy
     gx,gy, v,ox,oy
     gx,gy, v, n,ox,oy
     gx,gy,vx,vy,ox,oy
     gx,gy,vx,vy, n,ox,oy
  */
{
  int	iq, nx, ny, ix, iy, c, index, di, n, i, dims[MAX_DIMS],
    ngrid, type, dv;
  float	x1, y1, x2, y2, vx, vy, s, s0, ds, dslimit, s1;
  pointer	gx, gy, vx0, vy0, ox, oy;

  /* we treat all arguments. */
  if (!symbolIsRealArray(ps[0]))/* <gx> must be a real array */
    return cerror(ILL_CLASS, ps[0]);
  ngrid = array_size(ps[0]);	/* number of grid points */
  if (!symbolIsRealArray(ps[1]) || /* <gy> must be a real array */
      array_size(ps[1]) != ngrid) /* with the same size as <gx> */
    return cerror(INCMP_ARG, ps[1]);

  /* we keep track of the greatest data type */
  type = array_type(ps[0]);
  if (array_type(ps[1]) > type)
    type = array_type(ps[1]);

  if (!symbolIsRealArray(ps[2])) /* <vx> must be a real array */
    return cerror(NEED_ARR, ps[2]);
  if (array_type(ps[2]) > type)
    type = array_type(ps[2]);

  if (narg == 3 || narg == 5
      || (narg == 6 && symbolIsScalar(ps[3]))) {
    /* ps[2] is <v>; it must have 3 dimensions and the first one
     must have 2 elements. */
    if (array_num_dims(ps[2]) != 3)
      return anaerror("Need 3 dimensions here", ps[2]);
    if (array_dims(ps[2])[0] != 2)
      return anaerror("Need 2 elements in first dimension here", ps[2]);
    nx = array_dims(ps[2])[1];
    ny = array_dims(ps[2])[2];
  } else {
    /* ps[2] is <vx>, ps[3] is <vy>; both must have 2 dimensions and
       the dimensions of <vx> must equal those of <vy>. */
    if (!symbolIsRealArray(ps[3]))
      return cerror(NEED_ARR, ps[3]);
    if (array_num_dims(ps[2]) != 2)
      return cerror(NEED_2D_ARR, ps[2]);
    if (array_num_dims(ps[3]) != 2)
      return cerror(NEED_2D_ARR, ps[3]);
    nx = array_dims(ps[2])[0];
    ny = array_dims(ps[2])[1];
    if (array_size(ps[2]) != array_size(ps[3])
	|| array_dims(ps[3])[0] != nx)
      return cerror(INCMP_ARG, ps[3]);
    if (array_type(ps[3]) > type)
      type = array_type(ps[3]);
  }

  /* check for <n> */
  if (narg == 7) {
    if (!symbolIsScalar(ps[4]))
      return anaerror("Need a scalar here", ps[4]);
    n = int_arg(ps[4]);
    if (n < 1)
      return cerror(ILL_ARG, ps[4]);
  } else if (narg == 6 && symbolIsScalar(ps[3])) {
    if (!symbolIsScalar(ps[3]))
      return anaerror("Need a scalar here", ps[3]);
    n = int_arg(ps[3]);
    if (n < 1)
      return cerror(ILL_ARG, ps[3]);
  } else
    n = 1;

  /* check for <ox> and <oy>; prepare output variables */
  if (narg >= 5) {		/* have <ox> and <oy> */
    /* we prepare the output symbols */
    i = 0;
    if (n > 1)
      dims[i++] = n;
    memcpy(dims + i, array_dims(ps[0]), array_num_dims(ps[0])*sizeof(int));
    i += array_num_dims(ps[0]);
    to_scratch_array(ps[narg - 2], type, i, dims);
    ox.v = array_data(ps[narg - 2]);
    to_scratch_array(ps[narg - 1], type, i, dims);
    oy.v = array_data(ps[narg - 1]);
  } else {			/* use <gx> and <gy> for <ox> and <oy> */
    int ana_convert(int, int [], int, int);
    ana_convert(2, ps, type, 0);
    ox.v = array_data(ps[0]);
    oy.v = array_data(ps[1]);
  }

  /* now we do any promotion to the highest type and get pointers to */
  /* the data */
  iq = ana_converts[type](1, ps);
  if (iq == ANA_ERROR)
    return ANA_ERROR;
  gx.v = array_data(iq);
  iq = ana_converts[type](1, ps + 1);
  if (iq == ANA_ERROR)
    return ANA_ERROR;
  gy.v = array_data(iq);
  if (narg == 3 || narg == 5
      || (narg == 6 && symbolIsScalar(ps[3]))) { /* <v> */
    iq = ana_converts[type](1, ps + 2);
    if (iq == ANA_ERROR)
      return ANA_ERROR;
    vx0.v = array_data(iq);
    vy0.b = vx0.b + ana_type_size[type];
    dv = 2;
  } else {
    iq = ana_converts[type](1, ps + 2);
    if (iq == ANA_ERROR)
      return ANA_ERROR;
    vx0.v = array_data(iq);
    iq = ana_converts[type](1, ps + 3);
    if (iq == ANA_ERROR)
      return ANA_ERROR;
    vy0.v = array_data(iq);
    dv = 1;
  }

  while (ngrid--) {			/* all grid points */
    /* we work up to the desired distance by measuring the path
       lengths in each pixel and adding them up.  This scheme backfires
       when the velocities
       at some point circle the common corner of four pixels: then the
       algorithm can keep circling that point in very small steps for
       a very long time.  If many small steps follow one another in
       succession, then we are probably in such a situation and it is
       then unlikely that we'll get out of it any time soon.  We guard
       against this by maintaining an exponentially weighted average
       of the past steps and quitting if the weighted average gets too
       small.  We use an exponential decay scale of 2 steps and
       a limit value of 0.2. */
    for (i = 0; i < n; i++) {
      dslimit = 1.0;		/* current weighted average of step sizes */
      if (i) {
	x1 = x2;
	y1 = y2;
      } else {
	switch (type) {
	  case ANA_BYTE:
	    ix = (int) *gx.b;	/* x pixel coordinate */
	    iy = (int) *gy.b;	/* y pixel coordinate */
	    x1 = (double) *gx.b++ - ix;
	    y1 = (double) *gy.b++ - iy;
	    break;
	  case ANA_WORD:
	    ix = (int) *gx.w;	/* x pixel coordinate */
	    iy = (int) *gy.w;	/* y pixel coordinate */
	    x1 = (double) *gx.w++ - ix;
	    y1 = (double) *gy.w++ - iy;
	    break;
	  case ANA_LONG:
	    ix = (int) *gx.l;	/* x pixel coordinate */
	    iy = (int) *gy.l;	/* y pixel coordinate */
	    x1 = (double) *gx.l++ - ix;
	    y1 = (double) *gy.l++ - iy;
	    break;
	  case ANA_FLOAT:
	    ix = (int) *gx.f;	/* x pixel coordinate */
	    iy = (int) *gy.f;	/* y pixel coordinate */
	    x1 = (double) *gx.f++ - ix;
	    y1 = (double) *gy.f++ - iy;
	    break;
	  case ANA_DOUBLE:
	    ix = (int) *gx.d;	/* x pixel coordinate */
	    iy = (int) *gy.d;	/* y pixel coordinate */
	    x1 = (double) *gx.d++ - ix;
	    y1 = (double) *gy.d++ - iy;
	    break;
	}
	if (ix < 0 || ix > nx - 1 || iy < 0 || iy > ny - 1) { /* out of range */
	  zerobytes(ox.b, ana_type_size[type]);
	  zerobytes(oy.b, ana_type_size[type]);
	  ox.b += ana_type_size[type];
	  oy.b += ana_type_size[type];
	  continue;
	}
      }
      
      index = ix + iy*nx;	/* index relative to data start */
    
      switch (type) {
	case ANA_BYTE:
	  vx = (double) vx0.b[index*dv]; /* x velocity */
	  vy = (double) vy0.b[index*dv]; /* y velocity */
	  break;
	case ANA_WORD:
	  vx = (double) vx0.w[index*dv]; /* x velocity */
	  vy = (double) vy0.w[index*dv]; /* y velocity */
	  break;
	case ANA_LONG:
	  vx = (double) vx0.l[index*dv]; /* x velocity */
	  vy = (double) vy0.l[index*dv]; /* y velocity */
	  break;
	case ANA_FLOAT:
	  vx = (double) vx0.f[index*dv]; /* x velocity */
	  vy = (double) vy0.f[index*dv]; /* y velocity */
	  break;
	case ANA_DOUBLE:
	  vx = (double) vx0.d[index*dv]; /* x velocity */
	  vy = (double) vy0.d[index*dv]; /* y velocity */
	  break;
      }
    
      s0 = 0.5*hypot(vx, vy);	/* length indicates desired distance */
      s1 = s0;
      s = 0.0;
    
      while (s < s1) {		/* go the distance */
	c = traverseElement(x1, y1, vx, vy, &x2, &y2);
	/* calculate distance inside the current pixel */
	x1 -= x2;
	y1 -= y2;
	ds = hypot(x1, y1);	/* distance in the current pixel */
	if (s + ds > s1) {	/* we went a bit too far */
	  x2 -= x1*(s0 - s - ds)/ds;
	  y2 -= y1*(s0 - s - ds)/ds;
	  c = CENTER;
	}
	dslimit = 0.5*(dslimit + ds);
	if (dslimit < 0.2) {	/* we fear a semi-infinite loop here */
	  s = s1;		/* we break it off */
	  continue;
	}
	switch (c) {
	  case UP:
	    if (iy == ny - 1) { /* we're already at the top */
	      s += ds;
	      s1 = s;
	      continue;
	    }
	    di = nx;
	    x1 = x2;
	    y1 = 0.0;
	    iy++;
	    break;
	  case RIGHT:
	    if (ix == nx - 1) { /* already at right edge */
	      s += ds;
	      s1 = s;
	      continue;
	    }
	    di = 1;
	    x1 = 0.0;
	    y1 = y2;
	    ix++;
	    break;
	  case DOWN:
	    if (iy == 0) {	/* already at bottom */
	      s += ds;
	      s1 = s;
	      continue;
	    }
	    di = -nx;
	    x1 = x2;
	    y1 = 1.0;
	    iy--;
	    break;
	  case LEFT:
	    if (ix == 0) { 	/* already at left edge */
	      s += ds;
	      s1 = s;
	      continue;
	    }
	    di = -1;
	    x1 = 1.0;
	    y1 = y2;
	    ix--;
	    break;
	  case CENTER:
	    s1 = s;
	    continue;
	} /* end of switch (c) */
	index += di;
	s += ds;
	switch (type) {
	  case ANA_BYTE:
	    vx = (double) vx0.b[index*dv];
	    vy = (double) vy0.b[index*dv];
	    break;
	  case ANA_WORD:
	    vx = (double) vx0.w[index*dv];
	    vy = (double) vy0.w[index*dv];
	    break;
	  case ANA_LONG:
	    vx = (double) vx0.l[index*dv];
	    vy = (double) vy0.l[index*dv];
	    break;
	  case ANA_FLOAT:
	    vx = (double) vx0.f[index*dv];
	    vy = (double) vy0.f[index*dv];
	    break;
	  case ANA_DOUBLE:
	    vx = (double) vx0.d[index*dv];
	    vy = (double) vy0.d[index*dv];
	    break;
	}
      } /* end of while (s < s0) */

      /* if we get here then we are at the desired spot, or we have overshot */
      /* it a bit.  We adjust. */

      switch (type) {
	case ANA_BYTE:
	  *ox.b++ = ix + x2;
	  *oy.b++ = iy + y2;
	  break;
	case ANA_WORD:
	  *ox.w++ = ix + x2;
	  *oy.w++ = iy + y2;
	  break;
	case ANA_LONG:
	  *ox.l++ = ix + x2;
	  *oy.l++ = iy + y2;
	  break;
	case ANA_FLOAT:
	  *ox.f++ = ix + x2;
	  *oy.f++ = iy + y2;
	  break;
	case ANA_DOUBLE:
	  *ox.d++ = ix + x2;
	  *oy.d++ = iy + y2;
	  break;
      }
    } /* end of for (i = 0; i < n; i++) */
  } /* end of while (ngrid--) */
  return ANA_OK;
}
/*--------------------------------------------------------------------*/
void legendre(double x, int lmax, double *values)
/* calculates the values of the associate Legendre polynomials */
/* P_l^m(x) at ordinate <x> for all <l> from 0 through <lmax> and all <m> */
/* from 0 through <l> */
/* we store the results in predefined <values>, in the following order: */
/* l 0 1 1 2 2 2 3 3 3 3 */
/* m 0 0 1 0 1 2 0 1 2 3 */
/* based on the following (recursion) relations: */
/* (l - m) P_l^m = x (2 l - 1) P_{l-1}^m - (l + m - 1) P_{l-2}^m */
/* P_m^m = (-1)^m (2 m - 1)!! (1 - x^2)^{m/2} */
/* (with n!! the product of all *odd* values between 1 and n) */
/* P_{m+1}^m = x (2 m + 1) P_m^m */
{
  int	l, m, j1, j2, j3, j4;
  double	z, *p, v1, v2;

  zerobytes(values, (lmax + 1)*(lmax + 2)*sizeof(double)/2);

  p = values;
  z = sqrt(1 - x*x);
  /* first we calculate P_l^l for all <l> */
  v1 = *p = 1.0;		/* P_0^0 */
  p += 2;
  /*    l 0 1 1 2 2 2 3 3 3 3 4 4 4 4 4 */
  /*    m 0 0 1 0 1 2 0 1 2 3 0 1 2 3 4 */
  /* done + +                           */

  /* now p points at P_1_1 */
  j1 = 1;
  for (l = 1; l <= lmax; l++) {
    v1 = *p = -v1*j1*z;
    j1 += 2;
    p += l + 2;
  }
  /*    l 0 1 1 2 2 2 3 3 3 3 4 4 4 4 4 */
  /*    m 0 0 1 0 1 2 0 1 2 3 0 1 2 3 4 */
  /* done + + +     +       +         + */

  /* then we do the other combinations */
  /* the index of P_l^m is equal to l(l + 1)/2 + m */
  for (m = 0; m < lmax; m++) {
    v2 = values[(m + 1)*(m + 2)/2 - 1]; /* P_m^m */
    v1 = values[(m + 2)*(m + 3)/2 - 2] = x*(2*m + 1)*v2; /* P_{m+1}^m */
    p = values + (m + 3)*(m + 4)/2 - 3;	/* points at P_{m+2}^m */
    j1 = 2*m + 3;
    j2 = j1 - 2;
    j3 = 2;
    j4 = m + 2;
    for (l = m + 2; l <= lmax; l++) {
      *p = (x*j1*v1 - j2*v2)/j3;
      v2 = v1;
      v1 = *p;
      j1 += 2;
      j2++;
      j3++;
      j4++;
      p += j4;
    }
  }
}
/*--------------------------------------------------------------------*/
void spherical_harmonics(double x, int lmax, double *values)
/* calculates the values of the normalized associate Legendre polynomials */
/* y_l^m(x) at ordinate <x> for all <l> from 0 through <lmax> and all <m> */
/* from 0 through <l>.  The normalization is such that */
/* the normalized spherical harmonics are equal to */
/* Y_l^m(theta,phi) = y_l^m(cos(theta)) exp(i m phi). */
/* we store the results in predefined <values>, in the following order: */
/* l 0 1 1 2 2 2 3 3 3 3 */
/* m 0 0 1 0 1 2 0 1 2 3 */
/* NOTE: doesn't seem quite OK yet */
{
  int	l, m, j4;
  double	z, *p, v1, v2, w1, w2, w3, w4, w5, w6;

  p = values;
  z = sqrt(1 - x*x);
  /* first we calculate y_l^l for all <l> */
  v1 = *p = 0.25*M_2_SQRTPI;	/* P_0^0 */
  p += 2;
  /*    l 0 1 1 2 2 2 3 3 3 3 4 4 4 4 4 */
  /*    m 0 0 1 0 1 2 0 1 2 3 0 1 2 3 4 */
  /* done + +                           */

  /* now p points at y_1_1 */
  /* y_{m+1}^{m+1} = -y_m^m z sqrt((2*m + 3)/(2*m + 1)) */
  w1 = 3;
  w2 = 2;
  for (l = 1; l <= lmax; l++) {
    v1 = *p = -v1*z*sqrt(w1/w2);
    w1 += 2;
    w2 += 2;
    p += l + 2;
  }
  /*    l 0 1 1 2 2 2 3 3 3 3 4 4 4 4 4 */
  /*    m 0 0 1 0 1 2 0 1 2 3 0 1 2 3 4 */
  /* done + + +     +       +         + */

  /* then we do the other combinations */
  /* the index of y_l^m is equal to l(l + 1)/2 + m */
  /* y_{m+1}^m = y_m^m x sqrt(2m + 3) */
  /* y_l^m = x y_{l-1}^m sqrt((4l^2 - 1)/(l^2 - m^2)) */
  /*         - y_{l-2}^m sqrt((2l + 1)/(2l - 3)*((l - 1)^2 - m^2)/(l^2 - m^2))*/
  for (m = 0; m < lmax; m++) {
    v2 = values[(m + 1)*(m + 2)/2 - 1]; /* P_m^m */
    v1 = values[(m + 2)*(m + 3)/2 - 2] = x*sqrt(2*m + 3)*v2; /* P_{m+1}^m */
    p = values + (m + 3)*(m + 4)/2 - 3;	/* points at P_{m+2}^m */
    j4 = m + 2;
    w2 = m*m;
    w3 = 2*(m + 2) + 1;
    w4 = 2*(m + 2) - 3;
    w5 = (m + 2) - 1;
    for (l = m + 2; l <= lmax; l++) {
      w1 = l*l;
      w6 = w1 - w2;
      *p = x*v1*sqrt((4*w1 - 1)/w6) - v2*sqrt(w3*(w5*w5 - w2)/(w4*w6));
      v2 = v1;
      v1 = *p;
      w3 += 2;
      w4 += 2;
      w5++;
      j4++;
      p += j4;
    }
  }
}
/*--------------------------------------------------------------------*/
int ana_legendre(int narg, int ps[])
/* LEGENDRE(x, lmax) */
{
  double	x, *values;
  int	lmax, out, n;

  x = double_arg(ps[0]);
  if (x < -1 || x > 1)
    return anaerror("Illegal ordinate %g (not between -1 and +1)", ps[0], x);
  lmax = int_arg(ps[1]);
  if (lmax < 0)
    return anaerror("Illegal maximum order %d (must be nonnegative)", ps[1], lmax);
  n = ((lmax + 1)*(lmax + 2))/2;
  out = array_scratch(ANA_DOUBLE, 1, &n);
  values = array_data(out);
  if (internalMode & 1)
    spherical_harmonics(x, lmax, values);
  else
    legendre(x, lmax, values);
  return out;
}
/*--------------------------------------------------------------------*/
int ana_enhanceimage(int narg, int ps[])
/* ENHANCEIMAGE(<x> [, <part>, <tgt>, /SYMMETRIC]) enhances an image
  <x>.  The first dimension of <x> is assumed to select between color
  channels.  <x> is assumed to contain BYTE values between 0 and 255,
  inclusive.  <part> specifies which fraction of the full correction
  is to be applied; it defaults to 1.  <tgt> specifies at which grey
  level fraction the median of the enhanced image is expected to end
  up (if <part> is 1); it defaults to 100/256.  /SYMMETRIC means to
  apply enhancements from both the low and high ends; /NOSYMMETRIC
  means to enhance only from the low end.  LS 2006jun15 */
{
  pointer src, tgt;
  int ndim, *dims, nhist, *hist, nelem, i, result;
  float target, part;
  float a, b;
  float *m;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_NUM_ARR, ps[0]);
  if (symbol_type(ps[0]) != ANA_BYTE)
    return anaerror("Need BYTE array", ps[0]);
  numerical(ps[0], &dims, &ndim, &nelem, &src);
  if (ndim < 2)
    return anaerror("Need 2 or more dimensions", ps[0]);
  part = (narg > 1 && ps[1])? float_arg(ps[1]): 1;
  target = (narg > 2 && ps[2])? float_arg(ps[2]): 100.0/256;

  nhist = 256*dims[0];
  hist = calloc(nhist, sizeof(*hist));
  m = malloc(nhist*sizeof(*m));
  if (!hist || !m) {
    free(hist);
    return cerror(ALLOC_ERR, 0);
  }
  result = array_clone(ps[0], ANA_BYTE);
  if (result == ANA_ERROR) {
    free(hist);
    free(m);
    return result;
  }
  tgt.b = array_data(result);
  for (i = 0; i < nelem; i += dims[0]) {
    int j, x = 0;

    for (j = 0; j < dims[0]; j++) /* over all color channels */
      x += *src.b++;
    hist[x]++;
  }
  src.b = array_data(ps[0]);
  for (i = 1; i < nhist; i++)	/* calculate running sum */
    hist[i] += hist[i - 1];
  a = 2 - 4*target;
  b = 1 - a;
  m[0] = 1.0;
  for (i = 1; i < nhist; i++) {	/* calculate adjustment factors */
    float q;

    q = (float) hist[i]*dims[0]/nelem;
    m[i] = q*(a*q + b)/i*nhist;
    if (m[i] < 1 && !(internalMode & 1))
      m[i] = 1;
    if (part != 1)
      m[i] = part*m[i] + 1 - part;
  }
  for (i = 0; i < nelem; i += dims[0]) { /* calculate adjusted image */
    int j, x = 0;

    for (j = 0; j < dims[0]; j++)
      x += src.b[j];
    for (j = 0; j < dims[0]; j++) {
      int y;

      y = *src.b++ * m[x];
      if (y > 255)
	y = 255;
      *tgt.b++ = y;
    }
  }
  free(m);
  free(hist);
  return result;
}
/*--------------------------------------------------------------------*/
int ana_hamming(int narg, int ps[]) {
  int nelem, nelem2, ndim, *dims, result, i, type, nr2isarray;
  pointer src, src2, tgt;

  if (!symbolIsNumerical(ps[0]))
    return anaerror("Need a numerical argument", ps[0]);
  if (!symbolIsInteger(ps[0]))
    return cerror(NEED_INT_ARG, ps[0]);
  numerical(ps[0], &dims, &ndim, &nelem, &src);
  type = symbol_type(ps[0]);

  if (narg >= 2) {
    if (!symbolIsNumerical(ps[1]))
      return anaerror("Need a numerical argument", ps[1]);
    if (!symbolIsInteger(ps[1]))
      return cerror(NEED_INT_ARG, ps[1]);
    if (symbol_type(ps[1]) != type)
      return anaerror("Data type is different from previous argument", ps[1]);
    numerical(ps[1], NULL, NULL, &nelem2, &src2);
    if (nelem2 != nelem && nelem2 != 1)
      return cerror(INCMP_ARG, ps[1]);
    nr2isarray = (nelem2 > 1);
  }

  if (symbol_type(ps[0]) == ANA_SCALAR) {
    result = scalar_scratch(ANA_LONG);
    tgt.l = &scalar_value(result).l;
  } else {
    result = array_scratch(ANA_LONG, ndim, dims);
    tgt.l = array_data(result);
  }

  if (narg == 1) {
    for (i = 0; i < nelem; i++) {
      unsigned int dist = 0, val;
      switch (type) {
      case ANA_BYTE:
        val = *src.b++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.l++ = dist;
        break;
      case ANA_WORD:
        val = *src.w++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.l++ = dist;
        break;
      case ANA_LONG:
        val = *src.l++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.l++ = dist;
        break;
      }
    }
  } else {
    for (i = 0; i < nelem; i++) {
      unsigned int dist = 0, val;
      switch (type) {
      case ANA_BYTE:
        val = *src.b++ ^ *src2.b;
        if (nr2isarray)
          src2.b++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.l++ = dist;
        break;
      case ANA_WORD:
        val = *src.w++ ^ *src2.w;
        if (nr2isarray)
          src2.w++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.l++ = dist;
        break;
      case ANA_LONG:
        val = *src.l++ ^ *src2.l;
        if (nr2isarray)
          src2.l++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.l++ = dist;
        break;
      }
    }
  }
  return result;
}
/*--------------------------------------------------------------------*/
int compare_doubles(const void *a, const void *b)
{
  const double *da = (const double *) a;
  const double *db = (const double *) b;
  return (*da > *db) - (*da < *db);
}
int runord_d(double *data, int n, int width, int ord, double *result)
{
  int i;
  
  if (n < 1 || width < 1) {
    errno = EDOM;
    return -1;
  }
  double *temp = malloc(width*sizeof(double));
  if (!temp) {
    errno = ENOMEM;
    return -1;
  }
  if (width > n)
    width = n;
  if (ord < 0)
    ord = 0;
  if (ord > width - 1)
    ord = width - 1;
  int o = width/2;
  for (i = 0; i < n - width; i++) {
    memcpy(temp, data + i, width*sizeof(double));
    qsort(temp, width, sizeof(double), compare_doubles);
    result[o + i] = temp[ord];
  }
  for ( ; i < n - o; i++)
    result[i] = result[i - 1];
  for (i = 0; i < o; i++)
    result[i] = result[o];
  free(temp);
  return 0;
}
BIND(runord_d, ibLLLobrl, f, RUNORD, 3, 3, NULL);
/*--------------------------------------------------------------------*/
int runmax_d(double *data, int n, int width, double *result)
{
  return runord_d(data, n, width, width - 1, result);
}
BIND(runmax_d, ibLLobrl, f, RUNMAX, 2, 2, NULL);
/*--------------------------------------------------------------------*/
int runmin_d(double *data, int n, int width, double *result)
{
  return runord_d(data, n, width, 0, result);
}
BIND(runmin_d, ibLLobrl, f, RUNMIN, 2, 2, NULL);
/*--------------------------------------------------------------------*/
/*
  Returns <x> such that <x> = <cur> (mod <period) and
  <average> - <period>/2 <= <x> - <prev> < <average> + <period>/2
 */
double unmod(double cur, double prev, double period, double average)
{
  if (!period)
    return cur;
  return cur + period*ceil((prev - cur + average)/period - 0.5);
}
/*--------------------------------------------------------------------*/
int unmod_slice_d(double *srcptr, int srccount, int srcstride,
                  double period, double average,
                  double *tgtptr, int tgtcount, int tgtstride)
{
  int i;

  if (!period || !srcptr || srccount < 1 || !tgtptr || tgtcount < 1
      || tgtcount != srccount) {
    errno = EDOM;
    return 1;
  }
  *tgtptr = *srcptr;
  tgtptr += tgtstride;
  srcptr += srcstride;
  for (i = 1; i < srccount; i++) {
    *tgtptr = unmod(*srcptr, tgtptr[-tgtstride], period, average);
    tgtptr += tgtstride;
    srcptr += srcstride;
  }
  return 0;
}
BIND(unmod_slice_d, ivddovrl, f, UNMOD, 2, 4, ":AXIS:PERIOD:AVERAGE");
