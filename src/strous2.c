/* File strous2.c */
/* Various ANA routines by L. Strous. */
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include "action.h"
#include <unistd.h>		/* for sbrk() */
static char rcsid[] __attribute__ ((unused)) =
 "$Id: strous2.c,v 4.0 2001/02/07 20:37:04 strous Exp $";

Int	minmax(Int *, Int, Int), ana_convert(Int, Int [], Int, Int),
  copySym(Int), f_decomp(Float *, Int, Int), copyToSym(Int, Int),
  f_solve(Float *, Float *, Int, Int), to_scratch_array(Int, Int, Int, Int *);
/*---------------------------------------------------------*/
Int ana_noop(Int narg, Int ps[])
     /* no operation - a dummy routine that provides an entry point
	for debuggers */
{
  return 1;
}
/*---------------------------------------------------------*/
Int ana_zap(Int narg, Int ps[])
/* remove the specified symbols completely: i.e. leave no trace */
/* only allowed from main execution level, to prevent zapping of */
/* a variable which may be referenced later on in an already-compiled */
/* -and-now-executing piece of code */
{
  Int 	iq;
  void	cleanUpRoutine(Int context, char keepBase);

  if (curContext > 0)
    return anaerror("ZAP only allowed from main execution level!", 0);
  while (narg--) {
    if ((internalMode & 1) == 0
	|| (sym[*ps].class != ANA_POINTER
	    && sym[*ps].class != ANA_TRANSFER))
      iq = eval(*ps++);
    else
      iq = *ps++;
    if (symbol_class(iq) != ANA_FUNC_PTR)
      zap(iq);
    else {
      iq = func_ptr_routine_num(iq);
      if (iq < 0)		/* an internal routine or function */
	return anaerror("Cannot zap an internal routine or function!", *ps);
      cleanUpRoutine(iq, 0);
    }
  }
  return 1;
}
/*---------------------------------------------------------*/
Int showstats(Int narg, Int ps[])
{ 
  extern Word	listStack[], *listStackItem, curContext;
  extern Int	nNamedVariable, nTempVariable, markIndex,
  		tempExecutableIndex, executeLevel,
  		nSymbolStack, nExecutable, statementDepth, compileLevel;
  extern char	*firstbreak;
#if DEBUG
  extern Int	nAlloc;
  extern size_t	tSize;

  printf("NALL    TALLOC ");
#endif
  printf("       SZ IGN STD EXL CPL LST NVAR TVAR  EXE TEXE SSTK CTXT  TM\n");
#if DEBUG
  printf("%4d %9d ", nAlloc, tSize);
#endif
  printf("%9d %3d %3d %3d %3d %3d %4d %4d %4d %4d %4d %4d %3d\n",
	 (char *) sbrk(0) - firstbreak,
	 ignoreInput, statementDepth, executeLevel, compileLevel,
	 listStackItem - listStack, nNamedVariable,
	 nTempVariable, nExecutable,
	 tempExecutableIndex - TEMP_EXE_START, nSymbolStack,
	 curContext, markIndex - 1);
  return 1;
}
/*---------------------------------------------------------*/
Int ana_system(Int narg, Int ps[])
/* pass a string to the operating system */
{
  char	*name;

  name = string_arg(*ps);
  system(name);
  return ANA_OK;
}
/*---------------------------------------------------------*/
Int ana_psum(Int narg, Int ps[])
/* Y = PSUM(X, POWERS [,AXES, CLASS][,/ONEDIM,/NORMALIZE,/SINGLE]) returns a
   weighted sum of <x>. */
/* The weights are equal to the coordinates in the dimensions given in */
/* <axes> to the integer powers given in <powers>.  <powers> and <axes> */
/* may be scalars or arrays and must have the same number of elements. */
/* If <axes> is not specified, then INDGEN(POWERS) is assumed.
   Keyword */
/* /ONEDIM indicates that <x> should be treated as a 1D array.  LS
   8jan96 */
/* If CLASS is specified, then the results are gathered separately */
/* for each class.  /NORMALIZE forces normalization of the results. */
/* LS 17feb97 */
/* /SINGLE collects the results in a single scalar (unless CLASS is */
/* specified) LS 17mar97 */
{
  Int tally[MAX_DIMS], step[MAX_DIMS], *dims, done = 0, n, i, done2, done3,
  	ndim, type, *axes, naxes, result_sym = 0, size, iq, iStep[MAX_DIMS];
  pointer	src, trgt;
  Int	*exponent, nexponent, j, k, *temp, outdims[MAX_DIMS],
	in[MAX_DIMS], *index, nIndex, offset;
  Double	value, factor, weight, bigweight, *bigweights;
  char	haveClass = '\0';
  extern scalar	lastmin, lastmax;

  iq = ana_long(1, &ps[1]);	/* integer exponents only */
  switch (symbol_class(iq)) {	/* POWERS */
    case ANA_SCALAR:
      nexponent = 1;
      exponent = &scalar_value(iq).l;
      break;
    case ANA_ARRAY:
      nexponent = array_size(iq);
      exponent = (Int *) array_data(iq);
      break;
    default:
      return cerror(ILL_CLASS, ps[1]);
  }
  temp = exponent;
  for (i = 0; i < nexponent; i++) {
    if (temp[i++] < 0)
      return anaerror("PSUM - Only nonnegative integer exponents allowed", ps[1]);
  }
  switch (symbol_class(*ps)) {	/* X */
    case ANA_SCALAR:
      ndim = 1;
      dims = &ndim;
      src.l = &scalar_value(*ps).l;
      size = 1;
      break;
    case ANA_ARRAY:
      ndim = array_num_dims(*ps);
      dims = array_dims(*ps);
      src.l = (Int *) array_data(*ps);
      size = array_size(*ps);
      break;
    default:
      return cerror(ILL_CLASS, *ps);
  }
  if (internalMode & 1 && ndim > 1) { /* /ONEDIM */
    ndim = 1;
    dims = &size;
  }
  type = symbol_type(*ps);
  if (narg > 2) {
    iq = ana_long(1, &ps[2]);	/* AXES */
    switch (symbol_class(iq)) {
      case ANA_SCALAR:
	naxes = 1;
	axes = &scalar_value(iq).l;
	break;
      case ANA_ARRAY:
	naxes = array_size(iq);
	axes = (Int *) array_data(iq);
	break;
      default:
	return cerror(ILL_CLASS, *ps);
    }
    if (naxes != nexponent)
      return cerror(INCMP_ARG, ps[2]);
    temp = axes;
    for (i = 0; i < naxes; i++) {
      if (temp[i] < 0 || temp[i] >= ndim)
	return cerror(ILL_DIM, ps[2]);
    }
    /* determine the dimensions of the result */
    for (i = 0; i < ndim; i++)
      in[i] = 0;
    for (i = 0; i < naxes; i++)
      in[axes[i]]++;
    for (i = 0; i < ndim; i++)
      if (in[i] > 1)
	return anaerror("A dimension was specified multiple times", ps[2]);
    j = 0;
    for (i = 0; i < ndim; i++)
      if (!in[i])		/* no summing this dim, so is in result */
	outdims[j++] = dims[i];
    if (narg > 3) {		/* CLASS */
      iq = ana_long(1, &ps[3]);	/* integer classes */
      haveClass = 1;
      index = array_data(iq);
      nIndex = array_size(iq);
      if (nIndex != size)
	return cerror(INCMP_ARR, ps[3]);
      minmax(index, nIndex, ANA_LONG); /* determine range of indices */
      size = lastmax.l + 1;
      offset = 0;
      if (lastmin.l < 0)
	size += (offset = -lastmin.l);
      result_sym = array_scratch(ANA_DOUBLE, 1, &size);
      trgt.l = array_data(result_sym);
      zerobytes(trgt.b, size*sizeof(Double));
      allocate(bigweights, size, Double);
      zerobytes(bigweights, size*sizeof(Double));
    }
  } else {			/* no AXES, assume INDGEN(POWERS) */
    axes = NULL;
    naxes = nexponent;
    if (ndim > naxes)
      memcpy(outdims, dims + naxes, (ndim - naxes)*sizeof(Int));
  }
  if (!result_sym) {
    if (ndim == 1 || naxes == ndim || internalMode & 8) {
      result_sym = scalar_scratch(ANA_DOUBLE);
      trgt.b = &scalar_value(result_sym).b;
    } else {
      n = 1;
      for (i = 0; i < ndim - naxes; i++)
	n *= outdims[i];
      result_sym = array_scratch(ANA_DOUBLE, 1, &n);
      array_num_dims(result_sym) = ndim - naxes;
      memcpy(array_dims(result_sym), outdims, (ndim - naxes)*sizeof(Int));
      trgt.l = array_data(result_sym);
    }
  }

				/* initialize tally &c */
  n = *step = ana_type_size[type];
  for (i = 1; i < ndim; i++)
    step[i] = (n *= dims[i - 1]);
  if (axes) {			/* swap dims so requested ones are first */
    for (i = 0; i < naxes; i++) {
      outdims[i] = dims[axes[i]];
      tally[i] = step[axes[i]];
    }
    j = naxes;
    for (i = 0; i < ndim; i++)
      if (!in[i]) {
	outdims[j] = dims[i];
	tally[j++] = step[i];
      }
    memcpy(step, tally, ndim*sizeof(Int));
  } else
    memcpy(outdims, dims, ndim*sizeof(Int));
  for (i = ndim - 1; i; i--)
    step[i] -= step[i - 1]*outdims[i - 1];
  for (i = 0; i < ndim; i++)
    iStep[i] = step[i]/ana_type_size[type];
  for (i = 0; i < ndim; i++)
    tally[i] = 0;
  if (ndim == 1)
    done = 1;			/* only one go is needed */
  *trgt.d = bigweight = 0;
  done2 = (internalMode & 6) && !haveClass && !(internalMode & 8);
  done3 = !haveClass && !(internalMode & 8);
  do {			      /* loop over all elements */
    switch (type) {
      case ANA_BYTE:
	value = (Double) *src.b;
	break;
      case ANA_WORD:
	value = (Double) *src.w;
	break;
      case ANA_LONG:
	value = (Double) *src.l;
	break;
      case ANA_FLOAT:
	value = (Double) *src.f;
	break;
      case ANA_DOUBLE:
	value = *src.d;
	break; }
    weight = 1.0;
    for (j = 0; j < nexponent; j++)
      if (exponent[j]) {	/* not equal to 0 */
	factor = tally[j];	/* current jth coordinate */
	if (exponent[j] % 2)
	  weight *= factor;
	k = exponent[j]/2;
	while (k) {
	  factor *= factor;
	  if (k % 2)
	    weight *= factor;
	  k /= 2;
	}
      }
    if (haveClass) {
      if (internalMode & 2)	/* /VNORMALIZE */
	bigweights[*index] += value;
      else if (internalMode & 4) /* /CNORMALIZE */
	bigweights[*index] += weight;
      trgt.d[*index] += value*weight;
      index += iStep[0];
    } else {
      if (internalMode & 2)	/* /VNORMALIZE */
	bigweight += value;
      else if (internalMode & 4) /* /CNORMALIZE */
	bigweight += weight;
      *trgt.d += value*weight;
    }
    src.b += step[0];
    for (i = 0; i < ndim; i++) {  /* must another dimension be advanced? */
      if (tally[i]++ != outdims[i] - 1) {
	done = 0;
	break;			/* not yet finished with this dimension */
      }
      tally[i] = 0;
      done = 1;			/* done with this dimension anyway */
      src.b += step[i + 1]; 	/* advance another dimension */
      if (haveClass)
	index += iStep[i + 1];
    }
    if (i >= naxes) {
      if (done2) {
	if (bigweight)
	  *trgt.d /= bigweight;
	else
	  *trgt.d = 0.0;
      }
      if (done3 && !done)
	*++trgt.d = bigweight = 0.0;
    }
  } while (!done);
  if (haveClass) {
    for (i = 0; i < size; i++)
      if (bigweights[i])
	trgt.d[i] /= bigweights[i];
    free(bigweights);
  } else if (internalMode & 8 && internalMode & 6) /* /SINGLE + /.NORMALIZE */
    *trgt.d /= bigweight;
  return result_sym;
}
/*---------------------------------------------------------*/
pointer	multiCompData;
Int	multiCompNPoints, multiCompNCoord, multiCompType;
Int multiCompare(const void *arg1, const void *arg2)
/* comparison function for use with ana_tolookup.  LS 9nov95 */
{
  Int	i1, i2, i;
  scalar	d;

  i1 = *(Int *) arg1;
  i2 = *(Int *) arg2;
  switch (multiCompType) {
  case ANA_BYTE:
    for (i = 0; i < multiCompNCoord; i++) {
      d.l = (Int) multiCompData.b[i1 + i*multiCompNPoints]
	- (Int) multiCompData.b[i2 + i*multiCompNPoints];
      if (d.l)
	return d.l;
    }
    break;
  case ANA_WORD:
    for (i = 0; i < multiCompNCoord; i++) {
      d.l = (Int) multiCompData.w[i1 + i*multiCompNPoints]
	- (Int) multiCompData.w[i2 + i*multiCompNPoints];
      if (d.l)
	return d.l;
    }
    break;
  case ANA_LONG:
    for (i = 0; i < multiCompNCoord; i++) {
      d.l = (Int) multiCompData.l[i1 + i*multiCompNPoints]
	- (Int) multiCompData.l[i2 + i*multiCompNPoints];
      if (d.l)
	return d.l;
    }
    break;
  case ANA_FLOAT:
    for (i = 0; i < multiCompNCoord; i++) {
      d.f =  multiCompData.f[i1 + i*multiCompNPoints]
	- multiCompData.f[i2 + i*multiCompNPoints];
      if (d.f)
	return d.f > 0? 1: -1;
    }
    break;
  case ANA_DOUBLE:
    for (i = 0; i < multiCompNCoord; i++) {
      d.d =  multiCompData.d[i1 + i*multiCompNPoints]
	- multiCompData.d[i2 + i*multiCompNPoints];
      if (d.d)
	return d.d > 0? 1: -1;
    }
    break;
  }
  return 0;
}
/*---------------------------------------------------------*/
Int ana_tolookup(Int narg, Int ps[])
/* TOLOOKUP,src,list,index rewrites <src> as <list(index)>, with */
/* <list> a list of unique members of <src> (sorted in ascending */
/* order) and <index> ranging between 0 and NUM_ELEM(list) - 1. */
/* If <src> is multidimensional, then its last dimension is */
/* taken to run along a vector and <list> will be a list of */
/* vectors.  Keyword /ONE signals treatment of <src> as a big */
/* 1D vector.  LS 9nov95 7may96 16oct97 */
{
  Int	iq, *index, i, dims[2], curIndex, n, *order, size,
	nd, j;
  Byte	*list;

  if (symbol_class(*ps) != ANA_ARRAY)
    return cerror(NEED_ARR, *ps);
  iq = *ps;			/* SRC */
  multiCompData.l = (Int *) array_data(iq);
  multiCompType = array_type(iq);
  index = array_dims(iq);
  nd = array_num_dims(iq);
  if (internalMode & 1)		/* /ONE */
    multiCompNCoord = 1;
  else
    multiCompNCoord = nd > 1? index[nd - 1]: 1; /* # dimensions */
  multiCompNPoints = array_size(iq)/multiCompNCoord; /* # data points */
  allocate(order, multiCompNPoints, Int);
  for (i = 0; i < multiCompNPoints; i++)
    order[i] = i;		/* initialize */
  qsort(order, multiCompNPoints, sizeof(Int), multiCompare); /* sort */

  n = 1;
  for (i = 0; i < multiCompNPoints - 1; i++) /* count number of unique */
					     /* members */
    if (multiCompare(order + i, order + i + 1))
      n++;
  j = (multiCompNCoord > 1)? nd - 1: nd;
  redef_array(ps[2], ANA_LONG, j, index); /* INDEX */
  index = (Int *) array_data(ps[2]);
  dims[0] = n;
  dims[1] = multiCompNCoord;
  redef_array(ps[1], array_type(iq), dims[1] > 1? 2: 1, dims); /* LIST */

  list = (Byte *) array_data(ps[1]);
  size = ana_type_size[array_type(iq)];
  for (j = 0; j < multiCompNCoord; j++)
    memcpy(list + j*n*size,
	   multiCompData.b + (order[0] + j*multiCompNPoints)*size, size);
  index[order[0]] = curIndex = 0;
  for (i = 1; i < multiCompNPoints; i++)
    if (multiCompare(order + i - 1, order + i)) /* unequal */
    { index[order[i]] = ++curIndex;
      for (j = 0; j < multiCompNCoord; j++)
	memcpy(list + (curIndex + j*n)*size,
	       multiCompData.b + (order[i] + j*multiCompNPoints)*size, size);
    } else
      index[order[i]] = curIndex;
  free(order);
  return 1;
}
/*---------------------------------------------------------*/
pointer	src;
Int	type;
Int mcmp(const void *x1, const void *x2)
/* auxilliary for ana_medianfilter */
{
  extern pointer	src;
  extern Int	type;
  scalar	d1, d2;

  switch (type)
  { case ANA_BYTE:
      d1.b = src.b[*(Int *) x1];
      d2.b = src.b[*(Int *) x2];
      return d1.b < d2.b? -1: (d1.b > d2.b? 1: 0);
    case ANA_WORD:
      d1.w = src.w[*(Int *) x1];
      d2.w = src.w[*(Int *) x2];
      return d1.w < d2.w? -1: (d1.w > d2.w? 1: 0);
    case ANA_LONG:
      d1.l = src.l[*(Int *) x1];
      d2.l = src.l[*(Int *) x2];
      return d1.l < d2.l? -1: (d1.l > d2.l? 1: 0);
    case ANA_FLOAT:
      d1.f = src.f[*(Int *) x1];
      d2.f = src.f[*(Int *) x2];
      return d1.f < d2.f? -1: (d1.f > d2.f? 1: 0);
    case ANA_DOUBLE:
      d1.d = src.d[*(Int *) x1];
      d2.d = src.d[*(Int *) x2];
      return d1.d < d2.d? -1: (d1.d > d2.d? 1: 0); }
  return 1;
}
/*---------------------------------------------------------*/
Int cmp(const void *x1, const void *x2)
{
  extern Int	type;

  switch (type) {
    case ANA_BYTE:
      return *(Byte *) x1 < *(Byte *) x2? -1:
	(*(Byte *) x1 > *(Byte *) x2? 1: 0);
    case ANA_WORD:
      return *(Word *) x1 < *(Word *) x2? -1:
	(*(Word *) x1 > *(Word *) x2? 1: 0);
    case ANA_LONG:
      return *(Int *) x1 < *(Int *) x2? -1:
	(*(Int *) x1 > *(Int *) x2? 1: 0);
    case ANA_FLOAT:
      return *(Float *) x1 < *(Float *) x2? -1:
	(*(Float *) x1 > *(Float *) x2? 1: 0);
    case ANA_DOUBLE:
      return *(Double *) x1 < *(Double *) x2? -1:
	(*(Double *) x1 > *(Double *) x2? 1: 0);
  }
  return 1;			/* or some compilers complain */
}
/*---------------------------------------------------------*/
Int ana_orderfilter(Int narg, Int ps[])
/* Applies an ordinal filter to a data set */
/* syntax:  Y = ORDFILTER([ORDER=ORDER,] X [[,AXES],WIDTH,
                /MEDIAN,/MINIMUM,/MAXIMUM]) */
