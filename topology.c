/* File topology.c */
/* ANA routine for identifying and labeling disjoint areas in a bitmap. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: topology.c,v 4.0 2001/02/07 20:37:08 strous Exp $";

/*------------------------------------------------------------------*/
#define SEEK_MAXIMUM	(ANA_DOUBLE + 1)
int segment_2d(int narg, int ps[])
/* does image segmentation on two-dimensional arrays.
   it is assumed that's what <x> is!
   Y = SEGMENT(x [, sign]) */
{
  loopInfo	srcinfo, trgtinfo;
  pointer	src, trgt;
  scalar	value;
  int	nx, ny, n, result, sign;

  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT, ANA_LONG,
		   &srcinfo, &src, &result, &trgtinfo, &trgt) == ANA_ERROR)
    return ANA_ERROR;

  sign = (narg > 1 && ps[1])? int_arg(ps[1]): 1;

  nx = srcinfo.dims[0];
  ny = srcinfo.dims[1];
  
  /* top row: always zero */
  zerobytes(trgt.l, nx*sizeof(int));
  trgt.l += nx;
  src.b += nx*ana_type_size[array_type(ps[0])];
  ny -= 2;
  if (sign >= 0) {		/* seek hill-like objects */
    switch (array_type(ps[0])) {
      case ANA_BYTE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.b++;
	  n = nx - 2;
	  while (n--) {
	    value.w = *src.b * 2;
	    *trgt.l++ = ((word) src.b[-1] + (word) src.b[1] < value.w
			 && (word) src.b[nx] + (word) src.b[-nx] < value.w
			 && (word) src.b[1 + nx] + (word) src.b[-1 - nx] 
			 < value.w
			 && (word) src.b[1 - nx] + (word) src.b[-1 + nx]
			 < value.w);
	    src.b++;
	  }
	  *trgt.l++ = 0;
	  src.b++;
	}
	break;
      case ANA_WORD:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.w++;
	  n = nx - 2;
	  while (n--) {
	    value.l = (int) *src.w * 2;
	    *trgt.l++ = ((int) src.w[-1] + (int) src.w[1] < value.l
			 && (int) src.w[nx] + (int) src.w[-nx] < value.l
			 && (int) src.w[1 + nx] + (int) src.w[-1 - nx]
			    < value.l
			 && (int) src.w[1 - nx] + (int) src.w[-1 + nx]
			    < value.l);
	    src.w++;
	  }
	  *trgt.l++ = 0;
	  src.w++;
	}
	break;
      case ANA_LONG:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.l++;
	  n = nx - 2;
	  while (n--) {
	    value.l = *src.l * 2;
	    *trgt.l++ = (src.l[-1] + src.l[1] < value.l
			 && src.l[nx] + src.l[-nx] < value.l
			 && src.l[1 + nx] + src.l[-1 - nx] < value.l
			 && src.l[1 - nx] + src.l[-1 + nx] < value.l);
	    src.l++;
	  }
	  *trgt.l++ = 0;
	  src.l++;
	}
	break;
      case ANA_FLOAT:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.f++;
	  n = nx - 2;
	  while (n--) {
	    value.f = *src.f * 2;
	    *trgt.l++ = (src.f[-1] + src.f[1] < value.f
			 && src.f[nx] + src.f[-nx] < value.f
			 && src.f[1 + nx] + src.f[-1 - nx] < value.f
			 && src.f[1 - nx] + src.f[-1 + nx] < value.f);
	    src.f++;
	  }
	  *trgt.l++ = 0;
	  src.f++;
	}
	break;
      case ANA_DOUBLE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.d++;
	  n = nx - 2;
	  while (n--) {
	    value.d = *src.d * 2;
	    *trgt.l++ = (src.d[-1] + src.d[1] < value.d
			 && src.d[nx] + src.d[-nx] < value.d
			 && src.d[1 + nx] + src.d[-1 - nx] < value.d
			 && src.d[1 - nx] + src.d[-1 + nx] < value.d);
	    src.d++;
	  }
	  *trgt.l++ = 0;
	  src.d++;
	}
	break;
    }
  } else {			/* seek valley-like objects */
    switch (array_type(ps[0])) {
      case ANA_BYTE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.b++;
	  n = nx - 2;
	  while (n--) {
	    value.b = *src.b * (short) 2;
	    *trgt.l++ = (src.b[-1] + src.b[1] > value.b
			 && src.b[nx] + src.b[-nx] > value.b
			 && src.b[1 + nx] + src.b[-1 - nx] > value.b
			 && src.b[1 - nx] + src.b[-1 + nx] > value.b);
	    src.b++;
	  }
	  *trgt.l++ = 0;
	  src.b++;
	}
	break;
      case ANA_WORD:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.w++;
	  n = nx - 2;
	  while (n--) {
	    value.w = *src.w * 2;
	    *trgt.l++ = (src.w[-1] + src.w[1] > value.w
			 && src.w[nx] + src.w[-nx] > value.w
			 && src.w[1 + nx] + src.w[-1 - nx] > value.w
			 && src.w[1 - nx] + src.w[-1 + nx] > value.w);
	    src.w++;
	  }
	  *trgt.l++ = 0;
	  src.w++;
	}
	break;
      case ANA_LONG:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.l++;
	  n = nx - 2;
	  while (n--) {
	    value.l = *src.l * 2;
	    *trgt.l++ = (src.l[-1] + src.l[1] > value.l
			 && src.l[nx] + src.l[-nx] > value.l
			 && src.l[1 + nx] + src.l[-1 - nx] > value.l
			 && src.l[1 - nx] + src.l[-1 + nx] > value.l);
	    src.l++;
	  }
	  *trgt.l++ = 0;
	  src.l++;
	}
	break;
      case ANA_FLOAT:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.f++;
	  n = nx - 2;
	  while (n--) {
	    value.f = *src.f * 2;
	    *trgt.l++ = (src.f[-1] + src.f[1] > value.f
			 && src.f[nx] + src.f[-nx] > value.f
			 && src.f[1 + nx] + src.f[-1 - nx] > value.f
			 && src.f[1 - nx] + src.f[-1 + nx] > value.f);
	    src.f++;
	  }
	  *trgt.l++ = 0;
	  src.f++;
	}
	break;
      case ANA_DOUBLE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.d++;
	  n = nx - 2;
	  while (n--) {
	    value.d = *src.d * 2;
	    *trgt.l++ = (src.d[-1] + src.d[1] > value.d
			 && src.d[nx] + src.d[-nx] > value.d
			 && src.d[1 + nx] + src.d[-1 - nx] > value.d
			 && src.d[1 - nx] + src.d[-1 + nx] > value.d);
	    src.d++;
	  }
	  *trgt.l++ = 0;
	  src.d++;
	}
	break;
    }
  }
  zerobytes(trgt.l, nx*sizeof(int));
  return result;
}
/*-------------------------------------------------------------------------*/
int segment_general(int narg, int ps[])
/* Y = SEGMENT(X) curvature-based segmentation in multiple dimensions
   SEGMENT(x [, sign, DIAGONAL=diagonal, /DEGREE])
   <x>: data
   <sign>: sign of objects to look for, positive integer -> hill-like,
    negative integer -> valley-like.  If zero, then +1 is assumed.
   /DEGREE: returns number of OK curvatures per data element
   LS 18may95 4aug97 */
{
  int	result, sign, degree, n, i, *offset, k, j, ok, *edge, nok;
  scalar	value;
  pointer	src, trgt, srcl, srcr;
  loopInfo	srcinfo, trgtinfo;

  /* gather info about ps[0] and prepare a return symbol */
  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT,
		   ANA_LONG, &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return ANA_ERROR;
  if (symbol_class(ps[0]) != ANA_ARRAY)
    return cerror(ILL_CLASS, ps[0]);

  sign = (narg > 1 && ps[1])? int_arg(ps[1]): 1;
  sign = (sign >= 0)? 1: 0;	/* 1 -> hill-like, 0 -> valley-like */
  degree = (internalMode & 1);

  /* treat DIAGONAL */
  n = prepareDiagonals(narg > 2? ps[2]: 0, &srcinfo, 2, &offset, &edge, NULL,
		       NULL);
  if (n == ANA_ERROR)
    return ANA_ERROR;
  
  /* set the edges to zero */
  for (i = 0; i < 2*srcinfo.ndim; i++) {
    if (edge[i]) {
      rearrangeEdgeLoop(&trgtinfo, NULL, i);
      do
	*trgt.l = 0;
      while (advanceLoop(&trgtinfo, &trgt) < trgtinfo.ndim - 1);
    }
  }

  /* set up the range of coordinates that is to be checked */
  for (i = 0; i < srcinfo.ndim; i++)
    edge[2*i + 1] = srcinfo.dims[i] - 1 - edge[2*i + 1];
  subdataLoop(edge, &srcinfo);
  subdataLoop(edge, &trgtinfo);

  free(edge);			/* clean up */

  /* now do the loop work */
  if (sign && !degree && (internalMode & 2) == 0)
		/* standard form - make as fast as possible */
    switch (array_type(ps[0])) {
      case ANA_BYTE:
	do {
	  value.w = 2 * (word) *src.b;
	  for (j = 0; j < n; j++) {	/* all directions */	  
	    k = offset[j];
	    srcl.b = src.b + k;
	    srcr.b = src.b - k;
	    if (value.w <= (word) *srcl.b + (word) *srcr.b)
	      break;
	  }
	  *trgt.l = (j == n);
	} while (advanceLoop(&trgtinfo, &trgt), 
		 advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	break;
      case ANA_WORD:
	do {
	  value.l = 2 * *src.w;
	  for (j = 0; j < n; j++) {	/* all directions */	  
	    k = offset[j];
	    srcl.w = src.w + k;
	    srcr.w = src.w - k;
	    if (value.l <= (int) *srcl.w + (int) *srcr.w)
	      break;
	  }
	  *trgt.l = (j == n);
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	break;
      case ANA_LONG:
	do {
	  value.l = 2 * *src.l;
	  for (j = 0; j < n; j++) {	/* all directions */	  
	    k = offset[j];
	    srcl.l = src.l + k;
	    srcr.l = src.l - k;
	    if (value.l <= *srcl.l + *srcr.l)
	      break;
	  }
	  *trgt.l = (j == n);
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	break;
      case ANA_FLOAT:
	do {
	  value.f = 2 * *src.f;
	  for (j = 0; j < n; j++) {	/* all directions */	  
	    k = offset[j];
	    srcl.f = src.f + k;
	    srcr.f = src.f - k;
	    if (value.f <= *srcl.f + *srcr.f)
	      break;
	  }
	  *trgt.l = (j == n);
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	break;
      case ANA_DOUBLE:
	do {
	  value.d = 2 * *src.d;
	  for (j = 0; j < n; j++) {	/* all directions */	  
	    k = offset[j];
	    srcl.d = src.d + k;
	    srcr.d = src.d - k;
	    if (value.d <= *srcl.d + *srcr.d)
	      break;
	  }
	  *trgt.l = (j == n);
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	break;
    } else 				/* general case - a bit slower */
      switch (array_type(ps[0])) {
	case ANA_BYTE:
	  do {
	    nok = 1 - degree;
	    value.b = 2 * *src.b;
	    for (j = 0; j < n; j++) { /* all directions */	  
	      k = offset[j];
	      srcl.b = src.b + k;
	      srcr.b = src.b - k;
	      ok = (sign && value.b > *srcl.b + *srcr.b)
		|| (!sign && value.b < *srcl.b + *srcr.b);
	      if (degree)
		nok += ok;
	      else if (!ok) {
		nok = 0;
		break;
	      }
	    }
	    *trgt.l = nok;
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	  break;
	case ANA_WORD:
	  do {
	    nok = 1 - degree;
	    value.w = 2 * *src.w;
	    for (j = 0; j < n; j++) { /* all directions */	  
	      k = offset[j];
	      srcl.w = src.w + k;
	      srcr.w = src.w - k;
	      ok = (sign && value.w > *srcl.w + *srcr.w)
		|| (!sign && value.w < *srcl.w + *srcr.w);
	      if (degree)
		nok += ok;
	      else if (!ok) {
		nok = 0;
		break;
	      }
	    }
	    *trgt.l = nok;
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	  break;
	case ANA_LONG:
	  do {
	    nok = 1 - degree;
	    value.l = 2 * *src.l;
	    for (j = 0; j < n; j++) { /* all directions */	  
	      k = offset[j];
	      srcl.l = src.l + k;
	      srcr.l = src.l - k;
	      ok = (sign && value.l > *srcl.l + *srcr.l)
		|| (!sign && value.l < *srcl.l + *srcr.l);
	      if (degree)
		nok += ok;
	      else if (!ok) {
		nok = 0;
		break;
	      }
	    }
	    *trgt.l = nok;
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	  break;
	case ANA_FLOAT:
	  do {
	    nok = 1 - degree;
	    value.f = 2 * *src.f;
	    for (j = 0; j < n; j++) { /* all directions */	  
	      k = offset[j];
	      srcl.f = src.f + k;
	      srcr.f = src.f - k;
	      ok = (sign && value.f > *srcl.f + *srcr.f)
		|| (!sign && value.f < *srcl.f + *srcr.f);
	      if (degree)
		nok += ok;
	      else if (!ok) {
		nok = 0;
		break;
	      }
	    }
	    *trgt.l = nok;
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	  break;
	case ANA_DOUBLE:
	  do {
	    nok = 1 - degree;
	    value.d = 2 * *src.d;
	    for (j = 0; j < n; j++) { /* all directions */	  
	      k = offset[j];
	      srcl.d = src.d + k;
	      srcr.d = src.d - k;
	      ok = (sign && value.d > *srcl.d + *srcr.d)
		|| (!sign && value.d < *srcl.d + *srcr.d);
	      if (degree)
		nok += ok;
	      else if (!ok) {
		nok = 0;
		break;
	      }
	    }
	    *trgt.l = nok;
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	  break;
      }
  
  free(offset);
  return result;
}
/*------------------------------------------------------------------------- */
int ana_segment(int narg, int ps[])
{
  if (narg <= 2 && (internalMode == 0) && symbolIsNumericalArray(ps[0])
      && array_num_dims(ps[0]) == 2)
    return segment_2d(narg, ps); /* fast special-purpose routine */
  else
    return segment_general(narg, ps); /* general routine */
}
/*------------------------------------------------------------------------- */
#define DEG22_5	(M_PI/8)
int ana_segment_dir(int narg, int ps[])
/* y = SEGMENTDIR(<im>, <phi> [,<sign>])
   segmentates image <im> only one-dimensionally in the direction <phi>
   (radians measured counterclockwise from the positive x-axis).
   LS 9nov98 */
{
  loopInfo	srcinfo, trgtinfo;
  pointer	src, trgt;
  float	*angle, s, c, a;
  scalar	value;
  int	nx, ny, n, result, sign, class;
  int	off[4];

  if (!symbolIsNumericalArray(ps[0])
      || array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);

  if (!symbolIsNumericalArray(ps[1])
      || array_size(ps[1]) != array_size(ps[0]))
    return cerror(INCMP_ARG, ps[1]);

  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT, ANA_LONG,
		   &srcinfo, &src, &result, &trgtinfo, &trgt) == ANA_ERROR)
    return ANA_ERROR;

  n = ana_float(1, &ps[1]);
  angle = array_data(n);

  sign = (narg > 1 && ps[2])? int_arg(ps[2]): 1;

  nx = srcinfo.dims[0];
  ny = srcinfo.dims[1];
  off[0] = nx + 1;
  off[1] = 1;
  off[2] = nx - 1;
  off[3] = nx;

  /* top row: always zero */
  zerobytes(trgt.b, nx*sizeof(int));
  trgt.l += nx;
  angle += nx;
  src.b += nx*ana_type_size[array_type(ps[0])];
  ny -= 2;
  if (sign >= 0) {		/* seek hill-like objects */
    switch (array_type(ps[0])) {
      case ANA_BYTE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  angle++;
	  src.b++;
	  n = nx - 2;
	  while (n--) {
	    value.b = *src.b * (short) 2;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.l++ = (src.b[off[class]] + src.b[-off[class]] < value.b);
	    src.b++;
	  }
	  *trgt.l++ = 0;
	  src.b++;
	  angle++;
	}
	break;
      case ANA_WORD:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  angle++;
	  src.w++;
	  n = nx - 2;
	  while (n--) {
	    value.w = *src.w * (short) 2;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.l++ = (src.w[off[class]] + src.w[-off[class]] < value.w);
	    src.w++;
	  }
	  *trgt.l++ = 0;
	  src.w++;
	  angle++;
	}
	break;
      case ANA_LONG:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  angle++;
	  src.l++;
	  n = nx - 2;
	  while (n--) {
	    value.l = *src.l * (short) 2;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.l++ = (src.l[off[class]] + src.l[-off[class]] < value.l);
	    src.l++;
	  }
	  *trgt.l++ = 0;
	  src.l++;
	  angle++;
	}
	break;
      case ANA_FLOAT:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  angle++;
	  src.f++;
	  n = nx - 2;
	  while (n--) {
	    value.f = *src.f * (short) 2;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.l++ = (src.f[off[class]] + src.f[-off[class]] < value.f);
	    src.f++;
	  }
	  *trgt.l++ = 0;
	  src.f++;
	  angle++;
	}
	break;
      case ANA_DOUBLE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  angle++;
	  src.d++;
	  n = nx - 2;
	  while (n--) {
	    value.d = *src.d * (short) 2;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.l++ = (src.d[off[class]] + src.d[-off[class]] < value.d);
	    src.d++;
	  }
	  *trgt.l++ = 0;
	  src.d++;
	  angle++;
	}
	break;
    }
  } else {			/* seek valley-like objects */
    switch (array_type(ps[0])) {
      case ANA_BYTE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.b++;
	  angle++;
	  n = nx - 2;
	  while (n--) {
	    value.b = *src.b * (short) 2;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.l++ = (src.b[off[class]] + src.b[-off[class]] > value.b);
	    src.b++;
	  }
	  *trgt.l++ = 0;
	  angle++;
	  src.b++;
	}
	break;
      case ANA_WORD:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.w++;
	  angle++;
	  n = nx - 2;
	  while (n--) {
	    value.w = *src.w * (short) 2;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.l++ = (src.w[off[class]] + src.w[-off[class]] > value.w);
	    src.w++;
	  }
	  *trgt.l++ = 0;
	  angle++;
	  src.w++;
	}
	break;
      case ANA_LONG:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.l++;
	  angle++;
	  n = nx - 2;
	  while (n--) {
	    value.l = *src.l * (short) 2;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.l++ = (src.l[off[class]] + src.l[-off[class]] > value.l);
	    src.l++;
	  }
	  *trgt.l++ = 0;
	  angle++;
	  src.l++;
	}
	break;
      case ANA_FLOAT:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.f++;
	  angle++;
	  n = nx - 2;
	  while (n--) {
	    value.f = *src.f * (short) 2;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.l++ = (src.f[off[class]] + src.f[-off[class]] > value.f);
	    src.f++;
	  }
	  *trgt.l++ = 0;
	  angle++;
	  src.f++;
	}
	break;
      case ANA_DOUBLE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.l++ = 0;
	  src.d++;
	  angle++;
	  n = nx - 2;
	  while (n--) {
	    value.d = *src.d * (short) 2;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.l++ = (src.d[off[class]] + src.d[-off[class]] > value.d);
	    src.d++;
	  }
	  *trgt.l++ = 0;
	  angle++;
	  src.d++;
	}
	break;
    }
  }
  zerobytes(trgt.l, nx*sizeof(int));
  return result;
}
/*------------------------------------------------------------------------- */
int ana_max_dir(int narg, int ps[])
/* y = MAXDIR(<im>, <phi> [,<sign>])
   returns 1s at the locations of local extremes in image <im> only
   one-dimensionally in the direction <phi> (radians measured
   counterclockwise from the positive x-axis).  LS 9nov98 */
{
  loopInfo	srcinfo, trgtinfo;
  pointer	src, trgt;
  float	*angle, s, c, a;
  scalar	value;
  int	nx, ny, n, result, sign, class;
  int	off[4];

  if (!symbolIsNumericalArray(ps[0])
      || array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);

  if (!symbolIsNumericalArray(ps[1])
      || array_size(ps[1]) != array_size(ps[0]))
    return cerror(INCMP_ARG, ps[1]);

  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT, ANA_BYTE,
		   &srcinfo, &src, &result, &trgtinfo, &trgt) == ANA_ERROR)
    return ANA_ERROR;

  n = ana_float(1, &ps[1]);
  angle = array_data(n);

  sign = (narg > 1 && ps[2])? int_arg(ps[2]): 1;

  nx = srcinfo.dims[0];
  ny = srcinfo.dims[1];
  off[0] = nx + 1;
  off[1] = 1;
  off[2] = nx - 1;
  off[3] = nx;

  /* top row: always zero */
  zerobytes(trgt.b, nx);
  trgt.b += nx;
  angle += nx;
  src.b += nx*ana_type_size[array_type(ps[0])];
  ny -= 2;
  if (sign >= 0) {		/* seek hill-like objects */
    switch (array_type(ps[0])) {
      case ANA_BYTE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.b++ = 0;
	  angle++;
	  src.b++;
	  n = nx - 2;
	  while (n--) {
	    value.b = *src.b;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.b++ = (value.b > src.b[off[class]]
			 && value.b > src.b[-off[class]]);
	    src.b++;
	  }
	  *trgt.b++ = 0;
	  src.b++;
	  angle++;
	}
	break;
      case ANA_WORD:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.b++ = 0;
	  angle++;
	  src.w++;
	  n = nx - 2;
	  while (n--) {
	    value.w = *src.w;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.b++ = (value.w > src.w[off[class]]
			 && value.w > src.w[-off[class]]);
	    src.w++;
	  }
	  *trgt.b++ = 0;
	  src.w++;
	  angle++;
	}
	break;
      case ANA_LONG:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.b++ = 0;
	  angle++;
	  src.l++;
	  n = nx - 2;
	  while (n--) {
	    value.l = *src.l;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.b++ = (value.l > src.l[off[class]]
			 && value.l > src.l[-off[class]]);
	    src.l++;
	  }
	  *trgt.b++ = 0;
	  src.l++;
	  angle++;
	}
	break;
      case ANA_FLOAT:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.b++ = 0;
	  angle++;
	  src.f++;
	  n = nx - 2;
	  while (n--) {
	    value.f = *src.f;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.b++ = (value.f > src.f[off[class]]
			 && value.f > src.f[-off[class]]);
	    src.f++;
	  }
	  *trgt.b++ = 0;
	  src.f++;
	  angle++;
	}
	break;
      case ANA_DOUBLE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.b++ = 0;
	  angle++;
	  src.d++;
	  n = nx - 2;
	  while (n--) {
	    value.d = *src.d;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.b++ = (value.d > src.d[off[class]]
			 && value.d > src.d[-off[class]]);
	    src.d++;
	  }
	  *trgt.b++ = 0;
	  src.d++;
	  angle++;
	}
	break;
    }
  } else {			/* seek valley-like objects */
    switch (array_type(ps[0])) {
      case ANA_BYTE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.b++ = 0;
	  src.b++;
	  angle++;
	  n = nx - 2;
	  while (n--) {
	    value.b = *src.b;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.b++ = (value.b < src.b[off[class]]
			 && value.b < src.b[-off[class]]);
	    src.b++;
	  }
	  *trgt.b++ = 0;
	  angle++;
	  src.b++;
	}
	break;
      case ANA_WORD:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.b++ = 0;
	  src.w++;
	  angle++;
	  n = nx - 2;
	  while (n--) {
	    value.w = *src.w;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.b++ = (value.w < src.w[off[class]]
			 && value.w < src.w[-off[class]]);
	    src.w++;
	  }
	  *trgt.b++ = 0;
	  angle++;
	  src.w++;
	}
	break;
      case ANA_LONG:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.b++ = 0;
	  src.l++;
	  angle++;
	  n = nx - 2;
	  while (n--) {
	    value.l = *src.l;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.b++ = (value.l < src.l[off[class]]
			 && value.l < src.l[-off[class]]);
	    src.l++;
	  }
	  *trgt.b++ = 0;
	  angle++;
	  src.l++;
	}
	break;
      case ANA_FLOAT:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.b++ = 0;
	  src.f++;
	  angle++;
	  n = nx - 2;
	  while (n--) {
	    value.f = *src.f;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.b++ = (value.f < src.f[off[class]]
			 && value.f < src.f[-off[class]]);
	    src.f++;
	  }
	  *trgt.b++ = 0;
	  angle++;
	  src.f++;
	}
	break;
      case ANA_DOUBLE:
	while (ny--) {		/* all rows except for bottom one */
	  *trgt.b++ = 0;
	  src.d++;
	  angle++;
	  n = nx - 2;
	  while (n--) {
	    value.d = *src.d;
	    a = *angle++ + DEG22_5;
	    s = sin(a);
	    c = cos(a);
	    class = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
	    *trgt.b++ = (value.d < src.d[off[class]]
			 && value.d < src.d[-off[class]]);
	    src.d++;
	  }
	  *trgt.b++ = 0;
	  angle++;
	  src.d++;
	}
	break;
    }
  }
  zerobytes(trgt.b, nx);
  return result;
}
/*------------------------------------------------------------------------- */
#define N_DIRECTIONS	8
#define STACKBLOCK	256
#define EDGE		-1
#define MARKED		-2
#define EDGEMARKED	-3
int area_2d(int narg, int ps[])
/* AREA,image
 assigns labels to contiguous sets of non-zeros in the bitmap <image>.
 <image> must be a 2D LONG array. */
{
  int	*ptr, *ptr0, *ptr1, *ptrend, nx, ny, n, areaNumber, *ptr2,
    offsets[8], **stack, **stack0, **stackend, nStack, onEdge, ix = 0, iy = 0,
    ix2, iy2, direction;
  int	rcoords[8][2] = { 
    { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 },
    { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 }
  };
		      
  nx = array_dims(ps[0])[0];	/* width */
  ny = array_dims(ps[0])[1];	/* height */

  /* offsets[] lists the offsets (in elements) to one of the nearest
     neighbors in all accessible directions */
  offsets[0] = 1;
  offsets[1] = 1 + nx;
  offsets[2] = nx;
  offsets[3] = nx - 1;
  offsets[4] = -1;
  offsets[5] = -1 - nx;
  offsets[6] = -nx;
  offsets[7] = -nx + 1;

  ptr0 = ptr = array_data(ps[0]); /* data start */
  ptrend = ptr0 + array_size(ps[0]); /* points one beyond data end */
  
  /* The numbers in the input data <image> are interpreted as follows:
     Values equal to zero or greater than one indicate positions that
     are ignored.  Ones indicate positions that are to be labeled.
     Negative values are reserved for indicating intermediate results
     during execution of the subroutine: If negative values are
     present in <image> upon entry into the routine, then unexpected
     results may be obtained.
     
     Unfortunately, we cannot treat the data elements in a linear
     fashion, because the segments may not be convex in shape, so the
     intersection of a segment and any curve may have disjoint parts.
     When we find one element of a new segment, we have to find all
     other elements of the same segment before looking for another
     segment.

     When we walk through the data, trying to trace out a new segment,
     we must be careful not to run across one of the edges of the
     image.  One method to ensure this is to check for each and every
     considered element if it is on an edge, but this is inefficient
     because the checks have to be performed for all data elements
     even when the edges cover only a very small fraction of the
     image.  We do it differently: We ensure that the algorithm cannot
     run across any edges by substituting a distinct negative value
     (not confusable with other intermediate values) for any 1 found
     on the edges of the data volume.  Then checking for data on an
     edge is reduced to checking for that negative value. */

  ptr = ptr0;			/* the first data value */
  areaNumber = 2;		/* the label for the next segment */

  /* Look at the edges */
  n = nx - 1;
  while (n--) {
    if (*ptr == 1)		/* treatable data */
      *ptr = EDGE;
    ptr++;
  }
  n = ny - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr += nx;
  }
  n = nx - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr--;
  }
  n = ny - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr -= nx;
  }
  /* Data values equal to 1 or EDGE are taken to indicate elements
     that need to be assigned labels. */

  /* prepare a stack */
  nStack = STACKBLOCK;
  stack = stack0 = malloc(STACKBLOCK*sizeof(int *));
  if (!stack0)			/* allocation failed */
    return cerror(ALLOC_ERR, 0);
  stackend = stack0 + nStack;	/* pointer to one beyond end of stack */

  ptr1 = ptr0;
  do {				/* still more to do */
    while (*ptr1 != 1
	   && *ptr1 != EDGE
	   && ptr1 < ptrend)	/* current element need not be treated
				   and there are still more to go */
      ptr1++;

    if (ptr1 == ptrend)		/* we're all done */
      break;

    /* We have found a new segment to treat.  We deal with each
       segment through a width-first algorithm, as follows: For each
       found segment element (i.e., data element that contains a 1 or
       EDGE and needs to be assigned a label) connections in eight (or
       less for EDGE) directions must be checked.  We check each
       direction in turn, and push the directions in which we find
       other segment elements onto a stack.  Then we assign a label to
       the current position and start treating the elements that are
       on the stack.  If the stack becomes empty, then we are done
       with the current segment. 

       We use a width-first rather than a depth-first method because
       in the former method we only need check once if the current
       position is on an edge. */

    onEdge = (*ptr1 == EDGE);	/* 1 -> on edge; 0 -> in interior */
    if (onEdge) {		/* current position is on an edge:
				   we need the coordinates for edge testing */
      ix = (ptr1 - ptr0) % nx;
      iy = (ptr1 - ptr0)/nx;
    }

    ptr = ptr1;
    while (1) {
      if (onEdge) {
	for (direction = 0; direction < N_DIRECTIONS; direction++) {
	  /* now we must check if the neighbor is across an edge */
	  ix2 = ix + rcoords[direction][0];
	  iy2 = iy + rcoords[direction][1];
	  if (ix2 < 0 || ix2 >= nx || iy2 < 0 || iy2 >= ny)
	    /* across the edge: continue with next direction */
	    continue;		

	  /* the current direction does not lead across an edge */
	  ptr2 = ptr + offsets[direction]; /* the neighboring position */
	  if (*ptr2 == 1 || *ptr2 == EDGE) { /* neighbor must be treated */
	    *stack++ = ptr;	/* place neighbor position on stack */
	    if (stack == stackend) { /* need to enlarge the stack */
	      stack0 = realloc(stack0, (nStack + STACKBLOCK)*sizeof(int *));
	      if (!stack0)	/* allocation failed */
		return cerror(ALLOC_ERR, 0);
	      /* the call to realloc() may have moved the whole stack to
		 a different memory location; we ensure that <stack>
		 points at the same stack item as before the reallocation */
	      stack = stack0 + nStack;
	      nStack += STACKBLOCK; /* update stack size */
	      stackend = stack0 + nStack; /* and stack end pointer */
	    }
	    *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
	  }
	}
      } else {			/* not on an edge */
	for (direction = 0; direction < N_DIRECTIONS; direction++) {
	  ptr2 = ptr + offsets[direction]; /* the neighboring position */
	  if (*ptr2 == 1 || *ptr2 == EDGE) { /* neighbor must be treated */
	    *stack++ = ptr2;	/* place neighbor position on stack */
	    if (stack == stackend) { /* need to enlarge the stack */
	      stack0 = realloc(stack0, (nStack + STACKBLOCK)*sizeof(int *));
	      if (!stack0)	/* allocation failed */
		return cerror(ALLOC_ERR, 0);
	      /* the call to realloc() may have moved the whole stack to
		 a different memory location; we ensure that <stack>
		 points at the same stack item as before the reallocation */
	      stack = stack0 + nStack;
	      nStack += STACKBLOCK; /* update stack size */
	      stackend = stack0 + nStack;	/* and stack end pointer */
	    }
	    *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
	  }
	}
      }
      /* We have checked all directions for one position: assign a
	 label number and pop next position, if any, from stack */

      *ptr = areaNumber;	/* assign label */
      if (stack == stack0)	/* stack is empty: all done with this area */
	break;
      /* stack is not yet empty: pop last position from stack */
      ptr = *--stack;
    }
    /* we are done with the current segment: update label */
    areaNumber++;
  } while (1);
  free(stack0);
  return 1;
}
/*------------------------------------------------------------------------- */
int area_general(int narg, int ps[], int isFunction)
/* AREA,bitmap[,seeds,numbers,diagonal]
 <bitmap> is assumed to be a LONG array. */
