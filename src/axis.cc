/* This is file axis.cc.

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

/** \file
    LUX routines for dealing with individual dimensions
    of arrays. We put all information relevant to going through a
    dimensional structure in a struct so we can have several different
    loops going at once.

    Often one wants to go through an array along one particular
    dimension, or in succession along several dimensions.  The
    routines in this file allow the user to set up and perform such a
    traversal fairly easily.

    Most of the time, the user will want to use routine
    standardLoop(), which stores information about the array to be
    traversed, arranges for its traversal along the axis or axes
    specified by the user (if any), and generates an appropriate
    output symbol, if desired.

    The tricky bit of standardLoop() is that the user must specify the
    location of a pointer for use in traversing the array.  This
    pointer will be updated by routine advanceLoop() appropriate for
    the dimensional structure of the array and the axis along which
    the array is traversed.  See at standardLoop() for more info.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <limits.h>
#include "action.hh"
#include <obstack.h>
#include <errno.h>
#include <ctype.h>
#include "axis.hh"

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

void setupDimensionLoop(loopInfo *info, int32_t ndim, int32_t const *dims, 
                        Symboltype type, int32_t naxes, int32_t const *axes,
                        Pointer *data, int32_t mode);
void rearrangeDimensionLoop(loopInfo *info);
int32_t standardLoop(int32_t data, int32_t axisSym, int32_t mode, int32_t outType,
		 loopInfo *src, Pointer *srcptr, int32_t *output, loopInfo *trgt,
		 Pointer *trgtptr);
int32_t advanceLoop(loopInfo *info, Pointer *ptr),
  lux_convert(int32_t, int32_t [], Symboltype, int32_t);
int32_t nextLoop(loopInfo *info), nextLoops(loopInfo *info1, loopInfo *info2);
int32_t dimensionLoopResult(loopInfo const *sinfo, loopInfo *tinfo, int32_t type,
			Pointer *tptr);
int32_t standardLoop1(int32_t source,
                  int32_t nAxes, int32_t const * axes,
                  int32_t srcMode,
                  loopInfo *srcinf, Pointer *srcptr,
                  int32_t nMore, int32_t const * more,
                  int32_t nLess, int32_t const * less,
                  Symboltype tgtType, int32_t tgtMode,
                  int32_t *target, 
                  loopInfo *tgtinf, Pointer *tgtptr);
/*-------------------------------------------------------------------------*/
void setAxisMode(loopInfo *info, int32_t mode) {
  if ((mode & SL_EACHBLOCK) == SL_EACHBLOCK)
    info->advanceaxis = info->naxes;
  else if (mode & SL_EACHROW)
    info->advanceaxis = 1;
  else
    info->advanceaxis = 0;
  info->axisindex = 0;
  info->mode = mode;

  /* rearrange the dimensions for the first pass */
  rearrangeDimensionLoop(info);
}
/*-------------------------------------------------------------------------*/
int32_t setAxes(loopInfo *info, int32_t nAxes, int32_t *axes, int32_t mode)
{
  int32_t i;
  int32_t temp[MAX_DIMS];

  if (mode & SL_TAKEONED)	/* take data as 1D */
    nAxes = 0;			/* treat as 1D */
  else if (mode & SL_ALLAXES) { /* select all axes */
    nAxes = info->ndim;
    axes = NULL;		/* treat as if all axes were specified */
  } else if ((mode & SL_NEGONED)	/* negative-axis treatment */
	     && nAxes == 1		/* one axis specified */
	     && *axes < 0)	/* and it is negative */
    nAxes = 0;
  
  if ((mode & SL_ONEAXIS)	/* only one axis allowed */
      && nAxes > 1)		/* and more than one selected */
    return luxerror("Only one axis allowed", -1);

  /* check the specified axes for legality */
  if (nAxes && axes) {
    for (i = 0; i < nAxes; i++) /* check all specified axes */
      if (axes[i] < 0		/* axis is negative */
	  || axes[i] >= info->ndim)	/* or too great */
	return luxerror("Illegal axis %1d", -1, axes[i]);
    if (mode & SL_UNIQUEAXES) {	/* no axis must occur more than once */
      zerobytes(temp, info->ndim*sizeof(int32_t));
      for (i = 0; i < nAxes; i++)
	if (temp[axes[i]]++)
	  return luxerror("Axis %1d illegally specified more than once",
			  -1, axes[i]);
    }
  }
  info->naxes = nAxes;
  if (nAxes && axes)
    memcpy(info->axes, axes, nAxes*sizeof(*axes));
  setAxisMode(info, mode);
  return 0;
}
/*-------------------------------------------------------------------------*/
/** Gather information for looping through a LUX array.

    \param[in,out] info a pointer to the \c loopInfo structure in
    which the information is gathered.
    \param[in] ndim  the number of dimensions
    \param[in] dims  the array of dimensions
    \param[in] type  the LUX data type of the array elements
    \param[in] naxes the number of axes to loop along
    \param[in] axes  the array of axes to loop along
    \param[in] data  a pointer to a \c pointer to the data
    \param[in] mode flags that indicate how to loop through the axes,
    as for \c standardLoop()
 */
void setupDimensionLoop(loopInfo *info, int32_t ndim, int32_t const *dims, 
                        Symboltype type, int32_t naxes, int32_t const *axes,
                        Pointer *data, int32_t mode)
{
  int32_t	i;
  size_t	size;

  /* first copy the arguments into the structure
   NOTE: no checking to see if the values are legal. */

  /* the number of dimensions in the data: */
  info->ndim = ndim;
  /* the list of dimensions of the data: */
  if (ndim)
    memmove(info->dims, dims, ndim*sizeof(int32_t));
  else				/* a scalar */
    info->dims[0] = 1;		/* need something reasonable here or else
				   advanceLoop() won't work properly. */
  /* the number of dimensions along which should be looped (one at a time) */
  info->naxes = naxes;
  /* the number of elements in the array */
  size = 1;
  for (i = 0; i < info->ndim; i++)
    size *= info->dims[i];
  /* the size is small enough to fit in a (size_t), or else it would
     not have been able to be created by array_scratch() or
     array_clone(), but it may not be small enough to fit in an
     (off_t). */
  if (size > INT64_MAX/2)
    printf("WARNING - array size (%lu elements) may be too great\n"
           "for this operation!  Serious errors may occur!\n", size);
  info->nelem = size;
  /* the list of dimensions along which should be looped */
  if (axes)
    memmove(info->axes, axes, naxes*sizeof(int32_t));
  else
    for (i = 0; i < info->naxes; i++)
      info->axes[i] = i;		/* SL_ALLAXES was selected */
  /* the type of data: LUX_INT8, ..., LUX_DOUBLE */
  info->type = type;
  /* a pointer to a pointer to the data */
  info->data = data;
  info->data0 = data->v;

  /* now derive auxiliary data */
  /* the step size per dimension (measured in elements), i.e. by how many
   elements one has to advance a suitable pointer to point at the next
   element in the selected dimension: */
  info->singlestep[0] = 1;
  for (i = 0; i < info->ndim - 1; i++)
    info->singlestep[i + 1] = info->singlestep[i]*info->dims[i];
  /* the number of bytes per data element: */
  info->stride = lux_type_size[type];

  setAxisMode(info, mode);
}
/*-----------------------------------------------------------------------*/
/** Advance along a loop.  The coordinates and the pointer to the data
    are advanced, taking into account the dimensional structure of the
    data and the configured axes.

    \param[in,out] info the loop information
    \param[in,out] ptr  a pointer to the data
    \return the index of the first loop axis that is not yet
    completely traversed.
 */
int32_t advanceLoop(loopInfo *info, Pointer *ptr)
/* advance coordinates; return index of first encountered incomplete
 axis.  I.e., if the array has 4 by 5 by 6 elements, then advancement
 from element (2,0,0) (to element (3,0,0)) yields 0, (3,0,0) yields 1,
 (3,4,0) yields 2, and (3,4,5) yields 3. */
{
  int32_t	i, done;

  /* advance pointer */
  ptr->b += info->step[info->advanceaxis]*info->stride;

  if (info->advanceaxis >= info->rndim)	/* already done */
    done = info->rndim;
  else {
    done = info->advanceaxis;   /* default: not done yet */

    /* update coordinates */
    for (i = info->advanceaxis; i < info->rndim; i++) {
      if (++(info->coords[i]) < info->rdims[i])
	break;			/* not yet at end of this dimension */
      /* if we get here, we are at the end of a dimension */
      info->coords[i] = 0;	/* back to start */
      done = i + 1;		/* keep track of last advanced dimension */
      if (done < info->rndim)
	ptr->b += info->step[i + 1]*info->stride;
    }
  }
  return done;
}
/*-----------------------------------------------------------------------*/
int32_t loopIsAtStart(loopInfo const *info)
/* returns 1 if all coordinates are equal to 0, and 0 otherwise.
   Can be used to test if the loop is back at the start again -- i.e.,
   has completed.  See also advanceLoop(). */
{
  int32_t state = 1, i;

  for (i = 0; i < info->rndim; i++)
    state &= (info->coords[i] == 0);
  return state;
}
/*-----------------------------------------------------------------------*/
void rearrangeDimensionLoop(loopInfo *info)
/* rearranges dimensions suitable for walking along the selected axis
   <info>: information about the loop
   <info->axisindex>: index to the position in info->axes where the axis is
      stored along which is to be traveled
   <info->mode>: flags that indicate the desired treatment of the axes
      SL_EACHCOORD: the indicated axis goes first; the remaining axes
         come later in their original order; the user gets access to
	 all coordinates
      SL_AXISCOORD: the indicated axis goes first; the user gets access
         only to the coordinate along the indicated axis; remaining axes
         come later, lumped together as much as possible for faster
         execution
      SL_AXESBLOCK: the active axis goes first, then all remaining
         specified axes, and then the unspecified ones; in ascending order
	 within each group
*/
{
  int32_t	axis, i, temp[MAX_DIMS], j, axisIndex, mode, axis2;

  axisIndex = info->axisindex;
  mode = info->mode;
  if (axisIndex < info->naxes) {
    /* actually have an axis to put first */
    axis = info->axes[axisIndex];
    switch (mode & (SL_EACHCOORD | SL_AXISCOORD | SL_AXESBLOCK)) {
      case SL_EACHCOORD: default:
	/* put desired axis at front; leave order of remaining axes */
	/* get rearranged step sizes */
	info->rsinglestep[0] = info->singlestep[axis];
	memcpy(&info->rsinglestep[1], info->singlestep, axis*sizeof(int32_t));
	memcpy(&info->rsinglestep[axis + 1], &info->singlestep[axis + 1],
	       (info->ndim - axis - 1)*sizeof(int32_t));

	/* get rearranged dimensions */
	info->rndim = info->ndim;
	info->rdims[0] = info->dims[axis];
	memcpy(&info->rdims[1], info->dims, axis*sizeof(int32_t));
	memcpy(&info->rdims[axis + 1], &info->dims[axis + 1],
	       (info->ndim - axis - 1)*sizeof(int32_t));

	/* get mappings between original and rearranged axes */
	info->raxes[0] = axis;
	for (i = 0; i < axis; i++)
	  info->raxes[i + 1] = i;
	for (i = axis + 1; i < info->ndim; i++)
	  info->raxes[i] = i;
	for (i = 0; i < info->ndim; i++)
	  info->iraxes[info->raxes[i]] = i;
	break;
      case SL_AXISCOORD:
	/* only need the coordinate in the desired axis; we can lump the
	   axes preceding the selected one together into one pseudo-dimension,
	   and likewise for the axes following the selected one: this may
	   speed up the loop traversal.  We can only lump together contiguous
	   blocks of axes, so we cannot (in general) lump all remaining
	   dimensions together. */
	info->rdims[0] = info->dims[axis]; /* selected axis goes first */
	info->raxes[0] = axis;
	info->iraxes[axis] = 0;
	if (axis) {		/* selected axis was not the first one */
	  /* lump earlier axes together */
	  info->rdims[1] = 1;
	  for (i = 0; i < axis; i++) {
	    info->rdims[1] *= info->dims[i];
	    info->iraxes[i] = 1;
	  }
	  info->raxes[1] = 0;	/* smallest axis in this lump */
	  j = 2;
	} else
	  j = 1;
	if (axis < info->ndim - 1) { /* selected axis is not the last one */
	  /* lump later axes together */
	  info->rdims[j] = 1;
	  for (i = axis + 1; i < info->ndim; i++) {
	    info->rdims[j] *= info->dims[i];
	    info->iraxes[i] = j;
	  }
	  info->raxes[j++] = axis + 1; /* smallest axis in this lump */
	}
	info->rndim = j;
	
	/* get step sizes */
	for (i = 0; i < info->rndim; i++)
	  info->rsinglestep[i] = info->singlestep[info->raxes[i]];
	break;
      case SL_AXESBLOCK:
	/* the active axis goes first, then the remaining selected axes, */
	/* and then the ones that were not selected; in ascending order */
	/* within each group */
	zerobytes(temp, info->ndim*sizeof(int32_t));

	info->rdims[0] = info->dims[axis];
	temp[axis] = 1;
	info->raxes[0] = axis;
	info->rsinglestep[0] = info->singlestep[axis];

	/* treat the remaining selected axes */
	j = 1;
	for (i = 0; i < info->naxes; i++) {
	  axis2 = info->axes[i];
	  if (axis2 == axis)
	    continue;		/* already have the active one */
	  info->rdims[j] = info->dims[axis2];
	  temp[axis2] = 1;
	  info->raxes[j] = axis2;
	  info->rsinglestep[j++] = info->singlestep[axis2];
	}
	/* now all selected axes have 1 in temp */
	for (i = 0; j < info->ndim; i++)
	  if (!temp[i]) {	/* this axis not yet included */
	    info->rdims[j] = info->dims[i];
	    info->raxes[j] = i;
	    info->rsinglestep[j++] = info->singlestep[i];
	  }
	/* fix info->iraxes */
	for (i = 0; i < info->ndim; i++)
	  info->iraxes[info->raxes[i]] = i;

	info->rndim = info->ndim;
	break;
    }
  } else {
    if (info->naxes) {		/* do have axes */
      /* just keep the original order */
      memcpy(info->rsinglestep, info->singlestep, info->ndim*sizeof(int32_t));
      memcpy(info->rdims, info->dims, info->ndim*sizeof(int32_t));
      info->rndim = info->ndim;
      for (i = 0; i < info->ndim; i++)
	info->raxes[i] = info->iraxes[i] = i;
    } else {			/* treat as 1D */
      info->rdims[0] = info->dims[0];
      for (i = 1; i < info->ndim; i++)
	info->rdims[0] *= info->dims[i];
      info->rndim = 1;
      info->rsinglestep[0] = 1;
      info->raxes[0] = 0;
      for (i = 0; i < info->ndim; i++)
	info->iraxes[i] = 0;
    }
  }
  
  /* prepare step sizes for use in advanceLoop() */
  memcpy(info->step, info->rsinglestep, info->rndim*sizeof(int32_t));
  for (i = info->rndim - 1; i; i--)
    info->step[i] -= info->step[i - 1]*info->rdims[i - 1];

  for (i = 0; i < info->rndim; i++)
    info->coords[i] = 0;	/* initialize coordinates */
  info->data->v = info->data0;	/* initialize pointer */
}
/*-----------------------------------------------------------------------*/
int32_t dimensionLoopResult1(loopInfo const *sinfo,
                         int32_t tmode, Symboltype ttype,
                         int32_t nMore, int32_t const * more,
                         int32_t nLess, int32_t const * less,
                         loopInfo *tinfo, Pointer *tptr)