/* LS 23oct98 */
{
  Int	output, width, w, med, nelem, range[2*MAX_DIMS], *index,
    *offset, i, j;
  Float	order;
  loopInfo	srcinfo, trgtinfo, tmpinfo, tmpinfo2;
  pointer	trgt, tmpsrc, tmptrgt;
  /* we use a global pointer src */

  if (!ps[1])			/* no <data> */
    return anaerror("Need data array", 0);
  
  if (narg > 2) {		/* have <width> */
    width = int_arg((narg == 4)? ps[3]: ps[2]);
    if (width < 3)
      return ps[1];		/* nothing to do */
  } else
    width = 3;
  w = (width - 1)/2;
  width = 2*w + 1;		/* ensure that <width> is odd */

  if (ps[0]) {			/* have <order> */
    order = float_arg(ps[0]);
    if (order < 0.0 || order > 1.0)
      return anaerror("Order fraction must be between 0 and 1, was %f",
		   ps[0], order);
  } else
    order = -1;			/* flag that we didn't have one */

  if (standardLoop(ps[1], (narg == 4)? ps[2]: 0, 
		   (narg != 4? SL_ALLAXES: 0) | SL_UNIQUEAXES
		   | SL_AXESBLOCK | SL_KEEPTYPE, 0,
		   &srcinfo, &src, &output, &trgtinfo, &trgt) == ANA_ERROR)
    return ANA_ERROR;
  
  /* we calculate the number of elements that go into each sorting */
  nelem = 1;
  for (i = 0; i < srcinfo.naxes; i++)
    nelem *= width;
  if (order != -1) 		/* we had an explicit <order> */
    med = (Int) (order*(nelem - 0.0001)); /* the index in the sorted
					   data values of the value to be
					   returned. */
  else switch (internalMode) {
    case 0: case 1:		/* /MEDIAN or default */
      med = (nelem - 1)/2;
      break;
    case 2:
      med = 0;
      break;
    case 3:
      med = nelem - 1;
      break;
    default:
      return anaerror("Illegal keyword combination", 0);
  }

  /* we do not treat data that is on an edge.  Rather than checking for
     each data point if it is on an edge, we do a plain copy of the data
     along the edges and then just treat the interior points.
     First we copy the edges. */
  for (i = 0; i < 2*srcinfo.naxes; i++) {
    /* since rearrangeEdgeLoop() modifies its loopInfo arguments, we make
       copies to feed it so the original (srcinfo) remaines unchanged. */
    tmpinfo = srcinfo;
    tmpinfo.data = &tmpsrc;
    tmpsrc = src;
    tmpinfo2 = trgtinfo;
    tmpinfo2.data = &tmptrgt;
    tmptrgt = trgt;
    rearrangeEdgeLoop(&tmpinfo, &tmpinfo2, i); /* get ready */
    do
      memcpy(tmptrgt.b, tmpsrc.b, tmpinfo.stride); /* copy */
    while (advanceLoop(&tmpinfo2, &tmptrgt),
	   advanceLoop(&tmpinfo, &tmpsrc) < tmpinfo.ndim - 1);
  }

  /* we calculate the offsets of the elements to be considered */
  for (i = 0; i < srcinfo.naxes; i++)
    range[i] = width;
  tmpsrc = src;			/* just to be safe */
  setupDimensionLoop(&tmpinfo, srcinfo.naxes, range, ANA_LONG, srcinfo.naxes,
		     NULL, &tmpsrc, SL_EACHCOORD);

  index = malloc(nelem*sizeof(Int));
  offset = malloc(nelem*sizeof(Int));
  if (!index || !offset)
    return cerror(ALLOC_ERR, 0);

  i = 0;
  do {
    offset[i] = 0;
    for (j = 0; j < srcinfo.naxes; j++)
      offset[i] += srcinfo.rsinglestep[j]*(tmpinfo.coords[j] - w);
    i++;
  } while (advanceLoop(&tmpinfo, &tmpsrc) < tmpinfo.ndim);

  /* now we arrange to select only the interior area.
     we start by specifying ranges comprising the whole data set */
  for (i = 0; i < srcinfo.ndim; i++) {
    range[2*i] = 0;
    range[2*i + 1] = srcinfo.dims[i] - 1;
  }
  /* and then we modify those specified in <axis> */
  for (i = 0; i < srcinfo.naxes; i++) {
    range[2*srcinfo.axes[i]] = w;
    range[2*srcinfo.axes[i] + 1] -= w;
  }
  /* and then we modify the loops to reflect the reduced ranges */
  subdataLoop(range, &srcinfo);
  subdataLoop(range, &trgtinfo);

  type = srcinfo.type;		/* so mcmp() knows what type it is */

  /* now we do the actual filtering */
  do {
    memcpy(index, offset, nelem*sizeof(Int));
    qsort(index, nelem, sizeof(Int *), mcmp);
    switch (type) {
      case ANA_BYTE:
	*trgt.b = src.b[index[med]];
	break;
      case ANA_WORD:
	*trgt.w = src.w[index[med]];
	break;
      case ANA_LONG:
	*trgt.l = src.l[index[med]];
	break;
      case ANA_FLOAT:
	*trgt.f = src.f[index[med]];
	break;
      case ANA_DOUBLE:
	*trgt.d = src.d[index[med]];
	break;
    }
  } while (advanceLoop(&trgtinfo, &trgt),
	   advanceLoop(&srcinfo, &src) < srcinfo.ndim);

  free(index);
  free(offset);
  return output;
}
/*---------------------------------------------------------*/
Int ana_medfilter(Int narg, Int ps[])
     /* median filter */
{
  internalMode = 1;
  return ana_orderfilter(narg, ps);
}
/*---------------------------------------------------------*/
Int ana_quantile(Int narg, Int ps[])
/* QUANTILE([<order> ,] <data> [, <axes>]) */
{
  Int	output, med, nelem, i;
  Float	order;
  loopInfo	srcinfo, trgtinfo;
  pointer	trgt, tmp, tmp0;
  /* we use a global pointer src */

  if (!ps[1])			/* no <data> */
    return anaerror("Need data array", 0);
  
  if (ps[0]) {			/* have <order> */
    order = float_arg(ps[0]);
    if (order < 0.0 || order > 1.0)
      return anaerror("Order fraction must be between 0 and 1, was %f",
		   ps[0], order);
  } else
    order = -1;			/* flag that we didn't have one */

  if (standardLoop(ps[1], (narg > 2)? ps[2]: 0,
		   (narg <= 2? SL_ALLAXES: 0) | SL_UNIQUEAXES | SL_AXESBLOCK
		   | SL_KEEPTYPE | SL_COMPRESSALL,
		   0, &srcinfo, &src, &output, &trgtinfo, &trgt) == ANA_ERROR)
    return ANA_ERROR;
  
  /* we calculate the number of elements that go into each sorting */
  nelem = 1;
  for (i = 0; i < srcinfo.naxes; i++)
    nelem *= srcinfo.rdims[i];
  if (order != -1) 		/* we had an explicit <order> */
    med = (Int) (order*(nelem - 0.0001)); /* the index in the sorted
					   data values of the value to be
					   returned. */
  else switch (internalMode) {
    case 0: case 1:		/* /MEDIAN or default */
      med = (nelem - 1)/2;
      break;
    case 2:
      med = 0;
      break;
    case 3:
      med = nelem - 1;
      break;
    default:
      return anaerror("Illegal keyword combination", 0);
  }

  type = srcinfo.type;		/* so mcmp() knows what type it is */

  tmp0.b = malloc(nelem*srcinfo.stride);
  if (!tmp0.b)
    return cerror(ALLOC_ERR, 0);

  /* now we do the actual filtering */
  do {
    tmp.b = tmp0.b;
    do {
      memcpy(tmp.b, src.b, srcinfo.stride);
      tmp.b += srcinfo.stride;
    } while (advanceLoop(&srcinfo, &src) < srcinfo.naxes);
    qsort(tmp0.b, nelem, srcinfo.stride, cmp);
    switch (type) {
      case ANA_BYTE:
	if (nelem % 2)
	  *trgt.b = tmp0.b[med];
	else
	  *trgt.b = ((Word) tmp0.b[med] + tmp0.b[med + 1])/2;
	break;
      case ANA_WORD:
	if (nelem % 2)
	  *trgt.w = tmp0.w[med];
	else
	  *trgt.w = ((Int) tmp0.w[med] + tmp0.w[med + 1])/2;
	break;
      case ANA_LONG:
	if (nelem % 2)
	  *trgt.l = tmp0.l[med];
	else
	  *trgt.l = (tmp0.l[med] + tmp0.l[med + 1])/2;
	break;
      case ANA_FLOAT:
	if (nelem % 2)
	  *trgt.f = tmp0.f[med];
	else
	  *trgt.f = (tmp0.f[med] + tmp0.f[med + 1])/2;
	break;
      case ANA_DOUBLE:
	if (nelem % 2)
	  *trgt.d = tmp0.d[med];
	else
	  *trgt.d = (tmp0.d[med] + tmp0.d[med + 1])/2;
	break;
    }
  } while (advanceLoop(&trgtinfo, &trgt) < trgtinfo.rndim);

  free(tmp0.b);
  return output;
}
/*---------------------------------------------------------*/
Int ana_median(Int narg, Int ps[])
     /* median filter */
{
  internalMode = 1;
  return ana_quantile(narg, ps);
}
/*---------------------------------------------------------*/
Int ana_minfilter(Int narg, Int ps[])
     /* minimum filter */