/* identifies distinct areas with values equal to 1 in a bitmap.
   Syntax:  AREA,image[,seeds,NUMBERS=numbers,DIAGONAL=diagonal]
   All elements of each distinct area are replaced by a number (larger than
   1) which labels the area. <flag> indicates whether the call is a
   function call (non-zero) or a subroutine call (zero). 
   Arrays of arbitrary dimension are allowed.
   If <seeds> is specified, then only the areas in which the seeds
   (indices to the image, after conversion to LONG) lie are identified.  
   If <numbers> is a scalar, then the area numbers start with the
   specified value.  If <numbers> is an array, then it must have the
   same size as <seeds>, and contains area numbers for the areas
   in which the seeds lie.  If <numbers> is undefined, then area
   numbers start at 2.  If <numbers> has at most one element, then
   the area numbers are incremented by one for each new area.
   Note that this routine uses negative numbers to indicate intermediate
   results, so pre-existing negative numbers in the data may lead to
   faulty results.
   LS 4feb93 6aug97 */
     /* Strategy: find data point with value equal to 1.  Then check all */
     /* allowed directions in order.  If another data point is encountered */
     /* with value equal to 1, then remember how many directions were */
     /* already checked for the current point and treat the new point. */
     /* If all allowed directions are treated without finding any new */
     /* untreated points, then assign area number and resume treating */
     /* last partially-treated point. */