/* create an appropriate result symbol
   <sinfo>: contains information about the loops through the source
   <tmode>: specifies desired result
     SL_ONEDIMS:   set omitted dimensions to 1 in the output.  If this
        option is not selected, then omitted dimensions are really
	not present in the output.  If due to such omissions no dimensions
        are left, then a scalar is returned as output symbol
     SL_SAMEDIMS:  output has same dimensions as source
     SL_COMPRESS:  as SL_SAMEDIMS, except that the first specified axis
        is omitted
     SL_COMPRESSALL: as SL_SAMEDIMS, but all specified axes are omitted
   <tinfo>: is filled with info about the output symbol
   <type>: the desired output type
   <tptr>: a pointer to a pointer to the target data
 */
{
  int32_t	target, n, i, ndim, dims[MAX_DIMS], naxes, axes[MAX_DIMS], j,
    nOmitAxes = 0, omitAxes[MAX_DIMS];
  Pointer	ptr;

  ndim = sinfo->ndim;		/* default */
  memcpy(dims, sinfo->dims, ndim*sizeof(*dims));
  for (i = 0; i < ndim; i++)
    omitAxes[i] = 0;
  naxes = sinfo->naxes;
  memcpy(axes, sinfo->axes, naxes*sizeof(*axes));
  /* it is assumed that 0 <= axes[i] < ndim for i = 0..naxes-1 */

  if (nLess && less) {
    if (nLess < 1 || nLess > naxes)
      return luxerror("Illegal number %d of dimensions to reduce:"
                      " expected 1..%d", -1, nLess, naxes);
    for (i = 0; i < nLess; i++) {
      if (less[i] < 1)
        return luxerror("Illegal reduction factor %d for axis %d",
                        -1, less[i], i);
      if (dims[axes[i]] % less[i])
        return luxerror("Reduction factor %d is not a divisor of dimension"
                        " %d size %d", -1, less[i], axes[i], dims[axes[i]]);
      dims[axes[i]] /= less[i];
      if (dims[axes[i]] == 1 && less[i] > 1) /* this dimension becomes 1 */
	omitAxes[nOmitAxes++] = i;
    }
#if UNNEEDED
    for ( ; i < naxes; i++) {
      if (dims[axes[i]] % less[nLess - 1])
        return luxerror("Reduction factor %d is not a divisor of dimension"
                        " %d size %d", -1, less[nLess - 1], axes[i],
                        dims[axes[i]]);
      dims[axes[i]] /= less[nLess - 1];
      if (dims[axes[i]] == 1 && less[i] > 1) /* this dimension becomes 1 */
	omitAxes[nOmitAxes++] = i;
    }
#endif
    if (!(tmode & SL_ONEDIMS) && nOmitAxes) { 
      /* remove dimensions corresponding to axes mentioned in omit[] */
      int32_t retain[MAX_DIMS];
      for (i = 0; i < ndim; i++)
	retain[i] = 1;
      for (i = 0; i < nOmitAxes; i++)
	retain[axes[omitAxes[i]]] = 0;
      /* now retain[i] says whether dimension i is to be retained.  If
	 retain[] = { 0, 1, 0, 1 }, then new dimension 0 corresponds
	 to old dimension 1, and new dimension 1 corresponds to old
	 dimension 3 */
      int32_t newIndexToOld[MAX_DIMS];
      int j;
      for (i = j = 0; i < ndim; i++)
	if (retain[i])
	  newIndexToOld[j++] = i;
      /* update dims[] and ndim */
      int32_t newvalues[MAX_DIMS];
      if (j) {
	for (i = 0; i < j; i++)
	  newvalues[i] = dims[newIndexToOld[i]];
	memcpy(dims, newvalues, j*sizeof(*dims));
	ndim = j;
      } else {
	/* there are no dimensions left; add a dimension equal to 1 */
	dims[0] = 1;
	ndim = 1;
      }
      /* update axes[] and naxes */
      for (i = 0; i < naxes; i++)
	retain[i] = 1;
      for (i = 0; i < nOmitAxes; i++)
	retain[omitAxes[i]] = 0;
      /* now retain[i] says whether axis i is to be retained */
      for (i = j = 0; i < naxes; i++)
	if (retain[i])
	  newIndexToOld[j++] = i;
      if (j) {
	for (i = 0; i < j; i++)
	  newvalues[i] = axes[newIndexToOld[i]];
	memcpy(axes, newvalues, j*sizeof(*axes));
	naxes = j;
      } else {
	/* there are no axes left; add an axis equal to 0 */
	axes[0] = 0;
	naxes = 1;
      }
    }
  }

  /* do things work out OK if nMore and nLess are nonzero at the same
     time? */

  if (nMore && more) {
    if (nMore < 1)
      return luxerror("Illegal number %d of dimensions to add", -1, nMore);
    if (nMore + ndim > MAX_DIMS)
      return luxerror("Requested total number of dimensions %d "
                    "exceeds allowed maximum %d", -1, nMore + ndim, MAX_DIMS);
    if (nMore + naxes > MAX_DIMS)
      return luxerror("Total number of axes %d after growing "
                    "exceeds allowed maximum %d", nMore + naxes, MAX_DIMS);
    for (i = 0; i < nMore; i++)
      if (more[i] < 1)
        return luxerror("Illegal size %d requested for new dimension %d", -1,
                      more[i], i);
    memmove(dims + nMore, dims, ndim*sizeof(*dims));
    memcpy(dims, more, nMore*sizeof(*dims));
    ndim += nMore;
    for (i = 0; i < naxes; i++) /* adjust axes for new dimensions */
      axes[i] += nMore;
    memmove(axes + nMore, axes, naxes*sizeof(*axes));
    for (i = 0; i < nMore; i++)
      axes[i] = i;
    naxes += nMore;
  }

  if (!less && !more) {
    switch (tmode & (SL_COMPRESS | SL_COMPRESSALL)) {
    case SL_COMPRESS:		/* output has same dimensions as source,
                                   except that first selected axis dimension
                                   is omitted -- or set to 1 */
    case SL_COMPRESSALL:	/* output has same dimensions as source,
                                   except that all selected axis dimensions
                                   are omitted -- or set to 1 */
      if ((tmode & (SL_COMPRESS | SL_COMPRESSALL)) == SL_COMPRESS)
	n = 1;			/* omit one axis only */
      else
	n = sinfo->naxes;	/* omit all axes */
      
      if (sinfo->axes) {	/* have specific axes */
        if (tmode & SL_ONEDIMS)  /* replace by dimension of 1 */
	  for (i = 0; i < n; i++)
	    dims[sinfo->axes[i]] = 1;
	else {			/* really omit */
	  if (sinfo->naxes	/* no fake 1D */
	      && ndim > n) {	/* and no dimensions left either */
	    for (i = 0; i < n; i++)
	      dims[sinfo->axes[i]] = 0; /* set omitted dims to 0 */
	    ndim -= n;		/* adjust number of dimensions */
	    i = 0;		/* now remove the zeros */
	    for (j = 0; j < ndim; i++)
	      if (dims[i])
		dims[j++] = dims[i];
	  } else 		/* it yields a single number -> scalar */
	    ndim = 0;		/* scalar */
	} /* end of if (mode & ... ) */
      } else {			/* assume all axes were specified */
	ndim -= n;		/* adjust number of dimensions */
	memcpy(dims, dims + n, ndim*sizeof(int32_t)); /* move up by <n> */
      }
      naxes -= n;
      if (naxes < 0)
	naxes = 0;
      memcpy(axes, axes + n, naxes*sizeof(int32_t));
      break;
    }
  }

  /* create the output symbol */
  if (ndim) {		/* get an array */
    target = array_scratch(ttype, ndim, dims);
    ptr.l = (int32_t *) array_data(target);
  } else {			/* get a scalar */
    if (isStringType(ttype)) {
      target = string_scratch(0);
      /* we must produce a NULL pointer, because that's what happens
         for string arrays, too, and the interface doesn't know
         whether we're dealing with a string array or a string. */
      free(string_value(target));
      string_value(target) = NULL;
      ptr.sp = &string_value(target);
    } else {
      target = scalar_scratch(ttype);
      if (isComplexType(ttype))
        ptr.cf = complex_scalar_data(target).cf;
      else
        ptr.l = &scalar_value(target).l;
    }
  }

  *tptr = ptr;			/* store pointer to output data */
  /* we must do this first so the correct value is stored in the data0
   element of tinfo. */

  /* fill loop structure for output symbol */
  setupDimensionLoop(tinfo, ndim, dims, ttype, naxes, axes, tptr, tmode);
  return target;
}
/*-----------------------------------------------------------------------*/
int32_t dimensionLoopResult(loopInfo const *sinfo, loopInfo *tinfo,
                            Symboltype ttype, Pointer *tptr)
{
  return dimensionLoopResult1(sinfo, sinfo->mode, ttype,
                              0, NULL, 0, NULL, tinfo, tptr);
}
/*-----------------------------------------------------------------------*/
/** Initiates a standard array loop.  advanceLoop() runs through the loop.

    \param data source data symbol, must be numerical

    \param axes a pointer to the list of axes to treat

    \param nAxes the number of axes to treat

    \param mode flags that indicate desired action; see below

    \param outType desired data type for output symbol

    \param src returns loop info for \p data

    \param srcptr pointer to a pointer to the data in \p data

    \param output returns created output symbol, if unequal to \c
    NULL on entry

    \param trgt returns loop info for \p output, if that is unequal
    to \c NULL on entry

    \param trgtptr pointer to a pointer to the data in the output symbol

   \p mode flags:

   - about the axes:
     - \c SL_ALLAXES: same as specifying all dimensions of \p data in
       ascending order in \p axes; the values of \p axes and \p nAxes
       are ignored.
     - \c SL_TAKEONED: take the \p data as one-dimensional; the values of
       \p axes and \p nAxes are ignored

   - about specified axes:
     - \c SL_NEGONED: if the user supplies a single axis and that axis
       is negative, then \p data is treated as if it were
       one-dimensional (yet containing all of its data elements)
     - \c SL_ONEAXIS:  only a single axis is allowed
     - \c SL_UNIQUEAXES: all specified axes must be unique (no duplicates)

   - about the input \p data:
     - \c SL_SRCUPGRADE: use a copy of \p data that is upgraded to \p
       outType if necessary

   - about the output symbol, if any:
     - \c SL_EXACT: must get exactly the data type indicated by \p
       outType (DEFAULT)
     - \c SL_UPGRADE: must get the data type of \p data or \p outType,
       whichever is greater
     - \c SL_KEEPTYPE: same type as input symbol
     - \c SL_ONEDIMS: omitted dimensions are not really removed but
       rather set to 1
     - \c SL_SAMEDIMS: gets the same dimensions as \p data (DEFAULT)
     - \c SL_COMPRESS: gets the same dimensions as \p data, except
       that the currently selected axis is omitted.  if no dimensions
       are left, a scalar is returned
     - \c SL_COMPRESSALL: must have the same dimensions as \p data,
       except that all selected axes are omitted.  if no dimensions
       are left, a scalar is retuned

   - about the desired axis order and coordinate availability:
     - \c SL_EACHCOORD: the currently selected axis goes first; the
       remaining axes come later, in their original order; the user
       gets access to all coordinates (DEFAULT)
     - \c SL_AXISCOORD: the currently selected axis goes first; the
       user gets access only to the coordinate along the indicated
       axis; remaining axes come later, lumped together as much as
       possible for faster execution
     - \c SL_AXESBLOCK: all specified axes go first in the specified
       order; the remaining axes come later, in their original order;
       the user gets access to all coordinates.  Implies \c
       SL_UNIQUEAXES.

   - about the coordinate treatment:
     - \c SL_EACHROW: the data is advanced one row at a time; the user
       must take care of advancing the data pointer to the beginning
       of the next row
     - \c SL_EACHBLOCK: like \c SL_EACHROW, but all selected axes
       together are considered to be a "row".  Implies \c
       SL_AXESBLOCK.
*/
int32_t standardLoop(int32_t data, int32_t axisSym, int32_t mode,
                     Symboltype outType, loopInfo *src, Pointer *srcptr,
                     int32_t *output, loopInfo *trgt, Pointer *trgtptr)
{
  int32_t i, nAxes;
  Pointer axes;

  if (axisSym > 0) {		/* <axisSym> is a regular symbol */
    if (!symbolIsNumerical(axisSym))
      return luxerror("Need a numerical argument", axisSym); /* <axisSym> was not numerical */
    i = lux_long(1, &axisSym);	/* get a LONG copy */
    numerical(i, NULL, NULL, &nAxes, &axes); /* get info */
  } else {
    nAxes = 0;
    axes.l = NULL;
  }
  return standardLoop0(data, nAxes, axes.l, mode, outType,
		       src, srcptr, output, trgt, trgtptr);

}
/*-----------------------------------------------------------------------*/
/* Like standardLoop but can produce a target like the source
   but with adjusted dimensions. LS 2011-07-22 */