/* Applies a running minimum filter to a data set */
/* syntax:  Y = MINFILTER(X [[,AXIS], WIDTH]) */
/* LS 31dec95 30jul97 */
{
  Byte	type;
  Int	n, result, i, offset1, offset2, stride,	w1, j, ww, loop, iq, three=3,
    nWidth;
  pointer	src, trgt, width;
  scalar	value;
  loopInfo	srcinfo, trgtinfo;

  if (standardLoop(ps[0], narg > 2? ps[1]: 0,
		   SL_SAMEDIMS | SL_UPGRADE | SL_EACHROW, ANA_BYTE,
		   &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return ANA_ERROR;
  if (narg > 1) {		/* <width> */
    /* check that it is numerical and ensure that it is ANA_LONG */
    if (getNumerical(ps[narg - 1], ANA_LONG, &nWidth, &width,
		     GN_UPDATE | GN_UPGRADE, NULL, NULL) < 0)
      return ANA_ERROR;
    if (nWidth > 1 && nWidth != srcinfo.naxes)
      return anaerror("Number of widths must be 1 or equal to number of axes",
		   ps[2]);
    for (i = 0; i < nWidth; i++)
      if (width.l[i] < 1)
	return anaerror("Width(s) must be positive", ps[2]);
  } else {
    width.l = &three;
    nWidth = 1;
  }
  iq = ps[0];			/* data */
  type = symbol_type(iq);
  if (!srcinfo.naxes)
    srcinfo.naxes++;
  for (loop = 0; loop < srcinfo.naxes; loop++) {
    ww = (*width.l > srcinfo.rdims[0])? srcinfo.rdims[0]: *width.l;
    if (nWidth > 1)
      width.l++;
    stride = srcinfo.step[0];
    offset2 = -stride*ww;
    offset1 = offset2 + stride;
    w1 = (ww + 1)/2;
    switch (type) {
      default:
	return cerror(ILL_TYPE, ps[0]);
      case ANA_BYTE:
	do {
	  value.b = bounds.max.b;	/* initialize */
	  for (i = 0; i < ww; i++) { /* do the left edge */
	    if (*src.b < value.b)
	      value.b = *src.b;
	    src.b += stride;
	  }
	  for (i = 0; i < w1; i++) {
	    *trgt.b = value.b;
	    trgt.b += stride;
	  }
	  for (i = ww; i < srcinfo.rdims[0]; i++) { /* middle part */
	    if (*src.b < value.b) {
	      value.b = *src.b;
	      src.b += stride;
	    } else if (src.b[offset2] == value.b) {
	      src.b += offset1;
	      value.b = *src.b;
	      src.b += stride;
	      for (j = 1; j < ww; j++) {
		if (*src.b < value.b)
		value.b = *src.b;
		src.b += stride;
	      }
	    } else
	      src.b += stride;
	    *trgt.b = value.b;
	    trgt.b += stride;
	  }
	  for (i = w1; i < ww; i++) { /* right edge */
	    *trgt.b = value.b;
	    trgt.b += stride;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case ANA_WORD:
	do {
	  value.w = bounds.max.w;	/* initialize */
	  for (i = 0; i < ww; i++) { /* do the left edge */
	    if (*src.w < value.w)
	      value.w = *src.w;
	    src.w += stride;
	  }
	  for (i = 0; i < w1; i++) {
	    *trgt.w = value.w;
	    trgt.w += stride;
	  }
	  for (i = ww; i < srcinfo.rdims[0]; i++) { /* middle part */
	    if (*src.w < value.w) {
	      value.w = *src.w;
	      src.w += stride;
	    } else if (src.w[offset2] == value.w) {
	      src.w += offset1;
	      value.w = *src.w;
	      src.w += stride;
	      for (j = 1; j < ww; j++) {
		if (*src.w < value.w)
		value.w = *src.w;
		src.w += stride;
	      }
	    } else
	      src.w += stride;
	    *trgt.w = value.w;
	    trgt.w += stride;
	  }
	  for (i = w1; i < ww; i++) { /* right edge */
	    *trgt.w = value.w;
	    trgt.w += stride;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case ANA_LONG:
	do {
	  value.l = bounds.max.l;	/* initialize */
	  for (i = 0; i < ww; i++) { /* do the left edge */
	    if (*src.l < value.l)
	      value.l = *src.l;
	    src.l += stride;
	  }
	  for (i = 0; i < w1; i++) {
	    *trgt.l = value.l;
	    trgt.l += stride;
	  }
	  for (i = ww; i < srcinfo.rdims[0]; i++) { /* middle part */
	    if (*src.l < value.l) {
	      value.l = *src.l;
	      src.l += stride;
	    } else if (src.l[offset2] == value.l) {
	      src.l += offset1;
	      value.l = *src.l;
	      src.l += stride;
	      for (j = 1; j < ww; j++) {
		if (*src.l < value.l)
		value.l = *src.l;
		src.l += stride;
	      }
	    } else
	      src.l += stride;
	    *trgt.l = value.l;
	    trgt.l += stride;
	  }
	  for (i = w1; i < ww; i++) { /* right edge */
	    *trgt.l = value.l;
	    trgt.l += stride;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case ANA_FLOAT:
	do {
	  value.f = bounds.max.f;	/* initialize */
	  for (i = 0; i < ww; i++) { /* do the left edge */
	    if (*src.f < value.f)
	      value.f = *src.f;
	    src.f += stride;
	  }
	  for (i = 0; i < w1; i++) {
	    *trgt.f = value.f;
	    trgt.f += stride;
	  }
	  for (i = ww; i < srcinfo.rdims[0]; i++) { /* middle part */
	    if (*src.f < value.f) {
	      value.f = *src.f;
	      src.f += stride;
	    } else if (src.f[offset2] == value.f) {
	      src.f += offset1;
	      value.f = *src.f;
	      src.f += stride;
	      for (j = 1; j < ww; j++) {
		if (*src.f < value.f)
		value.f = *src.f;
		src.f += stride;
	      }
	    } else
	      src.f += stride;
	    *trgt.f = value.f;
	    trgt.f += stride;
	  }
	  for (i = w1; i < ww; i++) { /* right edge */
	    *trgt.f = value.f;
	    trgt.f += stride;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case ANA_DOUBLE:
	do {
	  value.d = bounds.max.d;	/* initialize */
	  for (i = 0; i < ww; i++) { /* do the left edge */
	    if (*src.d < value.d)
	      value.d = *src.d;
	    src.d += stride;
	  }
	  for (i = 0; i < w1; i++) {
	    *trgt.d = value.d;
	    trgt.d += stride;
	  }
	  for (i = ww; i < srcinfo.rdims[0]; i++) { /* middle part */
	    if (*src.d < value.d) {
	      value.d = *src.d;
	      src.d += stride;
	    } else if (src.d[offset2] == value.d) {
	      src.d += offset1;
	      value.d = *src.d;
	      src.d += stride;
	      for (j = 1; j < ww; j++) {
		if (*src.d < value.d)
		value.d = *src.d;
		src.d += stride;
	      }
	    } else
	      src.d += stride;
	    *trgt.d = value.d;
	    trgt.d += stride;
	  }
	  for (i = w1; i < ww; i++) { /* right edge */
	    *trgt.d = value.d;
	    trgt.d += stride;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
    }
    if (loop < srcinfo.naxes - 1) {
      nextLoops(&srcinfo, &trgtinfo);
      if (isFreeTemp(iq)) {	/* can use iq to hold next result */
	n = result;
	result = iq;
	iq = n;
      } else {			/* need new temp for next result */
	iq = result;
	result = array_clone(iq, type);
      }
      src.b = array_data(iq);
      trgt.b = array_data(result);
    }
  }
  return result;
}
/*---------------------------------------------------------*/
Int ana_maxfilter(Int narg, Int ps[])
/* maximum filter */
/* Applies a running maximum filter to a data set */
/* syntax:  Y = MAXFILTER(X [[,AXIS], WIDTH]) */
/* LS 31dec95 30jul97 */
{
  Byte	type;
  Int	n, result, i, offset1, offset2, stride,	w1, j, ww, loop, iq, three=3,
    nWidth;
  pointer	src, trgt, width;
  scalar	value;
  loopInfo	srcinfo, trgtinfo;

  if (standardLoop(ps[0], narg > 2? ps[1]: 0,
		   SL_SAMEDIMS | SL_UPGRADE | SL_EACHROW, ANA_BYTE,
		   &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return ANA_ERROR;
  if (narg > 1) {		/* <width> */
    /* check that it is numerical and ensure that it is ANA_LONG */
    if (getNumerical(ps[narg - 1], ANA_LONG, &nWidth, &width,
		     GN_UPGRADE | GN_UPDATE, NULL, NULL) < 0)
      return ANA_ERROR;
    if (nWidth > 1 && nWidth != srcinfo.naxes)
      return anaerror("Number of widths must be 1 or equal to number of axes",
		   ps[2]);
    for (i = 0; i < nWidth; i++)
      if (width.l[i] < 1)
	return anaerror("Width(s) must be positive", ps[2]);
  } else {
    width.l = &three;
    nWidth = 1;
  }
  iq = ps[0];			/* data */
  type = symbol_type(iq);
  if (!srcinfo.naxes)
    srcinfo.naxes++;
  for (loop = 0; loop < srcinfo.naxes; loop++) {
    ww = (*width.l > srcinfo.rdims[0])? srcinfo.rdims[0]: *width.l;
    if (nWidth > 1)
      width.l++;
    stride = srcinfo.step[0];
    offset2 = -stride*ww;
    offset1 = offset2 + stride;
    w1 = (ww + 1)/2;
    switch (type) {
      default:
	return cerror(ILL_TYPE, ps[0]);
      case ANA_BYTE:
	do {
	  value.b = bounds.min.b;	/* initialize */
	  for (i = 0; i < ww; i++) { /* do the left edge */
	    if (*src.b > value.b)
	      value.b = *src.b;
	    src.b += stride;
	  }
	  for (i = 0; i < w1; i++) {
	    *trgt.b = value.b;
	    trgt.b += stride;
	  }
	  for (i = ww; i < srcinfo.rdims[0]; i++) { /* middle part */
	    if (*src.b > value.b) {
	      value.b = *src.b;
	      src.b += stride;
	    } else if (src.b[offset2] == value.b) {
	      src.b += offset1;
	      value.b = *src.b;
	      src.b += stride;
	      for (j = 1; j < ww; j++) {
		if (*src.b > value.b)
		value.b = *src.b;
		src.b += stride;
	      }
	    } else
	      src.b += stride;
	    *trgt.b = value.b;
	    trgt.b += stride;
	  }
	  for (i = w1; i < ww; i++) { /* right edge */
	    *trgt.b = value.b;
	    trgt.b += stride;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case ANA_WORD:
	do {
	  value.w = bounds.min.w;	/* initialize */
	  for (i = 0; i < ww; i++) { /* do the left edge */
	    if (*src.w > value.w)
	      value.w = *src.w;
	    src.w += stride;
	  }
	  for (i = 0; i < w1; i++) {
	    *trgt.w = value.w;
	    trgt.w += stride;
	  }
	  for (i = ww; i < srcinfo.rdims[0]; i++) { /* middle part */
	    if (*src.w > value.w) {
	      value.w = *src.w;
	      src.w += stride;
	    } else if (src.w[offset2] == value.w) {
	      src.w += offset1;
	      value.w = *src.w;
	      src.w += stride;
	      for (j = 1; j < ww; j++) {
		if (*src.w > value.w)
		value.w = *src.w;
		src.w += stride;
	      }
	    } else
	      src.w += stride;
	    *trgt.w = value.w;
	    trgt.w += stride;
	  }
	  for (i = w1; i < ww; i++) { /* right edge */
	    *trgt.w = value.w;
	    trgt.w += stride;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case ANA_LONG:
	do {
	  value.l = bounds.min.l;	/* initialize */
	  for (i = 0; i < ww; i++) { /* do the left edge */
	    if (*src.l > value.l)
	      value.l = *src.l;
	    src.l += stride;
	  }
	  for (i = 0; i < w1; i++) {
	    *trgt.l = value.l;
	    trgt.l += stride;
	  }
	  for (i = ww; i < srcinfo.rdims[0]; i++) { /* middle part */
	    if (*src.l > value.l) {
	      value.l = *src.l;
	      src.l += stride;
	    } else if (src.l[offset2] == value.l) {
	      src.l += offset1;
	      value.l = *src.l;
	      src.l += stride;
	      for (j = 1; j < ww; j++) {
		if (*src.l > value.l)
		value.l = *src.l;
		src.l += stride;
	      }
	    } else
	      src.l += stride;
	    *trgt.l = value.l;
	    trgt.l += stride;
	  }
	  for (i = w1; i < ww; i++) { /* right edge */
	    *trgt.l = value.l;
	    trgt.l += stride;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case ANA_FLOAT:
	do {
	  value.f = bounds.min.f;	/* initialize */
	  for (i = 0; i < ww; i++) { /* do the left edge */
	    if (*src.f > value.f)
	      value.f = *src.f;
	    src.f += stride;
	  }
	  for (i = 0; i < w1; i++) {
	    *trgt.f = value.f;
	    trgt.f += stride;
	  }
	  for (i = ww; i < srcinfo.rdims[0]; i++) { /* middle part */
	    if (*src.f > value.f) {
	      value.f = *src.f;
	      src.f += stride;
	    } else if (src.f[offset2] == value.f) {
	      src.f += offset1;
	      value.f = *src.f;
	      src.f += stride;
	      for (j = 1; j < ww; j++) {
		if (*src.f > value.f)
		value.f = *src.f;
		src.f += stride;
	      }
	    } else
	      src.f += stride;
	    *trgt.f = value.f;
	    trgt.f += stride;
	  }
	  for (i = w1; i < ww; i++) { /* right edge */
	    *trgt.f = value.f;
	    trgt.f += stride;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case ANA_DOUBLE:
	do {
	  value.d = bounds.min.d;	/* initialize */
	  for (i = 0; i < ww; i++) { /* do the left edge */
	    if (*src.d > value.d)
	      value.d = *src.d;
	    src.d += stride;
	  }
	  for (i = 0; i < w1; i++) {
	    *trgt.d = value.d;
	    trgt.d += stride;
	  }
	  for (i = ww; i < srcinfo.rdims[0]; i++) { /* middle part */
	    if (*src.d > value.d) {
	      value.d = *src.d;
	      src.d += stride;
	    } else if (src.d[offset2] == value.d) {
	      src.d += offset1;
	      value.d = *src.d;
	      src.d += stride;
	      for (j = 1; j < ww; j++) {
		if (*src.d > value.d)
		value.d = *src.d;
		src.d += stride;
	      }
	    } else
	      src.d += stride;
	    *trgt.d = value.d;
	    trgt.d += stride;
	  }
	  for (i = w1; i < ww; i++) { /* right edge */
	    *trgt.d = value.d;
	    trgt.d += stride;
	  }
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
    }
    if (loop < srcinfo.naxes - 1) {
      nextLoops(&srcinfo, &trgtinfo);
      if (isFreeTemp(iq)) {	/* can use iq to hold next result */
	n = result;
	result = iq;
	iq = n;
      } else {			/* need new temp for next result */
	iq = result;
	result = array_clone(iq, type);
      }
      src.b = array_data(iq);
      trgt.b = array_data(result);
    }
  }
  return result;
}
/*---------------------------------------------------------*/
Int ana_distarr(Int narg, Int ps[])
/* DISTARR(dims[,center,stretch]) returns a FLOAT array of dimensions <dims> */
/* wherein each element contains the distance of the coordinates of */
/* that element to the point given in <center>, after stretching each */
/* coordinate by the appropriate factor from <stretch>.  <dims>, <center>, */
/* and <stretch> must have the same number of elements.  LS 9jan97 22may97 */
{
  Int	iq, *dims, ndim, i, result, done;
  Float	*center, temp, temptot, zerocenter[MAX_DIMS], *stretch,
	onestretch[MAX_DIMS];
  pointer	trgt;
  loopInfo	trgtinfo;

  iq = ana_long(1, &ps[0]);	/* DIMS */
  switch (symbol_class(iq))
  { case ANA_ARRAY:
      dims = (Int *) array_data(iq);
      ndim = array_size(iq);
      break;
    case ANA_SCALAR:
      dims = &scalar_value(iq).l;
      ndim = 1;
      break;
    default:
      return cerror(ILL_CLASS, ps[0]); }
  if (narg > 1)
  { iq = ana_float(1, &ps[1]);	/* CENTER */
    switch (symbol_class(iq))
    { case ANA_ARRAY:
	if (array_size(iq) != ndim)
	  return cerror(INCMP_ARG, ps[1]);
	center = (Float *) array_data(iq);
	break;
      case ANA_SCALAR:
	if (ndim != 1)
	  return cerror(INCMP_ARG, ps[1]);
	center = &scalar_value(iq).f;
	break;
      default:
	return cerror(ILL_CLASS, ps[1]); }
  } else
  { zerobytes(zerocenter, ndim*sizeof(Float));
    center = zerocenter; }
  if (narg > 2)
  { iq = ana_float(1, &ps[2]);	/* STRETCH */
    switch (symbol_class(iq))
    { case ANA_ARRAY:
	if (array_size(iq) != ndim)
	  return cerror(INCMP_ARG, ps[2]);
	stretch = (Float *) array_data(iq);
	break;
      case ANA_SCALAR:
	if (ndim != 1)
	  return cerror(INCMP_ARG, ps[2]);
	stretch = &scalar_value(iq).f;
	break;
      default:
	return cerror(ILL_CLASS, ps[2]); }
  }
  else
  { for (i = 0; i < ndim; i++)
      onestretch[i] = 1.0;
    stretch = onestretch; }
  
				/* check output dimensions */
  for (i = 0; i < ndim; i++)
    if (dims[i] <= 0)
      return cerror(ILL_DIM, ps[0]);
  result = array_scratch(ANA_FLOAT, ndim, dims);
  if (result == ANA_ERROR)	/* some error */
    return ANA_ERROR;
  trgt.f = (Float *) array_data(result);
  setupDimensionLoop(&trgtinfo, ndim, dims, ANA_FLOAT, ndim, NULL, &trgt,
		     SL_EACHCOORD);
  /* initialize walk through array */
  temptot = 0.0;
  for (i = 1; i < ndim; i++)
  { temp = center[i]*stretch[i];
    temptot += temp*temp; }
  do
  { temp = (trgtinfo.coords[0] - center[0])*stretch[0];
    *trgt.f = temp*temp + temptot;
    *trgt.f = sqrt((Double) *trgt.f); /* distance */
    done = advanceLoop(&trgtinfo, &trgt);
    if (done && done < ndim)
    { temptot = 0.0;
      for (i = 1; i < ndim; i++)
      { temp = (trgtinfo.coords[i] - center[i])*stretch[i];
	temptot += temp*temp; }
    }
  } while (done < ndim);
  return result;
}
/*---------------------------------------------------------*/
Int ana_multisieve(Int narg, Int ps[])
/*  MULTISIEVE, x, y, list, index
    returns <list> and <index> such that
    <x(list(index(j-1):index(j)-1))> EQ <y(j)>
    for all j
    LS 14apr97 - 15apr97 */
{
  Byte	maxType;
  pointer	xData, yData;
  Int	nx, ny, *listData, *indexData,
    n, *temp, nTemp, iq, nnTemp, noMatch, ix, i, match, *base, step,
    *reallocbase, nMatch, *number, j;

  if (getSimpleNumerical(ps[0], &xData, &nx) < 0)
    return -1;			/* some error occurred */
  if (getSimpleNumerical(ps[1], &yData, &ny) < 0)
    return -1;

  /* make arguments have same data type */
  maxType = symbol_type(ps[0]);
  if (symbol_type(ps[1]) > maxType) { /* need to upgrade ps[0] */
    maxType = symbol_type(ps[1]);
    iq = ana_convert(1, ps, maxType, 1);
    getSimpleNumerical(iq, &yData, &ny);
  } else if (symbol_type(ps[1]) < maxType) { /* upgrade ps[1] */
    maxType = symbol_type(ps[0]);
    iq = ana_convert(1, ps + 1, maxType, 1);
    getSimpleNumerical(iq, &xData, &nx);
  }
  
  nTemp = 1000;			/* start with 1000 elements */
  nnTemp = nTemp - 2;
  allocate(temp, nTemp, Int);
  base = temp;
  step = ana_type_size[maxType];

  n = ny + 1;
  if (redef_array(ps[3], ANA_LONG, 1, &n) < 0) /* <indices> */
    return -1;
  indexData = array_data(ps[3]);
  indexData[0] = 0;
  indexData++;
  zerobytes((char *) indexData, ny*sizeof(Int));

  /* get results */
  /* we maintain a list of matches as follows: the list consists of a */
  /* set of ranges of matches.  Each range is specified by first the */
  /* start index in <x>, then the number of matches that follow in the */
  /* current range, and then the matches for that range.  if fewer than */
  /* three no-matches follow the last match in the current range, then */
  /* the no-matches and the current match are entered in the current */
  /* range, otherwise a new range is started.  This scheme ensures that */
  /* the list gets at most two elements longer than <x> itself. */
  /* Example: if <x> = 1 2 3 7 1 6 7 5 7 5 and <y> = 1 5 7 then */
  /* the matches are as follows: */
  /*   x     1 2 3 3 7 1 6 7 5 7 5 */
  /*   match 0 - - - 2 0 - 2 1 2 1 */
  /* and the temporary match list comes out as follows: */
  /*   0 1 0 4 7 2 0 -1 2 1 2 1 */
  noMatch = 999;
  ix = nMatch = 0;
  *temp = 0;			/* no matches so far */
  while (1)
  { while (nnTemp > 3 && ix < nx)
    { match = -1;		/* default: no match */
      switch (maxType)
      { case ANA_BYTE:
	  for (i = 0; i < ny; i++)
	    if (*xData.b == yData.b[i])
	    { match = i;
	      indexData[match]++; /* count */
	      break; }
	  break;
	case ANA_WORD:
	  for (i = 0; i < ny; i++)
	    if (*xData.w == yData.w[i])
	    { match = i;
	      indexData[match]++; /* count */
	      break; }
	  break;
	case ANA_LONG:
	  for (i = 0; i < ny; i++)
	    if (*xData.l == yData.l[i])
	    { match = i;
	      indexData[match]++; /* count */
	      break; }
	  break;
	case ANA_FLOAT:
	  for (i = 0; i < ny; i++)
	    if (*xData.f == yData.f[i])
	    { match = i;
	      indexData[match]++; /* count */
	      break; }
	  break;
	case ANA_DOUBLE:
	  for (i = 0; i < ny; i++)
	    if (*xData.d == yData.d[i])
	    { match = i;
	      indexData[match]++; /* count */
	      break; }
	  break;
	}
      if (match >= 0) {	/* have a match */
	nMatch++;
	switch (noMatch)
	{ case 2:		/* two no-matches before */
	    temp[++(*temp)] = -1;
	    nnTemp--;
	    /* fall-thru */
	  case 1:		/* one no-match before */
	    temp[++(*temp)] = -1;
	    nnTemp--;
	    noMatch = 0;
	    /* fall-thru */
	  case 0:		/* previous one was a match, too */
	    temp[++(*temp)] = match;
	    nnTemp--;
	    break;
	  default:		/* three or more no-matches before: */
				/* start a new range */
	    temp += *temp + 1;
	    *temp++ = ix;
	    *temp = 1;
	    temp[*temp] = match;
	    nnTemp -= 3;
	    noMatch = 0;
	    break; }
      }
      else ++noMatch;
      xData.b += step;
      ix++; }
    if (ix != nx)		/* more to do */
    { nTemp += 1000;
      reallocbase = Realloc(base, nTemp*sizeof(Int));
      if (reallocbase == NULL)
      { free(base);
	return cerror(ALLOC_ERR, 0); }
      temp = (temp - base) + reallocbase;
      base = reallocbase;
      nnTemp += 1000; }
    else break; }

  if (!nMatch)			/* no matches at all: return -1 */
  { undefine(ps[2]);
    symbol_class(ps[2]) = ANA_SCALAR;
    scalar_type(ps[2]) = ANA_LONG;
    scalar_value(ps[2]).l = -1; }
  else
  { if (redef_array(ps[2], ANA_LONG, 1, &nMatch) < 0) /* <list> */
      return -1;		/* some error */
    listData = array_data(ps[2]);
  
    temp = indexData;
    n = 0;
    for (i = 0; i < ny; i++) {
      n += indexData[i];
      indexData[i] = n;
    }
    ix = indexData[ny - 1];
    
    number = Malloc(ny*sizeof(Int));
    if (!number) {
      free(base);
      return cerror(ALLOC_ERR, 0);
    }
    zerobytes((char *) number, ny*sizeof(Int));
    
    temp = base + 1;
    while (ix) {
      step = *temp++;		/* offset */
      n = *temp++;		/* number */
      for (j = 0; j < n; j++) {
	if (*temp < 0)		/* no match */
	{ temp++;
	  continue; }
	ix--;
	i = *temp? indexData[*temp - 1]: 0;
	listData[number[*temp] + i] = step + j;
	number[*temp++]++;
      }
    }

    free(number);
  }
  free(base);
  return 1;
}
/*---------------------------------------------------------*/
Int ana_temp(Int narg, Int ps[])
/* experimental routine: returns temp copy of symbol if it is */
/* a named symbol */
{
  Int	result;

  if (*ps >= NAMED_START && *ps < NAMED_END)
  { result = nextFreeTempVariable();
    return copyToSym(result, *ps); }
  else return *ps;
}
/*---------------------------------------------------------*/
Int shift(Int narg, Int ps[], Int isFunction)
/* SHIFT(x[,axes],dist[,BLANK=blank][,/TRANSLATE]) shifts the data in
 <x> over distances <dist> (a scalar or an array containing one
 integer number per dimension) along the indicated <axes> (a scalar or
 an array).  If /TRANSLATE and/or <blank> are specified, then the
 shifting is non-circular, and then elements in the result for which
 no corresponding data was present in <x> is filled with the value of
 <blank>, or 0 by default.  If /TRANSLATE and <blank> are not
 specified, then the shifting is circular, so that elements that shift
 out of the field of view at one edge reappear at the opposite edge,
 and no elements are lost.
  LS 7may97 6sep2000 */
{
  Int	axesSym, offset, i, step, j, distSym, iq, *ptr;
  pointer	src, trgt, shift, src0, trgt0, blank;
  char	*tmp, *tmp0 = NULL;
  loopInfo	srcinfo, trgtinfo;
  Int	ana_indgen(Int, Int []);
  Double	zero = 0.0;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_ARR, ps[0]);

  if (narg == 1 || (narg == 4 && !ps[1] && !ps[2])) {
    /* SHIFT(x) */
    /* create <dist> = DIMEN(<x>)/2 */
    iq = array_num_dims(ps[0]);
    distSym = array_scratch(ANA_LONG, 1, &iq);
    ptr = array_data(distSym);
    memcpy(ptr, array_dims(ps[0]), iq*sizeof(Int));
    for (i = 0; i < iq; i++) {
      *ptr = *ptr/2;
      ptr++;
    }
    axesSym = ana_indgen(1, &distSym); /* create <axes> = INDGEN(<dist>) */
  } else if (narg == 2 || (narg == 4 && !ps[2])) {
    /* SHIFT(x,dist) */
    axesSym = ana_indgen(1, ps + 1); /* <axes> = INDGEN(<dist>) */
    distSym = ps[1];		/* <dist> */
  } else if (narg > 2 && ps[2]) {
    /* SHIFT(x,axes,dist) */
    axesSym = ps[1];
    distSym = ps[2];
  }
  if (narg > 3 && ps[3]) {	/* <blank> */
    if (!symbolIsScalar(ps[3]))
      return cerror(NEED_SCAL, ps[3]);
    iq = ana_converts[array_type(ps[0])](1, ps + 3);
    blank.l = &scalar_value(iq).l;
    internalMode |= 1;		/* /TRANSLATE */
  } else
    blank.d = &zero;

  if (isFunction) {
    if (standardLoop(ps[0], axesSym,
		     SL_UPGRADE | SL_UNIQUEAXES | SL_EACHROW | SL_AXISCOORD,
		     ANA_BYTE, &srcinfo, &src, &iq, &trgtinfo, &trgt) < 0)
      return ANA_ERROR;
  } else {
    if (standardLoop(ps[0], axesSym,
		     SL_UPGRADE | SL_UNIQUEAXES | SL_EACHROW | SL_AXISCOORD,
		     ANA_BYTE, &srcinfo, &src, NULL, NULL, NULL) < 0)
      return ANA_ERROR;
    trgtinfo = srcinfo;
    trgtinfo.data = &trgt;
    trgt = src;
  }
  src0 = src;
  trgt0 = trgt;
  if (numerical(distSym, NULL, NULL, &i, NULL) < 0) /* distance */
    return ANA_ERROR;
  if (i != srcinfo.naxes)	/* must have one distance for each axis */
    return cerror(INCMP_ARG, distSym);
  i = ana_long(1, &distSym);	/* ensure LONG */
  numerical(i, NULL, NULL, NULL, &shift);
  if (internalMode & 1) {	/* non-circular shift */
    for (i = 0; i < srcinfo.naxes; i++)
      if (shift.l[i] >= srcinfo.rdims[0] || shift.l[i] <= -srcinfo.rdims[0]) {
	/* the requested shift distance is so great that no part of the */
	/* old image remains in the field of view */
	zerobytes(trgt.v, srcinfo.nelem*ana_type_size[srcinfo.type]);
	return isFunction? iq: ANA_OK;
      }
    do {
      i = *shift.l++;
      offset = i*srcinfo.stride;
      step = srcinfo.step[0]*srcinfo.stride;
      src = src0;
      trgt = trgt0;
      tmp0 = realloc(tmp0, srcinfo.rdims[0]*srcinfo.stride);
      if (!tmp0)
	return cerror(ALLOC_ERR, 0);
      if (i > 0)
	for (j = 0; j < i; j++)
	  memcpy(tmp0 + j*srcinfo.stride, blank.b, srcinfo.stride);
      else
	for (j = 0; j < -i; j++)
	  memcpy(tmp0 + (srcinfo.rdims[0] + i + j)*srcinfo.stride,
		 blank.b, srcinfo.stride);
      do {
	/* we assemble the shifted elements in some temporary storage */
	if (offset > 0) {	/* we're shifting forward */
	  tmp = tmp0 + offset;
	  for (j = 0; j < srcinfo.rdims[0] - i; j++) {
	    memcpy(tmp, src.b, srcinfo.stride);
	    tmp += srcinfo.stride;
	    src.b += step;
	  }
	  src.b += i*step;
	} else {		/* we're shifting backward */
	  tmp = tmp0;
	  src.b -= step*i;
	  for (j = -i; j < srcinfo.rdims[0]; j++) {
	    memcpy(tmp, src.b, srcinfo.stride);
	    tmp += srcinfo.stride;
	    src.b += step;
	  }
	}
	/* and we move the finished elements to their final location */
	tmp = tmp0;
	for (j = 0; j < srcinfo.rdims[0]; j++) {
	  memcpy(trgt.b, tmp, srcinfo.stride);
	  tmp += srcinfo.stride;
	  trgt.b += step;
	}
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      src0.b = trgt0.b;		/* take result of previous loop as source */
      /* of next one */
    } while (nextLoops(&srcinfo, &trgtinfo));
  } else			/* circular shift */
    do {
      i = *shift.l++;		/* shift distance in current dimension */
      /* reduce shift distance to interval (0:srcinfo.rdims[0]-1) */
      if (i < 0)
	i += srcinfo.rdims[0]*(1 - i/srcinfo.rdims[0]);
      if (i >= srcinfo.rdims[0])
	i -= srcinfo.rdims[0]*(i/srcinfo.rdims[0]);
      offset = i*srcinfo.stride;
      step = srcinfo.step[0]*srcinfo.stride;
      src = src0;
      trgt = trgt0;
      tmp0 = realloc(tmp0, srcinfo.rdims[0]*srcinfo.stride);
      if (!tmp0)
	return cerror(ALLOC_ERR, 0);
      do {
	/* we assemble the shifted elements in some temporary storage */
	/* first the elements that get shifted forward */
	tmp = tmp0 + offset;
	for (j = 0; j < srcinfo.rdims[0] - i; j++) {
	  memcpy(tmp, src.b, srcinfo.stride);
	  tmp += srcinfo.stride;
	  src.b += step;
	}
	/* then the elements that get shifted backward */
	tmp = tmp0;
	for (; j < srcinfo.rdims[0]; j++) {
	  memcpy(tmp, src.b, srcinfo.stride);
	  tmp += srcinfo.stride;
	  src.b += step;
	}
	/* finally, we move the finished elements to their final location */
	tmp = tmp0;
	for (j = 0; j < srcinfo.rdims[0]; j++) {
	  memcpy(trgt.b, tmp, srcinfo.stride);
	  tmp += srcinfo.stride;
	  trgt.b += step;
	}
      } while (advanceLoop(&trgtinfo, &trgt),
	       advanceLoop(&srcinfo, &src) < srcinfo.rndim);
      src0.b = trgt0.b;		/* take result of previous loop as source */
      /* of next one */
    } while (nextLoops(&srcinfo, &trgtinfo));
  free(tmp0);
  return isFunction? iq: ANA_OK;
}
/*---------------------------------------------------------*/
Int ana_shift_f(Int narg, Int ps[])
{
  return shift(narg, ps, 1);
}
/*---------------------------------------------------------*/
Int ana_shift(Int narg, Int ps[])
{
  return shift(narg, ps, 0);
}
/*---------------------------------------------------------*/
Int ana_swaphalf(Int narg, Int ps[])
/* SWAPHALF,d  swaps elements of <d> such that new coordinate x_i' is */
/* related to old coordinate x_i according to */
/* x_i' = (x_i + n_i/2) mod n_i  where n_i is the size of <d> in */
/* dimension i.  Such swapping is useful in conjunction with */
/* Fourier and Hartley transforms which (at least in ANA) return */
/* data such that wavevector 0 ends up in result element 0.  After */
/* swapping with SWAPHALF, wavevector 0 is in the middle of the */
/* array (at coordinates n_i/2) and the length of any wavevector */
/* is given by its cartesian distance to the middle of the array. */
/* LS 21may97 */
{
  Int	i, dims[MAX_DIMS], ndim, tally[MAX_DIMS], step[MAX_DIMS],
	n, width, jump;
  char	*src;
  scalar	temp;

  if (symbol_class(*ps) != ANA_ARRAY)
    return cerror(NEED_ARR, *ps);
  src = (char *) array_data(*ps);
  ndim = array_num_dims(*ps);
  memcpy(dims, array_dims(*ps), ndim*sizeof(Int));
  for (i = 0; i < ndim; i++)
    if (dims[i] % 2 == 1)
      return anaerror("SWAPHALF deals only with even-dimension arrays", *ps);

  n = width = ana_type_size[array_type(*ps)];
  for (i = 0; i < ndim; i++) {
    tally[i] = 0;
    n *= dims[i];
    step[i] = n;
  }
  
  dims[ndim - 1] /= 2;

  jump = 0;
  for (i = 0; i < ndim; i++)
    jump += step[i];
  jump /= 2;

  do {
    memcpy(&temp.b, src, width);
    memcpy(src, src + jump, width);
    memcpy(src + jump, &temp.b, width);
    src += width;
    for (i = 0; i < ndim; i++)
      if (++(tally[i]) == dims[i]) {
	tally[i] = 0;
	jump += step[i];
      } else {
	if (tally[i] == dims[i]/2 && i != ndim - 1)
	  jump -= step[i];
	break;
      }
  } while (i < ndim);
  return 1;
}
/*---------------------------------------------------------*/
Int ana_equivalence(Int narg, Int ps[])
/* returns equivalence classes.  If <x1> and <x2> are equivalence relations */
/* such that for all <i> elements <x1(i)> and <x2(i)> are in the same class */
/* then <y> = EQUIVALENCE(<x1>,<x2>) returns in each <y(i)> the smallest */
/* element of <x1> and <x2> that is in the same class as elements <x(i)> */
/* and <y(i)>.  LS 30jul97 */
{
  Int	result, n, *x1, *x2, *class, *trgt, mx, mn, nClass, i, a1, a2;

  if (symbol_class(ps[0]) != ANA_ARRAY)
    return cerror(ILL_CLASS, ps[0]);
  if (symbol_class(ps[1]) != ANA_ARRAY)
    return cerror(ILL_CLASS, ps[1]);
  n = array_size(ps[0]);
  if (n != array_size(ps[1]))
    return cerror(INCMP_ARR, ps[2]);
  x1 = (Int *) array_data(ana_long(1, ps));
  x2 = (Int *) array_data(ana_long(1, ps + 1));
  result = array_clone(ps[0], ANA_LONG);
  trgt = (Int *) array_data(result);
  mx = INT32_MIN;
  mn = INT32_MAX;
  for (i = 0; i < n; i++)
  { if (x1[i] < mn)
      mn = x1[i];
    else if (x1[i] > mx)
      mx = x1[i];
    if (x2[i] < mn)
      mn = x2[i];
    else if (x2[i] > mx)
      mx = x2[i]; }
  nClass = mx - mn + 1;
  class = (Int *) Malloc(nClass*sizeof(Int));
  if (!class)
    return cerror(ALLOC_ERR, 0);
  for (i = 0; i < nClass; i++)
    class[i] = i;
  for (i = 0; i < n; i++)
  { a1 = x1[i] - mn;
    a2 = x2[i] - mn;
    while (class[a1] != a1)	/* find most remote "ancestor" */
      a1 = class[a1];
    while (class[a2] != a2)
      a2 = class[a2];
    if (a1 < a2)		/* reclassify */
      class[a2] = a1;
    else
      class[a1] = a2; }
  for (i = 0; i < nClass; i++)
  { a1 = class[i];
    while (class[a1] != a1)
      a1 = class[a1];
    class[i] = a1; }
  for (i = 0; i < n; i++)
    *trgt++ = class[x1[i] - mn] + mn;
  free(class);
  return result;
}
/*---------------------------------------------------------*/
Int local_extrema(Int narg, Int ps[], Int code)
/* local extreme search in multiple dimensions
   (x [, diagonal], /DEGREE, /SUBGRID])
   <x>: data
   <sign>: sign of objects to look for, positive integer -> hill-like,
    negative integer -> valley-like.  If zero, then +1 is assumed.
   /DEGREE: returns number of OK directions per data element
   code: 0 -> return minimum values, 1 -> return minimum locations,
         2 -> return maximum values, 3 -> return maximum locations
   LS 18may95 4aug97 */
{
  Int	result, sign, degree, n, i, *offset, k, j, ok, *edge, nok,
	*diagonal, nDiagonal, nDoDim, index, subgrid, ready, done;
  pointer	src, trgt, trgt0, srcl, srcr, trgt00;
  Float	*grad, *grad2, *hessian, value;
  loopInfo	srcinfo, trgtinfo;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_ARR, ps[0]);
  degree = (internalMode & 1);
  if ((degree
       && standardLoop(ps[0], 0,
		       SL_ALLAXES | SL_SAMEDIMS | SL_EXACT | SL_EACHCOORD,
		       ANA_LONG, &srcinfo, &src, &result, &trgtinfo, &trgt)
       < 0)
      || standardLoop(ps[0], 0, SL_ALLAXES | SL_EACHCOORD, ANA_BYTE,
		      &srcinfo, &src, NULL, NULL, NULL) < 0)
    return ANA_ERROR;

  subgrid = (internalMode & 2);
  sign = (code & 2);		/* 1 -> seek maxima */

  n = prepareDiagonals(narg > 1? ps[1]: 0, &srcinfo, 2, &offset, &edge, NULL,
		       &diagonal);
  if (n == ANA_ERROR)
    return ANA_ERROR;


  if (!degree) {
    /* allocate some memory; we guess how many hits we might find */
    /* NOTE: must check that we don't run out of memory! */
    if (diagonal) {
      nDoDim = 0;
      for (i = 0; i < srcinfo.ndim; i++)
	if (diagonal[i])
	  nDoDim++;
    } else
      nDoDim = srcinfo.ndim;
    
    i = srcinfo.nelem;
    for (j = 0; j < nDoDim; j++)
      i /= 2;
    i++;
    trgt0.l = trgt.l = trgt00.l = (Int *) malloc(i*sizeof(Int));
    if (!trgt.l)
      return cerror(ALLOC_ERR, 0);
    zerobytes(trgt.l, i*sizeof(Int));
  }

  /* we exclude the selected edges */
  for (i = 0; i < srcinfo.ndim; i++)
    if (edge[2*i + 1])
      edge[2*i + 1] = srcinfo.dims[i] - 2;
  if (degree) {
    /* we set the edges to zero */
    for (i = 0; i < trgtinfo.ndim; i++)
      if (edge[i]) {
	rearrangeEdgeLoop(&trgtinfo, NULL, i);
	do
	  *trgt.l = 0;
	while (advanceLoop(&trgtinfo, &trgt) < trgtinfo.ndim - 1);
      }
    /* and exclude the edges from further dealings */
    subdataLoop(edge, &trgtinfo);
  }
  subdataLoop(edge, &srcinfo);
  free(edge);
  
  /* now do the loop work */
  switch (array_type(ps[0])) {
    case ANA_BYTE:
      do {
	nok = 1 - degree;
	for (j = 0; j < n; j++) {	/* all directions */	  
	  k = offset[j];
	  srcl.b = src.b + k;
	  srcr.b = src.b - k;
	  ok = (sign && *src.b > *srcl.b && *src.b > *srcr.b)
	    || (!sign && *src.b < *srcl.b && *src.b < *srcr.b);
	  if (degree)
	    nok += ok;
	  else if (!ok) {
	    nok = 0;
	    break;
	  }	/* end if (!ok) */
	} /* end for (j = 0; j < n; j++) */
	if (degree)
	  *trgt.l = nok;
	else if (nok)
	  *trgt.l++ = src.b - (Byte *) srcinfo.data0;
	done = degree? (advanceLoop(&trgtinfo, &trgt),
			advanceLoop(&srcinfo, &src)):
	  advanceLoop(&srcinfo, &src);
      } while (done < srcinfo.ndim);
      break;
    case ANA_WORD:
      do {
	nok = 1 - degree;
	for (j = 0; j < n; j++) {	/* all directions */	  
	  k = offset[j];
	  srcl.w = src.w + k;
	  srcr.w = src.w - k;
	  ok = (sign && *src.w > *srcl.w && *src.w > *srcr.w)
	    || (!sign && *src.w < *srcl.w && *src.w < *srcr.w);
	  if (degree)
	    nok += ok;
	  else if (!ok) {
	    nok = 0;
	    break;
	  }	/* end if (!ok) */
	} /* end for (j = 0; j < n; j++) */
	if (degree)
	  *trgt.l = nok;
	else if (nok)
	  *trgt.l++ = src.w - (Word *) srcinfo.data0;
	done = degree? (advanceLoop(&trgtinfo, &trgt),
			advanceLoop(&srcinfo, &src)):
	  advanceLoop(&srcinfo, &src);
      } while (done < srcinfo.ndim);
      break;
    case ANA_LONG:
      do {
	nok = 1 - degree;
	for (j = 0; j < n; j++) {	/* all directions */	  
	  k = offset[j];
	  srcl.l = src.l + k;
	  srcr.l = src.l - k;
	  ok = (sign && *src.l > *srcl.l && *src.l > *srcr.l)
	    || (!sign && *src.l < *srcl.l && *src.l < *srcr.l);
	  if (degree)
	    nok += ok;
	  else if (!ok) {
	    nok = 0;
	    break;
	  }	/* end if (!ok) */
	} /* end for (j = 0; j < n; j++) */
	if (degree)
	  *trgt.l = nok;
	else if (nok)
	  *trgt.l++ = src.l - (Int *) srcinfo.data0;
	done = degree? (advanceLoop(&trgtinfo, &trgt),
			advanceLoop(&srcinfo, &src)):
	  advanceLoop(&srcinfo, &src);
      } while (done < srcinfo.ndim);
      break;
    case ANA_FLOAT:
      do {
	nok = 1 - degree;
	for (j = 0; j < n; j++) {	/* all directions */	  
	  k = offset[j];
	  srcl.f = src.f + k;
	  srcr.f = src.f - k;
	  ok = (sign && *src.f > *srcl.f && *src.f > *srcr.f)
	    || (!sign && *src.f < *srcl.f && *src.f < *srcr.f);
	  if (degree)
	    nok += ok;
	  else if (!ok) {
	    nok = 0;
	    break;
	  }	/* end if (!ok) */
	} /* end for (j = 0; j < n; j++) */
	if (degree)
	  *trgt.l = nok;
	else if (nok)
	  *trgt.l++ = src.f - (Float *) srcinfo.data0;
	done = degree? (advanceLoop(&trgtinfo, &trgt),
			advanceLoop(&srcinfo, &src)):
	  advanceLoop(&srcinfo, &src);
      } while (done < srcinfo.ndim);
      break;
    case ANA_DOUBLE:
      do {
	nok = 1 - degree;
	for (j = 0; j < n; j++) {	/* all directions */	  
	  k = offset[j];
	  srcl.d = src.d + k;
	  srcr.d = src.d - k;
	  ok = (sign && *src.d > *srcl.d && *src.d > *srcr.d)
	    || (!sign && *src.d < *srcl.d && *src.d < *srcr.d);
	  if (degree)
	    nok += ok;
	  else if (!ok) {
	    nok = 0;
	    break;
	  }	/* end if (!ok) */
	} /* end for (j = 0; j < n; j++) */
	if (degree)
	  *trgt.l = nok;
	else if (nok)
	  *trgt.l++ = src.d - (Double *) srcinfo.data0;
	done = degree? (advanceLoop(&trgtinfo, &trgt),
			advanceLoop(&srcinfo, &src)):
	  advanceLoop(&srcinfo, &src);
      } while (done < srcinfo.ndim);
      break;
  }

  nDiagonal = 0;
  if (!degree) {
    n = trgt.l - trgt0.l;	/* number of found extrema */
    if (!n)			/* none found */
      return ANA_MINUS_ONE;
    switch (code) {
      case 0: case 2:		/* find values */
	result = array_scratch(ANA_FLOAT, 1, &n);
	trgt.f = (Float *) array_data(result);
      case 1: case 3:		/* find positions */
	if (!(internalMode & 4) && !subgrid) {	/* not /COORDS, not /SUBGRID */
	  srcinfo.coords[0] = n;	/* number of found extrema */
	  result = array_scratch(ANA_LONG, 1, srcinfo.coords);
	  trgt.l = (Int *) array_data(result);
	} else {
	  srcinfo.coords[0] = srcinfo.ndim; /* # dimensions in the data */
	  srcinfo.coords[1] = n;	/* number of found extrema */
	  result = array_scratch(subgrid? ANA_FLOAT: ANA_LONG, 2,
				 srcinfo.coords);
	  trgt.l = (Int *) array_data(result);
	}
    	break;
    }
    if (result < 0)
      return ANA_ERROR;		/* some error */
    if (subgrid) {
      /* count the number of dimensions that allow diagonal links:
	 the quadratic fits involve only those dimensions */
      if (!diagonal)
	nDiagonal = srcinfo.ndim;
      else {
	for (i = 0; i < srcinfo.ndim; i++)
	  if (diagonal[i])
	    nDiagonal++;
      }	/* end of if (!diagonal) else */
      if (nDiagonal) {
	allocate(grad2, nDiagonal, Float);
	allocate(grad, nDiagonal, Float);
	allocate(hessian, nDiagonal*nDiagonal, Float);
	k = 0;
	for (i = 0; i < srcinfo.ndim; i++)
	  if (!diagonal || diagonal[i])
	    srcinfo.step[k] = srcinfo.step[i]; /* participating dimensions */
      }	/* end of if (nDiagonal) */

      src.l = (Int *) array_data(ps[0]); /* data */
      while (n--) {		/* all found extrema */
	index = k = *trgt0.l++;	/* index of extremum */
	for (i = 0; i < srcinfo.ndim; i++) { /* calculate coordinates */
	  srcinfo.coords[i] = k % srcinfo.dims[i];
	  k /= srcinfo.dims[i];
	} /* end of for (i = 0;...) */
	switch (symbol_type(ps[0])) {
	  case ANA_BYTE:
	    srcl.b = src.b + index;
	    value = (Float) *srcl.b;
	    for (i = 0; i < nDiagonal; i++)
	      grad[i] = grad2[i]
		= ((Float) srcl.b[srcinfo.step[i]]
		   - (Float) srcl.b[-srcinfo.step[i]])/2;
	    ready = 1;	/* zero gradient? */
	    for (i = 0; i < nDiagonal; i++)
	      if (grad[i] != 0) {
		ready = 0;
		break;
	      } /* end of if (grad[i] != 0) */
	    if (ready)	/* yes, zero gradient */
	      break;
	    for (i = 0; i < nDiagonal; i++)
	      for (j = 0; j <= i; j++)
		if (i == j)
		  hessian[i + i*nDiagonal] 
		    = (Float) srcl.b[srcinfo.step[i]]
		    + (Float) srcl.b[-srcinfo.step[i]] - 2*value;
		else
		  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
		    = ((Float) srcl.b[srcinfo.step[i] + srcinfo.step[j]]
		       + (Float) srcl.b[-srcinfo.step[i] - srcinfo.step[j]]
		       - (Float) srcl.b[srcinfo.step[i] - srcinfo.step[j]]
		       - (Float) srcl.b[srcinfo.step[j]
				       - srcinfo.step[i]])/4;
	    break;
	  case ANA_WORD:
	    srcl.w = src.w + index;
	    value = (Float) *srcl.w;
	    for (i = 0; i < nDiagonal; i++)
	      grad[i] = grad2[i]
		= ((Float) srcl.w[srcinfo.step[i]]
		   - (Float) srcl.w[-srcinfo.step[i]])/2;
	    ready = 1;	/* zero gradient? */
	    for (i = 0; i < nDiagonal; i++)
	      if (grad[i] != 0) {
		ready = 0;
		break;
	      } /* end of if (grad[i] != 0) */
	    if (ready)	/* yes, zero gradient */
	      break;
	    for (i = 0; i < nDiagonal; i++)
	      for (j = 0; j <= i; j++)
		if (i == j)
		  hessian[i + i*nDiagonal] 
		    = (Float) srcl.w[srcinfo.step[i]]
		    + (Float) srcl.w[-srcinfo.step[i]] - 2*value;
		else
		  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
		    = ((Float) srcl.w[srcinfo.step[i] + srcinfo.step[j]]
		       + (Float) srcl.w[-srcinfo.step[i] - srcinfo.step[j]]
		       - (Float) srcl.w[srcinfo.step[i] - srcinfo.step[j]]
		       - (Float) srcl.w[srcinfo.step[j]
				       - srcinfo.step[i]])/4;
	    break;
	  case ANA_LONG:
	    srcl.l = src.l + index;
	    value = (Float) *srcl.l;
	    for (i = 0; i < nDiagonal; i++)
	      grad[i] = grad2[i]
		= ((Float) srcl.l[srcinfo.step[i]]
		   - (Float) srcl.l[-srcinfo.step[i]])/2;
	    ready = 1;	/* zero gradient? */
	    for (i = 0; i < nDiagonal; i++)
	      if (grad[i] != 0) {
		ready = 0;
		break;
	      } /* end of if (grad[i] != 0) */
	    if (ready)	/* yes, zero gradient */
	      break;
	    for (i = 0; i < nDiagonal; i++)
	      for (j = 0; j <= i; j++)
		if (i == j)
		  hessian[i + i*nDiagonal] 
		    = (Float) srcl.l[srcinfo.step[i]]
		    + (Float) srcl.l[-srcinfo.step[i]] - 2*value;
		else
		  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
		    = ((Float) srcl.l[srcinfo.step[i] + srcinfo.step[j]]
		       + (Float) srcl.l[-srcinfo.step[i] - srcinfo.step[j]]
		       - (Float) srcl.l[srcinfo.step[i] - srcinfo.step[j]]
		       - (Float) srcl.l[srcinfo.step[j]
				       - srcinfo.step[i]])/4;
	    break;
	  case ANA_FLOAT:
	    srcl.f = src.f + index;
	    value = (Float) *srcl.f;
	    for (i = 0; i < nDiagonal; i++)
	      grad[i] = grad2[i]
		= ((Float) srcl.f[srcinfo.step[i]]
		   - (Float) srcl.f[-srcinfo.step[i]])/2;
	    ready = 1;	/* zero gradient? */
	    for (i = 0; i < nDiagonal; i++)
	      if (grad[i] != 0) {
		ready = 0;
		break;
	      } /* end of if (grad[i] != 0) */
	    if (ready)	/* yes, zero gradient */
	      break;
	    for (i = 0; i < nDiagonal; i++)
	      for (j = 0; j <= i; j++)
		if (i == j)
		  hessian[i + i*nDiagonal] 
		    = (Float) srcl.f[srcinfo.step[i]]
		    + (Float) srcl.f[-srcinfo.step[i]] - 2*value;
		else
		  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
		    = ((Float) srcl.f[srcinfo.step[i] + srcinfo.step[j]]
		       + (Float) srcl.f[-srcinfo.step[i] - srcinfo.step[j]]
		       - (Float) srcl.f[srcinfo.step[i] - srcinfo.step[j]]
		       - (Float) srcl.f[srcinfo.step[j]
				       - srcinfo.step[i]])/4;
	    break;
	  case ANA_DOUBLE:
	    srcl.d = src.d + index;
	    value = (Float) *srcl.d;
	    for (i = 0; i < nDiagonal; i++)
	      grad[i] = grad2[i]
		= ((Float) srcl.d[srcinfo.step[i]]
		   - (Float) srcl.d[-srcinfo.step[i]])/2;
	    ready = 1;	/* zero gradient? */
	    for (i = 0; i < nDiagonal; i++)
	      if (grad[i] != 0) {
		ready = 0;
		break;
	      } /* end of if (grad[i] != 0) */
	    if (ready)	/* yes, zero gradient */
	      break;
	    for (i = 0; i < nDiagonal; i++)
	      for (j = 0; j <= i; j++)
		if (i == j)
		  hessian[i + i*nDiagonal] 
		    = (Float) srcl.d[srcinfo.step[i]]
		    + (Float) srcl.d[-srcinfo.step[i]] - 2*value;
		else
		  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
		    = ((Float) srcl.d[srcinfo.step[i] + srcinfo.step[j]]
		       + (Float) srcl.d[-srcinfo.step[i] - srcinfo.step[j]]
		       - (Float) srcl.d[srcinfo.step[i] - srcinfo.step[j]]
		       - (Float) srcl.d[srcinfo.step[j]
				       - srcinfo.step[i]])/4;
	    break;
	} /* end of switch (type) */
	if (ready)
	  for (i = 0; i < nDiagonal; i++)
	    grad2[i] = 0.0;
	else {		/* solve hessian.x = gradient */
	  f_decomp(hessian, nDiagonal, nDiagonal);
	  ready = 1;
	  for (i = nDiagonal*nDiagonal - 1; i >= 0; i -= nDiagonal + 1)
	    if (!hessian[i]) { /* zero determinant, indeterminate fit */
	      ready = 0;
	      for (i = 0; i < nDiagonal; i++)
		grad2[i] = 0.0;
	      break;
	    }	/* end of if (!hessian[i]) */
	  if (ready) {
	    f_solve(hessian, grad2, nDiagonal, nDiagonal);
	    for (i = 0; i < nDiagonal; i++)
	      value -= 0.5*grad2[i]*grad[i];
	  } /* end of if (ready) */
	} /* end of if (ready) else */
	if (code & 1) {		/* find position */
	  k = 0;
	  for (i = 0; i < srcinfo.ndim; i++)
	    *trgt.f++ = srcinfo.coords[i]
	      - ((!diagonal || diagonal[i])? grad2[k++]: 0);
	} else			/* find value */
	  *trgt.f++ = value;
      }	/* end of while (n--) */
    } else {			/* subgrid == 0 */
      if (code & 1) {		/* positions */
	while (n--) {
	  index = *trgt0.l++;
	  if (internalMode & 4)	/* /COORDS */
	    for (i = 0; i < srcinfo.ndim; i++) {
	      *trgt.l++ = index % srcinfo.dims[i];
	      index /= srcinfo.dims[i];
	    } /* end of for (i = 0;...) */
	  else
	    *trgt.l++ = index;
	} /* end of while (n--) */
      } else {			/* values */
	while (n--) {
	  index = *trgt0.l++;
	  switch (symbol_type(ps[0])) {
	    case ANA_BYTE:
	      value = (Float) src.b[index];
	      break;
	    case ANA_WORD:
	      value = (Float) src.w[index];
	      break;
	    case ANA_LONG:
	      value = (Float) src.l[index];
	      break;
	    case ANA_FLOAT:
	      value = (Float) src.f[index];
	      break;
	    case ANA_DOUBLE:
	      value = (Float) src.d[index];
	      break;
	  } /* end of switch (symbol_type(ps[0])) */
	  *trgt.f++ = value;
	} /* end of while (n--) */
      }	/* end of if (code & 1) */
    } /* end of if (subgrid) */
  } /* end of if (!degree) */
	
  free(offset);
  free(trgt00.b);
  if (nDiagonal) {
    free(grad);
    free(grad2);
    free(hessian);
  }
  return result;
}
/*---------------------------------------------------------*/
Int ana_find_maxloc(Int narg, Int ps[])
{
  return local_extrema(narg, ps, 3);
}
/*---------------------------------------------------------*/
Int ana_find_minloc(Int narg, Int ps[])
{
  return local_extrema(narg, ps, 1);
}
/*---------------------------------------------------------*/
Int ana_find_max(Int narg, Int ps[])
{
  return local_extrema(narg, ps, 2);
}
/*---------------------------------------------------------*/
Int ana_find_min(Int narg, Int ps[])
{
  return local_extrema(narg, ps, 0);
}
/*---------------------------------------------------------*/
Int ana_replace_values(Int narg, Int ps[])
/* REPLACE,x,src,trgt
  replaces all occurrences in <x> of each element of <src> by
  the corresponding element of <trgt>.  <src> must be in ascending order!
  LS 20feb98
*/
{
  Int	nData, nSrc, nTrgt, type, low, mid, high, ps1, ps2;
  pointer	data, src, trgt;
  scalar	v;

  ps1 = (symbol_type(ps[1]) == symbol_type(ps[0]))? ps[1]:
    ana_converts[symbol_type(ps[0])](1, &ps[1]);
  ps2 = (symbol_type(ps[2]) == symbol_type(ps[0]))? ps[2]:
    ana_converts[symbol_type(ps[0])](1, &ps[2]);

  if (numerical(ps[0], NULL, NULL, &nData, &data) < 0
      || numerical(ps1, NULL, NULL, &nSrc, &src) < 0
      || numerical(ps2, NULL, NULL, &nTrgt, &trgt) < 0)
    return ANA_ERROR;
  if (nTrgt != nSrc) /* src and trgt must have same number of elements */
    return cerror(INCMP_ARG, ps[2]);

  type = symbol_type(ps[0]);
  
  switch (type) {
  case ANA_BYTE:
    mid = 0;
    v.b = *data.b + 1;
    while (nData--) {		/* all data points */
      if (*data.b != v.b) { /* we added this check because we */
	/* hope there is coherence in the data so that the next data */
	/* value has a fair chance of being equal to the previous one. */
	/* In that case we can skip the binary search. */
	low = 0;
	high = nSrc - 1;
	v.b = *data.b;
	while (low <= high) {
	  mid = (low + high)/2;
	  if (v.b < src.b[mid])
	    high = mid - 1;
	  else if (v.b > src.b[mid])
	    low = mid + 1;
	  else {
	    mid = -mid - 1;	/* flag that we found it */
	    break;
	  }
	}
      }
      if (mid < 0) 		/* found it */
	*data.b = trgt.b[-mid - 1];
      data.b++;
    }
    break;
  case ANA_WORD:
    mid = 0;
    v.w = *data.w + 1;
    while (nData--) {		/* all data points */
      if (*data.w != v.w) { /* we added this check because we */
	/* hope there is coherence in the data so that the next data */
	/* value has a fair chance of being equal to the previous one. */
	/* In that case we can skip the binary search. */
	low = 0;
	high = nSrc - 1;
	v.w = *data.w;
	while (low <= high) {
	  mid = (low + high)/2;
	  if (v.w < src.w[mid])
	    high = mid - 1;
	  else if (v.w > src.w[mid])
	    low = mid + 1;
	  else {
	    mid = -mid - 1;	/* flag that we found it */
	    break;
	  }
	}
      }
      if (mid < 0) 		/* found it */
	*data.w = trgt.w[-mid - 1];
      data.w++;
    }
    break;
  case ANA_LONG:
    mid = 0;
    v.l = *data.l + 1;
    while (nData--) {		/* all data points */
      if (*data.l != v.l) { /* we added this check because we */
	/* hope there is coherence in the data so that the next data */
	/* value has a fair chance of being equal to the previous one. */
	/* In that case we can skip the binary search. */
	low = 0;
	high = nSrc - 1;
	v.l = *data.l;
	while (low <= high) {
	  mid = (low + high)/2;
	  if (v.l < src.l[mid])
	    high = mid - 1;
	  else if (v.l > src.l[mid])
	    low = mid + 1;
	  else {
	    mid = -mid - 1;	/* flag that we found it */
	    break;
	  }
	}
      }
      if (mid < 0) 		/* found it */
	*data.l = trgt.l[-mid - 1];
      data.l++;
    }
    break;
  case ANA_FLOAT:
    mid = 0;
    v.f = *data.f + 1;
    while (nData--) {		/* all data points */
      if (*data.f != v.f) { /* we added this check because we */
	/* hope there is coherence in the data so that the next data */
	/* value has a fair chance of being equal to the previous one. */
	/* In that case we can skip the binary search. */
	low = 0;
	high = nSrc - 1;
	v.f = *data.f;
	while (low <= high) {
	  mid = (low + high)/2;
	  if (v.f < src.f[mid])
	    high = mid - 1;
	  else if (v.f > src.f[mid])
	    low = mid + 1;
	  else {
	    mid = -mid - 1;	/* flag that we found it */
	    break;
	  }
	}
      }
      if (mid < 0) 		/* found it */
	*data.f = trgt.f[-mid - 1];
      data.f++;
    }
    break;
  case ANA_DOUBLE:
    mid = 0;
    v.d = *data.d + 1;
    while (nData--) {		/* all data points */
      if (*data.d != v.d) { /* we added this check because we */
	/* hope there is coherence in the data so that the next data */
	/* value has a fair chance of being equal to the previous one. */
	/* In that case we can skip the binary search. */
	low = 0;
	high = nSrc - 1;
	v.d = *data.d;
	while (low <= high) {
	  mid = (low + high)/2;
	  if (v.d < src.d[mid])
	    high = mid - 1;
	  else if (v.d > src.d[mid])
	    low = mid + 1;
	  else {
	    mid = -mid - 1;	/* flag that we found it */
	    break;
	  }
	}
      }
      if (mid < 0) 		/* found it */
	*data.d = trgt.d[-mid - 1];
      data.d++;
    }
    break;
  }
  return ANA_ONE;
}
/*---------------------------------------------------------*/
Int ana_lsq(Int narg, Int ps[])
/* linear least squares fit.
   a = LSQ(x,y[,w,COV=cov,ERR=err,CHISQ=chisq,/FORMAL]) 
   The model is that y = a(0)*x(...,0) + a(1)*x(...,1) + ... + error term
   The parameters <a> are those that minimize the weighted rms of the
   error term. <w> contains (optional) weights to assign to each of the <x>
   (if 1D), or a covariance matrix for the <y> (if multi-dim -- NOT YET).
   In <cov> is returned the covariance matrix of the <a>, in <err> the
   weighted residual error, in <chisq> the chi-square.  If /FORMAL is
   specified, then the covariances are multiplied by <err>^2 (suitable
   when no actual variances for <y> are known -- then it is assumed that
   <w> contains numbers of datapoints only [usually 1]).

   LS 4may98

Theory:
    V = (X'WX)^-1     covariance matrix of <a>
    a = VX'Wy         parameters <a>
    chi2 = y'Wy - a'V^-1a  chi-square
    err = sqrt(chi2/total(W))  residual error
   where W is C^-1, C is the covariance matrix of <y>.
   */
{
  Int	sx, sy, sw, *dims, ndim, nPar, i, nData, type, result, j, k, dw,
    newDims[MAX_DIMS];
  Float	onef = 1.0;
  Double	oned = 1.0, errv, f, chi2;
  scalar	value;
  pointer	px, py, pl, pr, p1, p2, pw, pc, err, chisq;
  Int	f_decomp(Float *, Int, Int), d_decomp(Double *, Int, Int),
    f_solve(Float *, Float *, Int, Int),
    d_solve(Double *, Double *, Int, Int);

  sx = ps[0];			/* <x>: independent data */
  if (symbol_class(sx) != ANA_ARRAY)
    return cerror(ILL_CLASS, sx);
  if (!symbolIsNumerical(sx))
    return cerror(ILL_TYPE, sx);
  dims = array_dims(sx);
  ndim = array_num_dims(sx);

  sy = ps[1];			/* <y>: dependent data */
  if (symbol_class(sy) != ANA_ARRAY)
    return cerror(ILL_CLASS, sy);
  if (!symbolIsNumerical(sy))
    return cerror(ILL_TYPE, sy);
  if (array_num_dims(sy) == ndim) /* must have same # dims as <x>... */
    nPar = 1;			/* number of parameters to fit */
  else if (array_num_dims(sy) == ndim - 1) /* ... or one less */
    nPar = dims[ndim - 1];
  else
    return cerror(INCMP_ARG, sy);
  nData = array_size(sy);	/* number of data points */

  for (i = 0; i < array_num_dims(sy); i++) /* dimensions must match */
    if (array_dims(sy)[i] != dims[i])
      return cerror(INCMP_ARG, sy);

  narg -= 2;
  ps += 2;

  if (narg > 0 && *ps) {		/* have <w>: weights (if 1D) */
    sw = *ps;
    if (symbol_class(sw) != ANA_ARRAY)
      return cerror(ILL_CLASS, sw);
    if (!symbolIsNumerical(sw))
      return cerror(ILL_TYPE, sw);
    if (array_num_dims(sw) != array_num_dims(sy))
      return cerror(INCMP_ARG, sw);
    for (i = 0; i < array_num_dims(sw); i++)
      if (array_dims(sy)[i] != array_dims(sw)[i])
	return cerror(INCMP_ARG, sw);
    dw = 1;
  } else
    dw = 0;

  type = ANA_FLOAT;		/* determine hightest data type: all will
				 be upgraded to that */
  if (array_type(sx) > type)
    type = array_type(sx);
  if (array_type(sy) > type)
    type = array_type(sy);
  if (dw && array_type(sw) > type)
    type = array_type(sw);
  switch (type) {
    case ANA_FLOAT:
      if (array_type(sx) < ANA_FLOAT)
	sx = ana_float(1, &sx);
      if (array_type(sy) < ANA_FLOAT)
	sy = ana_float(1, &sy);
      if (dw && array_type(sw) < ANA_FLOAT)
	sw = ana_float(1, &sw);
      break;
    case ANA_DOUBLE:
      if (array_type(sx) < ANA_DOUBLE)
	sx = ana_double(1, &sx);
      if (array_type(sy) < ANA_DOUBLE)
	sy = ana_double(1, &sy);
      if (dw && array_type(sw) < ANA_DOUBLE)
	sw = ana_double(1, &sw);
      break;
  }
  
  if (!dw) 			/* no <w>: default weights equal to 1 */
    switch (type) {
      case ANA_FLOAT:
	pw.f = &onef;
	break;
      case ANA_DOUBLE:
	pw.d = &oned;
	break;
    }
  else
    pw.f = (Float *) array_data(sw);

  narg--; ps++;
  if (narg > 0 && *ps) {	/* <cov>: place to return covariances */
    newDims[0] = newDims[1] = nPar;
    to_scratch_array(*ps, type, 2, newDims); /* make into suitable array */
    pc.f = (Float *) array_data(*ps);
    zerobytes(pc.f, nPar*nPar*ana_type_size[type]);
    switch (type) {		/* fill with identity matrix */
      case ANA_FLOAT:
	for (i = 0; i < nPar*nPar; i += nPar + 1)
	  pc.f[i] = 1;
	break;
      case ANA_DOUBLE:
	for (i = 0; i < nPar*nPar; i += nPar + 1)
	  pc.d[i] = 1;
	break;
    }
  } else
    pc.f = NULL;		/* no covariance return */

  narg--; ps++;
  if (narg > 0 && *ps) {	/* <err>: place to return residual error */
    undefine(*ps);		/* make into suitable scalar */
    symbol_class(*ps) = ANA_SCALAR;
    symbol_type(*ps) = type;
    err.b = &scalar_value(*ps).b;
  } else
    err.f = NULL;		/* no residual error return */

  narg--; ps++;
  if (narg > 0 && *ps) {	/* <chisq>: place to return chi-square */
    undefine(*ps);
    symbol_class(*ps) = ANA_SCALAR;
    symbol_type(*ps) = type;
    chisq.b = &scalar_value(*ps).b;
  } else
    chisq.f = NULL;

  px.f = (Float *) array_data(sx); /* x data */
  py.f = (Float *) array_data(sy); /* y data */

  /* now get some scratch space for intermediate results */
  pl.f = (Float *) malloc(nPar*nPar*ana_type_size[type]);
  zerobytes(pl.f, nPar*nPar*ana_type_size[type]);

  result = array_scratch(type, 1, &nPar); /* return symbol: parameters <a> */
  pr.f = (Float *) array_data(result);
  zerobytes(pr.f, nPar*ana_type_size[type]);

  /* now do the calculations */
  switch (type) {
    case ANA_FLOAT:
      /* get (X'WX) in pl */
      for (i = 0; i < nPar; i++) {
	p1.f = px.f + i*nData;
	for (j = 0; j < nPar; j++) {
	  p2.f = px.f + j*nData;
	  for (k = 0; k < nData; k++) {
	    *pl.f += p1.f[k]*p2.f[k]* *pw.f;
	    pw.f += dw;
	  }
	  pl.f++;
	  pw.f -= dw*nData;	/* back to the start */
	}
      }
      pl.f -= nPar*nPar;	/* back to the start */
      /* get (X'Wy) in pr */
      for (i = 0; i < nPar; i++) {
	for (j = 0; j < nData; j++) {
	  *pr.f += *px.f++ * *py.f++ * *pw.f;
	  pw.f += dw;
	}
	pr.f++;
	py.f -= nData;		/* back to the start */
	pw.f -= dw*nData;
      }
      px.f -= nData*nPar;	/* back to the start */
      pr.f -= nPar;
      f_decomp(pl.f, nPar, nPar); /* LU decomposition of pl */
      f_solve(pl.f, pr.f, nPar, nPar); /* solve: now pr has parameters <a> */
      if (chisq.f != NULL || err.f != NULL
	  || (pc.f != NULL && (internalMode & 1))) { /* need chi-square */
	chi2 = f = 0.0;
	for (i = 0; i < nData; i++) {
	  value.f = py.f[i];
	  for (j = 0; j < nPar; j++)
	    value.f -= pr.f[j]*px.f[j*nData];
	  chi2 += value.f*value.f**pw.f;
	  f += *pw.f;		/* collect weights for normalization */
	  px.f++;
	  pw.f += dw;
	}
	if (chisq.f != NULL)
	  *chisq.f = (Float) chi2;
	errv = sqrt(chi2/f);	/* normalize and get rms */
	if (err.f != NULL)
	  *err.f = (Float) errv; /* store */
      } else
	chi2 = 1.0;		/* don't calculate: assume 1. */
      if ((internalMode & 1) == 0)
	chi2 = 1.0;
      if (pc.f != NULL) {	/* get covariances */
	for (i = 0; i < nPar; i++) {
	  f_solve(pl.f, pc.f, nPar, nPar); /* now pc contains
					      the covariances */
	  pc.f += nPar;
	}
	pc.f -= nPar*nPar;	/* back to the start */
	if (internalMode & 1) {
	  if (chisq.f != NULL)
	    *chisq.f = f - 1;
	  chi2 = chi2/(f - 1);
	  for (i = 0; i < nPar*nPar; i++)
	    pc.f[i] *= chi2;
	}
	if (internalMode & 2) {
	  /* we transform the covariance matrix so its diagonal contains
	     standard errors, one triangle contains covariances, and the other
	     triangle contains correlation coefficients */
	  for (i = 0; i < nPar; i++)
	    for (j = 0; j < i; j++)
	      pc.f[i + j*nPar] /= sqrt(pc.f[i*(nPar + 1)]*pc.f[j*(nPar + 1)]);
	  for (i = 0; i < nPar; i++)
	    pc.f[i + i*nPar] = sqrt(pc.f[i + i*nPar]);
	}
      }
      break;
    case ANA_DOUBLE:
      /* get (X'WX) in pl */
      for (i = 0; i < nPar; i++) {
	p1.d = px.d + i*nData;
	for (j = 0; j < nPar; j++) {
	  p2.d = px.d + j*nData;
	  for (k = 0; k < nData; k++) {
	    *pl.d += p1.d[k]*p2.d[k]* *pw.d;
	    pw.d += dw;
	  }
	  pl.d++;
	  pw.d -= dw*nData;	/* back to the start */
	}
      }
      pl.d -= nPar*nPar;	/* back to the start */
      /* get (X'Wy) in pr */
      for (i = 0; i < nPar; i++) {
	for (j = 0; j < nData; j++) {
	  *pr.d += *px.d++ * *py.d++ * *pw.d;
	  pw.d += dw;
	}
	pr.d++;
	py.d -= nData;		/* back to the start */
	pw.d -= dw*nData;
      }
      px.d -= nData*nPar;	/* back to the start */
      pr.d -= nPar;
      d_decomp(pl.d, nPar, nPar); /* LU decomposition of pl */
      d_solve(pl.d, pr.d, nPar, nPar); /* solve: now pr has parameters <a> */
      if (chisq.d != NULL || err.d != NULL
	  || (pc.d != NULL && (internalMode & 1))) { /* need chi-square */
	chi2 = f = 0.0;
	for (i = 0; i < nData; i++) {
	  value.d = py.d[i];
	  for (j = 0; j < nPar; j++)
	    value.d -= pr.d[j]*px.d[j*nData];
	  chi2 += value.d*value.d**pw.d;
	  f += *pw.d;		/* collect weights for normalization */
	  px.d++;
	  pw.d += dw;
	}
	if (chisq.d != NULL)
	  *chisq.d = (Float) chi2;
	errv = sqrt(chi2/f);	/* normalize and get rms */
	if (err.d != NULL)
	  *err.d = (Float) errv; /* store */
      } else
	chi2 = 1.0;		/* don't calculate: assume 1. */
      if ((internalMode & 1) == 0)
	chi2 = 1.0;
      if (pc.d != NULL) {	/* get covariances */
	for (i = 0; i < nPar; i++) {
	  d_solve(pl.d, pc.d, nPar, nPar); /* now pc contains
					      the covariances */
	  pc.d += nPar;
	}
	pc.d -= nPar*nPar;	/* back to the start */
	if (internalMode & 1) {
	  if (chisq.f != NULL)
	    *chisq.d = f - 1;
	  chi2 = chi2/(f - 1);
	  for (i = 0; i < nPar*nPar; i++)
	    pc.d[i] *= chi2;
	}
	if (internalMode & 2) {
	  /* we transform the covariance matrix so its diagonal contains
	     standard errors, one triangle contains covariances, and the other
	     triangle contains correlation coefficients */
	  for (i = 0; i < nPar; i++)
	    for (j = 0; j < i; j++)
	      pc.d[i + j*nPar] /= sqrt(pc.d[i*(nPar + 1)]*pc.d[j*(nPar + 1)]);
	  for (i = 0; i < nPar; i++)
	    pc.d[i + i*nPar] = sqrt(pc.d[i + i*nPar]);
	}
      }
      break;
  }
  
  free(pl.f);
  return result;
}
/*---------------------------------------------------------*/
Int	ana_indgen(Int, Int []);
Int ana_lsq2(Int narg, Int ps[])
/* linear least squares fit.
   a = LLSQ(x,y[,axis,FWHM=fwhm,WEIGHTS=w,COV=cov,ERR=err,CHISQ=chisq,
       /FORMAL,/REDUCE])
   The model is that y = a(0)*x(...,0) + a(1)*x(...,1) + ... + error term
   The parameters <a> are those that minimize the weighted rms of the
   error term. <w> contains (optional) weights to assign to each of the <x>
   (if 1D), or a covariance matrix for the <y> (if multi-dim -- NOT YET).
   In <cov> is returned the covariance matrix of the <a>, in <err> the
   weighted residual error, in <chisq> the chi-square.  If /FORMAL is
   specified, then the covariances are multiplied by <err>^2 (suitable
   when no actual variances for <y> are known -- then it is assumed that
   <w> contains numbers of datapoints only [usually 1]).

   LS 4may98

Theory:
    V = (X'WX)^-1     covariance matrix of <a>
    a = VX'Wy         parameters <a>
    chi2 = y'Wy - a'V^-1a  chi-square
    err = sqrt(chi2/total(W))  residual error
   where W is C^-1, C is the covariance matrix of <y>.
   */

/* LSQ2(x, y [, axis, w]) */
{
  Int	i, j, k, nPar, nData, type, result, dims[MAX_DIMS], ndim, dw, ncc,
    axisSym, nRepeat, stride, bigstride, ysym, nkernel, n, i1, i2, cc, dk;
  Float	onef = 1.0;
  Double oned = 1.0, chi2, f, errv, fwhm, norm;
  scalar	value;
  pointer	px, py, pl, pr, p2, pw, pc, err, chisq, pk;
  loopInfo	xinfo, yinfo, p2info, winfo;

  if (isComplexType(symbol_type(ps[0])))
    return cerror(NO_COMPLEX, ps[0]);
  if (isComplexType(symbol_type(ps[1])))
    return cerror(NO_COMPLEX, ps[1]);
  if (!symbolIsRealArray(ps[0])) /* <x> */
    return cerror(NEED_NUM_ARR, ps[0]);
  if (!symbolIsRealArray(ps[1])) /* <y> */
    return cerror(NEED_NUM_ARR, ps[1]);

  /* we allow the user to specify zero or one axes.  If none are specified, */
  /* then <y> and the corresponding dimensions of <x> are regarded as */
  /* one-dimensional. */
  if (narg > 2 && ps[2]) {	/* <axes> */
    if (!symbolIsRealScalar(ps[2]))
      return cerror(NEED_SCAL, ps[2]);
    axisSym = ps[2];
  } else {			/* default: all <y> axes */
    axisSym = array_num_dims(ps[1]);
    axisSym = array_scratch(ANA_LONG, 1, &axisSym);
    axisSym = ana_indgen(1, &axisSym);
  }
  
  type = highestType(array_type(ps[0]), array_type(ps[1]));
  if (type < ANA_FLOAT)
    type = ANA_FLOAT;

  if (standardLoop(ps[0], axisSym, SL_SRCUPGRADE | SL_AXESBLOCK,
		   type, &xinfo, &px, NULL, NULL, NULL) < 0
      || standardLoop(ps[1], axisSym, SL_SRCUPGRADE | SL_AXESBLOCK,
		      type, &yinfo, &py, NULL, NULL, NULL) < 0)
    return ANA_ERROR;

  ysym = ps[1];

  /* <x> must have all the dimensions of <y>, and may have one more */
  /* dimension at the end */
  if (xinfo.ndim != yinfo.ndim && xinfo.ndim != yinfo.ndim + 1)
    return cerror(INCMP_ARG, ps[1]);
  for (i = 0; i < yinfo.ndim; i++)
    if (xinfo.dims[i] != yinfo.dims[i])
      return cerror(INCMP_ARG, ps[1]);
  
  /* find the number of parameters to fit */
  if (xinfo.ndim == yinfo.ndim)
    nPar = 1;
  else
    nPar = xinfo.dims[xinfo.ndim - 1];

  /* determine the number of data points that go in each fit */
  nData = 1;
  for (i = 0; i < yinfo.naxes; i++)
    nData *= yinfo.rdims[i];

  nRepeat = yinfo.nelem/nData;

  ps += 3;
  narg -= 3;

  if (narg > 0 && *ps) {	/* have <fhwm> */
    if (!symbolIsRealScalar(*ps))
      return cerror(NEED_SCAL, *ps);
    fwhm = double_arg(*ps);
    if (fwhm < 0)
      fwhm = 0.0;
  } else
    fwhm = 0.0;

  ps++; narg--;
  if (narg > 0 && *ps) {	/* have <w>: weights */
    if (!symbolIsRealArray(*ps))
      return cerror(ILL_CLASS, *ps);
    if (array_num_dims(*ps) != array_num_dims(ysym))
      return cerror(INCMP_ARG, *ps);
    for (i = 0; i < array_num_dims(*ps); i++)
      if (array_dims(ysym)[i] != array_dims(*ps)[i])
	return cerror(INCMP_ARG, *ps);
    standardLoop(*ps, axisSym, SL_SRCUPGRADE | SL_AXESBLOCK,
		 type, &winfo, &pw, NULL, NULL, NULL);
    dw = 1;
  } else {
    switch (type) {
      case ANA_FLOAT:
	pw.f = &onef;
	break;
      case ANA_DOUBLE:
	pw.d = &oned;
	break;
    }
    dw = 0;
  }

  k = fwhm? 0: yinfo.naxes;

  ps++; narg--;
  if (narg > 0 && *ps) {	/* <cov>: place to return covariances */
    if (yinfo.ndim + 2 - k >= MAX_DIMS)
      return anaerror("Too many dimensions for COV", 0);
    dims[0] = dims[1] = nPar;
    memcpy(dims + 2, yinfo.rdims + k, (yinfo.ndim - k)*sizeof(Int));
    ndim = 2 + yinfo.ndim - k;
    if (to_scratch_array(*ps, type, ndim, dims) == ANA_ERROR)
      return ANA_ERROR;
    pc.f = (Float *) array_data(*ps);
    zerobytes(pc.f, array_size(*ps)*ana_type_size[type]);
    j = fwhm? nRepeat: yinfo.nelem;
    switch (type) {		/* fill with identity matrix */
      case ANA_FLOAT:
	while (j--) { 
	  for (i = 0; i < nPar*nPar; i += nPar + 1)
	    pc.f[i] = 1;
	  pc.f += nPar*nPar;
	}
	break;
      case ANA_DOUBLE:
	while (j--) {
	  for (i = 0; i < nPar*nPar; i += nPar + 1)
	    pc.d[i] = 1;
	  pc.d += nPar*nPar;
	}
	break;
    }
    pc.f = array_data(*ps);
  } else
    pc.f = NULL;		/* no covariance return */

  narg--; ps++;
  if (narg > 0 && *ps) {	/* <err>: place to return residual error */
    if (nRepeat == 1 && !fwhm) { /* a scalar will do */
      undefine(*ps);		/* make into suitable scalar */
      symbol_class(*ps) = ANA_SCALAR;
      symbol_type(*ps) = type;
      err.b = &scalar_value(*ps).b;
    } else {			/* we need an array */
      ndim = yinfo.ndim - k;
      memcpy(dims, yinfo.rdims + k, ndim*sizeof(Int));
      if (to_scratch_array(*ps, type, ndim, dims) == ANA_ERROR)
	return ANA_ERROR;
      err.b = array_data(*ps);
    }
  } else
    err.f = NULL;		/* no residual error return */

  narg--; ps++;
  if (narg > 0 && *ps) {	/* <chisq>: place to return chi-square */
    if (nRepeat == 1 && !fwhm) { /* a scalar will do */
      undefine(*ps);		/* make into suitable scalar */
      symbol_class(*ps) = ANA_SCALAR;
      symbol_type(*ps) = type;
      chisq.b = &scalar_value(*ps).b;
    } else {			/* we need an array */
      ndim = yinfo.ndim - k;
      memcpy(dims, yinfo.rdims + k, ndim*sizeof(Int));
      if (to_scratch_array(*ps, type, ndim, dims) == ANA_ERROR)
	return ANA_ERROR;
      chisq.b = array_data(*ps);
    }
  } else
    chisq.f = NULL;		/* no residual error return */
    
  /* prepare the return symbol */
  dims[0] = nPar;
  memcpy(dims + 1, yinfo.rdims + k,
	 (yinfo.ndim - k)*sizeof(Int));
  ndim = 1 + yinfo.ndim - k;
  result = array_scratch(type, ndim, dims);
  if (result == ANA_ERROR)
    return ANA_ERROR;
  pr.f = array_data(result);

  /* prepare some scratch space */
  pl.f = malloc(nPar*nPar*ana_type_size[type]);
  if (!pl.f)
    return cerror(ALLOC_ERR, 0);

  /* prepare smoothing kernel */
  if (fwhm) {
    nkernel = MAX(MIN(2*fwhm, nData), nPar);
    pk.f = malloc((2*nkernel + 1)*ana_type_size[type]);
    errv = 0.600561204/fwhm;
    f = -nkernel*errv;
    for (i = 0; i < 2*nkernel + 1; i++) {
      *pk.f++ = exp(-f*f);
      f += errv;
    }
    pk.f -= 2*nkernel + 1;
    dk = 1;
  } else {
    switch (type) {
      case ANA_FLOAT:
	pk.f = &onef;
	break;
      case ANA_DOUBLE:
	pk.d = &oned;
	break;
    }
    dk = 0;
  }

  /* now do the work */
  stride = yinfo.rsinglestep[0];
  bigstride = stride*nData;
  n = nData;
  ncc = fwhm? nData: 1;
  do {
    for (cc = 0; cc < ncc; cc++) {
      zerobytes(pl.f, nPar*nPar*ana_type_size[type]);
      zerobytes(pr.f, nPar*ana_type_size[type]);
      p2info = xinfo;
      p2info.data = &p2;
      p2.f = px.f;
      
      if (fwhm) {
	i1 = MAX(cc - nkernel, 0);
	i2 = MIN(cc + nkernel, nData - 1);
	n = i2 - i1 + 1;
	bigstride = stride*n;
	px.f += i1;
	py.f += i1;
	p2.f += i1;
	if (dw)
	  pw.f += i1;
	pk.f += nkernel - cc + i1;
      }
      
      switch (type) {
	case ANA_FLOAT:
	  /* get (X'X) in pl */
	  do {
	    do {
	      for (i = 0; i < n; i++) {
		*pl.f += *px.f * *p2.f * *pw.f * *pk.f;
		px.f += stride;
		p2.f += stride;
		if (dw)
		  pw.f += stride;
		pk.f += dk;
	      }
	      px.f -= bigstride;
	      p2.f -= bigstride;
	      if (dw)
		pw.f -= bigstride;
	      pk.f -= n*dk;
	      pl.f++;
	    } while (moveLoop(&xinfo, yinfo.rndim, 1) <= yinfo.rndim);
	    moveLoop(&xinfo, yinfo.rndim, -nPar);
	  } while (moveLoop(&p2info, yinfo.rndim, 1) <= yinfo.rndim);
	  moveLoop(&p2info, yinfo.rndim, -nPar);
	  pl.f -= nPar*nPar;	/* back to the start */
	  
	  /* get (X'y) in pr */
	  j = yinfo.naxes - 1;
	  do {
	    for (i = 0; i < n; i++) {
	      *pr.f += *px.f * *py.f * *pw.f * *pk.f;
	      px.f += stride;
	      py.f += stride;
	      if (dw)
		pw.f += stride;
	      pk.f += dk;
	    }
	    pr.f++;
	    px.f -= bigstride;
	    py.f -= bigstride;
	    if (dw)
	      pw.f -= bigstride;
	    pk.f -= n*dk;
	  } while (moveLoop(&xinfo, yinfo.rndim, 1) <= yinfo.rndim);
	  moveLoop(&xinfo, yinfo.rndim, -nPar);
	  pr.f -= nPar;		/* back to the start */
	  f_decomp(pl.f, nPar, nPar); /* LU decomposition of pl */
	  f_solve(pl.f, pr.f, nPar, nPar); /* solve: pr gets parameters <a> */
	  
	  if (chisq.f != NULL || err.f != NULL
	      || (pc.f != NULL && (internalMode & 1))) { /* need chi-square */
	    chi2 = f = norm = 0.0;
	    /* calculate the deviations from the fit */
	    k = nData*nRepeat;
	    for (i = 0; i < n; i++) {
	      value.d = *py.f;
	      for (j = 0; j < nPar; j++)
		value.d -= pr.f[j]*px.f[j*k];
	      chi2 += value.d*value.d * *pw.f * *pk.f;
	      norm += *pk.f;
	      f += *pw.f * *pk.f;
	      px.f += stride;
	      py.f += stride;
	      if (dw)
		pw.f += stride;
	      pk.f += dk;
	    }
	    px.f -= bigstride;
	    py.f -= bigstride;
	    if (dw)
	      pw.f -= bigstride;
	    pk.f -= n*dk;
	    if (chisq.f != NULL)
	      *chisq.f++ = chi2/norm;
	    errv = sqrt(chi2/f);
	    if (err.f)
	      *err.f++ = errv;
	  } else
	    chi2 = 1.0;
	  if ((internalMode & 1) == 0)
	    chi2 = 1.0;
	  if (pc.f) {		/* get covariances */
	    for (i = 0; i < nPar; i++) {
	      f_solve(pl.f, pc.f, nPar, nPar);
	      pc.f += nPar;
	    }
	    pc.f -= nPar*nPar;	/* back to the start */
	    if (internalMode & 1) {
	      if (chisq.f != NULL)
		*chisq.f = f - 1;
	      chi2 = chi2/(f - 1);
	      for (i = 0; i < nPar*nPar; i++)
		pc.f[i] *= chi2;
	    }
	    if (internalMode & 2) {
	      /* we transform the covariance matrix so its diagonal contains
		 standard errors, one triangle contains covariances, and
		 the other triangle contains correlation coefficients */
	      for (i = 0; i < nPar; i++)
		for (j = 0; j < i; j++)
		  pc.f[i + j*nPar] /= sqrt(pc.f[i*(nPar + 1)]*pc.f[j*(nPar + 1)]);
	      for (i = 0; i < nPar; i++)
		pc.f[i + i*nPar] = sqrt(pc.f[i + i*nPar]);
	      pc.f += nPar*nPar;
	    }
	  }
	  pr.f += nPar;		/* get ready for the next one */
	  break;
      }	/* end of switch (type) */

      /* restore */
      if (fwhm) {
	px.f -= i1;
	py.f -= i1;
	p2.f -= i1;
	if (dw)
	  pw.f -= i1;
	pk.f -= nkernel - cc + i1;
      }
    } /* end of for (cc = ...) */
    moveLoop(&yinfo, yinfo.naxes, 1);
  } while (moveLoop(&xinfo, yinfo.naxes, 1) < yinfo.rndim);
  free(pl.f);
  return result;
}
/*---------------------------------------------------------*/
Int ana_runprod(Int narg, Int ps[])
/* RUNPROD( data [, axis]) returns a running product of <data> */
/* along the indicated <axis>.  LS 13jul98 */
{
  Int	n, result;
  scalar	value;
  pointer	src, trgt;
  loopInfo	srcinfo, trgtinfo;

  if (standardLoop(ps[0], narg > 1? ps[1]: 0,
		   SL_SAMEDIMS | SL_UPGRADE | SL_ONEAXIS | SL_NEGONED,
		   ANA_BYTE,
		   &srcinfo, &src, &result, &trgtinfo, &trgt) == ANA_ERROR)
    return ANA_ERROR;

  switch (srcinfo.type) {
    case ANA_BYTE:
      value.b = 1;
      do {
	value.b *= *src.b;
	*trgt.b = value.b;
	n = advanceLoop(&trgtinfo, &trgt), advanceLoop(&srcinfo, &src);
	if (n)
	  value.b = 1;
      } while (n < srcinfo.rndim);
      break;
    case ANA_WORD:
      value.w = 1;
      do {
	value.w *= *src.w;
	*trgt.w = value.w;
	n = advanceLoop(&trgtinfo, &trgt), advanceLoop(&srcinfo, &src);
	if (n)
	  value.w = 1;
      } while (n < srcinfo.rndim);
      break;
    case ANA_LONG:
      value.l = 1;
      do {
	value.l *= *src.l;
	*trgt.l = value.l;
	n = advanceLoop(&trgtinfo, &trgt), advanceLoop(&srcinfo, &src);
	if (n)
	  value.l = 1;
      } while (n < srcinfo.rndim);
      break;
    case ANA_FLOAT:
      value.f = 1;
      do {
	value.f *= *src.f;
	*trgt.f = value.f;
	n = advanceLoop(&trgtinfo, &trgt), advanceLoop(&srcinfo, &src);
	if (n)
	  value.f = 1;
      } while (n < srcinfo.rndim);
      break;
    case ANA_DOUBLE:
      value.d = 1;
      do {
	value.d *= *src.d;
	*trgt.d = value.d;
	n = advanceLoop(&trgtinfo, &trgt), advanceLoop(&srcinfo, &src);
	if (n)
	  value.d = 1;
      } while (n < srcinfo.rndim);
      break;
  default:
    break;
  }
  return result;
}
/*---------------------------------------------------------*/