/* <diagonal>: one element per dimension.  0: do not check the dimension.
   1: only allow connections to neighbors that share a face in this dimension
   2: allow connections to neighbors that share a face or a vertex in this
      dimension
 LS 17jun98, 5sep98 */
{
  int	iq, *dims, ndim, nelem, nSeed, nNumber, nDirection, *seed, *number,
    i, *rcoord, *offset, j, nStack, **stack, **stack0, areaNumber,
    direction, *edge;
  int	*ptr0, *ptr, *ptrend, **stackend, *ptr1, onEdge, ix, ix2, *ptr2;
  pointer	src;
  loopInfo	srcinfo;

  if (!symbolIsNumericalArray(ps[0])) /* not a numerical array */
    return cerror(NEED_NUM_ARR, ps[0]);
  iq = ana_long(1, ps);		/* ensure LONG */
  if (standardLoop(iq, 0, SL_ALLAXES | SL_EACHCOORD, 0, &srcinfo, &src,
		   NULL, NULL, NULL) == ANA_ERROR)
    return ANA_ERROR;

  nSeed = nNumber = nDirection = 0; /* defaults */
  seed = NULL;
  if (narg > 1 && ps[1]) {	/* have <seeds> */
    if (symbol_class(ps[1]) != ANA_ARRAY) /* must be ARRAY */
      return cerror(ILL_CLASS, ps[1]);
    iq = ana_long(1, &ps[1]);	/* ensure LONG */
    seed = array_data(iq);	/* seed indices */
    nSeed = array_size(iq);	/* number of seeds */
  }

  number = NULL;		/* default: no <number> */
  if (narg > 2 && ps[2]) {	/* have <number> */
    iq = ps[2];
    if (numerical(iq, NULL, NULL, &nNumber, NULL) < 0) /* get info */
      return cerror(ILL_CLASS, iq);
    if (nNumber != 1 && nNumber != nSeed) /* one element per seed, or */
					  /* a single number */
      return cerror(INCMP_ARG, ps[2]);
    iq = ana_long(1, &iq);	/* ensure LONG */
    number = array_data(iq);	/* numbers */
    nNumber = (nNumber == nSeed)? 1: 0;
  }

  /* treat DIAGONAL */
  nDirection = prepareDiagonals(narg > 3? ps[3]: 0, &srcinfo, 1, &offset,
				&edge, &rcoord, NULL);
  if (nDirection == ANA_ERROR)
    return ANA_ERROR;
  if (!nDirection)
    return anaerror("No directions satisfy the requirements", ps[3]);

  nelem = array_size(ps[0]);	/* number of elements in array */
  if (nSeed) {
    for (i = 0; i < nSeed; i++)
      if (seed[i] < 0 || seed[i] >= nelem) { /* check if in range */
	free(offset);
	free(edge);
	return anaerror("Seed position %1d (index %1d) outside of the data",
		     seed[i], i);
      }
  }
  
  /* treat all elements that are on an edge: if they are equal to 1, then
     set them to EDGE */
  for (i = 0; i < 2*srcinfo.ndim; i++) {
    if (edge[i]) {
      rearrangeEdgeLoop(&srcinfo, NULL, i);
      do
	if (*src.l == 1)
	  *src.l = EDGE;
      while (advanceLoop(&srcinfo, &src) < srcinfo.ndim - 1);
    }
  }
  free(edge);

  rearrangeDimensionLoop(&srcinfo);

  /* get space for temporary positions.  estimate how much is needed. */
  /* if this is too little, then more will be allocated on the fly. */
  /* find greatest dimension */
  dims = array_dims(iq);
  ndim = array_num_dims(iq);
  nStack = dims[0];
  for (j = 1; j < ndim; j++)
    if (dims[j] > nStack)
      nStack = dims[j];
  nStack *= nDirection;	/* times number of directions */
  /* get space for temporary positions */
  stack = stack0 = (int **) malloc(nStack*sizeof(int *));
  if (!stack) {
    free(offset);
    free(rcoord);
    return cerror(ALLOC_ERR, 0);
  }

  if (number) {			/* have specific area numbers to assign */
    areaNumber = *number;	/* first one */
    number += nNumber;		/* zero if scalar <number>, 1 if array */
  } else
    areaNumber = 2;		/* default start value */

  ptr0 = ptr = src.l;
  ptrend = ptr0 + array_size(ps[0]);
  
  ptr = ptr0;
  stackend = stack0 + nStack;

  ptr1 = ptr0;
  do {				/* still more to do */
    if (seed) {
      while (nSeed) {
	if (*seed < 0 || *seed >= nelem)
	  return anaerror("Index (%1d) out of bounds", ps[1], *seed);
	if (ptr0[*seed] == 1 || ptr0[*seed] == EDGE)
	  break;
	seed++;
	nSeed--;
      }
      if (nSeed <= 0)
	break;			/* all done */
      ptr1 = ptr0 + *seed++;
      nSeed--;
    } else {
      while (*ptr1 != 1
	     && *ptr1 != EDGE
	     && ptr1 < ptrend)	/* current element need not be treated
				   and there are still more to go */
	ptr1++;

      if (ptr1 == ptrend)	/* we're all done */
	break;
    }

    /* We have found a new segment to treat.  We deal with each
       segment through a width-first algorithm, as follows: For each
       found segment element (i.e., data element that contains a 1 or
       EDGE and needs to be assigned a label) connections in eight (or
       less for EDGE) directions must be checked.  We check each
       direction in turn, and push the directions in which we find
       other segment elements onto a stack.  Then we assign a label to
       the current position and start treating the elements that are
       on the stack.  If the stack becomes empty, then we are done
       with the current segment. 

       We use a width-first rather than a depth-first method because
       in the former method we only need check once if the current
       position is on an edge. */

    ptr = ptr1;
    while (1) {
      onEdge = (*ptr == EDGE || *ptr == EDGEMARKED); /* 1 -> on edge;
							0 -> in interior */
      if (onEdge) {		/* current position is on an edge:
				   we need the coordinates for edge testing */
	ix = ptr - ptr0;	/* index */
	for (i = 0; i < ndim; i++) {
	  srcinfo.coords[i] = ix % srcinfo.dims[i];
	  ix /= srcinfo.dims[i];
	} /* end of for (i = 0,...) */
	for (direction = 0; direction < nDirection; direction++) {
	  /* now we must check if the neighbor is across an edge */
	  for (i = 0; i < ndim; i++) {
	    ix2 = srcinfo.coords[i] + rcoord[i + direction*ndim];
	    if (ix2 < 0 || ix2 >= dims[i]) /* over the egde */
	      break;
	  } /* end of for (i = 0,...) */
	  if (i < ndim)		/* over the edge */
	    continue;

	  /* the current direction does not lead across an edge */
	  ptr2 = ptr + offset[direction]; /* the neighboring position */
	  if (*ptr2 == 1 || *ptr2 == EDGE) { /* neighbor must be treated */
	    *stack++ = ptr2;	/* place neighbor position on stack */
	    if (stack == stackend) { /* need to enlarge the stack */
	      stack0 = realloc(stack0, (nStack + STACKBLOCK)*sizeof(int *));
	      if (!stack0)	/* allocation failed */
		return cerror(ALLOC_ERR, 0);
	      /* the call to realloc() may have moved the whole stack to
		 a different memory location; we ensure that <stack>
		 points at the same stack item as before the reallocation */
	      stack = stack0 + nStack;
	      nStack += STACKBLOCK; /* update stack size */
	      stackend = stack0 + nStack; /* and stack end pointer */
	    } /* end of if (stack == stackend) */
	    *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED; /* mark it */
	  } /* end of if (*ptr2 == 1 | *ptr2 == EDGE) */
	} /* end of for (direction = 0; direction < nDirection; direction++) */
      } else {			/* not on an edge */
	for (direction = 0; direction < nDirection; direction++) {
	  ptr2 = ptr + offset[direction]; /* the neighboring position */
	  if (*ptr2 == 1 || *ptr2 == EDGE) { /* neighbor must be treated */
	    *stack++ = ptr2;	/* place neighbor position on stack */
	    if (stack == stackend) { /* need to enlarge the stack */
	      stack0 = realloc(stack0, (nStack + STACKBLOCK)*sizeof(int *));
	      if (!stack0)	/* allocation failed */
		return cerror(ALLOC_ERR, 0);
	      /* the call to realloc() may have moved the whole stack to
		 a different memory location; we ensure that <stack>
		 points at the same stack item as before the reallocation */
	      stack = stack0 + nStack;
	      nStack += STACKBLOCK; /* update stack size */
	      stackend = stack0 + nStack;	/* and stack end pointer */
	    } /* end of if (stack == stackend) */
	    *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
	  } /* end of if (*ptr2 == 1 || *ptr2 == EDGE)  */
	} /* end of for (direction = 0; ...) */
      }	/* end of if (onEdge) else */
      /* We have checked all directions for one position: assign a
	 label number and pop next position, if any, from stack */

      *ptr = areaNumber;	/* assign label */
      if (stack == stack0)	/* stack is empty: all done with this area */
	break;
      /* stack is not yet empty: pop last position from stack */
      ptr = *--stack;

    } /* end of while (1) */
    /* we are done with the current segment: update label */
    if (nNumber) {
      areaNumber = *number++;
      nNumber--;
    } else
      areaNumber++;
  } while (1);

  free(rcoord);
  free(offset);
  free(stack0);
  return 1;
}
/*------------------------------------------------------------------------- */
int ana_area(int narg, int ps[])
/* AREA,bitmap [, SEED=<seed>, NUMBERS=<numbers>, DIAGONAL=<diagonal>] */
{
  if (!symbolIsNumericalArray(ps[0]) || array_type(ps[0]) != ANA_LONG)
    return anaerror("Need LONG array", ps[0]);
  
  if (array_num_dims(ps[0]) == 2
      && narg == 1)
    return area_2d(narg, ps);

  return area_general(narg, ps, 0);
}
/*----------------------------------------------------------------------*/
#define SEEK_MAXIMUM	(ANA_DOUBLE + 1)
int area2_2d(int narg, int ps[])
/* AREA2,bitmap,data [,sign]
 assigns numerical labels to those elements of <bitmap> that have values
 equal to 1.  A particular label is assigned to all eligible elements that
 are connected in nearest-neighbor fashion through vertices or edes.
 It is assumed that (1) <bitmap> is a 2D LONG array; and (2) <data>
 is a 2D array with the same dimensions as <bitmap>.  LS 5sep98 */
{
  int	*ptr, *ptr0, *ptr1, *ptrend, nx, ny, n, areaNumber, *ptr2,
    offset[8], **stack, **stack0, **stackend, nStack, onEdge, ix = 0, iy = 0,
    ix2, iy2, direction, maximum, j;
  int	stride, type;
  int	rcoords[8][2] = { { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 },
			  { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 } };
  pointer	dataptr0, dataptr, dataptr2;
		      
  nx = array_dims(ps[0])[0];	/* width */
  ny = array_dims(ps[0])[1];	/* height */
  dataptr0.l = array_data(ps[1]);

  type = array_type(ps[1]);
  stride = ana_type_size[type];

  maximum = (narg > 2 && ps[2])? (int_arg(ps[2]) >= 0 ? 1: 0): 1;
  maximum *= SEEK_MAXIMUM;

  /* offsets[] lists the offsets (in elements) to one of the nearest
     neighbors in all accessible directions */
  offset[0] = 1;
  offset[1] = -1 + nx;
  offset[2] = nx;
  offset[3] = nx + 1;
  offset[4] = -nx - 1;
  offset[5] = -nx;
  offset[6] = 1 - nx;
  offset[7] = -1;

  ptr0 = ptr = array_data(ps[0]); /* bitmap start */
  ptrend = ptr0 + array_size(ps[0]); /* points one beyond bitmap end */
  
  /* The numbers in the input <bitmap> are interpreted as follows:
     Values equal to zero or greater than one indicate positions that
     are ignored.  Ones indicate positions that are to be labeled.
     Negative values are reserved for indicating intermediate results
     during execution of the subroutine: If negative values are
     present in <bitmap> upon entry into the routine, then unexpected
     results may be obtained.
     
     Unfortunately, we cannot treat the data elements in a linear
     fashion, because the segments may not be convex in shape, so the
     intersection of a segment and any curve may have disjoint parts.
     When we find one element of a new segment, we have to find all
     other elements of the same segment before looking for another
     segment.

     When we walk through the data, trying to trace out a new segment,
     we must be careful not to run across one of the edges of the
     image.  One method to ensure this is to check for each and every
     considered element if it is on an edge, but this is inefficient
     because the checks have to be performed for all data elements
     even when the edges cover only a very small fraction of the
     image.  We do it differently: We ensure that the algorithm cannot
     run across any edges by substituting a distinct negative value
     (not confusable with other intermediate values) for any 1 found
     on the edges of the data volume.  Then checking for data on an
     edge is reduced to checking for that negative value. */

  ptr = ptr0;			/* the first data value */
  areaNumber = 2;		/* the label for the next segment */

  /* Look at the edges */
  n = nx - 1;
  while (n--) {
    if (*ptr == 1)		/* treatable data */
      *ptr = EDGE;
    ptr++;
  }
  n = ny - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr += nx;
  }
  n = nx - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr--;
  }
  n = ny - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr -= nx;
  }
  /* Data values equal to 1 or EDGE are taken to indicate elements
     that need to be assigned labels. */

  /* prepare a stack */
  nStack = STACKBLOCK;
  stack = stack0 = malloc(STACKBLOCK*sizeof(int *));
  if (!stack0)			/* allocation failed */
    return cerror(ALLOC_ERR, 0);
  stackend = stack0 + nStack;	/* pointer to one beyond end of stack */

  ptr1 = ptr0;
  do {				/* still more to do */
    while (*ptr1 != 1
	   && *ptr1 != EDGE
	   && ptr1 < ptrend)	/* current element need not be treated
				   and there are still more to go */
      ptr1++;

    if (ptr1 == ptrend)		/* we're all done */
      break;

    /* We have found a new segment to treat.  Those elements that are
       connected to a single appropriate local extreme through
       steepest ascent/descent paths get the same numerical label.
       This means that different appropriate local extremes will be
       assigned to differently labeled areas.  It also means that in
       some cases the assignment of a particular element to one or
       another segment may depend on which element of the segment(s)
       happens to be seen first.

       We arrange this as follows: From the just discovered position,
       walk in steepest ascent/descent fashion to the associated local
       extreme in <data> but only through data elements that are
       flagged as part of some segment (i.e., that have a bitmap value
       equal to 1).  Then we start assigning labels as for the case
       where no data values are inspected.  We use this restricted
       sense of "local extreme" because it is possible for a segment
       to contain no local extreme when taking into account all
       neighbors whether they are segment elements or not.

       We deal with each segment through a width-first algorithm, as
       follows: For each found segment element (i.e., data element
       that contains a 1 or EDGE and needs to be assigned a label)
       connections in eight (or less for EDGE) directions must be
       checked.  We check each direction in turn, and push the
       directions in which we find other segment elements onto a
       stack.  Then we assign a label to the current position and
       start treating the elements that are on the stack.  If the
       stack becomes empty, then we are done with the current segment.

       We use a width-first rather than a depth-first method because
       in the former method we only need check once if the current
       position is on an edge.  */

    /* We walk to the nearest extreme of the sought kind */
    ptr = ptr1;
    dataptr.b = dataptr0.b + (ptr1 - ptr0)*stride; /* corresponding data */
    while (1) {
      j = -1;
      dataptr2 = dataptr;
      if (*ptr == EDGE) {	/* get the coordinates */
	iy = ptr - ptr0;
	ix = iy % nx;
	iy /= nx;
      }
      for (direction = 0; direction < N_DIRECTIONS; direction++) {
	if (ptr[offset[direction]] != 1
	    && ptr[offset[direction]] != EDGE) /* not active */
	  continue;
	if (*ptr == EDGE) {
	  /* now we must check if the neighbor is across an edge */
	  ix2 = ix + rcoords[direction][0];
	  iy2 = iy + rcoords[direction][1];
	  if (ix2 < 0 || ix2 >= nx || iy2 < 0 || iy2 >= ny)
	    /* across the edge: continue with next direction */
	    continue;
	}
	switch (maximum + type) {
	  case SEEK_MAXIMUM + ANA_BYTE:
	    if (dataptr.b[offset[direction]] > *dataptr2.b) {
	      j = direction;
	      dataptr2.b = dataptr.b + offset[direction];
	    }
	    break;
	  case SEEK_MAXIMUM + ANA_WORD:
	    if (dataptr.w[offset[direction]] > *dataptr2.w) {
	      j = direction;
	      dataptr2.w = dataptr.w + offset[direction];
	    }
	    break;
	  case SEEK_MAXIMUM + ANA_LONG:
	    if (dataptr.l[offset[direction]] > *dataptr2.l) {
	      j = direction;
	      dataptr2.l = dataptr.l + offset[direction];
	    }
	    break;
	  case SEEK_MAXIMUM + ANA_FLOAT:
	    if (dataptr.f[offset[direction]] > *dataptr2.f) {
	      j = direction;
	      dataptr2.f = dataptr.f + offset[direction];
	    }
	    break;
	  case SEEK_MAXIMUM + ANA_DOUBLE:
	    if (dataptr.d[offset[direction]] > *dataptr2.d) {
	      j = direction;
	      dataptr2.d = dataptr.d + offset[direction];
	    }
	    break;
	  case ANA_BYTE:
	    if (dataptr.b[offset[direction]] < *dataptr2.b) {
	      j = direction;
	      dataptr2.b = dataptr.b + offset[direction];
	    }
	    break;
	  case ANA_WORD:
	    if (dataptr.w[offset[direction]] < *dataptr2.w) {
	      j = direction;
	      dataptr2.w = dataptr.w + offset[direction];
	    }
	    break;
	  case ANA_LONG:
	    if (dataptr.l[offset[direction]] < *dataptr2.l) {
	      j = direction;
	      dataptr2.l = dataptr.l + offset[direction];
	    }
	    break;
	  case ANA_FLOAT:
	    if (dataptr.f[offset[direction]] < *dataptr2.f) {
	      j = direction;
	      dataptr2.f = dataptr.f + offset[direction];
	    }
	    break;
	  case ANA_DOUBLE:
	    if (dataptr.d[offset[direction]] < *dataptr2.d) {
	      j = direction;
	      dataptr2.d = dataptr.d + offset[direction];
	    }
	    break;
	}	
      }
      if (j != -1) {		/* not yet found the extreme */
	/* update pointers */
	ptr += offset[j];	/* to bitmap */
	dataptr.b += offset[j]*stride; /* to data */
      } else
	break;
    }

    /* We're at the position of the nearest desired extreme.
     Now we start identifying and assigning labels to the elements
     of the current segment.  We make sure to only walk downhill
     if we were looking for a maximum, or uphill if we were looking
     for a minimum. */

    onEdge = (*ptr == EDGE);	/* 1 -> on edge; 0 -> in interior */
    if (onEdge) {		/* current position is on an edge:
				   we need the coordinates for edge testing */
      ix = (ptr - ptr0) % nx;
      iy = (ptr - ptr0)/nx;
    }

    while (1) {
      if (onEdge) {
	for (direction = 0; direction < N_DIRECTIONS; direction++) {
	  /* now we must check if the neighbor is across an edge */
	  ix2 = ix + rcoords[direction][0];
	  iy2 = iy + rcoords[direction][1];
	  if (ix2 < 0 || ix2 >= nx || iy2 < 0 || iy2 >= ny)
	    /* across the edge: continue with next direction */
	    continue;		

	  /* the current direction does not lead across an edge */
	  ptr2 = ptr + offset[direction]; /* the neighboring position */
	  if (*ptr2 != 1 && *ptr2 != EDGE)
	    continue;
	  switch (maximum + type) {
	    case SEEK_MAXIMUM + ANA_BYTE:
	      if (dataptr.b[offset[direction]] >= *dataptr.b)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_WORD:
	      if (dataptr.w[offset[direction]] >= *dataptr.w)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_LONG:
	      if (dataptr.l[offset[direction]] >= *dataptr.l)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_FLOAT:
	      if (dataptr.f[offset[direction]] >= *dataptr.f)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_DOUBLE:
	      if (dataptr.d[offset[direction]] >= *dataptr.d)
		continue;
	      break;
	    case ANA_BYTE:
	      if (dataptr.b[offset[direction]] <= *dataptr.b)
		continue;
	      break;
	    case ANA_WORD:
	      if (dataptr.w[offset[direction]] <= *dataptr.w)
		continue;
	      break;
	    case ANA_LONG:
	      if (dataptr.l[offset[direction]] <= *dataptr.l)
		continue;
	      break;
	    case ANA_FLOAT:
	      if (dataptr.f[offset[direction]] <= *dataptr.f)
		continue;
	      break;
	    case ANA_DOUBLE:
	      if (dataptr.d[offset[direction]] <= *dataptr.d)
		continue;
	      break;
	  }

	  /* the current position is accepted */

	  *stack++ = ptr;	/* place neighbor position on stack */
	  if (stack == stackend) { /* need to enlarge the stack */
	    stack0 = realloc(stack0, (nStack + STACKBLOCK)*sizeof(int *));
	    if (!stack0)	/* allocation failed */
	      return cerror(ALLOC_ERR, 0);
	    /* the call to realloc() may have moved the whole stack to
	       a different memory location; we ensure that <stack>
	       points at the same stack item as before the reallocation */
	    stack = stack0 + nStack;
	    nStack += STACKBLOCK; /* update stack size */
	    stackend = stack0 + nStack; /* and stack end pointer */
	  }
	  *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
	}
      } else {			/* not on an edge */
	for (direction = 0; direction < N_DIRECTIONS; direction++) {
	  ptr2 = ptr + offset[direction]; /* the neighboring position */
	  if (*ptr2 != 1 && *ptr2 != EDGE)
	    continue;
	  switch (maximum + type) {
	    case SEEK_MAXIMUM + ANA_BYTE:
	      if (dataptr.b[offset[direction]] >= *dataptr.b)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_WORD:
	      if (dataptr.w[offset[direction]] >= *dataptr.w)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_LONG:
	      if (dataptr.l[offset[direction]] >= *dataptr.l)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_FLOAT:
	      if (dataptr.f[offset[direction]] >= *dataptr.f)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_DOUBLE:
	      if (dataptr.d[offset[direction]] >= *dataptr.d)
		continue;
	      break;
	    case ANA_BYTE:
	      if (dataptr.b[offset[direction]] <= *dataptr.b)
		continue;
	      break;
	    case ANA_WORD:
	      if (dataptr.w[offset[direction]] <= *dataptr.w)
		continue;
	      break;
	    case ANA_LONG:
	      if (dataptr.l[offset[direction]] <= *dataptr.l)
		continue;
	      break;
	    case ANA_FLOAT:
	      if (dataptr.f[offset[direction]] <= *dataptr.f)
		continue;
	      break;
	    case ANA_DOUBLE:
	      if (dataptr.d[offset[direction]] <= *dataptr.d)
		continue;
	      break;
	  }

	  /* the current position is accepted */
	  
	  *stack++ = ptr2;	/* place neighbor position on stack */
	  if (stack == stackend) { /* need to enlarge the stack */
	    stack0 = realloc(stack0, (nStack + STACKBLOCK)*sizeof(int *));
	    if (!stack0)	/* allocation failed */
	      return cerror(ALLOC_ERR, 0);
	    /* the call to realloc() may have moved the whole stack to
	       a different memory location; we ensure that <stack>
	       points at the same stack item as before the reallocation */
	    stack = stack0 + nStack;
	    nStack += STACKBLOCK; /* update stack size */
	    stackend = stack0 + nStack;	/* and stack end pointer */
	  }
	  *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
	}
      }

      /* We have checked all directions for one position: assign a
	 label number and pop next position, if any, from stack */

      *ptr = areaNumber;	/* assign label */
      if (stack == stack0)	/* stack is empty: all done with this area */
	break;
      /* stack is not yet empty: pop last position from stack */
      ptr = *--stack;
      dataptr.b = dataptr0.b + (ptr - ptr0)*stride; /* corresponding data */
    }
    /* we are done with the current segment: update label */
    areaNumber++;
  } while (1);
  free(stack0);
  return 1;
}
/*----------------------------------------------------------------------*/
int area2_general(int narg, int ps[])
/* AREA,bitmap,data[,seed,numbers,diagonal,sign]
 <bitmap> is assumed to be a LONG array. */
/* identifies distinct areas with values equal to 1 in a bitmap.
   Syntax:  AREA,image,data,[,seeds,NUMBERS=numbers,DIAGONAL=diagonal]
   All elements of each distinct area are replaced by a number (larger than
   1) which labels the area. <flag> indicates whether the call is a
   function call (non-zero) or a subroutine call (zero). 
   Arrays of arbitrary dimension are allowed.
   If <seeds> is specified, then only the areas in which the seeds
   (indices to the image, after conversion to LONG) lie are identified.  
   If <numbers> is a scalar, then the area numbers start with the
   specified value.  If <numbers> is an array, then it must have the
   same size as <seeds>, and contains area numbers for the areas
   in which the seeds lie.  If <numbers> is undefined, then area
   numbers start at 2.  If <numbers> has at most one element, then
   the area numbers are incremented by one for each new area.
   Note that this routine uses negative numbers to indicate intermediate
   results, so pre-existing negative numbers in the data may lead to
   faulty results.
   LS 4feb93 6aug97 */
     /* Strategy: find data point with value equal to 1.  Then check all */
     /* allowed directions in order.  If another data point is encountered */
     /* with value equal to 1, then remember how many directions were */
     /* already checked for the current point and treat the new point. */
     /* If all allowed directions are treated without finding any new */
     /* untreated points, then assign area number and resume treating */
     /* last partially-treated point. */