int32_t standardLoopX(int32_t source, int32_t axisSym, int32_t srcMode,
                  loopInfo *srcinf, Pointer *srcptr,
                  int32_t nMore, int32_t const * more,
                  int32_t nLess, int32_t const * less,
                  Symboltype tgtType, int32_t tgtMode,
                  int32_t *target,
                  loopInfo *tgtinf, Pointer *tgtptr)
{
  int32_t i, nAxes;
  Pointer axes;

  if (axisSym > 0) {		/* <axisSym> is a regular symbol */
    if (!symbolIsNumerical(axisSym))
      return luxerror("Need a numerical argument", axisSym); /* <axisSym> was not numerical */
    i = lux_long(1, &axisSym);	/* get a LONG copy */
    numerical(i, NULL, NULL, &nAxes, &axes); /* get info */
  } else {
    nAxes = 0;
    axes.l = NULL;
  }
  int32_t result = standardLoop1(source, nAxes, axes.l, srcMode,
                             srcinf, srcptr, nMore, more, nLess, less,
                             tgtType, tgtMode, target, tgtinf, tgtptr);
  return result;
}
/*-----------------------------------------------------------------------*/
int32_t standardLoop0(int32_t data, int32_t nAxes, int32_t *axes,
                      int32_t mode, Symboltype outType,
                      loopInfo *src, Pointer *srcptr, int32_t *output,
                      loopInfo *trgt, Pointer *trgtptr)
{
  int32_t	*dims, ndim, i, temp[MAX_DIMS];

#if DEBUG_VOCAL
  debugout("in standardLoop()");
  debugout("checking <data>");
#endif
  /* check if <data> is of proper class, and get some info about it */
  if (numerical(data, &dims, &ndim, NULL, srcptr) == LUX_ERROR)
    return LUX_ERROR;		/* some error */

#if DEBUG_VOCAL
  debugout("treating SL_TAKEONED, SL_ALLAXES");
#endif
  if (mode & SL_TAKEONED)	/* take data as 1D */
    nAxes = 0;			/* treat as 1D */
  else if (mode & SL_ALLAXES) { /* select all axes */
    nAxes = ndim;
    axes = NULL;		/* treat as if all axes were specified */
  } else if ((mode & SL_NEGONED)	/* negative-axis treatment */
	     && nAxes == 1		/* one axis specified */
	     && *axes < 0)	/* and it is negative */
    nAxes = 0;

#if DEBUG_VOCAL
  debugout("treating SL_ONEAXIS");
#endif
  if ((mode & SL_ONEAXIS)	/* only one axis allowed */
      && nAxes > 1)		/* and more than one selected */
    return luxerror("Only one axis allowed", -1);

#if DEBUG_VOCAL
  debugout("checking axes for legality");
#endif
  /* check the specified axes for legality */
  if (nAxes && axes) {
    for (i = 0; i < nAxes; i++) /* check all specified axes */
      if (axes[i] < 0		/* axis is negative */
	  || axes[i] >= ndim)	/* or too great */
	return luxerror("Illegal axis %1d", -1, axes[i]);
    if (mode & SL_UNIQUEAXES) {	/* no axis must occur more than once */
      zerobytes(temp, ndim*sizeof(int32_t));
      for (i = 0; i < nAxes; i++)
	if (temp[axes[i]]++)
	  return luxerror("Axis %1d illegally specified more than once",
			  -1, axes[i]);
    }
  }

  /* The input is of legal classes and types. */

#if DEBUG_VOCAL
  debugout("treating SL_SRCUPGRADE");
#endif
  if ((mode & SL_SRCUPGRADE)	/* upgrade source if necessary */
      && (symbol_type(data) < outType))	{ /* source needs upgrading */
    data = lux_convert(1, &data, outType, 1); /* upgrade */
    numerical(data, NULL, NULL, NULL, srcptr);
  }

#if DEBUG_VOCAL
  debugout("calling setupDimensionLoop()");
#endif
  setupDimensionLoop(src, ndim, dims, symbol_type(data), nAxes,
		     axes, srcptr, mode);
#if DEBUG_VOCAL
  debugout("back from setupDimensionLoop()");
#endif

#if DEBUG_VOCAL
  debugout("generating output symbol (if requested)");
#endif
  if (output) {			/* user wants an output symbol */
    if (((mode & SL_UPGRADE)
	 && outType < src->type)
	|| (mode & SL_KEEPTYPE))
      trgt->type = src->type;	/* output type equal to source type */
    else
      trgt->type = outType;	/* take specified output type */

    *output = dimensionLoopResult(src, trgt, trgt->type, trgtptr);
    if (*output == LUX_ERROR)
      /* but didn't get one */
      return LUX_ERROR;
  }
  
#if DEBUG_VOCAL
  debugout("treating SL_TAKEONED");
#endif 
  if (mode & SL_TAKEONED) { /* mimic 1D array */
    src->dims[0] = src->rdims[0] = src->nelem;
    src->ndim = src->rndim = 1;
  }
#if DEBUG_VOCAL
  debugout("exiting standardLoop()");
#endif

  return LUX_OK;
}
#undef DEBUG_VOCAL
/*-----------------------------------------------------------------------*/
int32_t standardLoop1(int32_t source,
                  int32_t nAxes, int32_t const * axes,
                  int32_t srcMode,
                  loopInfo *srcinf, Pointer *srcptr,
                  int32_t nMore, int32_t const * more,
                  int32_t nLess, int32_t const * less,
                  Symboltype tgtType, int32_t tgtMode,
                  int32_t *target, 
                  loopInfo *tgtinf, Pointer *tgtptr)
/* initiates a standard array loop.  advanceLoop() runs through the loop.
   <source> (in): source data symbol, must be numerical
   <nAxes> (in): the number of axes to treat
   <axes> (in): a pointer to the list of axes to treat
   <srcMode> (in): flags that indicate desired action for source
   <srcinf> (out): loop info for source
   <srcptr> (out): pointer to the data in source
   <nMore> (in): the number of axes (out of <axes>) of which to
     increase the size
   <more> (in): a pointer to the list of axis size factors
   <nLess> (in): the number of axes (out of <axes>) of which to
     decrease the size
   <less> (in): a pointer to the list of axis size divisors
   <tgtType> (in): desired data type for newly created target data symbol
   <tgtmode> (in): flags that indicate desired action for target
   <target> (out): created target data symbol
   <tgtinf> (out): returns loop info for target
   <trgtptr> (out): pointer to the data in target

   if <target> is NULL, then no target-related output variables are
   filled in.  If any of the output variables are NULL, then those
   variables are not filled in.

   If <axes> is NULL, then <nAxes> is ignored.  If <axes> is not NULL,
   then it must point to an array of <nAxes> numbers indicating the
   axes (dimensions) of interest.  If any of those axes are negative
   or greater than or equal to the number of dimensions in <source>,
   then an error is declared.  If <nAxes> is less than 1, then an
   error is declared.  Further restrictions on the axis values may be
   specified through <srcMode>.

   By default, <target> gets the same dimensions as <source>.  This
   can be changed through <more>, <nMore>, <less>, <nLess>, and
   <tgtMode>.  <nMore> and <more> specify how many dimensions to add,
   and of what size.

   If <more> is NULL, then <nMore> is ignored.  If <nMore> is zero,
   then <more> is ignored.  If <more> is not NULL, then it must point
   to an array of <nMore> numbers indicating the sizes of the new
   dimensions to be added to <target>.  If any of those sizes are less
   than 1, then an error is declared.  If <nMore> is less than 1, then
   an error is declared.  The extra dimensions are prefixed to the
   existing ones, in the indicated order.

   If <less> is NULL, then <nLess> is ignored.  If <nLess> is zero,
   then <less> is ignored.  If <less> is not NULL, then it must point
   to an array of <nLess> numbers indicating by which factors the
   sizes of the indicated axes should be reduced.  The "indicated
   axes" are those from <axes>.  If <nLess> is greater than the number
   of indicated axes, then an error is declared.  If <nLess> is less
   than 1, then an error is declared.  If <nLess> is greater than 0
   but less than the number of indicated axes, then the last element
   of <less> is implicitly repeated as needed.  If the size of one of
   the corresponding axes of <source> is not a multiple of the
   corresponding number from <more>, then an error is declared.  If
   after reduction the size of the dimension is equal to 1, then that
   dimension may be omitted from the result, depending on the value of
   <tgtMode>.

   If the final number of dimensions in <target> (taking into account
   <more> and <less> and <tgtMode>) would exceed MAX_DIMS, then an
   error is declared.
    
   <srcMode> flags:

   about the axes:
   SL_ALLAXES: same as specifying all dimensions of <data> in ascending order
           in <axes>; the values of <axes> and <nAxes> are ignored.
   SL_TAKEONED: take the <data> as one-dimensional; the values of <axes>
           and <nAxes> are ignored

   about specified axes:
   SL_NEGONED:  if the user supplies a single axis and that axis is negative,
           then <data> is treated as if it were one-dimensional (yet
	   containing all of its data elements)
   SL_ONEAXIS:  only a single axis is allowed
   SL_UNIQUEAXES: all specified axes must be unique (no duplicates)

   about the input <data>:
   SL_SRCUPGRADE: use a copy of <data> which is upgraded to <tgtType> if
           necessary

   about the desired axis order and coordinate availability:
   SL_EACHCOORD: the currently selected axis goes first; the remaining
           axes come later, in their original order; the user gets
	   access to all coordinates (DEFAULT)
   SL_AXISCOORD: the currently selected axis goes first; the user gets access
	    only to the coordinate along the indicated axis; remaining axes
	    come later, lumped together as much as possible for faster
	    execution
   SL_AXESBLOCK: all specified axes go first in the specified order;
	    the remaining axes come later, in their original order; the user
	    gets access to all coordinates.  Implies SL_UNIQUEAXES.

   about the coordinate treatment:
   SL_EACHROW: the data is advanced one row at a time; the user must
	    take care of advancing the data pointer to the beginning of
	    the next row   
   SL_EACHBLOCK: like SL_EACHROW, but all selected axes together are
	    considered to be a "row".  Implies SL_AXESBLOCK.

   <tgtMode> flags:

   about the output symbol:
   SL_EXACT: must get exactly the data type indicated by <tgtType> (DEFAULT)
   SL_UPGRADE: must get the data type of <source> or <tgtType>, whichever is
           greater
   SL_KEEPTYPE: gets the same type as <source>

   SL_ONEDIMS: omitted dimensions are not really removed but rather set to 1

   SL_COMPRESS: gets the same dimensions as <data>, except that
           the currently selected axis is omitted.  If no dimensions
           are left, then a scalar is returned.  Ignored unless <less>
           is NULL.
   SL_COMPRESSALL: gets the same dimensions as <data>, except that all
           selected axes are omitted.  If no dimensions are left, then
           a scalar is returned.  Ignored unless <less> is NULL.

   about the desired axis order and coordinate availability:
   SL_EACHCOORD: the currently selected axis goes first; the remaining
           axes come later, in their original order; the user gets
	   access to all coordinates (DEFAULT)
   SL_AXISCOORD: the currently selected axis goes first; the user gets access
	    only to the coordinate along the indicated axis; remaining axes
	    come later, lumped together as much as possible for faster
	    execution
   SL_AXESBLOCK: all specified axes go first in the specified order;
	    the remaining axes come later, in their original order; the user
	    gets access to all coordinates.  Implies SL_UNIQUEAXES.

   about the coordinate treatment:
   SL_EACHROW: the data is advanced one row at a time; the user must
	    take care of advancing the data pointer to the beginning of
	    the next row   
   SL_EACHBLOCK: like SL_EACHROW, but all selected axes together are
	    considered to be a "row".  Implies SL_AXESBLOCK.
  */
{
  int32_t *dims;
  int32_t ndim, i, temp[MAX_DIMS];

  /* check if <source> is of proper class, and get some info about it */
  if (numerical_or_string(source, &dims, &ndim, NULL, srcptr) == LUX_ERROR)
    return LUX_ERROR;		/* some error */
  
  if (srcMode & SL_TAKEONED)	/* take data as 1D */
    nAxes = 0;			/* treat as 1D */
  else if (srcMode & SL_ALLAXES) { /* select all axes */
    nAxes = ndim;
    axes = NULL;		/* treat as if all axes were specified */
  } else if ((srcMode & SL_NEGONED)	/* negative-axis treatment */
	     && nAxes == 1		/* one axis specified */
	     && *axes < 0)	/* and it is negative */
    nAxes = 0;
  
  if ((srcMode & SL_ONEAXIS)	/* only one axis allowed */
      && nAxes > 1)		/* and more than one selected */
    return luxerror("Only one axis allowed", -1);

  /* check the specified axes for legality */
  if (nAxes && axes) {
    for (i = 0; i < nAxes; i++) /* check all specified axes */
      if (axes[i] < 0		/* axis is negative */
	  || axes[i] >= ndim)	/* or too great */
	return luxerror("Illegal axis %1d", -1, axes[i]);
    if (srcMode & SL_UNIQUEAXES) { /* no axis must occur more than once */
      zerobytes(temp, ndim*sizeof(int32_t));
      for (i = 0; i < nAxes; i++)
	if (temp[axes[i]]++)
	  return luxerror("Axis %1d illegally specified more than once",
			  -1, axes[i]);
    }
  }
  
  /* The input is of legal classes and types. */
    
  if ((srcMode & SL_SRCUPGRADE)	/* upgrade source if necessary */
      && (symbol_type(source) < tgtType))	{ /* source needs upgrading */
    source = lux_convert(1, &source, tgtType, 1); /* upgrade */
    numerical_or_string(source, NULL, NULL, NULL, srcptr);
  }

  setupDimensionLoop(srcinf, ndim, dims, symbol_type(source), nAxes,
		     axes, srcptr, srcMode);

  if (target) {			/* user wants an output symbol */
    if (((tgtMode & SL_UPGRADE)
	 && tgtType < srcinf->type)
	|| (tgtMode & SL_KEEPTYPE))
      tgtinf->type = srcinf->type; /* output type equal to source type */
    else
      tgtinf->type = tgtType;	/* take specified output type */
        
    *target = dimensionLoopResult1(srcinf, tgtMode, tgtinf->type, 
                                   nMore, more, nLess, less, tgtinf,
                                   tgtptr);
    if (*target == LUX_ERROR)
      /* but didn't get one */
      return LUX_ERROR;
  }
  
  if (srcMode & SL_TAKEONED) { /* mimic 1D array */
    srcinf->dims[0] = srcinf->rdims[0] = srcinf->nelem;
    srcinf->ndim = srcinf->rndim = 1;
  }

  return LUX_OK;
}
/*-----------------------------------------------------------------------*/
int32_t nextLoop(loopInfo *info)
/* rearranges dimensions for a next loop through the array; returns 1
 if OK, 0 if we're beyond the last specified axis */
{
  if (++(info->axisindex) >= info->naxes)
    return 0;
  rearrangeDimensionLoop(info);
  return LUX_OK;
}
/*-----------------------------------------------------------------------*/
int32_t nextLoops(loopInfo *info1, loopInfo *info2)
/* rearranges dimensions for a next loop through the arrays; returns 1
 if OK, 0 if we're beyond the last specified axis */
{
  if (++(info1->axisindex) >= info1->naxes)
    return 0;
  info2->axisindex++;
  rearrangeDimensionLoop(info1);
  rearrangeDimensionLoop(info2);
  return 1;
}
/*-----------------------------------------------------------------------*/
void subdataLoop(int32_t *range, loopInfo *src)
/* this routine selects less than the full source data set for treatment */
/* by modifying src->step[] and src->rdims[] and src->data */
/* <range> must contain the beginning and end of the range in each of
 the source coordinates.  This routine should only be used if
 no dimension compression has been applied. LS 30apr98 */
{
  int32_t	i, offset;

  offset = 0;
  for (i = 0; i < src->ndim; i++) {
    offset += range[2*i]*src->singlestep[i];
    src->rdims[src->raxes[i]] = range[2*i + 1] - range[2*i] + 1;
  }
  
  memcpy(src->step, src->rsinglestep, src->ndim*sizeof(int32_t));

  for (i = src->ndim - 1; i > 0; i--)
    src->step[i] -= src->step[i - 1]*src->rdims[i - 1];

  (*src->data).b = (uint8_t *) src->data0 + offset*src->stride;
}
/*--------------------------------------------------------------------*/
void rearrangeEdgeLoop(loopInfo *src, loopInfo *trgt, int32_t index)
/* modifies <src> and <trgt> for walking along only the outer edges of the
   data set.  For this purpose, edge number <index> consists of all data
   points that have rearranged coordinate number <index>/2 equal to
   0 (if <index>%2 == 0) or to <src>->rdims[index/2] - 1 (if <index>%2 == 1).
   If <trgt> is not equal to NULL, then it is modified the same as <src>.
   In that case it is assumed that <trgt> contains the same dimensions
   and axes as <src>.  It is also assumed that <index> has a legal value,
   i.e., between 0 and 2*<src>->ndim - 1, and that <src>->data0 and
   <trgt>->data0 point at the start of their respective data.  LS 23oct98
   LS 2feb99 */
{
  uint8_t	back;
  int32_t	axis, trgtstride, i;
  Pointer	*trgtdata;
  void	*trgtdata0;

  back = index % 2;		/* 0 -> front edge, 1 -> back edge */
  index /= 2;			/* the target axis */
  axis = src->axes[index];
    
  /* put target dimension in the back; get rearranged step sizes */
  memcpy(src->rsinglestep, src->singlestep, axis*sizeof(int32_t));
  memcpy(src->rsinglestep + axis, src->singlestep + axis + 1,
	 (src->ndim - axis - 1)*sizeof(int32_t));
  src->rsinglestep[src->ndim - 1] = src->singlestep[axis];
    
  /* get rearranged dimensions */
  src->rndim = src->ndim;
  memcpy(src->rdims, src->dims, axis*sizeof(int32_t));
  memcpy(src->rdims + axis, src->dims + axis + 1,
	 (src->ndim - axis - 1)*sizeof(int32_t));
  src->rdims[src->ndim - 1] = src->dims[axis];

  /* get mappings between original and rearranged axes */
  for (i = 0; i < axis; i++)
    src->raxes[i] = i;
  for (i = axis; i < src->ndim - 1; i++)
    src->raxes[i] = i + 1;
  src->raxes[src->ndim - 1] = axis;
  for (i = 0; i < src->ndim; i++)
    src->iraxes[src->raxes[i]] = i;

  /* prepare step sizes for use in advanceLoop() */
  memcpy(src->step, src->rsinglestep, src->rndim*sizeof(int32_t));
  for (i = src->rndim - 1; i; i--)
    src->step[i] -= src->step[i - 1]*src->rdims[i - 1];

  zerobytes(src->coords, src->ndim*sizeof(int32_t));
  src->data->b = (uint8_t*) src->data0;

  if (back) {
    /* adjust coordinate and pointer to point at back side */
    index = src->ndim - 1;
    src->coords[index] = src->rdims[index] - 1;
    src->data->b += src->coords[index]*src->rsinglestep[index]*src->stride;
  }

  if (trgt) {
    trgtstride = trgt->stride;
    trgtdata = trgt->data;
    trgtdata0 = trgt->data0;
    *trgt = *src;
    trgt->stride = trgtstride;
    trgt->data = trgtdata;
    trgt->data0 = trgtdata0;
    trgt->data->b = (uint8_t*) trgt->data0;
    if (back) {
      /* adjust coordinate and pointer to point at back side */
      trgt->coords[index] = trgt->rdims[index] - 1;
      trgt->data->b += trgt->coords[index]*trgt->rsinglestep[index]*trgt->stride;
    }
  }
}
/*--------------------------------------------------------------------*/
int32_t prepareDiagonals(int32_t symbol, loopInfo *info, int32_t part,
		     int32_t **offset, int32_t **edge, int32_t **rcoord, int32_t **diagonal)