/* <diagonal>: one element per dimension.  0: do not check the dimension.
   1: only allow connections to neighbors that share a face in this dimension
   2: allow connections to neighbors that share a face or a vertex in this
      dimension
 LS 17jun98, 5sep98 */
{
  int	iq, *dims, ndim, nelem, nSeed, nNumber, nDirection, *seed, *number,
    i, *rcoord, *offset, j, nStack, **stack, **stack0, areaNumber,
    direction, stride, maximum, type, *edge;
  int	*ptr0, *ptr, *ptrend, **stackend, *ptr1, onEdge, ix, ix2, *ptr2;
  pointer	src, dataptr0, dataptr, dataptr2;
  loopInfo	srcinfo;

  if (!symbolIsNumericalArray(ps[0])) /* not a numerical array */
    return cerror(NEED_NUM_ARR, ps[0]);
  iq = ana_long(1, ps);		/* ensure LONG */
  if (standardLoop(iq, 0, SL_ALLAXES | SL_EACHCOORD, 0, &srcinfo, &src,
		   NULL, NULL, NULL) == ANA_ERROR)
    return ANA_ERROR;

  ptr0 = src.l;

  if (!symbolIsNumericalArray(ps[1]))
    return cerror(NEED_NUM_ARR, ps[1]);
  if (array_size(ps[1]) != array_size(iq)) /* must have same size */
    return cerror(INCMP_ARG, ps[1]);
  dataptr0.b = array_data(ps[1]);
  type = array_type(ps[1]);
  stride = ana_type_size[type];
  maximum = (narg > 5 && ps[5])? (int_arg(ps[5]) >= 0 ? 1: 0): 1;
  maximum *= SEEK_MAXIMUM;
 
  nSeed = nNumber = nDirection = 0;		/* defaults */
  seed = NULL;
  if (narg > 2 && ps[2]) {	/* have <seeds> */
    if (symbol_class(ps[2]) != ANA_ARRAY) /* must be ARRAY */
      return cerror(ILL_CLASS, ps[2]);
    iq = ana_long(1, &ps[2]);	/* ensure LONG */
    seed = array_data(iq);	/* seed indices */
    nSeed = array_size(iq);	/* number of seeds */
  }

  nelem = array_size(ps[0]);
  if (nSeed) {
    for (i = 0; i < nSeed; i++)
      if (seed[i] < 0 || seed[i] >= nelem) /* check if in range */
	return anaerror("Seed position %1d (index %1d) outside of the data",
		     ps[2], seed[i], i);
  }

  number = NULL;		/* default: no <number> */
  if (narg > 3 && ps[3]) {	/* have <number> */
    iq = ps[3];
    if (numerical(iq, NULL, NULL, &nNumber, NULL) < 0) /* get info */
      return cerror(ILL_CLASS, iq);
    if (nNumber != 1 && nNumber != nSeed) /* one element per seed, or */
					  /* a single number */
      return cerror(INCMP_ARG, ps[3]);
    iq = ana_long(1, &iq);	/* ensure LONG */
    number = array_data(iq);	/* numbers */
    nNumber = (nNumber == nSeed)? 1: 0;
  }

  nDirection = prepareDiagonals(narg > 4? ps[4]: 0, &srcinfo, 1, &offset,
				&edge, &rcoord, NULL);
  if (nDirection == ANA_ERROR)
    return ANA_ERROR;
  if (!nDirection)
    return anaerror("No directions satisfy the requirements", ps[4]);

  /* we mark all data to be treated that is on an edge */
  for (i = 0; i < 2*srcinfo.rndim; i++) {
    rearrangeEdgeLoop(&srcinfo, NULL, i);
    do {
      if (*src.l == 1)
	*src.l = EDGE;
    } while (advanceLoop(&srcinfo, &src) < srcinfo.rndim - 1);
  }
  free(edge);

  rearrangeDimensionLoop(&srcinfo);

  /* prepare a stack */
  nStack = STACKBLOCK;
  stack = stack0 = malloc(STACKBLOCK*sizeof(int *));
  if (!stack0) {
    free(offset);
    free(rcoord);
    return cerror(ALLOC_ERR, 0);
  }
  stackend = stack0 + nStack;

  if (number) {			/* have specific area numbers to assign */
    areaNumber = *number;	/* first one */
    number += nNumber;		/* zero if scalar <number>, 1 if array */
  } else
    areaNumber = 2;		/* default start value */

  ptrend = ptr0 + array_size(ps[0]);
  
  ptr = ptr0;
  stackend = stack0 + nStack;

  ndim = srcinfo.rndim;
  dims = srcinfo.dims;

  ptr1 = ptr0;
  do {				/* still more to do */
    if (seed) {
      while (nSeed) {
	if (ptr0[*seed] == 1 || ptr0[*seed] == EDGE)
	  break;
	seed++;
	nSeed--;
      }
      if (nSeed <= 0)
	break;			/* all done */
      ptr1 = ptr0 + *seed++;
      nSeed--;
    } else {
      while (*ptr1 != 1
	     && *ptr1 != EDGE
	     && ptr1 < ptrend)	/* current element need not be treated
				   and there are still more to go */
	ptr1++;

      if (ptr1 == ptrend)	/* we're all done */
	break;
    }

    /* We have found a new segment to treat.  We deal with each
       segment through a width-first algorithm, as follows: For each
       found segment element (i.e., data element that contains a 1 or
       EDGE and needs to be assigned a label) connections in all allowed
       directions must be checked.  We check each
       direction in turn, and push the directions in which we find
       other segment elements onto a stack.  Then we assign a label to
       the current position and start treating the elements that are
       on the stack.  If the stack becomes empty, then we are done
       with the current segment. 

       We use a width-first rather than a depth-first method because
       in the former method we only need check once if the current
       position is on an edge. */

    /* We walk to the nearest extreme of the sought kind */
    ptr = ptr1;
    dataptr.b = dataptr0.b + (ptr1 - ptr0)*stride; /* corresponding data */
    while (1) {
      j = -1;
      dataptr2 = dataptr;
      onEdge = (*ptr == EDGE);
      if (onEdge) {
	/* now we must check if the neighbor is across an edge */
	ix = ptr - ptr0;	/* index */
	for (i = 0; i < ndim; i++) {
	  srcinfo.coords[i] = ix % srcinfo.dims[i];
	  ix /= srcinfo.dims[i];
	}
      }

      for (direction = 0; direction < nDirection; direction++) {
	if (ptr[offset[direction]] != 1
	    && ptr[offset[direction]] != EDGE) /* not active */
	  continue;
	if (*ptr == EDGE) {
	  for (i = 0; i < ndim; i++) {
	    ix2 = srcinfo.coords[i] + rcoord[i + direction*ndim];
	    if (ix2 < 0 || ix2 >= dims[i])
	      /* across the edge: continue with next direction */
	      continue;
	  }
	}
	switch (maximum + type) {
	  case SEEK_MAXIMUM + ANA_BYTE:
	    if (dataptr.b[offset[direction]] > *dataptr2.b) {
	      j = direction;
	      dataptr2.b = dataptr.b + offset[direction];
	    }
	    break;
	  case SEEK_MAXIMUM + ANA_WORD:
	    if (dataptr.w[offset[direction]] > *dataptr2.w) {
	      j = direction;
	      dataptr2.w = dataptr.w + offset[direction];
	    }
	    break;
	  case SEEK_MAXIMUM + ANA_LONG:
	    if (dataptr.l[offset[direction]] > *dataptr2.l) {
	      j = direction;
	      dataptr2.l = dataptr.l + offset[direction];
	    }
	    break;
	  case SEEK_MAXIMUM + ANA_FLOAT:
	    if (dataptr.f[offset[direction]] > *dataptr2.f) {
	      j = direction;
	      dataptr2.f = dataptr.f + offset[direction];
	    }
	    break;
	  case SEEK_MAXIMUM + ANA_DOUBLE:
	    if (dataptr.d[offset[direction]] > *dataptr2.d) {
	      j = direction;
	      dataptr2.d = dataptr.d + offset[direction];
	    }
	    break;
	  case ANA_BYTE:
	    if (dataptr.b[offset[direction]] < *dataptr2.b) {
	      j = direction;
	      dataptr2.b = dataptr.b + offset[direction];
	    }
	    break;
	  case ANA_WORD:
	    if (dataptr.w[offset[direction]] < *dataptr2.w) {
	      j = direction;
	      dataptr2.w = dataptr.w + offset[direction];
	    }
	    break;
	  case ANA_LONG:
	    if (dataptr.l[offset[direction]] < *dataptr2.l) {
	      j = direction;
	      dataptr2.l = dataptr.l + offset[direction];
	    }
	    break;
	  case ANA_FLOAT:
	    if (dataptr.f[offset[direction]] < *dataptr2.f) {
	      j = direction;
	      dataptr2.f = dataptr.f + offset[direction];
	    }
	    break;
	  case ANA_DOUBLE:
	    if (dataptr.d[offset[direction]] < *dataptr2.d) {
	      j = direction;
	      dataptr2.d = dataptr.d + offset[direction];
	    }
	    break;
	}	
      }
      if (j != -1) {		/* not yet found the extreme */
	/* update pointers */
	ptr += offset[j];	/* to bitmap */
	dataptr.b += offset[j]*stride; /* to data */
      } else
	break;
    }

    /* We're at the position of the nearest desired extreme.
     Now we start identifying and assigning labels to the elements
     of the current segment.  We make sure to only walk downhill
     if we were looking for a maximum, or uphill if we were looking
     for a minimum. */

    onEdge = (*ptr == EDGE);	/* 1 -> on edge; 0 -> in interior */
    if (onEdge) {		/* current position is on an edge:
				   we need the coordinates for edge testing */
      ix = ptr - ptr0;		/* index */
      for (i = 0; i < ndim; i++) {
	srcinfo.coords[i] = ix % srcinfo.dims[i];
	ix /= srcinfo.dims[i];
      }
    }

    while (1) {
      if (onEdge) {
	for (direction = 0; direction < nDirection; direction++) {
	  /* now we must check if the neighbor is across an edge */
	  for (i = 0; i < ndim; i++) {
	    ix2 = srcinfo.coords[i] + rcoord[i + direction*ndim];
	    if (ix2 < 0 || ix2 >= dims[i]) /* over the egde */
	      continue;
	  }

	  /* the current direction does not lead across an edge */
	  ptr2 = ptr + offset[direction]; /* the neighboring position */
	  if (*ptr2 != 1 && *ptr2 != EDGE)
	    continue;
	  switch (maximum + type) {
	    case SEEK_MAXIMUM + ANA_BYTE:
	      if (dataptr.b[offset[direction]] >= *dataptr.b)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_WORD:
	      if (dataptr.w[offset[direction]] >= *dataptr.w)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_LONG:
	      if (dataptr.l[offset[direction]] >= *dataptr.l)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_FLOAT:
	      if (dataptr.f[offset[direction]] >= *dataptr.f)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_DOUBLE:
	      if (dataptr.d[offset[direction]] >= *dataptr.d)
		continue;
	      break;
	    case ANA_BYTE:
	      if (dataptr.b[offset[direction]] <= *dataptr.b)
		continue;
	      break;
	    case ANA_WORD:
	      if (dataptr.w[offset[direction]] <= *dataptr.w)
		continue;
	      break;
	    case ANA_LONG:
	      if (dataptr.l[offset[direction]] <= *dataptr.l)
		continue;
	      break;
	    case ANA_FLOAT:
	      if (dataptr.f[offset[direction]] <= *dataptr.f)
		continue;
	      break;
	    case ANA_DOUBLE:
	      if (dataptr.d[offset[direction]] <= *dataptr.d)
		continue;
	      break;
	  }

	  /* the current position is accepted */

	  *stack++ = ptr;	/* place neighbor position on stack */
	  if (stack == stackend) { /* need to enlarge the stack */
	    stack0 = realloc(stack0, (nStack + STACKBLOCK)*sizeof(int *));
	    if (!stack0)	/* allocation failed */
	      return cerror(ALLOC_ERR, 0);
	    /* the call to realloc() may have moved the whole stack to
	       a different memory location; we ensure that <stack>
	       points at the same stack item as before the reallocation */
	    stack = stack0 + nStack;
	    nStack += STACKBLOCK; /* update stack size */
	    stackend = stack0 + nStack; /* and stack end pointer */
	  }
	  *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
	}
      } else {			/* not on an edge */
	for (direction = 0; direction < nDirection; direction++) {
	  ptr2 = ptr + offset[direction]; /* the neighboring position */
	  if (*ptr2 != 1 && *ptr2 != EDGE)
	    continue;
	  switch (maximum + type) {
	    case SEEK_MAXIMUM + ANA_BYTE:
	      if (dataptr.b[offset[direction]] >= *dataptr.b)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_WORD:
	      if (dataptr.w[offset[direction]] >= *dataptr.w)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_LONG:
	      if (dataptr.l[offset[direction]] >= *dataptr.l)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_FLOAT:
	      if (dataptr.f[offset[direction]] >= *dataptr.f)
		continue;
	      break;
	    case SEEK_MAXIMUM + ANA_DOUBLE:
	      if (dataptr.d[offset[direction]] >= *dataptr.d)
		continue;
	      break;
	    case ANA_BYTE:
	      if (dataptr.b[offset[direction]] <= *dataptr.b)
		continue;
	      break;
	    case ANA_WORD:
	      if (dataptr.w[offset[direction]] <= *dataptr.w)
		continue;
	      break;
	    case ANA_LONG:
	      if (dataptr.l[offset[direction]] <= *dataptr.l)
		continue;
	      break;
	    case ANA_FLOAT:
	      if (dataptr.f[offset[direction]] <= *dataptr.f)
		continue;
	      break;
	    case ANA_DOUBLE:
	      if (dataptr.d[offset[direction]] <= *dataptr.d)
		continue;
	      break;
	  }

	  /* the current position is accepted */
	  
	  *stack++ = ptr2;	/* place neighbor position on stack */
	  if (stack == stackend) { /* need to enlarge the stack */
	    stack0 = realloc(stack0, (nStack + STACKBLOCK)*sizeof(int *));
	    if (!stack0)	/* allocation failed */
	      return cerror(ALLOC_ERR, 0);
	    /* the call to realloc() may have moved the whole stack to
	       a different memory location; we ensure that <stack>
	       points at the same stack item as before the reallocation */
	    stack = stack0 + nStack;
	    nStack += STACKBLOCK; /* update stack size */
	    stackend = stack0 + nStack;	/* and stack end pointer */
	  }
	  *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
	}
      }
      /* We have checked all directions for one position: assign a
	 label number and pop next position, if any, from stack */

      *ptr = areaNumber;	/* assign label */
      if (stack == stack0)	/* stack is empty: all done with this area */
	break;
      /* stack is not yet empty: pop last position from stack */
      ptr = *--stack;
      dataptr.b = dataptr0.b + (ptr - ptr0)*stride; /* corresponding data */
    }
    /* we are done with the current segment: update label */
    if (nNumber) {
      areaNumber = *number++;
      nNumber--;
    } else
      areaNumber++;
  } while (1);

  free(rcoord);
  free(offset);
  free(stack0);
  return 1;
}
/*----------------------------------------------------------------------*/
int ana_area2(int narg, int ps[])
{
  if (!symbolIsNumericalArray(ps[0]) || array_type(ps[0]) != ANA_LONG)
    return anaerror("Need LONG array", ps[0]);
  if (!symbolIsNumericalArray(ps[1]))
    return cerror(NEED_ARR, ps[1]);
  if (array_size(ps[0]) != array_size(ps[1]))
    return cerror(INCMP_ARG, ps[1]);
  
  if (array_num_dims(ps[0]) == 2
      && narg == 2)
    return area2_2d(narg, ps);

  return area2_general(narg, ps);
}
/*----------------------------------------------------------------------*/
int ana_basin(int narg, int ps[])
/* Returns a basin map derived from altitude map <im>
   Syntax:  y = basin(im [,/DIFFERENCE,/SINK,/NUMBER])
   <mode>:  1 /NUMBER -> assign different number to each basin, starting
              with number 1 and increasing in steps of one.
            2 /SINK -> assign index to <im> of the final sink corresponding
              to each pixel.  Similar to 1, but with different labels for
	      each basin
            4 /DIFFERENCE -> return index offset of steepest-descent
	      neighbor for each pixel
   LS 19feb93 9may97 */
{
 int	iq, result_sym, nx, ny, col, row, *wsh, nx0;
 int	mode, i, n, nsinks, *code, n0;
 register byte	loc;
 byte	end, locs[3];
 array	*h;
 register float	*alt, min0, min1 = 0.0, min2 = 0.0, min;

 iq = ps[0];				/* altitude map */
 CK_ARR(iq,1);
 iq = ana_float(1, &iq);		/* make float for easy programming */
 h = HEAD(iq);
 if (h->ndim != 2)
   return cerror(NEED_2D_ARR, iq);
 nx = h->dims[0];		/* dimensions */
 ny = h->dims[1];
 alt = (float *) LPTR(h);	/* altitudes (i.e. data) */
 result_sym = array_clone(iq, ANA_LONG);
 h = HEAD(result_sym);
 wsh = LPTR(h);			/* result map */
 switch (internalMode & 7) {
 case 1:			/* /NUMBER */
   mode = 0;
   break;
 case 2:			/* /SINK */
   mode = 1;
   break;
 case 4:			/* /DIFFERENCE */
   mode = 2;
   break;
 default:
   mode = 0;
   break;
 }

 /* strategy: we compare the data in units of a column of 3 elements */
 /* centered on the element of interest.  min2 will contain the minimum */
 /* value found in such a unit, and locs[2] the index of the location */
 /* of the minimum in the unit: 0 for the bottom (lowest y) element, */
 /* 1 for the middle element, and 2 for the top (highest y) element. */
 /* we shift the min/locs data to the left and repeat, until we have */
 /* collected data on a block of 3 by 3 elements.  Then we determine */
 /* which of those nine elements is the lowest one and assign an */
 /* according offset to the central element. */

 col = row = end = nsinks = 0;
 while (!end) {			/* all points */
   if (!col) {			/* at left edge */ 
     min1 = min2 = FLT_MAX;
     locs[1] = locs[2] = 0;	/* flags bottom element as minimum */
     if (row) {			/* not in bottom row */
       min2 = alt[-nx];
       locs[2] = 0;		/* flags bottom element as minimum */
     }
     if (*alt < min2) {
       min2 = *alt;
       locs[2] = 1;		/* flags middle element as minimum */
     }
     if (row < ny - 1 && alt[nx] < min2) {
       min2 = alt[nx];
       locs[2] = 2;		/* flags top element as minimum */
     }
     alt++;			/* move one spot to the right */
   }
   min0 = min1;			/* shift minimum values and flags */
   min1 = min2;
   locs[0] = locs[1];
   locs[1] = locs[2];
   min2 = FLT_MAX;
   if (row) {			/* not in bottom row */
     min2 = alt[-nx];		/* comparison value from next lower row */
     locs[2] = 0;		/* flags bottom element as minimum */
   }
   if (*alt < min2) {		/* middle element is smaller */
     min2 = *alt;		/* update minimum */
     locs[2] = 1;		/* flags middle element as minimum */
   }
   if (row < ny - 1 && alt[nx] < min2) { /* not in top row and */
					 /* top element is smaller */
     min2 = alt[nx];		/* update minimum */
     locs[2] = 2;		/* flag top element as minimum */
   }

   /* strategy: now min2 / locs[2] contain info on the current unit */
   /* (column 2), min1 / locs[1] on the previous unit (column 1), */
   /* and min0 / locs[0] on two units back (column 0).  All in all */
   /* we now have info on a block of 3 by 3 elements.  We determine */
   /* which one of the nine elements is the lowest one */

   min = min0;			/* info from column 0 */
   loc = 0;			/* flag left column as lowest one */
   if (min1 < min) {		/* middle column has smaller minimum */
     min = min1;		/* update minimum */
     loc = 1;			/* flag middle column as lowest one */
   }
   if (min2 < min) {		/* right column has smaller minimum */
     min = min2;		/* update minimum */
     loc = 2;			/* flag right column as lowest one */
   }
   if (!(*wsh++ = loc - 1 + (locs[loc] - 1)*nx)) /* offset from middle */
				/* element to lowest neighbor */
     nsinks++;			/* if no offset, then we found a sink */
   col++;			/* go to next element */
   alt++;
   if (col == nx - 1) {		/* at right edge */
     min = min1;
     loc = 1;
     if (min2 < min) {
       min = min2;
       loc = 2;
     }
     if (!(*wsh++ = loc - 2 + (locs[loc] - 1)*nx))
       nsinks++;
     col = 0;
     row++;
     if (row == ny)
       end = 1;
   }
 }
 switch (mode) {
 case 0: case 1:
   wsh = LPTR(h);		/* back to start of result map */
   n = n0 = nx*ny;
   row = col = 0;
   if (!(code = (int *) malloc(sizeof(int)*nsinks)))
     return cerror(ALLOC_ERR, 0);
   if (ny > nx)
     nx = ny;
   nx += 2;			/* we label the sinks with a number; */
				/* nx is the number for the next sink */
   i = nx;
   nx0 = nx - 1;
   do {
     col = row;
     while (wsh[col] && wsh[col] < nx) /* no sink or old track */
       col += wsh[col];		/* current offset */
     if (!wsh[col]) {		/* sink */
       code[i - nx] = col;	/* result for code i */
       wsh[col] = iq = i++;
       n--;
     } else
       iq = wsh[col];		/* iq = current code */
     col = row++;		/* start again, enter code */
     while (wsh[col] < nx) {	/* no old track */
       ny = wsh[col];
       wsh[col] = iq;
       col += ny;
       n--;
     }
   } while (n);
   if (mode == 1)
     /* now put in offset of sinks in stead of codes */
     while (n0--) {
       *wsh = code[*wsh - nx];
       wsh++;
     } else while (n0--) {
       *wsh = *wsh - nx0;
       wsh++;
     }
   free(code);
   break;
 }
 return result_sym;
}
/*----------------------------------------------------------------------*/
int ana_basin2(int narg, int ps[])
/* Returns a basin map derived from "altitude" map <data>
   Syntax:  y = basin(data [, sign, /DIFFERENCE,/SINK,/NUMBER])
      <data>: data array
      <sign>: sign of sought basins: +1 -> "mountains", -1 -> "basins"
              defaults to -1.
   /NUMBER -> assign different number to each basin, starting
              with number 1 and increasing in steps of one.
   /SINK -> assign index to <im> of the final sink corresponding
              to each pixel.  Similar to 1, but with different labels for
	      each basin
   /DIFFERENCE -> return index offset of steepest-descent
	      neighbor for each pixel
   LS 19feb93 9may97 24jun98 */
{
  int	result, mode, n, i, j, k, *offsets, *rcoords, edge = 0,
    mini, loc[3], nel, label = 0, sign, maxi = 0;
  pointer	src, trgt, trgt0;
  scalar	min[3], max[3];
  extern struct boundsStruct	bounds;
  loopInfo	srcinfo, trgtinfo;

  if (narg == 1 && symbolIsNumericalArray(ps[0]) && array_num_dims(ps[0]) == 2)
    return ana_basin(narg, ps);	/* use old (but faster) routine */

  /* check <data>, create output symbol, and prepare for walk through */
  /* <data> */
  if (symbol_type(ps[0]) != ANA_FLOAT)
    return anaerror("Sorry, must be FLOAT at the moment", ps[0]);

  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EACHCOORD | SL_EXACT,
		   ANA_LONG,
		   &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return ANA_ERROR;
  trgt0 = trgt;
  nel = array_size(ps[0]);
  
  if (narg > 1 && ps[1]) {	/* SIGN */
    sign = int_arg(ps[1]);
    sign = (sign < 0)? 0: 0x20;
  } else
    sign = 0;

  /* treat keyword switches */
  switch (internalMode & 7) {
    case 1:
      mode = 0;			/* /NUMBER */
      break;
    case 2:
      mode = 1;			/* /SINK */
      break;
    case 4:
      mode = 2;			/* /DIFFERENCE */
      break;
    default:
      mode = 0;
      break;
  }

  /* we calculate the index offsets for all directions we want to */
  /* investigate, which means all directions with the first dimension */
  /* equal to +1. */
  srcinfo.coords[0] = 1;		/* set first dimension equal to +1 */
  /* calculate the number of nearest-neighbor data points with their */
  /* first dimension equal to +1 */
  n = 1;
  for (i = 1; i < srcinfo.ndim; i++)
    n *= 3;
  offsets = (int *) malloc(n*sizeof(int));
  rcoords = (int *) malloc(n*(srcinfo.ndim - 1)*sizeof(int));
  if (!offsets || !rcoords)
    return cerror(ALLOC_ERR, 0);
  for (k = 0; k < n; k++) {
    offsets[k] = 1;
    for (j = 1; j < srcinfo.ndim; j++)
				/* calculate index offset */
      offsets[k] += srcinfo.coords[j]*srcinfo.singlestep[srcinfo.raxes[j]];
    memcpy(rcoords + k*(srcinfo.ndim - 1), &srcinfo.coords[1],
	   (srcinfo.ndim - 1)*sizeof(int));
    for (j = 1; j < srcinfo.ndim; j++) { /* to next direction: don't change */
				/* the first dimension */
      srcinfo.coords[j]++;
      if (srcinfo.coords[j] < 2)
	break;
      for (i = 1; i <= j; i++)
	srcinfo.coords[i] = -1;
    }
  }
  zerobytes(srcinfo.coords, srcinfo.ndim*sizeof(int)); /* back to zeros */

  mini = 2;
  switch (symbol_type(ps[0]) | sign) {
    case ANA_BYTE:
      min[0].b = min[1].b = bounds.max.b;
      do {
	min[2].b = min[1].b;
	min[1].b = min[0].b;
	loc[2] = loc[1];
	loc[1] = loc[0];
	min[0].b = bounds.max.b;
	mini++;
	if (!srcinfo.coords[0]) {	/* at left edge */
	  src.b--;
	  min[2].b = min[1].b = bounds.max.b;
	  mini = 3;
	  edge = 0;
	  for (i = 1; i < srcinfo.ndim; i++)
	    if (!srcinfo.coords[i]
		|| srcinfo.coords[i] == srcinfo.dims[i] - 1) {
	      edge = 1;
	      break;
	    }
	  if (edge)
	    for (i = 0; i < n; i++) { /* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any other edge */
		if (src.b[offsets[i]] < min[0].b) { /* update */
		  min[0].b = src.b[offsets[i]];
		  loc[0] = i;
		}
	      }
	    } else for (i = 0; i < n; i++) {
	      if (src.b[offsets[i]] < min[0].b) { /* update */
		min[0].b = src.b[offsets[i]];
		loc[0] = i;
	      }
	    }
	  if (mini < 3) {	/* previous lowest is now in 1 or 2 */
	    if (min[0].b < min[mini].b)
	      mini = 0;
	  } else {		/* previous lowest is too far away: find new */
	    if (min[0].b < min[1].b) {
	      if (min[0].b < min[2].b)
		mini = 0;
	      else
		mini = 2;
	    } else {
	      if (min[1].b < min[2].b)
		mini = 1;
	      else
		mini = 2;
	    }
	  }
	  src.b++;		/* back to current position */
	  min[2].b = min[1].b;
	  min[1].b = min[0].b;
	  loc[2] = loc[1];
	  loc[1] = loc[0];
	  min[0].b = bounds.max.b;
	  mini++;
	}
	
	if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { /* at right edge */
	  if (mini == 3) {
	    if (min[1].b < min[2].b)
	      mini = 1;
	    else
	      mini = 2;
	  }
	} else {
	  if (edge)
	    for (i = 0; i < n; i++) {	/* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any edge */
		if (src.b[offsets[i]] < min[0].b) { /* update */
		  min[0].b = src.b[offsets[i]];
		  loc[0] = i;
		}
	      }
	    }
	  else for (i = 0; i < n; i++)
	    if (src.b[offsets[i]] < min[0].b) { /* update */
	      min[0].b = src.b[offsets[i]];
	      loc[0] = i;
	    }
	  if (mini < 3) {	/* previous lowest is now in 1 or 2 */
	    if (min[0].b < min[mini].b)
	      mini = 0;
	  } else {		/* previous lowest is too far away: find new */
	    if (min[0].b < min[1].b) {
	      if (min[0].b < min[2].b)
		mini = 0;
	      else
		mini = 2;
	    } else {
	      if (min[1].b < min[2].b)
		mini = 1;
	      else
		mini = 2;
	    }
	  }
	}
	/* now min[mini].b is the lowest value */
	*trgt.l = -loc[mini] - n*mini;
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      break;
    case ANA_WORD:
      min[0].w = min[1].w = bounds.max.w;
      do {
	min[2].w = min[1].w;
	min[1].w = min[0].w;
	loc[2] = loc[1];
	loc[1] = loc[0];
	min[0].w = bounds.max.w;
	mini++;
	if (!srcinfo.coords[0]) {	/* at left edge */
	  src.w--;
	  min[2].w = min[1].w = bounds.max.w;
	  mini = 3;
	  edge = 0;
	  for (i = 1; i < srcinfo.ndim; i++)
	    if (!srcinfo.coords[i]
		|| srcinfo.coords[i] == srcinfo.dims[i] - 1) {
	      edge = 1;
	      break;
	    }
	  if (edge)
	    for (i = 0; i < n; i++) { /* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any other edge */
		if (src.w[offsets[i]] < min[0].w) { /* update */
		  min[0].w = src.w[offsets[i]];
		  loc[0] = i;
		}
	      }
	    } else for (i = 0; i < n; i++) {
	      if (src.w[offsets[i]] < min[0].w) { /* update */
		min[0].w = src.w[offsets[i]];
		loc[0] = i;
	      }
	    }
	  if (mini < 3) {	/* previous lowest is now in 1 or 2 */
	    if (min[0].w < min[mini].w)
	      mini = 0;
	  } else {		/* previous lowest is too far away: find new */
	    if (min[0].w < min[1].w) {
	      if (min[0].w < min[2].w)
		mini = 0;
	      else
		mini = 2;
	    } else {
	      if (min[1].w < min[2].w)
		mini = 1;
	      else
		mini = 2;
	    }
	  }
	  src.w++;		/* back to current position */
	  min[2].w = min[1].w;
	  min[1].w = min[0].w;
	  loc[2] = loc[1];
	  loc[1] = loc[0];
	  min[0].w = bounds.max.w;
	  mini++;
	}
	
	if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { /* at right edge */
	  if (mini == 3) {
	    if (min[1].w < min[2].w)
	      mini = 1;
	    else
	      mini = 2;
	  }
	} else {
	  if (edge)
	    for (i = 0; i < n; i++) {	/* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any edge */
		if (src.w[offsets[i]] < min[0].w) { /* update */
		  min[0].w = src.w[offsets[i]];
		  loc[0] = i;
		}
	      }
	    }
	  else for (i = 0; i < n; i++)
	    if (src.w[offsets[i]] < min[0].w) { /* update */
	      min[0].w = src.w[offsets[i]];
	      loc[0] = i;
	    }
	  if (mini < 3) {	/* previous lowest is now in 1 or 2 */
	    if (min[0].w < min[mini].w)
	      mini = 0;
	  } else {		/* previous lowest is too far away: find new */
	    if (min[0].w < min[1].w) {
	      if (min[0].w < min[2].w)
		mini = 0;
	      else
		mini = 2;
	    } else {
	      if (min[1].w < min[2].w)
		mini = 1;
	      else
		mini = 2;
	    }
	  }
	}
	/* now min[mini].w is the lowest value */
	*trgt.l = -loc[mini] - n*mini;
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      break;
    case ANA_LONG:
      min[0].l = min[1].l = bounds.max.l;
      do {
	min[2].l = min[1].l;
	min[1].l = min[0].l;
	loc[2] = loc[1];
	loc[1] = loc[0];
	min[0].l = bounds.max.l;
	mini++;
	if (!srcinfo.coords[0]) {	/* at left edge */
	  src.l--;
	  min[2].l = min[1].l = bounds.max.l;
	  mini = 3;
	  edge = 0;
	  for (i = 1; i < srcinfo.ndim; i++)
	    if (!srcinfo.coords[i]
		|| srcinfo.coords[i] == srcinfo.dims[i] - 1) {
	      edge = 1;
	      break;
	    }
	  if (edge)
	    for (i = 0; i < n; i++) { /* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any other edge */
		if (src.l[offsets[i]] < min[0].l) { /* update */
		  min[0].l = src.l[offsets[i]];
		  loc[0] = i;
		}
	      }
	    } else for (i = 0; i < n; i++) {
	      if (src.l[offsets[i]] < min[0].l) { /* update */
		min[0].l = src.l[offsets[i]];
		loc[0] = i;
	      }
	    }
	  if (mini < 3) {	/* previous lowest is now in 1 or 2 */
	    if (min[0].l < min[mini].l)
	      mini = 0;
	  } else {		/* previous lowest is too far away: find new */
	    if (min[0].l < min[1].l) {
	      if (min[0].l < min[2].l)
		mini = 0;
	      else
		mini = 2;
	    } else {
	      if (min[1].l < min[2].l)
		mini = 1;
	      else
		mini = 2;
	    }
	  }
	  src.l++;		/* back to current position */
	  min[2].l = min[1].l;
	  min[1].l = min[0].l;
	  loc[2] = loc[1];
	  loc[1] = loc[0];
	  min[0].l = bounds.max.l;
	  mini++;
	}
	
	if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { /* at right edge */
	  if (mini == 3) {
	    if (min[1].l < min[2].l)
	      mini = 1;
	    else
	      mini = 2;
	  }
	} else {
	  if (edge)
	    for (i = 0; i < n; i++) {	/* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any edge */
		if (src.l[offsets[i]] < min[0].l) { /* update */
		  min[0].l = src.l[offsets[i]];
		  loc[0] = i;
		}
	      }
	    }
	  else for (i = 0; i < n; i++)
	    if (src.l[offsets[i]] < min[0].l) { /* update */
	      min[0].l = src.l[offsets[i]];
	      loc[0] = i;
	    }
	  if (mini < 3) {	/* previous lowest is now in 1 or 2 */
	    if (min[0].l < min[mini].l)
	      mini = 0;
	  } else {		/* previous lowest is too far away: find new */
	    if (min[0].l < min[1].l) {
	      if (min[0].l < min[2].l)
		mini = 0;
	      else
		mini = 2;
	    } else {
	      if (min[1].l < min[2].l)
		mini = 1;
	      else
		mini = 2;
	    }
	  }
	}
	/* now min[mini].l is the lowest value */
	*trgt.l = -loc[mini] - n*mini;
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      break;
    case ANA_FLOAT:
      min[0].f = min[1].f = bounds.max.f;
      do {
	min[2].f = min[1].f;
	min[1].f = min[0].f;
	loc[2] = loc[1];
	loc[1] = loc[0];
	min[0].f = bounds.max.f;
	mini++;
	if (!srcinfo.coords[0]) {	/* at left edge */
	  src.f--;
	  min[2].f = min[1].f = bounds.max.f;
	  mini = 3;
	  edge = 0;
	  for (i = 1; i < srcinfo.ndim; i++)
	    if (!srcinfo.coords[i]
		|| srcinfo.coords[i] == srcinfo.dims[i] - 1) {
	      edge = 1;
	      break;
	    }
	  if (edge)
	    for (i = 0; i < n; i++) { /* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any other edge */
		if (src.f[offsets[i]] < min[0].f) { /* update */
		  min[0].f = src.f[offsets[i]];
		  loc[0] = i;
		}
	      }
	    } else for (i = 0; i < n; i++) {
	      if (src.f[offsets[i]] < min[0].f) { /* update */
		min[0].f = src.f[offsets[i]];
		loc[0] = i;
	      }
	    }
	  if (mini < 3) {	/* previous lowest is now in 1 or 2 */
	    if (min[0].f < min[mini].f)
	      mini = 0;
	  } else {		/* previous lowest is too far away: find new */
	    if (min[0].f < min[1].f) {
	      if (min[0].f < min[2].f)
		mini = 0;
	      else
		mini = 2;
	    } else {
	      if (min[1].f < min[2].f)
		mini = 1;
	      else
		mini = 2;
	    }
	  }
	  src.f++;		/* back to current position */
	  min[2].f = min[1].f;
	  min[1].f = min[0].f;
	  loc[2] = loc[1];
	  loc[1] = loc[0];
	  min[0].f = bounds.max.f;
	  mini++;
	}
	
	if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { /* at right edge */
	  if (mini == 3) {
	    if (min[1].f < min[2].f)
	      mini = 1;
	    else
	      mini = 2;
	  }
	} else {
	  if (edge)
	    for (i = 0; i < n; i++) {	/* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any edge */
		if (src.f[offsets[i]] < min[0].f) { /* update */
		  min[0].f = src.f[offsets[i]];
		  loc[0] = i;
		}
	      }
	    }
	  else for (i = 0; i < n; i++)
	    if (src.f[offsets[i]] < min[0].f) { /* update */
	      min[0].f = src.f[offsets[i]];
	      loc[0] = i;
	    }
	  if (mini < 3) {	/* previous lowest is now in 1 or 2 */
	    if (min[0].f < min[mini].f)
	      mini = 0;
	  } else {		/* previous lowest is too far away: find new */
	    if (min[0].f < min[1].f) {
	      if (min[0].f < min[2].f)
		mini = 0;
	      else
		mini = 2;
	    } else {
	      if (min[1].f < min[2].f)
		mini = 1;
	      else
		mini = 2;
	    }
	  }
	}
	/* now min[mini].f is the lowest value */
	*trgt.l = -loc[mini] - n*mini;
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      break;
    case ANA_DOUBLE:
      min[0].d = min[1].d = bounds.max.d;
      do {
	min[2].d = min[1].d;
	min[1].d = min[0].d;
	loc[2] = loc[1];
	loc[1] = loc[0];
	min[0].d = bounds.max.d;
	mini++;
	if (!srcinfo.coords[0]) {	/* at left edge */
	  src.d--;
	  min[2].d = min[1].d = bounds.max.d;
	  mini = 3;
	  edge = 0;
	  for (i = 1; i < srcinfo.ndim; i++)
	    if (!srcinfo.coords[i]
		|| srcinfo.coords[i] == srcinfo.dims[i] - 1) {
	      edge = 1;
	      break;
	    }
	  if (edge)
	    for (i = 0; i < n; i++) { /* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any other edge */
		if (src.d[offsets[i]] < min[0].d) { /* update */
		  min[0].d = src.d[offsets[i]];
		  loc[0] = i;
		}
	      }
	    } else for (i = 0; i < n; i++) {
	      if (src.d[offsets[i]] < min[0].d) { /* update */
		min[0].d = src.d[offsets[i]];
		loc[0] = i;
	      }
	    }
	  if (mini < 3) {	/* previous lowest is now in 1 or 2 */
	    if (min[0].d < min[mini].d)
	      mini = 0;
	  } else {		/* previous lowest is too far away: find new */
	    if (min[0].d < min[1].d) {
	      if (min[0].d < min[2].d)
		mini = 0;
	      else
		mini = 2;
	    } else {
	      if (min[1].d < min[2].d)
		mini = 1;
	      else
		mini = 2;
	    }
	  }
	  src.d++;		/* back to current position */
	  min[2].d = min[1].d;
	  min[1].d = min[0].d;
	  loc[2] = loc[1];
	  loc[1] = loc[0];
	  min[0].d = bounds.max.d;
	  mini++;
	}
	
	if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { /* at right edge */
	  if (mini == 3) {
	    if (min[1].d < min[2].d)
	      mini = 1;
	    else
	      mini = 2;
	  }
	} else {
	  if (edge)
	    for (i = 0; i < n; i++) {	/* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any edge */
		if (src.d[offsets[i]] < min[0].d) { /* update */
		  min[0].d = src.d[offsets[i]];
		  loc[0] = i;
		}
	      }
	    }
	  else for (i = 0; i < n; i++)
	    if (src.d[offsets[i]] < min[0].d) { /* update */
	      min[0].d = src.d[offsets[i]];
	      loc[0] = i;
	    }
	  if (mini < 3) {	/* previous lowest is now in 1 or 2 */
	    if (min[0].d < min[mini].d)
	      mini = 0;
	  } else {		/* previous lowest is too far away: find new */
	    if (min[0].d < min[1].d) {
	      if (min[0].d < min[2].d)
		mini = 0;
	      else
		mini = 2;
	    } else {
	      if (min[1].d < min[2].d)
		mini = 1;
	      else
		mini = 2;
	    }
	  }
	}
	/* now min[mini].d is the lowest value */
	*trgt.l = -loc[mini] - n*mini;
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      break;

    case ANA_BYTE | 0x20:
      max[0].b = max[1].b = bounds.min.b;
      do {
	max[2].b = max[1].b;
	max[1].b = max[0].b;
	loc[2] = loc[1];
	loc[1] = loc[0];
	max[0].b = bounds.min.b;
	maxi++;
	if (!srcinfo.coords[0]) {	/* at left edge */
	  src.b--;
	  max[2].b = max[1].b = bounds.min.b;
	  maxi = 3;
	  edge = 0;
	  for (i = 1; i < srcinfo.ndim; i++)
	    if (!srcinfo.coords[i]
		|| srcinfo.coords[i] == srcinfo.dims[i] - 1) {
	      edge = 1;
	      break;
	    }
	  if (edge)
	    for (i = 0; i < n; i++) { /* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any other edge */
		if (src.b[offsets[i]] > max[0].b) { /* update */
		  max[0].b = src.b[offsets[i]];
		  loc[0] = i;
		}
	      }
	    } else for (i = 0; i < n; i++) {
	      if (src.b[offsets[i]] > max[0].b) { /* update */
		max[0].b = src.b[offsets[i]];
		loc[0] = i;
	      }
	    }
	  if (maxi < 3) {	/* previous highest is now in 1 or 2 */
	    if (max[0].b > max[maxi].b)
	      maxi = 0;
	  } else {		/* previous highest is too far away: find new */
	    if (max[0].b > max[1].b) {
	      if (max[0].b > max[2].b)
		maxi = 0;
	      else
		maxi = 2;
	    } else {
	      if (max[1].b > max[2].b)
		maxi = 1;
	      else
		maxi = 2;
	    }
	  }
	  src.b++;		/* back to current position */
	  max[2].b = max[1].b;
	  max[1].b = max[0].b;
	  loc[2] = loc[1];
	  loc[1] = loc[0];
	  max[0].b = bounds.min.b;
	  maxi++;
	}
	
	if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { /* at right edge */
	  if (maxi == 3) {
	    if (max[1].b > max[2].b)
	      maxi = 1;
	    else
	      maxi = 2;
	  }
	} else {
	  if (edge)
	    for (i = 0; i < n; i++) {	/* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any edge */
		if (src.b[offsets[i]] > max[0].b) { /* update */
		  max[0].b = src.b[offsets[i]];
		  loc[0] = i;
		}
	      }
	    }
	  else for (i = 0; i < n; i++)
	    if (src.b[offsets[i]] > max[0].b) { /* update */
	      max[0].b = src.b[offsets[i]];
	      loc[0] = i;
	    }
	  if (maxi < 3) {	/* previous highest is now in 1 or 2 */
	    if (max[0].b > max[maxi].b)
	      maxi = 0;
	  } else { /* previous highest is too far away: find new */
	    if (max[0].b > max[1].b) {
	      if (max[0].b > max[2].b)
		maxi = 0;
	      else
		maxi = 2;
	    } else {
	      if (max[1].b > max[2].b)
		maxi = 1;
	      else
		maxi = 2;
	    }
	  }
	}
	/* now max[maxi].b is the highest value */
	*trgt.l = -loc[maxi] - n*maxi;
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      break;
    case ANA_WORD | 0x20:
      max[0].w = max[1].w = bounds.min.w;
      do {
	max[2].w = max[1].w;
	max[1].w = max[0].w;
	loc[2] = loc[1];
	loc[1] = loc[0];
	max[0].w = bounds.min.w;
	maxi++;
	if (!srcinfo.coords[0]) {	/* at left edge */
	  src.w--;
	  max[2].w = max[1].w = bounds.min.w;
	  maxi = 3;
	  edge = 0;
	  for (i = 1; i < srcinfo.ndim; i++)
	    if (!srcinfo.coords[i]
		|| srcinfo.coords[i] == srcinfo.dims[i] - 1) {
	      edge = 1;
	      break;
	    }
	  if (edge)
	    for (i = 0; i < n; i++) { /* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any other edge */
		if (src.w[offsets[i]] > max[0].w) { /* update */
		  max[0].w = src.w[offsets[i]];
		  loc[0] = i;
		}
	      }
	    } else for (i = 0; i < n; i++) {
	      if (src.w[offsets[i]] > max[0].w) { /* update */
		max[0].w = src.w[offsets[i]];
		loc[0] = i;
	      }
	    }
	  if (maxi < 3) {	/* previous highest is now in 1 or 2 */
	    if (max[0].w > max[maxi].w)
	      maxi = 0;
	  } else { /* previous highest is too far away: find new */
	    if (max[0].w > max[1].w) {
	      if (max[0].w > max[2].w)
		maxi = 0;
	      else
		maxi = 2;
	    } else {
	      if (max[1].w > max[2].w)
		maxi = 1;
	      else
		maxi = 2;
	    }
	  }
	  src.w++;		/* back to current position */
	  max[2].w = max[1].w;
	  max[1].w = max[0].w;
	  loc[2] = loc[1];
	  loc[1] = loc[0];
	  max[0].w = bounds.min.w;
	  maxi++;
	}
	
	if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { /* at right edge */
	  if (maxi == 3) {
	    if (max[1].w > max[2].w)
	      maxi = 1;
	    else
	      maxi = 2;
	  }
	} else {
	  if (edge)
	    for (i = 0; i < n; i++) {	/* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any edge */
		if (src.w[offsets[i]] > max[0].w) { /* update */
		  max[0].w = src.w[offsets[i]];
		  loc[0] = i;
		}
	      }
	    }
	  else for (i = 0; i < n; i++)
	    if (src.w[offsets[i]] > max[0].w) { /* update */
	      max[0].w = src.w[offsets[i]];
	      loc[0] = i;
	    }
	  if (maxi < 3) {	/* previous highest is now in 1 or 2 */
	    if (max[0].w > max[maxi].w)
	      maxi = 0;
	  } else { /* previous highest is too far away: find new */
	    if (max[0].w > max[1].w) {
	      if (max[0].w > max[2].w)
		maxi = 0;
	      else
		maxi = 2;
	    } else {
	      if (max[1].w > max[2].w)
		maxi = 1;
	      else
		maxi = 2;
	    }
	  }
	}
	/* now max[maxi].w is the highest value */
	*trgt.l = -loc[maxi] - n*maxi;
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      break;
    case ANA_LONG | 0x20:
      max[0].l = max[1].l = bounds.min.l;
      do {
	max[2].l = max[1].l;
	max[1].l = max[0].l;
	loc[2] = loc[1];
	loc[1] = loc[0];
	max[0].l = bounds.min.l;
	maxi++;
	if (!srcinfo.coords[0]) {	/* at left edge */
	  src.l--;
	  max[2].l = max[1].l = bounds.min.l;
	  maxi = 3;
	  edge = 0;
	  for (i = 1; i < srcinfo.ndim; i++)
	    if (!srcinfo.coords[i]
		|| srcinfo.coords[i] == srcinfo.dims[i] - 1) {
	      edge = 1;
	      break;
	    }
	  if (edge)
	    for (i = 0; i < n; i++) { /* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any other edge */
		if (src.l[offsets[i]] > max[0].l) { /* update */
		  max[0].l = src.l[offsets[i]];
		  loc[0] = i;
		}
	      }
	    } else for (i = 0; i < n; i++) {
	      if (src.l[offsets[i]] > max[0].l) { /* update */
		max[0].l = src.l[offsets[i]];
		loc[0] = i;
	      }
	    }
	  if (maxi < 3) {	/* previous highest is now in 1 or 2 */
	    if (max[0].l > max[maxi].l)
	      maxi = 0;
	  } else { /* previous highest is too far away: find new */
	    if (max[0].l > max[1].l) {
	      if (max[0].l > max[2].l)
		maxi = 0;
	      else
		maxi = 2;
	    } else {
	      if (max[1].l > max[2].l)
		maxi = 1;
	      else
		maxi = 2;
	    }
	  }
	  src.l++;		/* back to current position */
	  max[2].l = max[1].l;
	  max[1].l = max[0].l;
	  loc[2] = loc[1];
	  loc[1] = loc[0];
	  max[0].l = bounds.min.l;
	  maxi++;
	}
	
	if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { /* at right edge */
	  if (maxi == 3) {
	    if (max[1].l > max[2].l)
	      maxi = 1;
	    else
	      maxi = 2;
	  }
	} else {
	  if (edge)
	    for (i = 0; i < n; i++) {	/* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any edge */
		if (src.l[offsets[i]] > max[0].l) { /* update */
		  max[0].l = src.l[offsets[i]];
		  loc[0] = i;
		}
	      }
	    }
	  else for (i = 0; i < n; i++)
	    if (src.l[offsets[i]] > max[0].l) { /* update */
	      max[0].l = src.l[offsets[i]];
	      loc[0] = i;
	    }
	  if (maxi < 3) {	/* previous highest is now in 1 or 2 */
	    if (max[0].l > max[maxi].l)
	      maxi = 0;
	  } else { /* previous highest is too far away: find new */
	    if (max[0].l > max[1].l) {
	      if (max[0].l > max[2].l)
		maxi = 0;
	      else
		maxi = 2;
	    } else {
	      if (max[1].l > max[2].l)
		maxi = 1;
	      else
		maxi = 2;
	    }
	  }
	}
	/* now max[maxi].l is the highest value */
	*trgt.l = -loc[maxi] - n*maxi;
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      break;
    case ANA_FLOAT | 0x20:
      max[0].f = max[1].f = bounds.min.f;
      do {
	max[2].f = max[1].f;
	max[1].f = max[0].f;
	loc[2] = loc[1];
	loc[1] = loc[0];
	max[0].f = bounds.min.f;
	maxi++;
	if (!srcinfo.coords[0]) {	/* at left edge */
	  src.f--;
	  max[2].f = max[1].f = bounds.min.f;
	  maxi = 3;
	  edge = 0;
	  for (i = 1; i < srcinfo.ndim; i++)
	    if (!srcinfo.coords[i]
		|| srcinfo.coords[i] == srcinfo.dims[i] - 1) {
	      edge = 1;
	      break;
	    }
	  if (edge)
	    for (i = 0; i < n; i++) { /* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any other edge */
		if (src.f[offsets[i]] > max[0].f) { /* update */
		  max[0].f = src.f[offsets[i]];
		  loc[0] = i;
		}
	      }
	    } else for (i = 0; i < n; i++) {
	      if (src.f[offsets[i]] > max[0].f) { /* update */
		max[0].f = src.f[offsets[i]];
		loc[0] = i;
	      }
	    }
	  if (maxi < 3) {	/* previous highest is now in 1 or 2 */
	    if (max[0].f > max[maxi].f)
	      maxi = 0;
	  } else { /* previous highest is too far away: find new */
	    if (max[0].f > max[1].f) {
	      if (max[0].f > max[2].f)
		maxi = 0;
	      else
		maxi = 2;
	    } else {
	      if (max[1].f > max[2].f)
		maxi = 1;
	      else
		maxi = 2;
	    }
	  }
	  src.f++;		/* back to current position */
	  max[2].f = max[1].f;
	  max[1].f = max[0].f;
	  loc[2] = loc[1];
	  loc[1] = loc[0];
	  max[0].f = bounds.min.f;
	  maxi++;
	}
	
	if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { /* at right edge */
	  if (maxi == 3) {
	    if (max[1].f > max[2].f)
	      maxi = 1;
	    else
	      maxi = 2;
	  }
	} else {
	  if (edge)
	    for (i = 0; i < n; i++) {	/* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any edge */
		if (src.f[offsets[i]] > max[0].f) { /* update */
		  max[0].f = src.f[offsets[i]];
		  loc[0] = i;
		}
	      }
	    }
	  else for (i = 0; i < n; i++)
	    if (src.f[offsets[i]] > max[0].f) { /* update */
	      max[0].f = src.f[offsets[i]];
	      loc[0] = i;
	    }
	  if (maxi < 3) {	/* previous highest is now in 1 or 2 */
	    if (max[0].f > max[maxi].f)
	      maxi = 0;
	  } else { /* previous highest is too far away: find new */
	    if (max[0].f > max[1].f) {
	      if (max[0].f > max[2].f)
		maxi = 0;
	      else
		maxi = 2;
	    } else {
	      if (max[1].f > max[2].f)
		maxi = 1;
	      else
		maxi = 2;
	    }
	  }
	}
	/* now max[maxi].f is the highest value */
	*trgt.l = -loc[maxi] - n*maxi;
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      break;
    case ANA_DOUBLE | 0x20:
      max[0].d = max[1].d = bounds.min.d;
      do {
	max[2].d = max[1].d;
	max[1].d = max[0].d;
	loc[2] = loc[1];
	loc[1] = loc[0];
	max[0].d = bounds.min.d;
	maxi++;
	if (!srcinfo.coords[0]) {	/* at left edge */
	  src.d--;
	  max[2].d = max[1].d = bounds.min.d;
	  maxi = 3;
	  edge = 0;
	  for (i = 1; i < srcinfo.ndim; i++)
	    if (!srcinfo.coords[i]
		|| srcinfo.coords[i] == srcinfo.dims[i] - 1) {
	      edge = 1;
	      break;
	    }
	  if (edge)
	    for (i = 0; i < n; i++) { /* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any other edge */
		if (src.d[offsets[i]] > max[0].d) { /* update */
		  max[0].d = src.d[offsets[i]];
		  loc[0] = i;
		}
	      }
	    } else for (i = 0; i < n; i++) {
	      if (src.d[offsets[i]] > max[0].d) { /* update */
		max[0].d = src.d[offsets[i]];
		loc[0] = i;
	      }
	    }
	  if (maxi < 3) {	/* previous highest is now in 1 or 2 */
	    if (max[0].d > max[maxi].d)
	      maxi = 0;
	  } else { /* previous highest is too far away: find new */
	    if (max[0].d > max[1].d) {
	      if (max[0].d > max[2].d)
		maxi = 0;
	      else
		maxi = 2;
	    } else {
	      if (max[1].d > max[2].d)
		maxi = 1;
	      else
		maxi = 2;
	    }
	  }
	  src.d++;		/* back to current position */
	  max[2].d = max[1].d;
	  max[1].d = max[0].d;
	  loc[2] = loc[1];
	  loc[1] = loc[0];
	  max[0].d = bounds.min.d;
	  maxi++;
	}
	
	if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { /* at right edge */
	  if (maxi == 3) {
	    if (max[1].d > max[2].d)
	      maxi = 1;
	    else
	      maxi = 2;
	  }
	} else {
	  if (edge)
	    for (i = 0; i < n; i++) {	/* all directions */
	      for (j = 0; j < srcinfo.ndim - 1; j++) {
		/* check if across edge */
		k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
		if (k < 0 || k == srcinfo.dims[j + 1])
		  break;
	      }
	      if (j == srcinfo.ndim - 1) {	/* not across any edge */
		if (src.d[offsets[i]] > max[0].d) { /* update */
		  max[0].d = src.d[offsets[i]];
		  loc[0] = i;
		}
	      }
	    }
	  else for (i = 0; i < n; i++)
	    if (src.d[offsets[i]] > max[0].d) { /* update */
	      max[0].d = src.d[offsets[i]];
	      loc[0] = i;
	    }
	  if (maxi < 3) {	/* previous highest is now in 1 or 2 */
	    if (max[0].d > max[maxi].d)
	      maxi = 0;
	  } else { /* previous highest is too far away: find new */
	    if (max[0].d > max[1].d) {
	      if (max[0].d > max[2].d)
		maxi = 0;
	      else
		maxi = 2;
	    } else {
	      if (max[1].d > max[2].d)
		maxi = 1;
	      else
		maxi = 2;
	    }
	  }
	}
	/* now max[maxi].d is the highest value */
	*trgt.l = -loc[maxi] - n*maxi;
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      break;
  }

  trgt.f = trgt0.f;		/* back to the start */
  switch (mode) {
    case 0:			/* /NUMBER: assign subsequent numbers */
      label = 1;
      /* fall-thru */
    case 1:			/* /SINK: assign label equal to sink index */
      while (nel--) {
	if (*trgt.l <= 0) {	/* still needs treatment */
	  i = 0;
	  do {
	    j = -trgt.l[i];
	    if (j >= 0) {
	      j = offsets[j % n] - (j/n);	/* 0 -> found minimum */
	      if (j)
		i += j;
	    } else
	      break;
	  } while (j);
	  if (j) 		/* found one already treated: copy number */
	    *trgt.l = trgt.l[i];
	  else {		/* found sink: assign number & copy */
	    mini = mode? trgt.l + i - trgt0.l: label++;
	    i = 0;
	    do {
	      j = -trgt.l[i];
	      if (j >= 0) {
		trgt.l[i] = mini;
		j = offsets[j % n] - (j/n);
		if (j)
		  i += j;
	      }
	    } while (j > 0);
	  }
	}
	trgt.l++;
      }
      break;
    case 2:			/* /DIFFERENCE: index difference */
      while (nel--) {
	i = -*trgt.l;
	*trgt.l++ = offsets[i % n] - (i/n);
      }
      break;
  }
  free(rcoords);
  free(offsets);
  return result;
}
/*----------------------------------------------------------------------*/
int ana_extreme_general(int narg, int ps[])
/* Y = ESEGMENT(X) seeks positions of local extremes
   ESEGMENT(x [, sign, DIAGONAL=diagonal, THRESHOLD=threshold])
   <x>: data
   <sign>: sign of objects to look for, positive integer -> maximum,
    negative integer -> minimum.  If zero, then +1 is assumed.
   <threshold>: threshold for acceptance (defaults to zero)
   returns number of OK extremes per data element
   LS 18may95 4aug97 16nov98 2aug99 */
{
  int	result, sign, n, i, *offset, k, j, nElem, edge,
	*diagonal, nDiagonal, n1, n2, nDoDim, i1, i2, n0, haveThreshold;
  double	zero = 0.0;
  pointer	src, trgt, srcl, srcr, t;
  loopInfo	srcinfo, trgtinfo;

  /* gather info about ps[0] and prepare a return symbol */
  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT | SL_EACHROW,
		   ANA_BYTE, &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return ANA_ERROR;
  if (symbol_class(ps[0]) != ANA_ARRAY)
    return cerror(ILL_CLASS, ps[0]);

  sign = (narg > 1 && ps[1])? int_arg(ps[1]): 1;
  sign = (sign >= 0)? 1: 0;	/* 1 -> hill-like, 0 -> valley-like */

  if (narg > 2 && ps[2]) {	/* have <diagonal> */
    if (symbol_class(ps[2]) != ANA_ARRAY)
      return cerror(NEED_ARR, ps[2]);
    if (array_size(ps[2]) != srcinfo.ndim)
      return cerror(INCMP_ARG, ps[2]);
    i = ana_long(1, &ps[2]);	/* ensure LONG */
    diagonal = array_data(i);
    nDiagonal = nDoDim = 0;
    for (i = 0; i < srcinfo.ndim; i++)
      if (diagonal[i]) {
	nDoDim++;		/* # dimensions that are considered
				 at all */
	if (diagonal[i] != 1)
	  nDiagonal++;		/* # dimensions that allow diagonal
				 links */
      }
  } else {
    diagonal = NULL;
    nDiagonal = nDoDim = srcinfo.ndim;
  }

  if (narg > 3 && ps[3]) {	/* have threshold */
    if (symbolIsRealScalar(ps[3])) { /* it's valid  */
      i = ana_converts[symbol_type(ps[0])](1, ps + 3);
      t.b = &scalar_value(i).b;
      haveThreshold = 1;
    } else
      return cerror(ILL_CLASS, ps[3]);
  } else {
    t.d = &zero;
    haveThreshold = 0;
  }
      
  /* now calculate the number of directions to treat;
     equal to (3^nDiagonal - 1)/2 + nDoDim - nDiagonal */
  n = 1;
  for (i = 0; i < nDiagonal; i++)
    n *= 3;
  n = (n - 1)/2 + nDoDim - nDiagonal;

  offset = (int *) malloc(n*sizeof(int)); /* offsets to elements to be */
					  /* investigated */
  if (!offset)
    return cerror(ALLOC_ERR, 0);

  /* calculate offsets to elements to be investigated */
  /* we only need to treat n directions */
  /* see local_extreme() for more info */
  for (i = 0; i < srcinfo.ndim; i++)
    srcinfo.coords[i] = 0;
  srcinfo.coords[0] = 1;
    
  n0 = n1 = 0;
  n2 = 1;			/* defaults for when diagonal == 0 */
  for (k = 0; k < n; ) {
    if (diagonal) {
      n0 = n1 = n2 = 0;
      for (i = 0; i < srcinfo.ndim; i++)
	if (srcinfo.coords[i])
	  switch (diagonal[i]) {
	    case 0:
	      n0++;
	      break;
	    case 1:
	      n1++;
	      break;
	    case 2:
	      n2++;
	      break;
	  }
    }
    if (!n0 && ((n2 && !n1) || (n1 == 1 && !n2))) {
      /* OK: treat this direction */
      offset[k] = 0;
      for (j = 0; j < srcinfo.ndim; j++)
	offset[k] += srcinfo.rsinglestep[j]*srcinfo.coords[j];
      k++;
    }
    /* go to next direction.  If this algorithm were allowed to run
     all the way to (-1,0,0), then it would have cycled through all
     directions accessible from (0,0,0).  However, in this case we
     want only one member of each pair of diametrically opposite
     directions, because we compare the central element with the
     elements adjacent to it on both (diametrically opposite) sides.
     Fortunately, the last half of the complete cycle of this
     algorithm is the negative mirror image of the first half, so
     we can limit ourselves to the correct half by stopping when
     we have reached the correct number of directions: this is
     taken care of by the "for (k = 0; k < n; )" statement.  LS 19jun98 */
    for (j = 0; j < srcinfo.ndim; j++) {
      srcinfo.coords[j]++;
      if (srcinfo.coords[j] <= 1)
	break;
      for (i = 0; i <= j; i++)
	srcinfo.coords[i] = -1;
    }
  }

  zerobytes(srcinfo.coords, srcinfo.ndim*sizeof(int));
  nElem = srcinfo.dims[0];

  if (!diagonal || diagonal[0]) {
    i1 = 1;
    i2 = nElem - 1;
  } else {
    i1 = 0;
    i2 = nElem;
  }
  /* now do the loop work */
  if (sign && (internalMode & 2) == 0 && !diagonal && !haveThreshold)
		/* standard form - make as fast as possible */
    switch (array_type(ps[0])) {
      case ANA_BYTE:
	do {
	  for (edge = srcinfo.ndim - 1; edge; edge--)
	    if (!srcinfo.coords[edge]
		|| srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
	      break;		/* at edge */
	  if (edge) {
	    zerobytes(trgt.b, nElem);
	    trgt.b += nElem;
	    src.b += nElem;
	  } else {
	    *trgt.b++ = 0;		/* left edge */
	    src.b++;
	    for (i = i1; i < i2; i++) { /* center data points */
	      *trgt.b = 0;
	      for (j = 0; j < n; j++) {	/* all directions */	  
		k = offset[j];
		srcl.b = src.b + k;
		srcr.b = src.b - k;
		*trgt.b += (*src.b > *srcl.b && *src.b > *srcr.b);
	      }
	      src.b++;
	      trgt.b++;
	    }
	    *trgt.b++ = 0;		/* right edge */
	    src.b++;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	break;
      case ANA_WORD:
	do {
	  for (edge = srcinfo.ndim - 1; edge; edge--)
	    if (!srcinfo.coords[edge]
		|| srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
	      break;		/* at edge */
	  if (edge) {
	    zerobytes(trgt.b, nElem);
	    trgt.b += nElem;
	    src.w += nElem;
	  } else {
	    *trgt.b++ = 0;		/* left edge */
	    src.w++;
	    for (i = i1; i < i2; i++) { /* center data points */
	      *trgt.b = 0;
	      for (j = 0; j < n; j++) {	/* all directions */	  
		k = offset[j];
		srcl.w = src.w + k;
		srcr.w = src.w - k;
		*trgt.b += (*src.w > *srcl.w && *src.w > *srcr.w);
	      }
	      src.w++;
	      trgt.b++;
	    }
	    *trgt.b++ = 0;		/* right edge */
	    src.w++;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	break;
      case ANA_LONG:
	do {
	  for (edge = srcinfo.ndim - 1; edge; edge--)
	    if (!srcinfo.coords[edge]
		|| srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
	      break;		/* at edge */
	  if (edge) {
	    zerobytes(trgt.b, nElem);
	    trgt.b += nElem;
	    src.l += nElem;
	  } else {
	    *trgt.b++ = 0;		/* left edge */
	    src.l++;
	    for (i = i1; i < i2; i++) { /* center data points */
	      *trgt.b = 0;
	      for (j = 0; j < n; j++) {	/* all directions */	  
		k = offset[j];
		srcl.l = src.l + k;
		srcr.l = src.l - k;
		*trgt.b += (*src.l > *srcl.l && *src.l > *srcr.l);
	      }
	      src.l++;
	      trgt.b++;
	    }
	    *trgt.b++ = 0;		/* right edge */
	    src.l++;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	break;
      case ANA_FLOAT:
	do {
	  for (edge = srcinfo.ndim - 1; edge; edge--)
	    if (!srcinfo.coords[edge]
		|| srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
	      break;		/* at edge */
	  if (edge) {
	    zerobytes(trgt.b, nElem);
	    trgt.b += nElem;
	    src.f += nElem;
	  } else {
	    *trgt.b++ = 0;		/* left edge */
	    src.f++;
	    for (i = i1; i < i2; i++) { /* center data points */
	      *trgt.b = 0;
	      for (j = 0; j < n; j++) {	/* all directions */	  
		k = offset[j];
		srcl.f = src.f + k;
		srcr.f = src.f - k;
		*trgt.b += (*src.f > *srcl.f && *src.f > *srcr.f);
	      }
	      src.f++;
	      trgt.b++;
	    }
	    *trgt.b++ = 0;		/* right edge */
	    src.f++;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	break;
      case ANA_DOUBLE:
	do {
	  for (edge = srcinfo.ndim - 1; edge; edge--)
	    if (!srcinfo.coords[edge]
		|| srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
	      break;		/* at edge */
	  if (edge) {
	    zerobytes(trgt.b, nElem);
	    trgt.b += nElem;
	    src.d += nElem;
	  } else {
	    *trgt.b++ = 0;		/* left edge */
	    src.d++;
	    for (i = i1; i < i2; i++) { /* center data points */
	      *trgt.b = 0;
	      for (j = 0; j < n; j++) {	/* all directions */	  
		k = offset[j];
		srcl.d = src.d + k;
		srcr.d = src.d - k;
		*trgt.b += (*src.d > *srcl.d && *src.d > *srcr.d);
	      }
	      src.d++;
	      trgt.b++;
	    }
	    *trgt.b++ = 0;		/* right edge */
	    src.d++;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	break;
    } else 				/* general case - a bit slower */
      switch (array_type(ps[0])) {
	case ANA_BYTE:
	  do {
	    for (edge = srcinfo.ndim - 1; edge; edge--)
	      if ((!diagonal || diagonal[edge])
		  && (!srcinfo.coords[edge]
		      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
		break;		/* at edge */
	    if (edge) {
	      zerobytes(trgt.b, nElem);
	      trgt.b += nElem;
	      src.b += nElem;
	    } else {
	      if (!diagonal || diagonal[0]) {
		*trgt.b++ = 0; /* left edge */
		src.b++;
	      }
	      for (i = i1; i < i2; i++) { /* center data points */
		*trgt.b = 0;
		for (j = 0; j < n; j++) { /* all directions */	  
		  k = offset[j];
		  srcl.b = src.b + k;
		  srcr.b = src.b - k;
		  *trgt.b += ((sign && *src.b > *srcl.b + *t.b
			       && *src.b > *srcr.b + *t.b)
			      || (!sign && *src.b < *srcl.b - *t.b
				  && *src.b < *srcr.b - *t.b));
		}
		trgt.b++;
		src.b++;
	      }
	      if (!diagonal || diagonal[0]) {
		*trgt.b++ = 0;		/* right edge */
		src.b++;
	      }
	    }
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	  break;
	case ANA_WORD:
	  do {
	    for (edge = srcinfo.ndim - 1; edge; edge--)
	      if ((!diagonal || diagonal[edge])
		  && (!srcinfo.coords[edge]
		      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
		break;		/* at edge */
	    if (edge) {
	      zerobytes(trgt.b, nElem);
	      trgt.b += nElem;
	      src.w += nElem;
	    } else {
	      if (!diagonal || diagonal[0]) {
		*trgt.b++ = 0; /* left edge */
		src.w++;
	      }
	      for (i = i1; i < i2; i++) { /* center data points */
		*trgt.b = 0;
		for (j = 0; j < n; j++) { /* all directions */	  
		  k = offset[j];
		  srcl.w = src.w + k;
		  srcr.w = src.w - k;
		  *trgt.b += ((sign && *src.w > *srcl.w + *t.w
			       && *src.w > *srcr.w + *t.w)
			      || (!sign && *src.w < *srcl.w - *t.w
				  && *src.w < *srcr.w - *t.w));
		}
		trgt.b++;
		src.w++;
	      }
	      if (!diagonal || diagonal[0]) {
		*trgt.b++ = 0;		/* right edge */
		src.w++;
	      }
	    }
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	  break;
	case ANA_LONG:
	  do {
	    for (edge = srcinfo.ndim - 1; edge; edge--)
	      if ((!diagonal || diagonal[edge])
		  && (!srcinfo.coords[edge]
		      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
		break;		/* at edge */
	    if (edge) {
	      zerobytes(trgt.b, nElem);
	      trgt.b += nElem;
	      src.l += nElem;
	    } else {
	      if (!diagonal || diagonal[0]) {
		*trgt.b++ = 0; /* left edge */
		src.l++;
	      }
	      for (i = i1; i < i2; i++) { /* center data points */
		*trgt.b = 0;
		for (j = 0; j < n; j++) { /* all directions */	  
		  k = offset[j];
		  srcl.l = src.l + k;
		  srcr.l = src.l - k;
		  *trgt.b += ((sign && *src.l > *srcl.l + *t.l
			       && *src.l > *srcr.l + *t.l)
			      || (!sign && *src.l < *srcl.l - *t.l
				  && *src.l < *srcr.l - *t.l));
		}
		trgt.b++;
		src.l++;
	      }
	      if (!diagonal || diagonal[0]) {
		*trgt.b++ = 0;		/* right edge */
		src.l++;
	      }
	    }
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	  break;
	case ANA_FLOAT:
	  do {
	    for (edge = srcinfo.ndim - 1; edge; edge--)
	      if ((!diagonal || diagonal[edge])
		  && (!srcinfo.coords[edge]
		      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
		break;		/* at edge */
	    if (edge) {
	      zerobytes(trgt.b, nElem);
	      trgt.b += nElem;
	      src.f += nElem;
	    } else {
	      if (!diagonal || diagonal[0]) {
		*trgt.b++ = 0; /* left edge */
		src.f++;
	      }
	      for (i = i1; i < i2; i++) { /* center data points */
		*trgt.b = 0;
		for (j = 0; j < n; j++) { /* all directions */	  
		  k = offset[j];
		  srcl.f = src.f + k;
		  srcr.f = src.f - k;
		  *trgt.b += ((sign && *src.f > *srcl.f + *t.f
			       && *src.f > *srcr.f + *t.f)
			      || (!sign && *src.f < *srcl.f - *t.f
				  && *src.f < *srcr.f - *t.f));
		}
		trgt.b++;
		src.f++;
	      }
	      if (!diagonal || diagonal[0]) {
		*trgt.b++ = 0;		/* right edge */
		src.f++;
	      }
	    }
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	  break;
	case ANA_DOUBLE:
	  do {
	    for (edge = srcinfo.ndim - 1; edge; edge--)
	      if ((!diagonal || diagonal[edge])
		  && (!srcinfo.coords[edge]
		      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
		break;		/* at edge */
	    if (edge) {
	      zerobytes(trgt.b, nElem);
	      trgt.b += nElem;
	      src.d += nElem;
	    } else {
	      if (!diagonal || diagonal[0]) {
		*trgt.b++ = 0; /* left edge */
		src.d++;
	      }
	      for (i = i1; i < i2; i++) { /* center data points */
		*trgt.b = 0;
		for (j = 0; j < n; j++) { /* all directions */	  
		  k = offset[j];
		  srcl.d = src.d + k;
		  srcr.d = src.d - k;
		  *trgt.b += ((sign && *src.d > *srcl.d + *t.d
			       && *src.d > *srcr.d + *t.d)
			      || (!sign && *src.d < *srcl.d - *t.d
				  && *src.d < *srcr.d - *t.d));
		}
		trgt.b++;
		src.d++;
	      }
	      if (!diagonal || diagonal[0]) {
		*trgt.b++ = 0;		/* right edge */
		src.d++;
	      }
	    }
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.ndim);
	  break;
      }

  free(offset);
  return result;
}
/*----------------------------------------------------------------------*/
int ana_inpolygon(int narg, int ps[])
/* INPOLYGON(x,y,lx,ly) returns the indices of those points <x,y> that
 lie within the polyhon defined by points <lx,ly>.  LS 24nov98 */
{
  int	n, np, result, iq, type, *trgt, temptype, i, count, *trgt0, j;
  pointer	x, y, lx, ly;
  scalar	thisx, thisy, yc;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  n = array_size(ps[0]);
  if (!symbolIsNumericalArray(ps[1])
      || array_size(ps[1]) != n)
    return cerror(INCMP_ARG, ps[1]);
  if (!symbolIsNumericalArray(ps[2]))
    return cerror(ILL_CLASS, ps[2]);
  np = array_size(ps[2]);
  if (!symbolIsNumericalArray(ps[3])
      || array_size(ps[3]) != np)
    return cerror(INCMP_ARG, ps[3]);

  result = array_clone(ps[0], ANA_LONG);
  array_num_dims(result) = 1;
  array_dims(result)[0] = n;
  trgt = trgt0 = array_data(result);

  type = array_type(ps[0]);
  x.v = array_data(ps[0]);
  if (array_type(ps[1]) != type)
    iq = ana_converts[type](1, &ps[1]);
  else
    iq = ps[1];
  y.v = array_data(iq);

  temptype = (type == ANA_DOUBLE)? ANA_DOUBLE: ANA_FLOAT;

  if (array_type(ps[2]) != temptype)
    iq = ana_converts[temptype](1, &ps[2]);
  else
    iq = ps[2];
  lx.v = array_data(iq);
  if (array_type(ps[3]) != temptype)
    iq = ana_converts[temptype](1, &ps[3]);
  else
    iq = ps[3];
  ly.v = array_data(iq);

  j = 0;
  while (n--) {
    switch (type) {
      case ANA_BYTE:
	thisx.f = (float) *x.b++;
	thisy.f = (float) *y.b++;
	break;
      case ANA_WORD:
	thisx.f = (float) *x.w++;
	thisy.f = (float) *y.w++;
	break;
      case ANA_LONG:
	thisx.f = (float) *x.l++;
	thisy.f = (float) *y.l++;
	break;
      case ANA_FLOAT:
	thisx.f = *x.f++;
	thisy.f = *y.f++;
	break;
      case ANA_DOUBLE:
	thisx.d = *x.d++;
	thisy.d = *y.d++;
	break;
    }
    switch (temptype) {
      case ANA_FLOAT:
	count = 0;
	for (i = 0; i < np - 1; i++)
	  if ((thisx.f >= lx.f[i] || thisx.f >= lx.f[i + 1])
	      && (thisx.f < lx.f[i] || thisx.f < lx.f[i + 1])) {
	    yc.f = (ly.f[i]*(lx.f[i + 1] - thisx.f)
		    + ly.f[i + 1]*(thisx.f - lx.f[i]))/(lx.f[i + 1] - lx.f[i]);
	    count += (yc.f > thisy.f);
	  }
	if ((thisx.f >= lx.f[np - 1] || thisx.f >= lx.f[0])
	    && (thisx.f < lx.f[np - 1] || thisx.f < lx.f[0])) {
	  yc.f = (ly.f[np - 1]*(lx.f[0] - thisx.f)
		  + ly.f[0]*(thisx.f - lx.f[np - 1]))/(lx.f[0] - lx.f[np - 1]);
	  count += (yc.f > thisy.f);
	}
	if (count % 2)
	  *trgt++ = j;
	break;
      case ANA_DOUBLE:
	count = 0;
	for (i = 0; i < np - 1; i++)
	  if ((thisx.d >= lx.d[i] || thisx.d >= lx.d[i + 1])
	      && (thisx.d < lx.d[i] || thisx.d < lx.d[i + 1])) {
	    yc.d = (ly.d[i]*(lx.d[i + 1] - thisx.d)
		    + ly.d[i + 1]*(thisx.d - lx.d[i]))/(lx.d[i + 1] - lx.d[i]);
	    count += (yc.d > thisy.d);
	  }
	if ((thisx.d >= lx.d[np - 1] || thisx.d >= lx.d[0])
	    && (thisx.d < lx.d[np - 1] || thisx.d < lx.d[0])) {
	  yc.d = (ly.d[np - 1]*(lx.d[0] - thisx.d)
		  + ly.d[0]*(thisx.d - lx.d[np - 1]))/(lx.d[0] - lx.d[np - 1]);
	  count += (yc.d > thisy.d);
	}
	if (count % 2)
	  *trgt++ = j;
	break;
    }
    j++;
  }
  n = trgt - trgt0;
  if (n) {
    symbol_memory(result) = sizeof(array) + ana_type_size[temptype]*n;
    symbol_data(result) = realloc(symbol_data(result), symbol_memory(result));
    array_dims(result)[0] = n;
  } else {
    undefine(result);
    symbol_class(result) = ANA_SCALAR;
    scalar_type(result) = ANA_LONG;
    scalar_value(result).l = -1;
  }
  return result;
}
/*----------------------------------------------------------------------*/
int	*ptr1, *ptr2;
int ac_compare(const void *arg1, const void *arg2)
{
  int	i1, i2, d;

  i1 = *(int *) arg1;
  i2 = *(int *) arg2;
  d = ptr1[i1] - ptr1[i2];
  return d? d: ptr2[i1] - ptr2[i2];
}
/*----------------------------------------------------------------------*/
int intcmp(const void *arg1, const void *arg2)
{
  return *(int *) arg1 - *(int *) arg2;
}
/*----------------------------------------------------------------------*/
int ana_area_connect(int narg, int ps[])
/* AREACONNECT(im1, im2 [, compact] [, /RAW]) determines connections
 between areas in segmented images <im1> and <im2>.  If /RAW is specified,
 then the raw links
 are returned in the form of a 2D array with 3 elements in its second
 dimension.  If that array is called r, then r(*,0) contains area
 numbers in <im1>, r(*,1) area numbers in <im2>, and r(*,2) the number
 of pixels that have those area numbers in <im1> and <im2>.  By
 default (<raw> not zero) the topological classes of the areas are
 reported in variables $app (appearing in <im2>), $disapp
 (disappearing from <im1>), $split1 (splitting in <im1>), $split2
 (split products in <im2>), $merge1 (merge components in <im1>),
 $merge2 (merge product in <im2>), and $stay (remaining topologically
 the same), and the area numbers in <im2> are modified so that
 topologically connected areas have the same area number in <im2> as
 in <im1>, as far as possible. */
{
  char	raw = 0, compact = 1;
  byte	*flags;
  int	n, n2, dims[2], i, result, *order, *ptr, i2, v1, v2,
    max1, max2, *ptr0, j, i0, *out1, *out2, *order2, *list,
    qapp, qdisapp, qmerge1, qmerge2, qmerge1list, qsplit1, qsplit2,
    qsplit2list, qstay1, qstay2;

  raw = (internalMode & 1);
  if (narg > 2 && ps[2])
    compact = int_arg(ps[2]);

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_NUM_ARR, ps[0]);
  if (!symbolIsNumericalArray(ps[1]))
    return cerror(NEED_NUM_ARR, ps[1]);
  n = array_num_dims(ps[0]);
  for (i = 0; i < n; i++)
    if (array_dims(ps[0])[i] != array_dims(ps[1])[i])
      return cerror(INCMP_DIMS, ps[1]);
  if (array_type(ps[0]) != ANA_LONG)
    return anaerror("Need a LONG array", ps[0]);
  if (array_type(ps[1]) != ANA_LONG)
    return anaerror("Need a LONG array", ps[1]);
  ptr1 = array_data(ps[0]);
  ptr2 = array_data(ps[1]);
  n = array_size(ps[0]);

  /* we sort the entries by *ptr1 value and secondarily by *ptr2 value */
  order = malloc(n*sizeof(int));
  if (!order)
    return cerror(ALLOC_ERR, 0);
  for (i = 0; i < n; i++)
    order[i] = i;
  qsort(order, n, sizeof(int), ac_compare); /* sort */
  /* now ptr1[order[...]] is in ascending order, and */
  /* ptr2[order[...]] is in ascending order for each fixed value of */
  /* ptr1[order[...]] */

  /* we need to count how many unique entries there are, and we want to
     exclude pixels with zeros in both images */
  /* first we skip all entries (pixels) with zeros in both images */
  for (i = 0; i < n; i++)
    if (ptr1[order[i]] || ptr2[order[i]])
      break;

  if (i == n)			/* there aren't any entries that are */
				/* non-zero in both images */
    n2 = 0;
  else {			/* we do have some non-zero entries */
    n2 = 1;			/* count the first one */
    for (i2 = i + 1; i2 < n; i2++)
      /* if the current entry is unequal to the previous one, then it is */
      /* unique */
      if (ac_compare(order + i2 - 1, order + i2))
	n2++;			/* count the unique ones */
  }

  if (n2) {			/* we have some non-zero entries */
    /* prepare an output array */
    dims[0] = n2;		/* number of non-zero entries */
    dims[1] = 3;		/* three output numbers per entry */
    result = array_scratch(ANA_LONG, 2, dims);
    ptr = ptr0 = array_data(result); /* output pointer */
    /* we store the area numbers in both images, count the number of */
    /* pixels with that combination, and keep track of the maximum */
    /* included area numbers in both images. */
    max1 = ptr[0] = ptr1[order[i]];
    max2 = ptr[n2] = ptr2[order[i]];
    ptr[2*n2] = 1;		/* start with one entry */
    for (i2 = i + 1; i2 < n; i2++) { /*  */
      v1 = ptr1[order[i2]];	/* image 1 area number */
      v2 = ptr2[order[i2]];	/* image 2 area number */
      if (v1 > max1)
	max1 = v1;		/* track maximum value in image 1 */
      if (v2 > max2)
	max2 = v2;		/* track maximum value in image 2 */
      if (ac_compare(order + i2 - 1, order + i2)) {
	/* this entry is unequal to the previous one, so it is a new one */
	ptr++;			/* move to next slot */
	ptr[0] = v1;		/* store area numbers */
	ptr[n2] = v2;
	ptr[2*n2] = 1;		/* and start counting entries at 1 */
      } else
	ptr[2*n2]++;		/* it's the same as the previous one, */
				/* so we need only update the count */
    }
    free(order);		/* no longer needed */
  } else { 			/* no overlapping segments at all */
    free(order);		/* no longer needed */
    return ANA_MINUS_ONE;
  }

  if (raw)			/* we're done */
    return result;

  /* we now have a list of connections between areas in image 1 and */
  /* areas in image 2, that is sorted in ascending order first of */
  /* area number in image 1 and then, within each group with the */
  /* same image-1 area number, by area number in image 2. */
  /* For what follows, it is convenient to also have the ordering */
  /* by image 2 first and image 1 last.  We get that one now. */
  order = (int *) malloc(n2*sizeof(int));
  order2 = (int *) malloc(n2*sizeof(int));
  if (!order || !order2)
    return cerror(ALLOC_ERR, 0);
  for (i = 0; i < n2; i++)
    order[i] = i;
  ptr1 = ptr0 + n2;
  ptr2 = ptr0;
  qsort(order, n2, sizeof(int), ac_compare);
  /* now ptr1[order] is in ascending order of image-2 area numbers */
  /* and ptr2[order] in ascending order of image-1 area numbers for */
  /* each set of constant image-2 area numbers. */
  ptr1 = ptr0;			/* image-1 numbers */
  ptr2 = ptr0 + n2;		/* image-2 numbers */
  for (i = 0; i < n2; i++)	/* get the reverse ordering */
    order2[order[i]] = i;

  /* Those areas in image 1 that have more than one connection to */
  /* non-background areas in image 2 are splitting up.  We find them. */
  i = j = v1 = v2 = 0;
  /* seek the next all-non-zero connection */
  while (i < n2) {		/* not at the end yet */
    if (!ptr1[i] || !ptr2[i]) {	/* background (0) in either image */
      i++;			/* skip */
      continue;
    }
    if (ptr1[i] != j) 		/* a new image-1 area */
      j = ptr1[i++];
    else {			/* the second hit on this image-1 area */
      v1++;			/* increment count of hits in image1*/
      v2++;			/* and hits in image2 */
      for ( ; i < n2 && ptr1[i] == j; i++) /* count image-2 areas */
	if (ptr2[i] != ptr2[i - 1])
	  v2++;
    }
  }

  /* now v1 counts the number of image-1 areas that are splitting up, */
  /* and v2 counts the total number of image-2 areas that correspond */
  /* to the splitting image-1 areas. */

  /* we prepare output symbols for the results */

  /* we store the image-1 area numbers of splitters in $SPLIT1 */
  /* and the corresponding image-2 area numbers in $SPLIT2. */
  /* we store index values in $SPLIT2_INDEX such that the image-2 areas */
  /* that correspond to image-1 area $SPLIT1(I) are given by */
  /* $SPLIT2($SPLIT2_INDEX(I):$SPLIT2_INDEX(I+1)-1). */
  qsplit1 = findVarName("$SPLIT1", 0);
  if (qsplit1 == ANA_ERROR) {
    free(order);
    free(order2);
    return ANA_ERROR;
  }
  if (v1) {			/* we have <v1> splitters to store */
    if (to_scratch_array(qsplit1, ANA_LONG, 1, &v1) == ANA_ERROR) {
      free(order);
      free(order2);
      return ANA_ERROR;
    }
    out1 = array_data(qsplit1);
  } else {			/* no splitters to store: output scalar -1 */
    to_scalar(qsplit1, ANA_LONG);
    scalar_value(qsplit1).l = -1;
  }
  qsplit2list = findVarName("$SPLIT2_INDEX", 0);
  if (qsplit2list == ANA_ERROR) {
    free(order);
    free(order2);
    return ANA_ERROR;
  }
  if (v1) {
    v1++;			/* add one extra element to point just */
    /* beyond the end of the $SPLIT2 array, so that */
    /* $SPLIT2($SPLIT2_INDEX(I):$SPLIT2_INDEX(I+1)-1) works even for */
    /* the last valid value of I. */
    if (to_scratch_array(qsplit2list, ANA_LONG, 1, &v1) == ANA_ERROR) {
      free(order);
      free(order2);
      return ANA_ERROR;
    }
    list = array_data(qsplit2list);
    v1--;			/* return to previous value */
  } else {
    to_scalar(qsplit2list, ANA_LONG);
    scalar_value(qsplit2list).l = -1;
  }
  qsplit2 = findVarName("$SPLIT2", 0);
  if (qsplit2 == ANA_ERROR) {
    free(order);
    free(order2);
    return ANA_ERROR;
  }
  if (v2) {
    if (to_scratch_array(qsplit2, ANA_LONG, 1, &v2) == ANA_ERROR) {
      free(order);
      free(order2);
      return ANA_ERROR;
    }
    out2 = array_data(qsplit2);
  } else {
    to_scalar(qsplit2, ANA_LONG);
    scalar_value(qsplit2).l = -1;
  }
    
  /* now write the area numbers to the output variables. */
  i = j = v1 = v2 = 0;
  while (i < n2) {		/* not yet at end */
    if (!ptr1[i] || !ptr2[i]) {	/* seek non-zero values in both images */
      i++;
      continue;
    }
    if (ptr1[i] != j) 		/* a new image-1 area */
      j = ptr1[i++];		/* remember this area number */
    else {			/* the second hit on this image-1 area */
      *out1++ = ptr1[i];	/* store image-1 area number */
      *out2++ = ptr2[i - 1];	/* we must not forget the first hit: store */
				/* its image-2 value */
      *list++ = v2++;		/* store the index */
      for ( ; i < n2 && ptr1[i] == j; i++) /* count image-2 areas */
	if (ptr2[i] != ptr2[i - 1]) {
	  *out2++ = ptr2[i];
	  v2++;
	}
    }
  }
  /* we now add one element at the end of the $SPLIT2_INDEX array that */
  /* points to one beyond the end of the $SPLIT2 array */
  if (v2)
    *list = v2;
  
  /* Those areas in image 2 that have more than one connection to */
  /* non-background areas in image 1 have undergone merging.  We find them. */
  i = j = v1 = v2 = 0;
  /* seek the next all-non-zero connection */
  while (i < n2) {
    if (!ptr1[order[i]] || !ptr2[order[i]]) {
      i++;
      continue;
    }
    if (ptr2[order[i]] != j)	/* a new image-2 area */
      j = ptr2[order[i++]];
    else {			/* the second hit on this image-2 area */
      v1++;			/* increment count */
      v2++;
      for ( ; i < n2 && ptr2[order[i]] == j; i++) /* count image-1 areas */
	if (ptr1[order[i]] != ptr1[order[i - 1]])
	  v2++;
    }
  }

  qmerge2 = findVarName("$MERGE2", 0);
  if (qmerge2 == ANA_ERROR) {
    free(order);
    free(order2);
    return ANA_ERROR;
  }
  if (v1) {
    if (to_scratch_array(qmerge2, ANA_LONG, 1, &v1) == ANA_ERROR) {
      free(order);
      free(order2);
      return ANA_ERROR;
    }
    out1 = array_data(qmerge2);
  } else {
    to_scalar(qmerge2, ANA_LONG);
    scalar_value(qmerge2).l = -1;
  }
  qmerge1list = findVarName("$MERGE1_INDEX", 0);
  if (qmerge1list == ANA_ERROR) {
    free(order);
    free(order2);
    return ANA_ERROR;
  }
  if (v1) {
    v1++;
    if (to_scratch_array(qmerge1list, ANA_LONG, 1, &v1) == ANA_ERROR) {
      free(order);
      free(order2);
      return ANA_ERROR;
    }
    list = array_data(qmerge1list);
    v1--;
  } else {
    to_scalar(qmerge1list, ANA_LONG);
    scalar_value(qmerge1list).l = -1;
  }
  qmerge1 = findVarName("$MERGE1", 0);
  if (qmerge1 == ANA_ERROR) {
    free(order);
    free(order2);
    return ANA_ERROR;
  }
  if (v2) {
    if (to_scratch_array(qmerge1, ANA_LONG, 1, &v2) == ANA_ERROR) {
      free(order);
      free(order2);
      return ANA_ERROR;
    }
    out2 = array_data(qmerge1);
  } else {
    to_scalar(qmerge1, ANA_LONG);
    scalar_value(qmerge1).l = -1;
  }
    
  i = j = v1 = v2 = 0;
  /* seek the next all-non-zero connection */
  while (i < n2) {
    if (!ptr1[order[i]] || !ptr2[order[i]]) {
      i++;
      continue;
    }
    if (ptr2[order[i]] != j)	/* a new image-2 area */
      j = ptr2[order[i++]];
    else {			/* the second hit on this image-2 area */
      *out1++ = ptr2[order[i]];
      *out2++ = ptr1[order[i - 1]];
      *list++ = v2++;
      for ( ; i < n2 && ptr2[order[i]] == j; i++) /* count image-1 areas */
	if (ptr1[order[i]] != ptr1[order[i - 1]]) {
	  *out2++ = ptr1[order[i]];
	  v2++;
	}
    }
  }
  if (v2)
    *list = v2;

  /* The areas in image 2 that have no counterparts in image 1 have just
     appeared.  They have only a single entry in the list. */
  /* First we skip the links with zero in image 2 */
  for (i = 0; i < n2; i++)
    if (ptr2[order[i]])
      break;
  i0 = i;
  n = 0;
  for ( ; i < n2 - 1; i++)
    if (ptr1[order[i]] || ptr2[order[i + 1]] == ptr2[order[i]])
      continue;
    else
      n++;
  /* check the last one separately */
  if (!ptr1[order[n2 - 1]] && ptr2[order[n2 - 2]] != ptr2[order[n2 - 1]])
    n++;

  qapp = findVarName("$APP", 0);
  if (qapp == ANA_ERROR) {
    free(order);
    free(order2);
    return ANA_ERROR;
  }
  if (n) {
    if (to_scratch_array(qapp, ANA_LONG, 1, &n) == ANA_ERROR) {
      free(order);
      free(order2);
      return ANA_ERROR;
    }
    ptr = array_data(qapp);
    for (i = i0 ; i < n2 - 1; i++)
      if (ptr1[order[i]] || ptr2[order[i + 1]] == ptr2[order[i]])
	continue;
      else
	*ptr++ = ptr2[order[i]];
    /* check the last one separately */
    if (!ptr1[order[n2 - 1]] && ptr2[order[n2 - 2]] != ptr2[order[n2 - 1]])
      *ptr++ = ptr2[order[n2 - 1]];
  } else {
    to_scalar(qapp, ANA_LONG);
    scalar_value(qapp).l = -1;
  }

  /* The areas in image 1 that have no counterparts in image 2 have just
     disappeared.  They have only a single entry in the list. */
  /* First we skip the links with zero in image 1 */
  for (i = 0; i < n2; i++)
    if (ptr1[i])
      break;
  i0 = i;
  n = 0;
  for ( ; i < n2 - 1; i++)
    if (ptr2[i] || ptr1[i + 1] == ptr1[i])
      continue;
    else
      n++;
  /* check the last one separately */
  if (!ptr2[n2 - 1] && ptr1[n2 - 2] != ptr1[n2 - 1])
    n++;

  qdisapp = findVarName("$DISAPP", 0);
  if (qdisapp == ANA_ERROR) {
    free(order);
    free(order2);
    return ANA_ERROR;
  }
  if (n) {
    if (to_scratch_array(qdisapp, ANA_LONG, 1, &n) == ANA_ERROR) {
      free(order);
      free(order2);
      return ANA_ERROR;
    }
    ptr = array_data(qdisapp);
    for (i = i0 ; i < n2 - 1; i++)
      if (ptr2[i] || ptr1[i + 1] == ptr1[i])
	continue;
      else
	*ptr++ = ptr1[i];
    /* check the last one separately */
    if (!ptr2[n2 - 1] && ptr1[n2 - 2] != ptr1[n2 - 1])
      *ptr++ = ptr1[n2 - 1];
  } else {
    to_scalar(qdisapp, ANA_LONG);
    scalar_value(qdisapp).l = -1;
  }

  /* Those regions that neither appear nor disappear, split, or merge,
     are "stayers".  These can have only a single entry in the list
     with non-zero area numbers in both images */
  n = 0;
  for (i = 0; i < n2; i++) {
    if (!ptr1[i] || !ptr2[i])
      /* must have image-1 and image-2 area numbers unequal to zero */
      continue;
    if (i < n2 - 1 && ptr1[i + 1] == ptr1[i]) {
      /* must have only a single entry with non-zero image-2 for any */
      /* given image-1.  If a second such entry exists, then it follows */
      /* immediately upon the first one, because of the ordering of the */
      /* entries.  We found one such, so we can skip the rest of this */
      /* image-1 area number. */
      j = ptr1[i++];
      while (i < n2 - 1 && ptr1[i + 1] == j)
	i++;
      continue;
    }
    /* now we must check the same thing for the image-2 area */
    j = order2[i];		/* now order[order2[j]] == i */
    if (j < n2 - 1
	&& ptr2[order[j + 1]] == ptr2[order[j]]
	&& ptr1[order[j + 1]])
      /* found a seocnd entry for the image-2 area */
      continue;
    if (j > 0
	&& ptr2[order[j - 1]] == ptr2[order[j]]
	&& ptr1[order[j - 1]])
      /* found a seocnd entry for the image-2 area */
      continue;
    n++;
  }

  qstay1 = findVarName("$STAY1", 0);
  if (qstay1 == ANA_ERROR) {
    free(order);
    free(order2);
    return ANA_ERROR;
  }
  if (n) {
    if (to_scratch_array(qstay1, ANA_LONG, 1, &n) == ANA_ERROR) {
      free(order);
      free(order2);
      return ANA_ERROR;
    }
    out1 = array_data(qstay1);
  } else {
    to_scalar(qstay1, ANA_LONG);
    scalar_value(qstay1).l = -1;
  }

  qstay2 = findVarName("$STAY2", 0);
  if (qstay2 == ANA_ERROR) {
    free(order);
    free(order2);
    return ANA_ERROR;
  }
  if (n) {
    if (to_scratch_array(qstay2, ANA_LONG, 1, &n) == ANA_ERROR) {
      free(order);
      free(order2);
      return ANA_ERROR;
    }
    out2 = array_data(qstay2);
  } else {
    to_scalar(qstay2, ANA_LONG);
    scalar_value(qstay2).l = -1;
  }

  if (n) {
    for (i = 0; i < n2; i++) {
      if (!ptr1[i] || !ptr2[i])
	/* must have image-1 and image-2 area numbers unequal to zero */
	continue;
      if (i < n2 - 1 && ptr1[i + 1] == ptr1[i]) {
	/* must have only a single entry with non-zero image-2 for any */
	/* given image-1.  If a second such entry exists, then it follows */
	/* immediately upon the first one, because of the ordering of the */
	/* entries.  We found one such, so we can skip the rest of this */
	/* image-1 area number. */
	j = ptr1[i++];
	while (i < n2 - 1 && ptr1[i + 1] == j)
	  i++;
	continue;
      }
      /* now we must check the same thing for the image-2 area */
      j = order2[i];		/* now order[order2[j]] == i */
      if (j < n2 - 1
	  && ptr2[order[j + 1]] == ptr2[order[j]]
	  && ptr1[order[j + 1]])
	/* found a seocnd entry for the image-2 area */
	continue;
      if (j > 0
	  && ptr2[order[j - 1]] == ptr2[order[j]]
	  && ptr1[order[j - 1]])
	/* found a seocnd entry for the image-2 area */
	continue;
      *out1++ = ptr1[i];
      *out2++ = ptr2[i];
    }
  }

  /* now we do any requested area number compacting.  We only modify */
  /* the <im2> area numbers, and related global variables -- if */
  /* <im2> is a named variable. */
  /* if <compact> is equal to zero or negative, */
  /* then nothing is modified.  Otherwise, the <im2> numbers of staying */
  /* areas are set equal to the corresponding numbers from <im1> */

  if (compact > 0 && symbolProperName(ps[1])) {
    /* we need sorted lists of all <im1> and <im2> area numbers */
    out2 = order2;
    /* skip the <im2> zeros */
    for (i = 0; i < n2; i++)
      if (ptr2[order[i]])
	break;
    if (i == n2) {		/* don't have any */
      free(order);
      free(order2);
      return result;		/* so we're done */
    }    
    j = 0;
    for ( ; i < n2; i++)
      if (ptr2[order[i]] != j)  /* a new area number */
	*out2++ = j = ptr2[order[i]];
    n = n2;
    n2 = out2 - order2;		/* the number of them */
    /* now <order2> contains a sorted list of image-2 numbers */
    /* we go for the <im1> areas */
    out1 = order;
    for (i = 0; i < n; i++)
      if (ptr1[i])
	break;
    if (i == n)			/* don't have any */
      n = 0;
    else {
      j = 0;
      for ( ; i < n; i++)
	if (ptr1[i] != j)	/* a new area number */
	  *out1++ = j = ptr1[i];
      n = out1 - order;	/* the number of them */
    }

    /* we keep a list of flags to indicate which image-1 numbers */
    /* have already been assigned to some image-2 areas */
    flags = malloc(n);
    if (!flags) {
      free(order);
      free(order2);
      return cerror(ALLOC_ERR, 0);
    }
    zerobytes(flags, n);

    /* we store the replacement numbers in <ptr0> */
    ptr0 = malloc(n2*sizeof(int));
    if (!ptr0) {
      free(order);
      free(order2);
      free(flags);
      return cerror(ALLOC_ERR, 0);
    }
    zerobytes(ptr0, n2*sizeof(int));
    /* we start with stayers. */
    if (symbolIsArray(qstay1)) {
      out1 = array_data(qstay1);
      out2 = array_data(qstay2);
      j = array_size(qstay1);
      while (j--) {
	/* seek the encountered image-2 number in the list */
	ptr = bsearch(out2++, order2, n2, sizeof(int), intcmp);
	ptr0[ptr - order2] = *out1; /* replacement number */
	ptr = bsearch(out1++, order, n, sizeof(int), intcmp);
	flags[ptr - order] = 1; /* flag use of this number */
      }
    }
    /* now we work on the splitters; the image-1 number is assigned to */
    /* the first of the image-2 counterparts that does not yet have */
    /* a new assigned number. */
    if (symbolIsArray(qsplit1)) {
      out1 = array_data(qsplit1);
      out2 = array_data(qsplit2);
      list = array_data(qsplit2list);
      j = array_size(qsplit1);
      while (j--) {		/* all pre-split areas */
	i = list[1] - list[0];	/* the number of post-split areas */
	list++;
	/* corresponding to the current pre-split */
	while (i--) {		/* all corresponding post-split areas */
	  ptr = bsearch(out2++, order2, n2, sizeof(int), intcmp);
	  if (!ptr0[ptr - order2]) { /* not yet assigned */
	    ptr0[ptr - order2] = *out1;
	    ptr = bsearch(out1, order, n, sizeof(int), intcmp);
	    flags[ptr - order] = 1; /* flag use of this number */
	    break;
	  }
	}
	if (i)			/* done before all post-splits were treated */
	  out2 += i;
	out1++;			/* done with this one */
      }
    }
    /* and finally the mergers; the image-1 number of the first available */
    /* pre-merger area is assigned to the post-merger area, if that one */
    /* has not already been assigned to.  A given area may be part of */
    /* splitting and merging processes at the same time, so we must */
    /* be careful. */
    if (symbolIsArray(qmerge1)) {
      out1 = array_data(qmerge1);
      out2 = array_data(qmerge2);
      list = array_data(qmerge1list);
      j = array_size(qmerge2);
      while (j--) {		/* all post-merge areas */
	i = list[1] - list[0];	/* the number of pre-merge areas */
	list++;
	/* corresponding to the current post-merge */
	ptr = bsearch(out2, order2, n2, sizeof(int), intcmp);
	if (ptr0[ptr - order2]) {	/* already assigned to */
	  out2++;
	  out1 += i;
	  continue;
	}
	while (i--) {		/* all corresponding pre-merge areas */
	  ptr = bsearch(out1, order, n, sizeof(int), intcmp);
	  if (!flags[ptr - order]) { /* not yet assigned */
	    flags[ptr - order] = 1; /* flag use of this number */
	    ptr = bsearch(out2, order2, n2, sizeof(int), intcmp);
	    ptr0[ptr - order2] = *out1;
	    break;
	  }
	}
	if (i)			/* done before all pre-merges were treated */
	  out1 += i;
	out1++;
	out2++;			/* done with this one */
      }
    }
    /* we must still assign new numbers to all image-2 areas that have not */
    /* yet received any.  How this is dealt with depends on the value of */
    /* <compact>. */
    /*  < 1: no new numbers */
    /* == 1: contiguous numbers, starting at one greater than the */
    /*       maximum image-1 number */
    /* == 2: smallest available numbers no smaller than 2 */
    /*  > 2: contiguous numbers, starting at one greater than */
    /*       the value of <compact>. */
    if (compact == 2) {		/* we assign new numbers that are as small */
				/* as possible yet still unused */
      /* we locate the first available number at least equal to 2 */
      j = 2;
      i2 = 0;
      while (order[i2] < j && i2 <= n)
	i2++;
      if (i2 > n) {		/* we've reached the end of the range of */
				/* numbers used in image 1 */
	j = order[n - 1];
	for (i = 0; i < n2; i++)
	  if (!ptr0[i])
	    ptr0[i] = ++j;
      } else {
	for (i = 0; i < n2; i++)
	  if (!ptr0[i]) {	/* not yet assigned to */
	    while (j == order[i2]
		   && i2 <= n) {
	      j++;
	      i2++;
	    }
	    if (i2 > n)
	      break;
	    ptr0[i] = j++;
	  }
	if (i < n2)		/* we reached the end of the image-1 range */
	  for ( ; i < n2; i++)
	    if (!ptr0[i])
	      ptr0[i] = j++;
      }
    } else {			/* we assign new numbers starting at one */
      /* greater than the greatest image-1 number (if compact == 1) or */
      /* starting at one greater than the value of <compact> (if */
      /* compact > 1). */
      if (compact == 1 || compact < order[n - 1])
	j = order[n - 1];
      else
	j = compact;
      for (i = 0; i < n2; i++)
	if (!ptr0[i])		/* not yet assigned to */
	  ptr0[i] = ++j;
      if (compact > 1 && symbolProperName(ps[2])) {
	/* store highest assigned number in <compact>, if it is a */
	/* named variable (and not an expression) */
	to_scalar(ps[2], ANA_LONG);
	scalar_value(ps[2]).l = j;
      }
    }
    free(flags);

    /* now we have the new numbers, and we modify the image-2 numbers and */
    /* relevant global variables accordingly. */
    if (symbolIsArray(ps[1])) {
      list = array_data(ps[1]);	/* <im2> */
      i2 = array_size(ps[1]);
      j = -1;
      i = 0;
      while (i2--) {
	if (*list > 0) {
	  if (*list == j)
	    *list = i;
	  else {
	    j = *list;
	    ptr = bsearch(list, order2, n2, sizeof(int), intcmp);
	    *list = i = ptr0[ptr - order2];
	  }
	}
	list++;
      }
    }
    /* modify the return value */
    if (symbolIsArray(result)) {
      list = array_data(result);
      i2 = array_size(result)/3; /* we only want to change the image-2 */
				 /* numbers, i.e. 1/3rd of the whole. */
      list += i2;		/* move to start of image-2 numbers */
      while (i2--) {
	if (*list > 0) {
	  ptr = bsearch(list, order2, n2, sizeof(int), intcmp);
	  *list = ptr0[ptr - order2];
	}
	list++;
      }
    }
    /* modify $APP */
    if (symbolIsArray(qapp)) {
      list = array_data(qapp);
      i2 = array_size(qapp);
      while (i2--) {
	if (*list > 0) {
	  ptr = bsearch(list, order2, n2, sizeof(int), intcmp);
	  *list = ptr0[ptr - order2];
	}
	list++;
      }
    }
    /* modify $STAY2 */
    if (symbolIsArray(qstay2)) {
      list = array_data(qstay2);
      i2 = array_size(qstay2);
      while (i2--) {
	if (*list > 0) {
	  ptr = bsearch(list, order2, n2, sizeof(int), intcmp);
	  *list = ptr0[ptr - order2];
	}
	list++;
      }
    }
    /* modify $SPLIT2 */
    if (symbolIsArray(qsplit2)) {
      list = array_data(qsplit2);
      i2 = array_size(qsplit2);
      while (i2--) {
	if (*list > 0) {
	  ptr = bsearch(list, order2, n2, sizeof(int), intcmp);
	  *list = ptr0[ptr - order2];
	}
	list++;
      }
    }
    /* modify $MERGE2 */
    if (symbolIsArray(qmerge2)) {
      list = array_data(qmerge2);
      i2 = array_size(qmerge2);
      while (i2--) {
	if (*list > 0) {
	  ptr = bsearch(list, order2, n2, sizeof(int), intcmp);
	  *list = ptr0[ptr - order2];
	}
	list++;
      }
    }
    /* all done with this */
    free(ptr0);
  } /* end of if (compact > 0) */

  free(order);
  free(order2);

  return result;
}
/*----------------------------------------------------------------------*/