/* takes the numerical array <symbol> that specifies which connections
   to nearest neighbors to accept and calculates various associated
   numbers.  The target array dimensions are taken from <info>.
   <symbol> must contain one number for each dimension of the target
   array.  Currently, these numbers may have values 0, 1, or 2.

   0: neighbors in this dimension are not recognized.

   1: neighbors in this dimension are only recognized as such if they
   share a face.  I.e., if an allowed direction has a non-zero
   component in this dimension, then that component is the *only*
   non-zero component of that direction.

   2: neighbors in this dimension are recognized as such if they share
   a face or a vertex.

   If <symbol> is equal to zero, then ONE(LONARR(info->ndim))*2 is
   assumed for it; i.e., all directions are allowed.

   If <offset> is non-zero, then it is made to point at an array
   containing the offsets from the central position to service all of
   the allowed diagonals, based on the array dimensions taken from
   <info->dims>.  Memory for this array is allocated by the routine.

   If <edge> is non-zero, then it is made to point at an array with
   two elements for each dimension; the first of each pair refers to
   the near edge (with the appropriate coordinate equal to 0) and the
   second one to the far edge (with the appropriate coordinate equal
   to info.dims[] - 1).  Each of these numbers is set to 1 if one or
   more of the offsets point at the corresponding edge, and to 0
   otherwise.  Memory for this array is allocated by the routine.

   If <rcoord> is non-zero, then it is made to point at an array with
   one set of coordinates for each direction (corresponding to one
   member of <offset>), with each number in each set equal to the
   offset expressed in the corresponding dimension.  All elements of
   <rcoord> are equal to -1, 0, or +1.  Memory for this array is
   allocated by the routine.
   
   If <diagonal> is non-zero, then in it is returned a pointer to the
   start of the list of diagonal code numbers (i.e., the contents of
   <symbol>) -- or a pointer to NULL if <symbol> is equal to 0.

   The user must free <offset>, <edge>, and <rcoord> when the user is
   done with them.  if you want to reach all allowed neighbors, then
   set <part> equal to 1; if you want to service all allowed
   directions but do not distinguish between diametrically opposite
   directions, then set <part> equal to 2.  Returns the number of
   offsets, or LUX_ERROR if an error occurred.  LS 10feb99 */
{
  int32_t	i, j, *d, nDiagonal, nDoDim, n, n0, n1, n2, k;

  if (symbol) {			/* have <diagonal> */
    if (symbol_class(symbol) != LUX_ARRAY)
      return cerror(NEED_ARR, symbol);
    if (array_size(symbol) != info->ndim)
      return cerror(INCMP_ARG, symbol);
    i = lux_long(1, &symbol);	/* ensure LONG */
    d = (int32_t*) array_data(i);
    nDiagonal = nDoDim = 0;
    for (i = 0; i < info->ndim; i++)
      if (d[i]) {
	nDoDim++;		/* # dimensions that are considered
				 at all */
	if (d[i] != 1)
	  nDiagonal++;		/* # dimensions that allow diagonal
				   links */
      }
  } else {
    d = NULL;
    nDiagonal = nDoDim = info->ndim;
  }

  /* now calculate the number of directions to treat;
     equal to (3^nDiagonal - 1)/2 + nDoDim - nDiagonal */
  n = 1;
  for (i = 0; i < nDiagonal; i++)
    n *= 3;
  n = ((n - 1) + 2*(nDoDim - nDiagonal))/part;
  /* we divide by <part> so we can select either all allowed neighbors
     (part = 1) or all unique directions through the central point (part = 2).
     LS 10feb99 */

  if (offset) {
    *offset = (int32_t *) malloc(n*sizeof(int32_t)); /* offsets to elements to be 
						investigated */
    if (!*offset)
      return cerror(ALLOC_ERR, 0);
  }
  if (edge) {
    *edge = (int32_t *) malloc(info->ndim*2*sizeof(int32_t));
    if (!*edge)
      return cerror(ALLOC_ERR, 0);
    zerobytes(*edge, info->ndim*2*sizeof(int32_t));
  }
  if (rcoord) {
    *rcoord = (int32_t *) malloc(n*info->ndim*sizeof(int32_t));
    if (!*rcoord)
      return cerror(ALLOC_ERR, 0);
  }
  
  /* calculate offsets to elements to be investigated */
  /* we need to treat n directions */
  for (i = 0; i < info->ndim; i++)
    info->coords[i] = 0;
  info->coords[0] = 1;
  
  n0 = n1 = 0;
  n2 = 1;			/* defaults for when diagonal == 0 */
  for (k = 0; k < n; ) {
    if (d) {
      n0 = n1 = n2 = 0;
      for (i = 0; i < info->ndim; i++)
	if (info->coords[i])
	  switch (d[i]) {
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
      if (offset)
	(*offset)[k] = 0;
      for (j = 0; j < info->ndim; j++) {
	if (offset)
	  (*offset)[k] += info->rsinglestep[j]*info->coords[j];
	if (edge) {
	  if (info->coords[j] > 0) {
	    (*edge)[2*j + 1] = 1;
	    if (part == 2)
	      (*edge)[2*j] = 1;
	  } else if (info->coords[j] < 0)
	    (*edge)[2*j] = 1;
	}
      }
      if (rcoord)
	memcpy(*rcoord + k*info->ndim, info->coords, info->ndim*sizeof(int32_t));
      k++;
    }
    /* go to next direction. */
    for (j = 0; j < info->ndim; j++) {
      info->coords[j]++;
      if (info->coords[j] <= 1)
	break;
      for (i = 0; i <= j; i++)
	info->coords[i] = -1;
    }
  }

  if (diagonal)
    *diagonal = d;

  /* now that we have everything we need, we clean up */
  zerobytes(info->coords, info->ndim*sizeof(int32_t));
  return n;
}
/*--------------------------------------------------------------------*/
int32_t moveLoop(loopInfo *info, int32_t index, int32_t distance)
/* moves along rearranged axis number <index> over the indicated <distance>, */
/* updating the coordinates and pointers in <info>.  <distance> may be
   negative and may have any magnitude.  If <index> points at a non-existent
   axis, then no adjustments are made.  If <index> is negative, then
   <info->rndim> is returned.  If <index> is greater than or equal to
   <info->rndim>, then <index> + 1 is returned.
   Otherwise, returns the index of the last affected dimension.
   LS 9apr99 */
{
  int32_t	i;

  if (index < 0)		/* illegal axis; don't do anything */
    return info->rndim;
  if (index >= info->rndim)	/* illegal axis; don't do anything */
    return index + 1;
  /* adjust data pointer */
  info->data->b += distance*info->rsinglestep[index]*info->stride;
  /* adjust the coordinate */
  info->coords[index] += distance;
  /* the new coordinate may be out of range; either negative or greater */
  /* than the dimension */
  if (info->coords[index] >= info->rdims[index]) {/* too great */
    i = info->coords[index];
    do {
      i /= info->rdims[index];	/* carry */
      info->coords[index] -= i*info->rdims[index]; /* carry over */
      if (++index == info->rndim)
	break;
      info->coords[index] += i;
      info->data->b += i*info->step[index]*info->stride;
    } while (i && index < info->rndim);
  } else if (info->coords[index] < 0) {	/* negative */
    i = info->coords[index];
    do {
      i = (i + 1)/info->rdims[index] - 1;
      info->coords[index] -= i*info->rdims[index];
      if (++index == info->rndim)
	break;
      info->coords[index] += i;
      info->data->b += i*info->step[index]*info->stride;
    } while (info->coords[index] < 0);
  }
  return index;
}
/*--------------------------------------------------------------------*/
void returnLoop(loopInfo *info, Pointer *ptr, int32_t index)
/* moves to the start of the rearranged axis indicated by <index>, */
/* zeroing all rearranged coordinates up to and including that one and */
/* adjusting the pointer accordingly.  LS 9apr99 */
{
  int32_t	i;
  
  for (i = 0; i <= index; i++) {
    ptr->b -= info->coords[index]*info->rsinglestep[index]*info->stride;
    info->coords[index] = 0;
  }
}
/*--------------------------------------------------------------------*/
static int32_t numerical_or_string_choice(int32_t data, int32_t **dims, int32_t *nDim, int32_t *size, Pointer *src, int32_t string_is_ok)
/* checks that <data> is of numerical type and returns dimensional */
/* information in <*dims>, <*nDim>, and <*size> (if these are non-zero), */
/* and a pointer to the data in <*src>.  If <*dims> is non-null, then
   the user must have made sure it points at properly reserved memory
   space. LS 21apr97 */
{
  static int32_t	one = 1;

  switch (symbol_class(data)) {
  default:
      return LUX_ERROR;         /* no message, because not always wanted */
  case LUX_SCAL_PTR:
    data = dereferenceScalPointer(data);
    /* fall-thru */
  case LUX_SCALAR:
    if (dims) 
      *dims = &one;
    if (nDim)
      *nDim = 1;
    if (size)
      *size = 1;
    if (src)
      (*src).l = &scalar_value(data).l;
    break;
  case LUX_CSCALAR:
    if (dims)
      *dims = &one;
    if (nDim)
      *nDim = 1;
    if (size)
      *size = 1;
    if (src)
      (*src).cf = complex_scalar_data(data).cf;
    break;
  case LUX_STRING:
    if (dims)
      *dims = &one;
    if (nDim)
      *nDim = 1;
    if (size)
      *size = 1;
    if (src)
      (*src).sp = &string_value(data);
    break;
  case LUX_ARRAY: case LUX_CARRAY:
    if (dims)
      *dims = array_dims(data);
    if (nDim)
      *nDim = array_num_dims(data);
    if (size)
      *size = array_size(data);
    if (src)
      (*src).l = (int32_t*) array_data(data);
    break;
  }
  return 1;
}
/*---------------------------------------------------------------------*/
int32_t numerical(int32_t data, int32_t **dims, int32_t *nDim, int32_t *size, Pointer *src)
/* checks that <data> is of numerical type and returns dimensional */
/* information in <*dims>, <*nDim>, and <*size> (if these are non-zero), */
/* and a pointer to the data in <*src>.  If <*dims> is non-null, then
   the user must have made sure it points at properly reserved memory
   space. LS 21apr97 */
{
  return numerical_or_string_choice(data, dims, nDim, size, src, 0);
}
/*---------------------------------------------------------------------*/
int32_t numerical_or_string(int32_t data, int32_t **dims, int32_t *nDim, int32_t *size, Pointer *src)
/* checks that <data> is of numerical or string type and returns
   dimensional information in <*dims>, <*nDim>, and <*size> (if these
   are non-zero), and a pointer to the data in <*src>.  If <*dims> is
   non-null, then the user must have made sure it points at properly
   reserved memory space. LS 21apr97 */
{
  return numerical_or_string_choice(data, dims, nDim, size, src, 1);
}
/*--------------------------------------------------------------------*/
void standard_redef_array(int32_t iq, Symboltype type,
			  int32_t num_dims, int32_t *dims, 
			  int32_t naxes, int32_t *axes,
			  Pointer *ptr, loopInfo *info)
{
  redef_array(iq, type, num_dims, dims);
  ptr->v = array_data(iq);
  setupDimensionLoop(info, num_dims, dims, type, naxes, axes, ptr, info->mode);
}
/*--------------------------------------------------------------------*/
void free_param_spec_list(struct param_spec_list *psl)
{
  if (psl) {
    size_t i;
    for (i = 0; i < psl->num_param_specs; i++) {
      struct param_spec *p = &psl->param_specs[i];
      free(p->dims_spec);
    }
    free(psl->param_specs);
    free(psl);
  }
}

struct param_spec_list *parse_standard_arg_fmt(char const *fmt)
{
  struct obstack ops, ods;
  struct param_spec_list *psl = NULL;
  struct param_spec *ps = NULL;
  struct dims_spec *ds = NULL;
  size_t i, prev_ods_num_elem;
  int32_t return_param_index = -1;
  int32_t param_index;
  char const *fmt0 = fmt;

  if (!fmt || !*fmt)
    return NULL;

  obstack_init(&ops);
  obstack_init(&ods);
  param_index = 0;
  prev_ods_num_elem = 0;
  int bad = 0;
  while (*fmt) {
    struct param_spec p_spec;
    memset(&p_spec, '\0', sizeof(p_spec));

    while (*fmt && *fmt != ';') { /* every parameter specification */
      /* required parameter kind specification */
      switch (*fmt) {
      case 'i':
        p_spec.logical_type = PS_INPUT;
        break;
      case 'o':
        p_spec.logical_type = PS_OUTPUT;
        break;
      case 'r':
        p_spec.logical_type = PS_RETURN;
        if (return_param_index >= 0) {
          /* already had a return parameter */
          luxerror("Specified multiple return parameters", 0);
          errno = EINVAL;
          bad = 1;
          break;
        } else
          return_param_index = param_index;
        break;
      default:
        /* illegal parameter kind specification */
        luxerror("Illegal parameter kind %d specified", 0, *fmt);
        errno = EINVAL;
        bad = 1;
        break;
      } /* end of switch (*fmt) */
      fmt++;

      /* optional data type limit specification */
      switch (*fmt) {
      case '>':
        p_spec.data_type_limit = PS_LOWER_LIMIT;
        fmt++;
        break;
      default:
        p_spec.data_type_limit = PS_EXACT;
        break;
      } /* end of switch (*fmt) */

      /* optional data type specification */
      switch (*fmt) {
      case 'B':
        p_spec.data_type = LUX_INT8;
        fmt++;
        break;
      case 'W':
        p_spec.data_type = LUX_INT16;
        fmt++;
        break;
      case 'L':
        p_spec.data_type = LUX_INT32;
        fmt++;
        break;
      case 'Q':
        p_spec.data_type = LUX_INT64;
        fmt++;
        break;
      case 'F':
        p_spec.data_type = LUX_FLOAT;
        fmt++;
        break;
      case 'D':
        p_spec.data_type = LUX_DOUBLE;
        fmt++;
        break;
      case 'S':
        p_spec.data_type = LUX_TEMP_STRING;
        fmt++;
        break;
      default:
        p_spec.data_type = LUX_NO_SYMBOLTYPE;
        break;
      } /* end of switch (*fmt) */

      if (*fmt == '^') {
        p_spec.common_type = true;
        ++fmt;
      } else {
        p_spec.common_type = false;
      }

      /* optional dims-specs */
      struct dims_spec d_spec;
      if (*fmt == '[') {       /* reference parameter specification */
        fmt++;
        if (*fmt++ == '-')
          p_spec.ref_par = -1;  /* point at previous parameter */
        else if (isdigit(*fmt)) { /* a specific parameter */
          char *p;
          p_spec.ref_par = strtol(fmt, &p, 10);
        } else {
          luxerror("Expected a digit or minus sign after [ in"
                   " reference parameter specification but found %c", 0, *fmt);
          errno = EINVAL;
          bad = 1;
          break;
        }
        if (*fmt == ']')
          fmt++;
        else {
          luxerror("Expected ] instead of %c at end of reference "
                   "parameter specification", 0, *fmt);
          errno = EINVAL;
          bad = 1;
          break;
        }
      }
      if (*fmt == '{') {   /* optional axis parameter specification */
	if (p_spec.logical_type == PS_INPUT) {
	  luxerror("Axis parameter illegally specified for input parameter",
		   0, fmt);
	  errno = EINVAL;
          bad = 1;
          break;
	}
        fmt++;
        if (*fmt++ == '-')
          p_spec.axis_par = -1;  /* point at previous parameter */
        else if (isdigit(*fmt)) { /* a specific parameter */
          char *p;
          p_spec.axis_par = strtol(fmt, &p, 10);
        } else {
          luxerror("Expected a digit or minus sign after { in"
                   " reference parameter specification but found %c", 0, *fmt);
          errno = EINVAL;
          bad = 1;
          break;
        }
        if (*fmt == '}')
          fmt++;
        else {
          luxerror("Expected } instead of %c at end of reference "
                   "parameter specification", 0, *fmt);
          errno = EINVAL;
          bad = 1;
          break;
        }
      } else
	p_spec.axis_par = -2;		     /* indicates "none" */
      if (bad)
        break;
      while (*fmt && !strchr("*?;&#", *fmt)) { /* all dims */
        memset(&d_spec, '\0', sizeof(d_spec));
        while (*fmt && !strchr(",?*;&#", *fmt)) { /* every dim */
          dim_spec_type type = (dim_spec_type) 0;
          size_t size = 0;
          switch (*fmt) {
          case '+':
            type = DS_ADD;
            fmt++;
            break;
          case '-':
            type = DS_REMOVE;
            fmt++;
            break;
          case '=':
            type = DS_COPY_REF;
            fmt++;
            break;
          case ':':
            type = DS_ACCEPT;
            break;
          case '>':
            type = DS_ATLEAST;
            fmt++;
            break;
          default:
            type = DS_EXACT;
            break;
          } /* end of switch (*fmt) */
          if (isdigit(*fmt)) {
            char *p;
            size = strtol(fmt, &p, 10);
            fmt = p;
          }
          switch (type) {
          case DS_ADD:
            if (d_spec.type == DS_NONE || d_spec.type == DS_REMOVE) {
              d_spec.size_add = size;
              d_spec.type = (dim_spec_type) (d_spec.type | type);
            } else {
              luxerror("Illegal combination of multiple types for dimension; parameter specification #%d: %s", 0, param_index + 1, fmt0);
              errno = EINVAL;
              bad = 1;
              break;
            }
            break;
          case DS_REMOVE:
            if (d_spec.type == DS_NONE || d_spec.type == DS_ADD) {
              d_spec.size_remove = size;
              d_spec.type = (dim_spec_type) (d_spec.type | type);
            } else {
              luxerror("Illegal combination of multiple types for dimension; parameter specification #%d: %s", 0, param_index + 1, fmt0);
              errno = EINVAL;
              bad = 1;
              break;
            }
            break;
          default:
            if (d_spec.type == DS_NONE || d_spec.type == DS_ATLEAST) {
              d_spec.size_add = size;
              d_spec.type = type;
            } else {
              luxerror("Illegal combination of multiple types for dimension; parameter specification #%d: %s", 0, param_index + 1, fmt0);
              errno = EINVAL;
              bad = 1;
              break;
            }
            break;
          } /* end switch type */
          if (bad)
            break;
        } /* end of while *fmt && !strchr(",*;&") */
        if (bad)
          break;
        obstack_grow(&ods, &d_spec, sizeof(d_spec));
        if (*fmt == ',')
          ++fmt;
      } /* end of while *fmt && !strchr("*;&", *fmt) */
      if (bad)
        break;
      switch (*fmt) {
      case '*':
        p_spec.remaining_dims = PS_ARBITRARY;
        ++fmt;
        break;
      case '&':
        p_spec.remaining_dims = PS_EQUAL_TO_REFERENCE;
        ++fmt;
        break;
      case '#':
        p_spec.remaining_dims = PS_ONE_OR_EQUAL_TO_REFERENCE;
        ++fmt;
        break;
        /* default is PS_ABSENT */
      }

      if (*fmt == '?') {        /* optional argument */
        if (p_spec.logical_type == PS_RETURN) {
          /* return parameter cannot be optional */
          luxerror("Return parameter was illegally specified as optional", 0);
          errno = EINVAL;
          bad = 1;
          break;
        } else
          p_spec.is_optional = 1;
        fmt++;
      } else
        p_spec.is_optional = 0;
      if (bad)
        break;
      if (*fmt && *fmt != ';') {
        luxerror("Expected ; instead of %c at end of parameter "
                 "specification", 0, *fmt);
        errno = EINVAL;
        bad = 1;
        break;
      }
      if (bad)
        break;
      /* determine the number of dims_specs added to the list for this
         parameter */
      size_t n = obstack_object_size(&ods)/sizeof(struct dims_spec) - prev_ods_num_elem;
      p_spec.num_dims_spec = n;
      p_spec.dims_spec = NULL;                     /* will be filled in later */
      obstack_grow(&ops, &p_spec, sizeof(p_spec)); /* the param_spec */
      prev_ods_num_elem += n;
    } /* end of while (*fmt && *fmt != ';') */
    if (bad)
      break;
    if (*fmt == ';')
      fmt++;
    else if (*fmt) {
      /* unexpected character */
      luxerror("Expected ; instead of %c at end of parameter specification",
               0, *fmt);
      errno = EINVAL;
      bad = 1;
      break;
    }
    param_index++;
  }   /* end of while (*fmt) */
  if (!bad) {
    /* now we copy the information into the final allocated memory */
    psl = (param_spec_list*) malloc(sizeof(struct param_spec_list));
    if (!psl) {                   /* malloc sets errno */
      cerror(ALLOC_ERR, 0);
      bad = 1;
    }
  }
  if (!bad) {
    psl->param_specs = (param_spec*) calloc(param_index, sizeof(struct param_spec));
    if (!psl->param_specs) {        /* malloc sets errno */
      cerror(ALLOC_ERR, 0);
      bad = 1;
    }
  }
  if (!bad) {
    /* the return parameter, if any, gets moved to the end */
    psl->num_param_specs = param_index;
    psl->return_param_index = -1; /* default, may be updated later */
    ps = (param_spec*) obstack_finish(&ops);
    ds = (dims_spec*) obstack_finish(&ods);

    struct param_spec *pstgt;
    size_t ds_ix, j;

    pstgt = psl->param_specs;
    ds_ix = 0;
    for (i = j = 0; i < psl->num_param_specs; i++) {
      size_t j0;
      if (i == return_param_index) {
        j0 = j;
        j = psl->return_param_index = psl->num_param_specs - 1;
      }
      memcpy(pstgt + j, ps + i, sizeof(struct param_spec));
      if (pstgt[j].num_dims_spec) {
        size_t size = pstgt[j].num_dims_spec*sizeof(struct dims_spec);
        pstgt[j].dims_spec = (dims_spec*) malloc(size);
        if (!pstgt[j].dims_spec) {
          cerror(ALLOC_ERR, 0);
          bad = 1;
          break;
        }
        memcpy(pstgt[j].dims_spec, ds + ds_ix, size);
        ds_ix += pstgt[j].num_dims_spec;
      } /* else pstgt[j].dims_spec == NULL */
      if (i == return_param_index)
        j = j0;
      else
        j++;
    }
  }
  if (!bad) {
    /* check that the reference parameter does not point outside the list */
    int32_t n = psl->num_param_specs - (psl->return_param_index >= 0);
    if (n) {
      for (i = 0; i < psl->num_param_specs; i++) {
        if (psl->param_specs[i].ref_par >= n) {
          errno = EINVAL;
          luxerror("Reference parameter %d for parameter %d points outside of the list (size %d)", 0, psl->param_specs[i].ref_par + 1, i + 1, n);
          bad = 1;
          break;
        }
      }
    }
  }
  obstack_free(&ops, NULL);
  obstack_free(&ods, NULL);
  if (bad) {
    free_param_spec_list(psl);
    psl = NULL;
  }
  return psl;
}

Symboltype standard_args_common_symboltype(int32_t num_param_specs,
                                           param_spec* param_specs,
                                           int32_t narg,
                                           int32_t* ps)
{
  Symboltype common_type = LUX_NO_SYMBOLTYPE;

  for (int param_ix = 0; param_ix < num_param_specs; ++param_ix) {
    if (param_ix >= narg)
      break;
    param_spec* pspec = &param_specs[param_ix];
    if (pspec->common_type) {
      int32_t iq = ps[param_ix];
      Symboltype type;
      if (pspec->logical_type == PS_INPUT)
        type = symbol_type(iq);
      else
        type = LUX_NO_SYMBOLTYPE;
      if ((pspec->data_type_limit == PS_LOWER_LIMIT
           && (type == LUX_NO_SYMBOLTYPE || type < pspec->data_type))
          || (pspec->data_type_limit == PS_EXACT
              && pspec->data_type != LUX_NO_SYMBOLTYPE
              && type != pspec->data_type))
        type = pspec->data_type;
      if (common_type == LUX_NO_SYMBOLTYPE
          || type < common_type)
        common_type = type;
    }
  }
  return common_type;
}

/** Prepares for looping through input and output variables based on a
    standard arguments specification.

    \param [in] narg is the number of arguments in `ps`

    \param [in] ps is the array of arguments

    \param [in] fmt is the arguments specification in standard format
    (see below)

    \param [out] ptrs is the address of a pointer in which is returned
    `NULL` (in case of a problem) or a pointer to a freshly allocated
    list of pointers, one for each argument (whether mandatory or
    optional) and possibly one for the return symbol.  Memory for the
    list is allocated using `malloc`.  The user is responsible for
    `free`ing the memory when it is no longer needed.

    \param [out] infos is the address of a pointer in which is
    returned `NULL` (in case of a problem) or a pointer to a list of
    loop information structures, one for each argument (mandatory or
    optional) and possibly one for the return symbol.  Memory for the
    list is allocated using `malloc`.  The user is responsible for
    `free`ing the memory when it is no longer needed.

    \return the return symbol, or -1 if an error occurred.

    The standard format is schematically as follows, where something
    between quotes ‘’ stands for that literal character, something
    between non-literal square brackets [] is optional, something
    between non-literal curly braces {} is a group of alternatives, a
    non-literal pipe symbol | separates alternatives, and a
    non-literal asterisk * indicates repetition zero or more times:

    \verbatim
    <format> = <param-spec>[;<param-spec>]*
    <param-spec> = {‘i’|‘o’|‘r’}[<type-spec>][<dims-spec>][‘?’]
    <type-spec> = {{[‘>’]{‘B’|‘W’|‘L’|'Q'|‘F’|‘D’}}|‘S’}['^']
    <dims-spec> = [‘[’<ref-par>‘]’][‘{’<axis-par>‘}’]
                  <dim-spec>[,<dim-spec>]*[‘*’|‘&’|'#']
    <dim-spec> = [{‘+’NUMBER|‘-’|‘-’NUMBER|‘=’|‘=’NUMBER|‘:’}]*
    \endverbatim

    In words,
    - the format consists of one or more parameter specifications
      separated by semicolons ‘;’.
    - each parameter specification begins with an ‘i’, ‘o’, or ‘r’,
      optionally followed by a question mark ‘?’, followed by a type
      specification and a dimensions specification.
    - a type specification consists of an ‘S', or else of an optional
      greater-than sign ‘>’ followed by one of ‘B’, ‘W’, ‘L’, 'Q',
      ‘F’, or ‘D’.  Optionally, a '^' follows.
    - a dimensions specification consists of a optional reference
      parameter number between square brackets ‘[]’, followed by an
      optional axis parameter number between curly braces ‘{}’,
      followed by one or more dimension specifications separated by
      commas ‘,’, optionally followed by an asterisk ‘*’ or ampersand
      ‘&’ or hash symbol '#'.
    - a dimension specification consists of a minus sign ‘-’, an
      equals sign ‘=’, a colon ‘:’, or a number preceded by a plus
      sign ‘+’, a minus sign ‘-’, or an equals sign ‘=’; followed by
      any number of additional instances of the preceding.

    Some of the characteristics of a parameter may depend on those of
    a reference parameter.  That reference parameter is the very first
    parameter (parameter \c ps[0]) unless a different reference
    parameters is specified at the beginning of the dimension
    specification.

    For the parameter specification \c param-spec:
    - ‘i’ = input parameter.
    - ‘o’ = output parameter.  An error is declared if this is not a
      named parameter.
    - ‘r’ = return value.  For functions, there must be exactly one of
      these in the parameters specification.  Subroutines must not
      have one of these.
    - ‘?’ = optional parameter.

    For the type specification \c type-spec:
    - ‘>’ = the type should be at least equal to the indicated type.
    - ‘B’ = LUX_INT8
    - ‘W’ = LUX_INT16
    - ‘L’ = LUX_INT32
    - 'Q' = LUX_INT64
    - ‘F’ = LUX_FLOAT
    - ‘D’ = LUX_DOUBLE
    - ‘S’ = LUX_STRING
    - '^' = all numerical parameters marked like this get the same
      data type, which is the greatest numerical data type among them
      that would have applied if no '^' had been specified.

    For input parameters, a copy is created with the indicated
    (minimum) type if the input parameter does not meet the condition,
    and further processing is based on that copy.  For output
    parameters, an array is created with the indicate type, unless ‘>’
    is specified and the reference parameter has a greater type, in
    which case that type is used.

    For the reference parameter \c ref-par:
    - If absent, then 0 is taken for it (i.e., the first parameter).
    - If a number, then the indicated parameter is taken for it.
    - If ‘-’, then the previous parameter is taken for it.

    For the axis parameter \c axis-par:
    - The specified parameter is expected to indicate one or more
      unique axes to remove from the current parameter, which must be
      of type ‘r’ or ‘o’.
    - If a number, then the indicated parameter is taken for it.
    - If ‘-’, then the previous parameter is taken for it.

    The removal of dimensions is done, and the axis numbers apply,
    only after all the other dimension specifications have been
    processed.

    For the dimension specification \c dim-spec:
    - NUMBER = the current dimension has the specified size.  For
      input parameters, an error is declared if the dimension does not
      have the specified size.
    - '>'NUMBER = the current dimension has at least the specified
      size.  For input parameters, an error is declared if the
      dimension does not have at least the specified size.
    - ‘+’NUMBER = for output or return parameters, a new dimension with
      the specified size is inserted here.
    - ‘=’ = for output or return parameters, the current dimension is
      taken from the reference parameter.
    - ‘=’NUMBER = for output or return parameters, the current dimension
      is taken from the reference parameter, and must be equal to the
      specified number.  An error is declared if the reference
      parameter's dimension does not have the indicated size
    - ‘-’ = the corresponding dimension from the reference parameter is
      skipped.
    - ‘:’ = for input parameters, accept the current dimension.
    - ‘&’ = the remaining dimensions must be equal to those of the
      reference parameter.
    - '#' = the element count must be equal to 1 or to that of the
      reference parameter.
    - ‘*’ = the remaining dimensions are unrestricted.

    Both a ‘+’NUMBER and a ‘-’NUMBER may be given in the same
    dimension specification \c dim_spec.
  */
int32_t standard_args(int32_t narg, int32_t ps[], char const *fmt, Pointer **ptrs,
                  loopInfo **infos)
{
  int32_t returnSym, *ref_dims, tgt_dims[MAX_DIMS], prev_ref_param, *final;
  struct param_spec *pspec;
  struct param_spec_list *psl;
  struct dims_spec *dims_spec;
  struct obstack o;
  int32_t param_ix, num_ref_dims;
  loopInfo li;
  Pointer p;
  Symboltype type;

  returnSym = LUX_ONE;
  psl = parse_standard_arg_fmt(fmt);
  if (!psl) {
    if (ptrs)
      *ptrs = NULL;
    if (infos)
      *infos = NULL;
    return luxerror("Illegal standard arguments specification %s", 0, fmt);
  }
  int32_t num_in_out_params = psl->num_param_specs
    - (psl->return_param_index >= 0);
  /* determine mininum and maximum required number of arguments */
  int32_t nmin;
  for (nmin = num_in_out_params; nmin > 0; nmin--)
    if (!psl->param_specs[nmin - 1].is_optional)
      break;
  if (narg < nmin || narg > num_in_out_params) {
    if (ptrs)
      *ptrs = NULL;
    if (infos)
      *infos = NULL;
    return luxerror("Standard arguments specification asks for between %d and %d input/output arguments but %d are specified (%s)", 0, nmin, num_in_out_params, narg, fmt);
  }
  if (ptrs)
    *ptrs = (Pointer*) malloc(psl->num_param_specs*sizeof(Pointer));
  if (infos)
    *infos = (loopInfo*) malloc(psl->num_param_specs*sizeof(loopInfo));
  final = (int32_t*) calloc(psl->num_param_specs, sizeof(int32_t));

  Symboltype common_type
    = standard_args_common_symboltype(psl->num_param_specs,
                                      psl->param_specs,
                                      narg, ps);

  obstack_init(&o);
  /* now we treat the parameters. */
  prev_ref_param = -1; /* < 0 indicates no reference parameter set yet */
  for (param_ix = 0; param_ix < psl->num_param_specs; param_ix++) {
    int32_t pspec_dims_ix; /* parameter dimension specification index */
    int32_t tgt_dims_ix;   /* target dimension index */
    int32_t ref_dims_ix;   /* reference dimension index */
    int32_t src_dims_ix;   /* input dimension index */
    int32_t *src_dims;        /* dimensions of input parameter */
    int32_t num_src_dims;      /* number of dimensions of input parameter */
    int32_t iq, d;

    pspec = &psl->param_specs[param_ix];
    dims_spec = pspec->dims_spec;
    if (param_ix == num_in_out_params || param_ix >= narg || !ps[param_ix] ||
        numerical(ps[param_ix], &src_dims, &num_src_dims, NULL, NULL) < 0) {
      src_dims = NULL;
      num_src_dims = 0;
    }
    int32_t ref_param = pspec->ref_par;
    if (ref_param < 0)
      ref_param = (param_ix? param_ix - 1: 0);
    if (param_ix > 0            /* first parameter has no reference */
        && (!ref_dims           /* no reference yet */
            || ref_param != prev_ref_param)) { /* or different from before */
      /* get reference parameter's information */
      /* if the reference parameter is an output parameter, then
         we must get the information from its *final* value */
      switch (psl->param_specs[ref_param].logical_type) {
      case PS_INPUT:
        if (numerical(ps[ref_param], &ref_dims, &num_ref_dims, NULL, NULL) < 0) {
          returnSym = luxerror("Reference parameter %d must be an array",
                               ps[param_ix], ref_param + 1);
          goto error;
        }
        break;
      case PS_OUTPUT: case PS_RETURN:
        if (!final[ref_param]) {
          returnSym = luxerror("Illegal forward output/return reference "
                               "parameter %d for parameter %d", 0,
                               ref_param + 1, param_ix + 1);
          goto error;
        }
        if (numerical(final[ref_param], &ref_dims, &num_ref_dims, NULL, NULL)
            < 0) {
          returnSym = luxerror("Reference parameter %d must be an array",
                               final[param_ix], ref_param + 1);
          goto error;
        }
        break;
      }
      prev_ref_param = ref_param;
    } else if (!param_ix) {
      ref_dims = NULL;
      num_ref_dims = 0;
    }
    if (!pspec->is_optional || param_ix == num_in_out_params
        || (param_ix < narg && ps[param_ix])) {
      for (pspec_dims_ix = 0, tgt_dims_ix = 0, src_dims_ix = 0, ref_dims_ix = 0;
           pspec_dims_ix < pspec->num_dims_spec; pspec_dims_ix++) {
        int src_dim_size = (src_dims_ix < num_src_dims? src_dims[src_dims_ix]: 0);
        switch (dims_spec[pspec_dims_ix].type) {
        case DS_EXACT: /* an input parameter must have the exact
                          specified dimension */
        case DS_ATLEAST: // or at least the specified size
          if (pspec->logical_type == PS_INPUT) {
            if (dims_spec[pspec_dims_ix].type == DS_EXACT
                && src_dim_size != dims_spec[pspec_dims_ix].size_add) {
              returnSym = luxerror("Expected size %d for dimension %d "
                                   "but found %d", ps[param_ix],
                                   dims_spec[pspec_dims_ix].size_add,
                                   src_dims_ix, src_dim_size);
              goto error;
            }
            else if (dims_spec[pspec_dims_ix].type == DS_ATLEAST
                && src_dim_size < dims_spec[pspec_dims_ix].size_add) {
              returnSym = luxerror("Expected at least size %d for dimension %d "
                                   "but found %d", ps[param_ix],
                                   dims_spec[pspec_dims_ix].size_add,
                                   src_dims_ix, src_dim_size);
              goto error;
            }
          }
          /* the target gets the exact specified dimension */
          tgt_dims[tgt_dims_ix++] = dims_spec[pspec_dims_ix].size_add;
          src_dims_ix++;
          ref_dims_ix++;
          break;
        case DS_COPY_REF:       /* copy from reference */
          if (src_dims_ix >= num_ref_dims) {
            returnSym = luxerror("Requested copying dimension %d from the reference parameter which has only %d dimensions", ps[param_ix], src_dims_ix, num_ref_dims);
            goto error;
          }
          tgt_dims[tgt_dims_ix++] = ref_dims[ref_dims_ix++];
          src_dims_ix++;
          break;
        case DS_ADD:
          d = dims_spec[pspec_dims_ix].size_add;
          switch (pspec->logical_type) {
          case PS_INPUT:
            if (src_dim_size != d) {
              returnSym = luxerror("Expected size %d for dimension %d "
                                   "but found %d", ps[param_ix],
                                   d, src_dims_ix, src_dim_size);
              goto error;
            }
            src_dims_ix++;
            tgt_dims[tgt_dims_ix++] = d;
            break;
          case PS_OUTPUT: case PS_RETURN:
            tgt_dims[tgt_dims_ix++] = d;
            break;
          }
          break;
        case DS_REMOVE: case DS_ADD_REMOVE:
          switch (pspec->logical_type) {
          case PS_INPUT:
            {
              int32_t d = dims_spec[pspec_dims_ix].size_remove;
              if (d && ref_dims[ref_dims_ix] != d) {
                returnSym = luxerror("Expected size %d for dimension %d "
                                     "but found %d", ps[param_ix],
                                     d, ref_dims_ix, ref_dims[ref_dims_ix]);
                goto error;
              }
            }
            break;
          case PS_OUTPUT: case PS_RETURN:
            {
              int32_t d = dims_spec[pspec_dims_ix].size_remove;
              if (d && ref_dims[ref_dims_ix] != d) {
                returnSym = luxerror("Expected size %d for dimension %d "
                                     "but found %d", ps[param_ix],
                                     d, ref_dims_ix, ref_dims[ref_dims_ix]);
                goto error;
              }
            }
            if (dims_spec[pspec_dims_ix].type == DS_ADD_REMOVE)
              tgt_dims[tgt_dims_ix++] = dims_spec[pspec_dims_ix].size_add;
            break;
          }
          ref_dims_ix++;
          break;
        case DS_ACCEPT:         /* copy from input */
          if (src_dims_ix >= num_src_dims) {
            returnSym = luxerror("Cannot copy non-existent dimension %d",
                                 ps[param_ix], src_dims_ix);
            goto error;
          } else {
            tgt_dims[tgt_dims_ix++] = src_dims[src_dims_ix++];
            ref_dims_ix++;
          }
          break;
        default:
          returnSym = luxerror("Dimension specification type %d "
                               "not implemented yet", ps[param_ix],
                               dims_spec[pspec_dims_ix].type);
          goto error;
          break;
        }
      }
      switch (pspec->logical_type) {
      case PS_INPUT:
        switch (pspec->remaining_dims) {
        case PS_EQUAL_TO_REFERENCE:
          if (ref_dims && ref_dims_ix < num_ref_dims) {
	    int32_t expect = num_ref_dims + src_dims_ix - ref_dims_ix;
            if (expect != num_src_dims) {
              returnSym = luxerror("Expected %d dimensions but found %d",
                                   ps[param_ix],
                                   num_ref_dims + src_dims_ix - ref_dims_ix,
                                   num_src_dims);
              goto error;
            }
            int32_t i, j;
            for (i = ref_dims_ix, j = src_dims_ix; i < num_ref_dims; i++, j++)
              if (ref_dims[i] != src_dims[j]) {
                returnSym = luxerror("Expected dimension %d equal to %d "
                                     "but found %d", ps[param_ix], i + 1,
                                     ref_dims[i], src_dims[j]);
                goto error;
              }
          } else {
            returnSym = luxerror("Dimensions of parameter %d required to be "
                                 "equal to those of the reference, but no "
                                 "reference is available",
                                 ps[param_ix], param_ix + 1);
            goto error;
          }
          break;
        case PS_ONE_OR_EQUAL_TO_REFERENCE:
          if (ref_dims && ref_dims_ix < num_ref_dims) {
	    int32_t expect = num_ref_dims + src_dims_ix - ref_dims_ix;
            if (expect != num_src_dims) {
              returnSym = luxerror("Expected %d dimensions but found %d",
                                   ps[param_ix],
                                   num_ref_dims + src_dims_ix - ref_dims_ix,
                                   num_src_dims);
              goto error;
            }
            int32_t i, j;
            for (i = ref_dims_ix, j = src_dims_ix; i < num_ref_dims; i++, j++)
              if (src_dims[j] != 1 && ref_dims[i] != src_dims[j]) {
                if (ref_dims[i] == 1)
                  returnSym = luxerror("Expected dimension %d equal to %d "
                                       "but found %d", ps[param_ix], i + 1,
                                       ref_dims[i], src_dims[j]);
                else
                  returnSym = luxerror("Expected dimension %d equal to 1 or %d "
                                       "but found %d", ps[param_ix], i + 1,
                                       ref_dims[i], src_dims[j]);
                goto error;
              }
          } else {
            returnSym = luxerror("Dimensions of parameter %d required to be "
                                 "equal to those of the reference, but no "
                                 "reference is available",
                                 ps[param_ix], param_ix + 1);
            goto error;
          }
          break;
        case PS_ARBITRARY:
          break;
        case PS_ABSENT:
          if (!pspec_dims_ix) {     /* had no dimensions */
            /* assume dimension equal to 1 */
            if (src_dims[src_dims_ix] != 1) {
              returnSym = luxerror("Expected dimension %d equal to 1 "
                                   "but found %d", ps[param_ix],
                                   src_dims_ix + 1, src_dims[src_dims_ix]);
              goto error;
            } else
              src_dims_ix++;
          }
          if (src_dims_ix < num_src_dims) {
            returnSym = luxerror("Specification (parameter %d) says %d dimensions but source has %d dimensions",
                                 ps[param_ix], param_ix, src_dims_ix, num_src_dims);
            goto error;
          }
          break;
        }
        iq = ps[param_ix];
        type = symbol_type(iq);
        if (pspec->common_type)
          type = common_type;
        else if ((pspec->data_type_limit == PS_LOWER_LIMIT
                  && type < pspec->data_type)
                 || (pspec->data_type_limit == PS_EXACT
                     && type != pspec->data_type))
          type = pspec->data_type;
        iq = lux_convert(1, &iq, type, 1);
        break;
      case PS_OUTPUT: case PS_RETURN:
        switch (pspec->remaining_dims) {
        case PS_EQUAL_TO_REFERENCE:
          if (ref_dims_ix < num_ref_dims) {
            /* append remaining dimensions from reference parameter*/
            size_t e = num_ref_dims - ref_dims_ix;
            memcpy(tgt_dims + tgt_dims_ix, ref_dims + ref_dims_ix, e*sizeof(int32_t));
            tgt_dims_ix += e;
            src_dims_ix += e;
            ref_dims_ix += e;
          }
          break;
        case PS_ARBITRARY:
          returnSym = luxerror("'Arbitrary' remaining dimensions makes no "
                               "sense for an output or return parameter "
                               " (number %d)", 0, param_ix + 1);
          goto error;
        case PS_ABSENT:
          break;
        }
	if (pspec->axis_par > -2) {
	  /* We have an axis parameter specified for this one. */
	  int32_t axis_param = pspec->axis_par;
	  if (axis_param == -1) /* points at previous parameter */
	    axis_param = param_ix - 1;

	  if (axis_param < 0 || axis_param >= narg) {
	    returnSym = luxerror("Axis parameter %d for parameter %d is "
				 "out of bounds", 0, axis_param, param_ix + 1);
	    goto error;
	  }
	  if (axis_param == param_ix) {
	    returnSym = luxerror("Parameter %d cannot be its own axis"
				 " parameter", 0, param_ix + 1);
	    goto error;
	  }
	  if (!final[axis_param]) {
	    returnSym = luxerror("Illegal forward output/return axis "
				 "parameter %d for parameter %d", 0,
				 axis_param + 1, param_ix + 1);
	    goto error;
	  }
	  int32_t aq = ps[axis_param];
	  if (!symbolIsNumerical(aq)) {
	    returnSym = luxerror("Axis parameter %d is not numerical for"
				 " parameter %d", 0,
				 axis_param + 1, param_ix + 1);
	    goto error;
	  }
	  aq = lux_long(1, &aq);
	  int32_t nAxes;
	  Pointer axes;
	  numerical(aq, NULL, NULL, &nAxes, &axes);
	  int32_t j;
	  for (j = 0; j < nAxes; j++) {
	    if (axes.l[j] < 0 || axes.l[j] >= tgt_dims_ix) {
	      returnSym = luxerror("Axis %d out of bounds for"
				   " parameter %d", 0,
				   axes.l[j], param_ix + 1);
	      goto error;
	    }
	    tgt_dims[axes.l[j]] = 0; /* flags removal.  Note: no check for
				      duplicate axes */
	  }
	  int32_t k;
	  /* remove flagged dimensions */
	  for (j = k = 0; j < tgt_dims_ix; j++) {
	    if (tgt_dims[j])
	      tgt_dims[k++] = tgt_dims[j];
	  }
	  tgt_dims_ix = k;
	}
        /* get rid of trailing dimensions equal to 1 */
        while (tgt_dims_ix > 0 && tgt_dims[tgt_dims_ix - 1] == 1)
          tgt_dims_ix--;
        if (param_ix == num_in_out_params) {      /* a return parameter */
          if (ref_param >= 0
              && pspec->data_type_limit == PS_LOWER_LIMIT) {
            iq = ps[ref_param];
            type = symbol_type(iq);
            if (type < pspec->data_type)
              type = pspec->data_type;
          } else
            type = pspec->data_type;
          if (pspec->common_type
              && common_type != LUX_NO_SYMBOLTYPE)
            type = common_type;
	  if (tgt_dims_ix)
	    iq = returnSym = array_scratch(type, tgt_dims_ix, tgt_dims);
	  else
	    iq = returnSym = scalar_scratch(type);
        } else {
          iq = ps[param_ix];
          type = symbol_type(iq);
          if (symbol_class(iq) == LUX_UNUSED
              || ((pspec->data_type_limit == PS_LOWER_LIMIT
                   && type < pspec->data_type)
                  || (pspec->data_type_limit == PS_EXACT
                      && type != pspec->data_type)))
            type = pspec->data_type;
	  if (tgt_dims_ix)
	    redef_array(iq, type, tgt_dims_ix, tgt_dims);
	  else
	    redef_scalar(iq, type, NULL);
        }
        break;
      }
      final[param_ix] = iq;
      standardLoop(iq, 0, SL_ALLAXES, symbol_type(iq), &li, &p, NULL, NULL, NULL);
      if (infos)
        (*infos)[param_ix] = li;
      if (ptrs)
        (*ptrs)[param_ix] = p;
    } else {
      if (infos)
        memset(&(*infos)[param_ix], 0, sizeof(loopInfo));
      if (ptrs)
        (*ptrs)[param_ix].v = NULL;
    }
  }

  free_param_spec_list(psl);
  obstack_free(&o, NULL);
  return returnSym;

  error:
  obstack_free(&o, NULL);
  if (ptrs) {
    free(*ptrs);
    *ptrs = NULL;
  }
  if (infos) {
    free(*infos);
    *infos = NULL;
  }
  return returnSym;
}

