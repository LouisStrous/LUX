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

#include <algorithm>            // for std::remove_if
#include <cassert>
#include <cstring>              // for std::memset
#include <functional>           // for std::multiplies
#include <numeric>              // for std::accumulate

#include "NumericDataDescriptor.hh"

#include <string.h>
#include <limits.h>
#include "action.hh"
#include <obstack.h>
#include <errno.h>
#include <ctype.h>
#include "axis.hh"

/** Define which memory allocation routine to use for obstacks. */
#define obstack_chunk_alloc malloc

/** Define which memory freeing routine to use for obstacks. */
#define obstack_chunk_free free

/*-------------------------------------------------------------------------*/
/** Adjusts LoopInfo for axis treatment.

    \param[in,out] info is a pointer to the \c LoopInfo structure to
    adjust.

    \param[in] nAxes is the number of axes.

    \param[in] axes points to the array of axes.

    \param[in] mode says how to treat the axes.

    \returns 0 for success, -1 for failure.
 */
int32_t setAxes(LoopInfo *info, int32_t nAxes, int32_t *axes, int32_t mode)
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

    \param[in,out] info points to the predefined \c LoopInfo structure
    in which the information is gathered.

    \param[in] ndim is the number of dimensions.  If it is less than 1
    then \p dims is ignored and the data is assumed to consist of a
    single element.

    \param[in] dims points to a list of \p ndim dimensions.  If it is
    \c NULL, then \p ndim is ignored and the data is assumed to
    consist of a single element.  The elements of \dims are not
    checked for validity.

    \param[in] type is the LUX data type of the data.  It is not
    checked for validity.

    \param[in] naxes is the number of axes to loop along.  If it is 0
    or negative, then \p axes is ignored and it is assumed that all
    axes were specified.

    \param[in] axes points to a list of \p naxes indexes of axes to
    loop along.  If it is \c NULL but \p naxes is at least 1, then the
    first \p naxes axes are assumed.  The elements of \p axes are not
    checked for validity.

    \param[in] data points to a predefined \c Pointer to use for
    pointing at the currently selected data element.

    \param[in] mode is a collection of bit flags that indicate how to
    loop through the axes, as for standardLoop().

    \returns 0 if everything is OK, non-0 if an error occurred.  If
    the element count (the product of all dimensions) overflows, then
    `EDOM` is returned.
 */
int setupDimensionLoop(LoopInfo *info, int32_t ndim, int32_t const *dims,
                       Symboltype type, int32_t naxes, int32_t const *axes,
                       Pointer *data, int32_t mode)
{
  int32_t	i;
  size_t	size;

  /* first copy the arguments into the structure
     NOTE: no checking to see if the values are legal. */

  if (dims && ndim >= 0) {
    info->ndim = ndim;          // the dimensions count
    memmove(info->dims, dims, ndim*sizeof(int32_t)); // the dimensions
  } else {                   // a scalar
    // treat as single-element array
    info->ndim = 1;
    info->dims[0] = 1;
  }
  info->naxes = naxes;
  if (naxes > 0) {
    if (axes)
      memmove(info->axes, axes, naxes*sizeof(*axes));
    else                        // do all axes
      for (i = 0; i < info->naxes; i++)
        info->axes[i] = i;
  }

  // calculate the number of elements in the array.
  size = 1;
  size_t oldsize;
  for (i = 0; i < info->ndim; i++) {
    oldsize = size;
    size *= info->dims[i];
    if (size < oldsize) {       // overflow occurred
      return EDOM;
    }
  }
  if (size >= INT64_MAX/2)
    return EDOM;        // element count may be too large for an off_t

  info->nelem = size;
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
  return 0;
}
/*-----------------------------------------------------------------------*/
/** Advance along a loop.  The coordinates and the pointer to the data
    are advanced, taking into account the dimensional structure of the
    data and the configured axes.

    \param[in,out] info points to the LoopInfo that describes the
    dimensional structure and the way to traverse it.

    \param[in,out] ptr points to a Pointer that indicates the current
    data element.

    \returns the index of the first loop axis that is not yet
    completely traversed.

    For example, if the array has 4 by 5 by 6 elements, then
    advancement from element (2,0,0) (to element (3,0,0)) yields 0,
    (3,0,0) yields 1, (3,4,0) yields 2, and (3,4,5) yields 3.
 */
int32_t advanceLoop(LoopInfo *info, Pointer *ptr)
/* advance coordinates; return index of first encountered incomplete
 axis.   */
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
/** Are we at the start of the loop?

    \param[in] info is the LoopInfo to query.

    \returns 1 if all coordinates are equal to 0, and 0 otherwise.
    Can be used to test if the loop is back at the start again --
    i.e., has completed.  See also advanceLoop().
*/
int32_t loopIsAtStart(LoopInfo const *info)
{
  int32_t state = 1, i;

  for (i = 0; i < info->rndim; i++)
    state &= (info->coords[i] == 0);
  return state;
}
/*-----------------------------------------------------------------------*/
/** Rearranges dimensions suitable for walking along the selected
    axes.

    \param[in,out] info points at the LoopInfo that gets adjusted.
*/
void rearrangeDimensionLoop(LoopInfo *info)
{
  int32_t	axis, i, temp[MAX_DIMS], j, axisIndex, mode, axis2;

  /*
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
/*-------------------------------------------------------------------------*/
/** Sets the axes mode and rearranges the dimensions accordingly.

    \param[in,out] info points at the LoopInfo that gets adjusted.

    \param[in] mode is a collection of bit flags that says how to
    treat the axes.

    \par Axes Modes

    - `SL_EACHCOORD`: the indicated axis goes first; the remaining
      axes come later in their original order; the user gets access to
      all coordinates.

    - `SL_AXISCOORD`: the indicated axis goes first; the user gets
      access only to the coordinate along the indicated axis;
      remaining axes come later, lumped together as much as possible
      for faster execution.

    - `SL_AXESBLOCK`: the active axis goes first, then all remaining
      specified axes, and then the unspecified ones; in ascending
      order within each group.
 */
void setAxisMode(LoopInfo *info, int32_t mode) {
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
/*-----------------------------------------------------------------------*/
/** Create an appropriate result symbol.

   \param[in] sinfo points to a LoopInfo that describes the source.

   \param[in] tmode is a collection of bit flags that specifies the
   desired result.  See below.

   \param[in] ttype is the desired output type.

   \param[in] nMore is the count of elements in \p more.  If it is 0,
   then \p more is ignored.  It is an error if it is negative or if
   `nMore` plus the number of source dimensions exceeds `MAX_DIMS`.

   \param[in] more points at a list of \p nMore dimensions to add to
   the target, compared to the source.  If it is \c NULL, then it and
   \p nMore are ignored.  Otherwise, the new dimensions, prefixed to
   those from the source, are applied to the target.

   \param[in] nLess is the count of elements in \p less.  If it is 0,
   then \p less is ignored.  It is an error if it is negative or
   greater than \p nAxes.

   \param[in] less if not \c NULL, points at a list of \p nLess
   divisors to apply to the source dimensions to get the target
   dimensions.  It is an error if one of the \c less values does not
   evenly divide the corresponding source dimension.  If \p less is \c
   NULL, then it and \p nLess are ignored.

   \param[out] tinfo points at a predefined LoopInfo that gets filled
   with info about the output symbol.

   \param[out] tptr points to a Pointer to the current location in the
   target data.

   \returns the newly created target symbol.

   \par Output Mode Specification

   - `SL_ONEDIMS` says to set omitted dimensions to 1 in the output.
     If this option is not selected, then omitted dimensions are
     really not present in the output.  If due to such omissions no
     dimensions are left, then a scalar is returned as output symbol

   - `SL_SAMEDIMS` says that the output has same dimensions as the
     source.

   - `SL_COMPRESS` is as `SL_SAMEDIMS`, except that the first
        specified axis is omitted.

   - `SL_COMPRESSALL` is as `SL_SAMEDIMS`, but all specified axes are
      omitted.
*/
int32_t dimensionLoopResult1(LoopInfo const *sinfo,
                         int32_t tmode, Symboltype ttype,
                         int32_t nMore, int32_t const * more,
                         int32_t nLess, int32_t const * less,
                         LoopInfo *tinfo, Pointer *tptr)
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
/** Create an appropriate result symbol without adjusting dimensions.
    The arguments and return value are the same as the corresponding
    ones of dimensionLoopResult1(), except that no `more` and `less`
    arguments are specified.
 */
int32_t dimensionLoopResult(LoopInfo const *sinfo, LoopInfo *tinfo,
                            Symboltype ttype, Pointer *tptr)
{
  return dimensionLoopResult1(sinfo, sinfo->mode, ttype,
                              0, NULL, 0, NULL, tinfo, tptr);
}
/*-----------------------------------------------------------------------*/
/** Initiates a standard array loop taking the axes from a LUX symbol.
    advanceLoop() runs through the loop.

    \param[in] data source data symbol, must be numerical.

    \param[in] axes a pointer to the list of axes to treat.

    \param[in] nAxes the number of axes to treat.

    \param[in] mode flags that indicate desired action; see below.

    \param[in] outType desired data type for output symbol.

    \param[out] src returns loop info for \p data.

    \param[out] srcptr pointer to a pointer to the data in \p data.

    \param[out] output returns created output symbol, if unequal to \c
    NULL on entry.

    \param[out] trgt returns loop info for \p output, if that is
    unequal to \c NULL on entry.

    \param[out] trgtptr pointer to a pointer to the data in the output
    symbol.

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
                     Symboltype outType, LoopInfo *src, Pointer *srcptr,
                     int32_t *output, LoopInfo *trgt, Pointer *trgtptr)
{
  /*
   */
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
/** Like standardLoop() but can produce a target with adjusted
    dimensions.
 */
int32_t standardLoopX(int32_t source, int32_t axisSym, int32_t srcMode,
                  LoopInfo *srcinf, Pointer *srcptr,
                  int32_t nMore, int32_t const * more,
                  int32_t nLess, int32_t const * less,
                  Symboltype tgtType, int32_t tgtMode,
                  int32_t *target,
                  LoopInfo *tgtinf, Pointer *tgtptr)
{
  // LS 2011-07-22
  int32_t i, nAxes;
  Pointer axes;
  int32_t standardLoop1(int32_t source,
                        int32_t nAxes, int32_t const * axes,
                        int32_t srcMode,
                        LoopInfo *srcinf, Pointer *srcptr,
                        int32_t nMore, int32_t const * more,
                        int32_t nLess, int32_t const * less,
                        Symboltype tgtType, int32_t tgtMode,
                        int32_t *target,
                        LoopInfo *tgtinf, Pointer *tgtptr);

  if (axisSym > 0) {		/* <axisSym> is a regular symbol */
    if (!symbolIsNumerical(axisSym))
      return luxerror("Need a numerical argument", axisSym);
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
/**
   Initiates a standard loop based on a list of axes.  Is like
   standardLoop() but has an explicit list of axes instead of a LUX
   symbol indicating the axes.

   The arguments and return value are like those of standardLoop(),
   except for:

   \param[in] nAxes is the count of axes in \p axes.

   \param[in] axes points at a list of axes to walk along.
 */
int32_t standardLoop0(int32_t data, int32_t nAxes, int32_t *axes,
                      int32_t mode, Symboltype outType,
                      LoopInfo *src, Pointer *srcptr, int32_t *output,
                      LoopInfo *trgt, Pointer *trgtptr)
{
  int32_t	*dims, ndim, i, temp[MAX_DIMS];
  int32_t lux_convert(int32_t, int32_t [], Symboltype, int32_t);

  /* check if <data> is of proper class, and get some info about it */
  if (numerical(data, &dims, &ndim, NULL, srcptr) == LUX_ERROR)
    return LUX_ERROR;		/* some error */

  if (mode & SL_TAKEONED)	/* take data as 1D */
    nAxes = 0;			/* treat as 1D */
  else if (mode & SL_ALLAXES) { /* select all axes */
    nAxes = ndim;
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

  if ((mode & SL_SRCUPGRADE)	/* upgrade source if necessary */
      && (symbol_type(data) < outType))	{ /* source needs upgrading */
    data = lux_convert(1, &data, outType, 1); /* upgrade */
    numerical(data, NULL, NULL, NULL, srcptr);
  }

  setupDimensionLoop(src, ndim, dims, symbol_type(data), nAxes,
		     axes, srcptr, mode);

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

  if (mode & SL_TAKEONED) { /* mimic 1D array */
    src->dims[0] = src->rdims[0] = src->nelem;
    src->ndim = src->rndim = 1;
  }

  return LUX_OK;
}
/*-----------------------------------------------------------------------*/
/** Initiates a standard array loop.  advanceLoop() runs through the loop.

   \param[in] source is the source data symbol, must be numerical.

   \param[in] nAxes is the number of elements stored at \p axes.  If
   it is less than 0, then an error is declared.  If it is 0, then \p
   axes is ignored.

   \param[in] axes points to the list of axes to treat.  If it is \c
   NULL, then \p nAxes is ignored.  Otherwise, the list must contain
   at least \p nAxes elements indicating the axes (dimensions) of
   interest.  If any of those axes are negative or greater than or
   equal to the number of dimensions in \p source, then an error is
   declared.  Further restrictions on the axis values may be specified
   through \p srcMode.

   \param[in] srcMode contains bit flags that indicate desired action
   for \p source, see below.

   \param[out] srcinf points at the LoopInfo for the source.

   \param[out] srcptr points at the Pointer for the source.

   \param[in] nMore is the number of elements stored at \p more.  If
   it is less than 0, then an error is declared.  If it is 0, then \p
   more is ignored.

   \param[in] more is a pointer to a list of dimensions to add.  If it
   is \c NULL, then \p nMore is ignored.  Otherwise, \p more must
   point at a list of \p nMore numbers indicating the sizes of the new
   dimensions to add to \p target, in addition to the dimensions of \p
   source.  If any of those sizes are less than 1, then an error is
   declared.  The extra dimensions are prefixed to the existing ones,
   in the indicated order.

   \param[in] nLess is the number of elements stored at \p less.  If
   it is less than 0, then an error is declared.  If it is 0, then \p
   less is ignored.  If \p nLess is greater than \p nAxes, then an
   error is declared.

   \param[in] less is a pointer to a list of dimension divisors.  If
   it is \c NULL, then \p nLess is ignored.  Otherwise, \p less must
   point at a list of \p nLess numbers indicating by which factors the
   sizes of the corresponding axes from \p axes are reduced.  If \p
   nLess is greater than 0 but less than \p nAxes, then the last
   element of \p less is repeated as needed.  If the size of one of
   the corresponding dimensions of \p source is not a multiple of the
   corresponding number from \p less, then an error is declared.  If
   after reduction the size of the dimension is equal to 1, then that
   dimension may be omitted from the result, depending on the value of
   \p tgtMode.

   \param[in] tgtType is the desired data type for newly created
   target data symbol.

   \param[in] tgtmode contains bit flags that indicate desired action
   for target, see below.

   \param[out] target points at the created target data symbol.  If it
   is \c NULL, then no target-related output variables are filled in.
   By default, \p target gets the same dimensions as \p source.  This
   can be changed through \p more, \p less, and \p tgtMode.  If the
   final number of dimensions in \p target (taking into account \p
   more and \p less and \p tgtMode) would exceed \c MAX_DIMS, then an
   error is declared.

   \param[out] tgtinf points at the LoopInfo for the target.  If \p
   tgtinf or \p target are \c NULL, then \p tgtinf is not filled in.

   \param[out] trgtptr points at the Pointer the target.  If \p tgtptr
   or \p target are \c NULL, then \p tgtptr is not filled in.

   \par \p srcMode flags

   About the axes:
   - `SL_ALLAXES`: same as specifying all dimensions of \p data in
     ascending order in \p axes; the values of \p axes and \p nAxes
     are ignored.
   - `SL_TAKEONED`: take the \p data as one-dimensional; the values of
     \p axes and \p nAxes are ignored.

   About specified axes:
   - `SL_NEGONED`: if the user supplies a single axis and that axis is
     negative, then \p data is treated as if it were one-dimensional
     (yet containing all of its data elements).
   - `SL_ONEAXIS`:  only a single axis is allowed.
   - `SL_UNIQUEAXES`: all specified axes must be unique (no duplicates).

   About the input \p data:
   - `SL_SRCUPGRADE`: use a copy of \p data which is upgraded to \p
      tgtType if necessary.

   About the desired axis order and coordinate availability:
   - `SL_EACHCOORD`: the currently selected axis goes first; the
     remaining axes come later, in their original order; the user gets
     access to all coordinates.  This is the default.
   - `SL_AXISCOORD`: the currently selected axis goes first; the user
     gets access only to the coordinate along the indicated axis;
     remaining axes come later, lumped together as much as possible
     for faster execution.
   - `SL_AXESBLOCK`: all specified axes go first in the specified
     order; the remaining axes come later, in their original order;
     the user gets access to all coordinates.  Implies
     `SL_UNIQUEAXES`.

   About the coordinate treatment:
   - `SL_EACHROW`: the data is advanced one row at a time; the user
     must take care of advancing the data pointer to the beginning of
     the next row.
   - `SL_EACHBLOCK`: like `SL_EACHROW`, but all selected axes together
     are considered to be a "row".  Implies `SL_AXESBLOCK`.

   \par \p tgtMode flags

   About the output symbol:
   - `SL_EXACT`: must get exactly the data type indicated by \p
     tgtType.  This is the default.
   - `SL_UPGRADE`: must get the data type of \p source or \p tgtType,
     whichever is greater
   - `SL_KEEPTYPE`: gets the same type as \p source.
   - `SL_ONEDIMS`: omitted dimensions are not really removed but
     rather set to 1.
   - `SL_COMPRESS`: gets the same dimensions as \p data, except that
     the currently selected axis is omitted.  If no dimensions are
     left, then a scalar is returned.  Ignored unless \p less is \c
     NULL.
   - `SL_COMPRESSALL`: gets the same dimensions as \p data, except
     that all selected axes are omitted.  If no dimensions are left,
     then a scalar is returned.  Ignored unless \p less is \c NULL.

   About the desired axis order and coordinate availability:
   - `SL_EACHCOORD`: the currently selected axis goes first; the
     remaining axes come later, in their original order; the user gets
     access to all coordinates.  This is the default.
   - `SL_AXISCOORD`: the currently selected axis goes first; the user
     gets access only to the coordinate along the indicated axis;
     remaining axes come later, lumped together as much as possible
     for faster execution.
   - `SL_AXESBLOCK`: all specified axes go first in the specified
     order; the remaining axes come later, in their original order;
     the user gets access to all coordinates.  Implies
     `SL_UNIQUEAXES`.

   About the coordinate treatment:
   - `SL_EACHROW`: the data is advanced one row at a time; the user
     must take care of advancing the data pointer to the beginning of
     the next row.
   - `SL_EACHBLOCK`: like `SL_EACHROW`, but all selected axes together
     are considered to be a "row".  Implies `SL_AXESBLOCK`.

  */
int32_t standardLoop1(int32_t source,
                  int32_t nAxes, int32_t const * axes,
                  int32_t srcMode,
                  LoopInfo *srcinf, Pointer *srcptr,
                  int32_t nMore, int32_t const * more,
                  int32_t nLess, int32_t const * less,
                  Symboltype tgtType, int32_t tgtMode,
                  int32_t *target,
                  LoopInfo *tgtinf, Pointer *tgtptr)
{
  int32_t *dims;
  int32_t ndim, i, temp[MAX_DIMS];
  int32_t lux_convert(int32_t, int32_t [], Symboltype, int32_t);

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
/** Rearranges dimensions for a next loop through the array.

    \param[in,out] info is the LoopInfo to adjust.

    \returns 1 if OK, 0 if we're beyond the last specified axis.
*/
int32_t nextLoop(LoopInfo *info)
{
  if (++(info->axisindex) >= info->naxes)
    return 0;
  rearrangeDimensionLoop(info);
  return LUX_OK;
}
/*-----------------------------------------------------------------------*/
/** Rearranges dimensions for a next loop through two arrays.

    \param[in,out] info1 is the first LoopInfo to adjust.

    \param[in,out] info2 is the second LoopInfo to adjust.

    \returns 1 if OK, 0 if we're beyond the last specified axis <em>for
    the first LoopInfo</em>.
*/
int32_t nextLoops(LoopInfo *info1, LoopInfo *info2)
{
  if (++(info1->axisindex) >= info1->naxes)
    return 0;
  info2->axisindex++;
  rearrangeDimensionLoop(info1);
  rearrangeDimensionLoop(info2);
  return 1;
}
/*-----------------------------------------------------------------------*/
/** This routine selects a subset of the source data for treatment.

    \param[in] range points at a list of elements describing the
    beginning and end of the range in each of the source coordinates.
    The first element specifies the beginning of the range for the
    first rearranged dimension, the second element the end of that
    range.  Each next pair applies to the next rearranged dimension.

    \param[in,out] src is the LoopInfo to adjust.

    This routine should only be used if no dimension compression has
    been applied.
 */
void subdataLoop(int32_t *range, LoopInfo *src)
{
  /* LS 30apr98 */
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
/** Modifies LoopInfo objects for for walking along only the outer
    edges of the data set.

    \param[in,out] src is the LoopInfo describing the source.  It is
    assumed to be configured to point to the beginning of the data.

    \param[in,out] trgt is the LoopInfo describing the target.  If it
    is not \c NULL, then it is modified the same as \p src.  It is
    then assumed to contain the same dimensions and axes as \p src,
    and to be configured to point to the beginning of the data.

    \param[in] index is the index of the edge to walk along.  It must
    be between 0 and `2*src->ndim - 1`, but this is not checked.

    Edge number \p index consists of all data points that have
    rearranged coordinate number `index/2` equal to 0 (if `index%2 ==
    0`) or to `src->rdims[index/2] - 1` (if `index%2 == 1`).  So,

    - index 0 yields all elements at the beginning of the first
      rearranged coordinate; schematically: `x(0,*)`.

    - index 1 yields all elements at the end of the first rearranged
      coordinate.  Schematically: `x(*-1,*)`.

    - index 2 yields all elements at the beginning of the second
      rearranged coordinate.  Schematically: `x(*,0)`.

    - index 3 yields all elements at the end of the second rearranged
      coordinate.  Schematically: `x(*,*-1)`.

    - and so on.

*/
void rearrangeEdgeLoop(LoopInfo *src, LoopInfo *trgt, int32_t index)
{
  // LS 23oct98 LS 2feb99
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
/** Prepares for handling diagonals.

    \param[in] symbol is the symbol number of a LUX array whose
    contents specify which connections to nearest neighbors to accept.
    The array must one number for each dimension of the target array.
    These numbers may have values 0, 1, or 2, see below.  If \p symbol
    is equal to 0, then `one(lonarr(info->ndim))*2` is assumed for it;
    i.e., all directions are allowed.

    \param[in] info is a LoopInfo that describes the dimensional
    structure of the target.

    \param[in] part says which part of all directions to service.  If
    you want to reach all allowed neighbors, then set \p part equal
    to 1.  If you want to service all allowed directions but do not
    distinguish between diametrically opposite directions, then set \p
    part equal to 2.

    \param[out] offset if not \c NULL, is made to point at an array
    containing the offsets from the central position to service all of
    the allowed diagonals, based on the array dimensions taken from
    `info->dims`.  Memory for this array is allocated by the routine.
    The user should `free` the memory when done.

    \param[out] edge if not \c NULL, is made to point at an array
    with two elements for each dimension; the first of each pair
    refers to the near edge (with the appropriate coordinate equal to
    0) and the second one to the far edge (with the appropriate
    coordinate equal to `info.dims[] - 1`).  Each of these numbers is
    set to 1 if one or more of the offsets point at the corresponding
    edge, and to 0 otherwise.  Memory for this array is allocated by
    the routine.  The user should `free` the memory when done.

    \param[out] rcoord if not \c NULL, is made to point at an array
    with one set of coordinates for each direction (corresponding to
    one member of \p offset), with each number in each set equal to
    the offset expressed in the corresponding dimension.  All elements
    of \p rcoord are equal to -1, 0, or +1.  Memory for this array is
    allocated by the routine.  The user should `free` the memory when
    done.

    \param[out] diagonal if not \c NULL, is made to point at the
    start of the list of diagonal code numbers (i.e., the contents of
    \p symbol) -- or a pointer to \c NULL if \p symbol is equal to 0.

    \returns the number of offsets, or \c LUX_ERROR if an error
    occurred.

    \par Neighbor numbers

    The acceptable values 0, 1, 2 for the array elements in \p symbol
    have the following meaning:

    0. Neighbors in this dimension are not recognized.

    1. Neighbors in this dimension are only recognized as such if they
    share a face.  I.e., if an allowed direction has a non-zero
    component in this dimension, then that component is the
    <b>only</b> non-zero component of that direction.

    2. Neighbors in this dimension are recognized as such if they
    share a face or a vertex.
*/
int32_t prepareDiagonals(int32_t symbol, LoopInfo *info, int32_t part,
                         int32_t **offset, int32_t **edge, int32_t **rcoord,
                         int32_t **diagonal)
{
  // LS 10feb99
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
    *offset = (int32_t *) malloc(n*sizeof(int32_t)); /* offsets to
                                                        elements to be
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
/** Adjust the position along an axis.

    \param[in,out] info is the LoopInfo object to adjust.

    \param[in] index is the index of the rearrange axis to move along.
    If it points at a non-existent axis, then no adjustments are made.

    \param[in] distance is the distance to move over.  It may be
    negative and may have any magnitude.

    Moves along rearranged axis number \p index over the indicated \p
    distance, updating the coordinates and pointers in \p info.

    \returns `info->rndim` if \p index is negative.  Otherwise, if
    `index` is greater than or equal to `info->rndim`, then returns
    `index + 1`.  Otherwise, returns the index of the last affected
    dimension.
*/
int32_t moveLoop(LoopInfo *info, int32_t index, int32_t distance)
{
  // LS 9apr99
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
/** Move to the start of the rearranged axis.

    \param[in,out] info points to the LoopInfo to adjust.

    \param[in,out] ptr points to the Pointer to adjust.

    \param[in] index is the index of the rearranged axis.

    Moves to the start of the rearranged axis indicated by \p index,
    zeroing all rearranged coordinates up to and including that one,
    and adjusting the pointer accordingly.
*/
void returnLoop(LoopInfo *info, Pointer *ptr, int32_t index)
{
  // LS 9apr99
  int32_t	i;

  for (i = 0; i <= index; i++) {
    ptr->b -= info->coords[index]*info->rsinglestep[index]*info->stride;
    info->coords[index] = 0;
  }
}
/*--------------------------------------------------------------------*/
/** Gets information about a numerical or string symbol.

    \param[in] data is the number of the symbol to inspect.

    \param[out] dims if not \c NULL, points at memory where a list of
    dimensions of the symbol will be stored.

    \param[out] nDim if not \c NULL, points at memory where the
    dimension count of the symbol will be stored.

    \param[out] size if not \c NULL, points at memory where the
    element count of the symbol will be stored.

    \param[out] src if not \c NULL, points at a Pointer in which the
    location of the beginning of the data of the symbol will be
    stored.

    \param[in] string_is_ok says whether or not the symbol may be of a
    `string` type.

    \returns 1 if everything is OK, `LUX_ERROR` if the symbol is not
    numerical or is of type `string` and `string_is_ok` is 0.

    The user must provide adequate memory for each of the returned
    values.
*/
static int32_t numerical_or_string_choice(int32_t data, int32_t **dims, int32_t *nDim, int32_t *size, Pointer *src, int32_t string_is_ok)
{
  // LS 21apr97
  static int32_t	one = 1;

  if (symbolIsString(data)
      && !string_is_ok)
    return LUX_ERROR;

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
/** Gets information about a numerical symbol.

    \param[in] data is the number of the symbol to inspect.

    \param[out] dims, if not \c NULL, points at the location where the
    address of the beginning of the list of dimensions of the symbol
    will be stored.

    \param[out] nDim, if not \c NULL, points at the location where the
    dimension count of the symbol will be stored.

    \param[out] size, if not \c NULL, points at the location where the
    element count of the symbol will be stored.

    \param[out] src, if not \c NULL, points at the location where the
    Pointer pointing at the beginning of the data of the symbol will
    be stored.

    \returns 1 if everything is OK, `LUX_ERROR` if the symbol is not
    numerical.

    The user must provide adequate memory for each of the returned
    values.
*/
int32_t numerical(int32_t data, int32_t **dims, int32_t *nDim, int32_t *size, Pointer *src)
{
  // LS 21apr97
  return numerical_or_string_choice(data, dims, nDim, size, src, 0);
}
/*---------------------------------------------------------------------*/
/** Gets information about a numerical or string symbol.

    \param[in] data is the number of the symbol to inspect.

    \param[out] dims if not \c NULL, points at memory where a list of
    dimensions of the symbol will be stored.

    \param[out] nDim if not \c NULL, points at memory where the
    dimension count of the symbol will be stored.

    \param[out] size if not \c NULL, points at memory where the
    element count of the symbol will be stored.

    \param[out] src if not \c NULL, points at a Pointer in which the
    location of the beginning of the data of the symbol will be
    stored.

    \returns 1 if everything is OK, `LUX_ERROR` if the symbol is not
    numerical or `string`.

    The user must provide adequate memory for each of the returned
    values.
*/
int32_t numerical_or_string(int32_t data, int32_t **dims, int32_t *nDim, int32_t *size, Pointer *src)
{
  // LS 21apr97
  return numerical_or_string_choice(data, dims, nDim, size, src, 1);
}
/*--------------------------------------------------------------------*/
/** Redefine an array and set up for moving through it.

    \param[in] iq is the symbol to process.

    \param[in] type is the data type the symbol should get.

    \param[in] num_dims is the count of dimensions in \p dims.

    \param[in] dims points to a list of \p num_dims dimensions that
    the symbol should get.

    \param[in] naxes is the count of axes in \p axes.

    \param[in] axes points to a list of \p naxes axes for walking
    through the array.

    \param[in] mode is the desired treatment mode.

    \param[out] ptr if not \c NULL, points to a predefined Pointer for
    indicating the current data position in the array.

    \param[in,out] info points at a preexisting LoopInfo that gets
    adjusted for walking through the new array.
 */
void standard_redef_array(int32_t iq, Symboltype type,
			  int32_t num_dims, int32_t *dims,
			  int32_t naxes, int32_t *axes,
                          int32_t mode,
			  Pointer *ptr, LoopInfo *info)
{
  redef_array(iq, type, num_dims, dims);
  ptr->v = array_data(iq);
  setupDimensionLoop(info, num_dims, dims, type, naxes, axes, ptr, mode);
}
/*--------------------------------------------------------------------*/
/** Free a param_spec_list.

    \param[in] psl points to the param_spec_list for which to free the
    allocated memory.
 */
void free_param_spec_list(struct param_spec_list *psl)
{
  if (psl) {
    size_t i;
    for (i = 0; i < psl->num_param_specs; i++) {
      Param_spec *p = &psl->param_specs[i];
      free(p->dims_spec);
    }
    free(psl->param_specs);
    free(psl);
  }
}

/** Parse a standard arguments format.

    \param [in] fmt points at the format to parse.

    \returns a list of parsed parameter specifications.
 */
struct param_spec_list *parse_standard_arg_fmt(char const *fmt)
{
  struct obstack ops, ods;
  struct param_spec_list *psl = NULL;
  Param_spec *ps = NULL;
  Dims_spec *ds = NULL;
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
    Param_spec p_spec;
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
      Dims_spec d_spec;
      if (*fmt == '[') {       /* reference parameter specification */
        fmt++;
        if (*fmt == '-') {
          p_spec.ref_par = -1;  /* point at previous parameter */
          ++fmt;
        } else if (isdigit(*fmt)) { /* a specific parameter */
          char *p;
          p_spec.ref_par = strtol(fmt, &p, 10);
          fmt = p;
        } else {
          luxerror("Expected a digit or hyphen after [ in"
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
	// if (p_spec.logical_type == PS_INPUT) {
	//   luxerror("Axis parameter illegally specified for input parameter",
	// 	   0, fmt);
	//   errno = EINVAL;
        //   bad = 1;
        //   break;
	// }
        fmt++;
        if (*fmt == '-') {
          p_spec.axis_par = -1;  /* point at previous parameter */
          ++fmt;
        } else if (isdigit(*fmt)) { /* a specific parameter */
          char *p;
          p_spec.axis_par = strtol(fmt, &p, 10);
          fmt = p;
        } else {
          luxerror("Expected a digit or hyphen after { in"
                   " reference parameter specification but found %c", 0, *fmt);
          errno = EINVAL;
          bad = 1;
          break;
        }
        // TODO: parse axis modes
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
      if (*fmt == '@') {
        p_spec.omit_dimensions_equal_to_one = true;
        ++fmt;
      } else {
        p_spec.omit_dimensions_equal_to_one = false;
      }
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
            fmt++;
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
      size_t n = obstack_object_size(&ods)/sizeof(Dims_spec) - prev_ods_num_elem;
      p_spec.num_dims_spec = n;
      p_spec.dims_spec = NULL;                     /* will be filled in later */
      obstack_grow(&ops, &p_spec, sizeof(p_spec)); /* the Param_spec */
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
    psl->param_specs = (Param_spec*) calloc(param_index, sizeof(Param_spec));
    if (!psl->param_specs) {        /* malloc sets errno */
      cerror(ALLOC_ERR, 0);
      bad = 1;
    }
  }
  if (!bad) {
    /* the return parameter, if any, gets moved to the end */
    psl->num_param_specs = param_index;
    psl->return_param_index = -1; /* default, may be updated later */
    ps = (Param_spec*) obstack_finish(&ops);
    ds = (Dims_spec*) obstack_finish(&ods);

    Param_spec *pstgt;
    size_t ds_ix, j;

    pstgt = psl->param_specs;
    ds_ix = 0;
    for (i = j = 0; i < psl->num_param_specs; i++) {
      size_t j0;
      if (i == return_param_index) {
        j0 = j;
        j = psl->return_param_index = psl->num_param_specs - 1;
      }
      memcpy(pstgt + j, ps + i, sizeof(Param_spec));
      if (pstgt[j].num_dims_spec) {
        size_t size = pstgt[j].num_dims_spec*sizeof(Dims_spec);
        pstgt[j].dims_spec = (Dims_spec*) malloc(size);
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

/** Determines the common symbol type for standard arguments.

    \param[in] num_param_specs is the count of Param_spec in \p
    param_specs.

    \param[in] param_specs points at the parameter specifications to
    inspect.

    \param[in] narg is the count of arguments in \p ps.

    \param[in] ps points at the arguments.

    \returns the highest Symboltype among the arguments for which the
    common symbol type is requested, or `LUX_NO_SYMBOLTYPE` if there
    aren't any.
 */
Symboltype standard_args_common_symboltype(int32_t num_param_specs,
                                           Param_spec* param_specs,
                                           int32_t narg,
                                           int32_t* ps)
{
  Symboltype common_type = LUX_NO_SYMBOLTYPE;

  for (int param_ix = 0; param_ix < num_param_specs; ++param_ix) {
    Param_spec* pspec = &param_specs[param_ix];
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
      if (type != LUX_NO_SYMBOLTYPE
          && (common_type == LUX_NO_SYMBOLTYPE
              || type > common_type))
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
    list is allocated using malloc().  The user is responsible for
    releasing the memory (using free()) when it is no longer needed.
    See class StandardArguments for a way to release the memory
    automatically.

    \param [out] infos is the address of a pointer in which is
    returned `NULL` (in case of a problem) or a pointer to a list of
    loop information structures, one for each argument (mandatory or
    optional) and possibly one for the return symbol.  Memory for the
    list is allocated using malloc().  The user is responsible for
    releasing the memory (using free()) when it is no longer needed.
    See class StandardArguments for a way to release the memory
    automatically.

    \param[out] out_size points at a location where the element count
    of \a ptrs and \a infos is returned, if the pointer is not `NULL`.

    \return the return symbol, or -1 if an error occurred.

    \par Introduction

    The standard format can look rather daunting.  It has several
    tasks:

    1. Specify which of the parameters are input parameters, output
       parameters, or return parameters.

    2. Specify which of the parameters are optional.

    3. Specify the data types of the input, output, and return
       parameter that are made available to the back-end (through the
       `ptrs` parameter of standard_args()).  These data types may
       depend on the type of an earlier parameter.

    4. Specify the expectations for the dimensions of the input
       parameters, and specify the dimensions of the output parameters
       and the return value.  These may depend on the dimensions of an
       earlier parameter, but may have dimensions added or removed
       relative to that other parameter.  The removed dimensions may
       be explicit ones or may be identified by the contents (not the
       dimensions) of another parameter (the axis parameter).

    \par Parameter Types

    The specification parts for different parameters are separated by
    semicolons (`;`).  The specification for each parameter begins
    with a letter that identifies the parameter type.

    An input parameter (`i`) is an existing named or unnamed LUX
    symbol whose contents are going to be processed.

    An output parameter (`o`) is an existing named LUX symbol that
    will be reshaped to receive output.  Its previous contents are
    lost.  It must be a named symbol, because otherwise the user
    couldn't access the contents afterwards.

    A return parameter (`r`) is a new unnamed LUX symbol that gets
    created to act as the return value of a LUX function.  There can
    be at most one return parameter in a specification.

    \verbatim
    i;r
    \endverbatim

    The above specification says that the first parameter is an input
    parameter and the second one is a return parameter.

    No dimensions are specified for the input parameter, so it must
    contain exactly one value (be either a scalar or an array with one
    element).  No dimensions or type are specified for the return
    parameter, so it gets the same as its <em>reference
    parameter</em>, which by default is the first parameter.  A
    corresponding call to a fictitous LUX function `foo` might be `y =
    foo(3)`.

    \verbatim
    i;i;o;o
    \endverbatim

    The above specification says that the first two parameters are
    single-element input parameters, and the next two are output
    parameters with the same data type and dimensions as the first
    parameter.  An example call is `foo,3,5,x,y`.

    \par Reference Parameter

    A reference parameter can be indicated for all but the first
    parameter.  Some missing information (like a data type or a
    dimension) may be copied from the reference parameter.  The
    reference parameter is indicated by a number or a hyphen (`-`)
    between square brackets (`[]`) just after the parameter data type
    (described later), which itself follows the parameter type.  A
    number indicates a particular parameter (0 indicates the first
    one), and a hyphen indicates the parameter preceding the current
    one.  If no reference parameter is explicitly given, then the
    first parameter is the reference parameter.  The reference
    parameter must have a smaller index than the current parameter.

    \verbatim
    i;i;o[1]
    \endverbatim

    says that the output parameter's reference parameter is the one
    with index 1 (i.e., the 2nd parameter).  The output parameter gets
    the same type and dimensions as the second parameter.

    \verbatim
    i;i;o[-]
    \endverbatim

    has the same effect as the previous specification.  Now the output
    parameter's reference parameter is the parameter preceding the
    output parameter, which is the 2nd parameter as before.

    \par Optional Parameters

    An input or output parameter specification that has a question
    mark (`?`) at its very end means that that parameter is optional.
    A return parameter cannot be optional.

    \verbatim
    i;i?;r
    \endverbatim

    says that the second parameter is optional, so does not have to be
    specified.  Example calls are `y = foo(3,5)` but also `y =
    foo(3)`.

    \par Parameter Data Types

    Parameter data types may be specified for any parameter,
    immediately after the parameter type.  Explicit parameter data
    types are indicated by one of the letters `B W L Q F D S`
    corresponding to `int8` through `int64`, `float`, `double`, and
    `string`, respectively.

    An output or return parameter for which an explicit data type is
    specified gets set to that data type.

    An explicit data type for an input parameter does't say what data
    type the argument must have, but defines what data type is made
    available to the back-end.  If an input argument's data type is
    equal to the corresponding explicit input parameter's data type,
    then a pointer to that argument's data is made available.  If an
    input argument's data type differs from the corresponding explicit
    input parameter's data type, then a copy of the argument is
    created and converted to the explicit data type, and a pointer to
    that copy's data is made available instead.

    \verbatim
    iF;rL
    \endverbatim

    says that the first argument must be a single-element argument and
    that a `float` copy is made available for processing, if the
    argument isn't `float` already.  Also, a single-element `int32`
    return value is created and made available for receiving output.

    If the explicit data type is numeric (i.e., not `S`) and is
    preceded by a greater-than sign (`>`), then the data type is a
    minimum.  If the data type that would have applied if no explicit
    data type were given is less than the minimum, then that minimum
    is used instead.

    \verbatim
    i>L;r
    \endverbatim

    says that if the data type of the first argument is less than
    `int32`, then an `int32` copy is made available instead.  No
    explicit data type is given for the return parameter, so it gets
    the same as its reference parameter, which by default is the first
    parameter.  So the data type of the return parameter is equal to
    that of the first parameter, which is at least `int32`.

    \verbatim
    i>L;r>F
    \endverbatim

    is like the previous case, but now the return parameter has a
    minimum data type of `float`.  If the input parameter type is at
    least `float`, then the return parameter gets the same type as the
    input parameter.  If the input parameter type is less than
    `float`, then an `int32` version of the input is made available,
    and the return parameter is of type `float`.  If the input
    parameter type is at least `float`, then the return value gets the
    same type as the input parameter.

    If the data type specifications for more than one numerical
    parameter are followed by a caret (`^`), then all of those
    parameters get the same data type, which is equal to the greatest
    data type among them that would have applied if no carets had been
    specified.

    \verbatim
    i>L^;i>L^;iW;r^
    \endverbatim

    says that the first two input parameters and the return value get
    the same data type applied, which is the greatest one among them
    that would have applied if there were no carets.  So, if the first
    parameter is an `int64` and the second one is a `float`, then the
    parameters made available to the back-end have data types `float`,
    `float`, `int16`, and `float`, respectively.

    \par Parameter Dimensions

    Expectations for the dimensions of input parameters can be
    specified, and also how to determine the dimensions of output and
    return parameters.

    At its simplest, the dimensions are specified in a comma-separated
    list after the data type.

    \verbatim
    iF3,6;rD3
    \endverbatim

    says that the input parameter must be an array of 3 by 6 elements,
    of which a `float` version is made available to the back-end, and
    that the return value is a one-dimensional `double` array of 3
    elements.

    A greater-than sign (`>`) before a dimension number means that the
    dimension must be at least as great as the number.

    \verbatim
    i>7
    \endverbatim

    says that the first (and only) dimension must be at least 7.

    For input parameters, a colon (`:`) means to accept the current
    dimension.

    \verbatim
    i:,4,:
    \endverbatim

    says that the input parameter must have 3 dimensions of which the
    2nd one is equal to 4.

    An at sign (`@`) at the beginning of the dimensions specification
    means that dimensions equal to 1 are ignored, as far as possible.
    If omitting all dimensions equal to 1 would mean that there are no
    dimensions left, then a single dimension equal to 1 is retained.

    \verbatim
    i@:,:
    \endverbatim

    says that the input parameter must have two dimensions after
    dimensions equal to 1 are omitted.

    Dimensions for output parameters and the return value can be
    copied from the reference parameter.  An equals sign (`=`) means
    that the corresponding dimension of the reference parameter is
    copied.  If a number follows the equals sign immediately, then it
    says what the dimension of the reference parameter must be.  A
    hyphen (`-`) means that the corresponding dimension of the
    reference parameter is skipped.  A plus sign (`+`) followed by a
    number means that, relative to the reference parameter, a
    dimension equal to that number is inserted.

    \verbatim
    i7,3,2;o=,=
    \endverbatim

    says that the output parameter is an array of 7 by 3 elements.

    \verbatim
    i7,3,2;o=,-,=
    \endverbatim

    says that the output parameter is an array of 7 by 2 elements,
    because the 3 was skipped.

    \verbatim
    i7,3,2;o=,+5,=
    \endverbatim

    says that the output parameter is an array of 7 by 5 by 3
    elements.

    \verbatim
    i7,3,2;o=,5,=
    \endverbatim

    says that the output parameter is an array of 7 by 5 by 2
    elements.

    \verbatim
    i7,3,2;o=2
    \endverbatim

    produces an error because the output parameter's specification
    says that the first dimension of its reference parameter (which is
    the first parameter) should be equal to 2, but the first
    parameter's specification says that its first dimension should be
    equal to 7, and those cannot both be true.

    An asterisk (`*`) at the end of the dimensions list for an input
    parameter says that the remaining dimensions are unrestricted.

    \verbatim
    iF3*;i*
    \endverbatim

    says that the first dimension of the first input parameter must be
    equal to 3 but that any following dimensions are unrestricted, so,
    for example, a one-dimensional array of 3 elements is accepted,
    and also an array of 3 by 5 elements, or 3 by 1 by 17 elements.
    The second input parameter has no restrictions on its dimensions,
    so a scalar is acceptable, and also any array.

    An ampersand (`&`) at the end of the dimensions list for any
    parameter says that the remaining dimensions must be equal to the
    dimensions of the reference parameter.

    \verbatim
    i*;rD6&
    \endverbatim

    says that the input parameter may have any data type and
    dimensions and that the return value is a `double` array with
    dimension 6 followed by the dimensions of the reference parameter,
    which by default is the first parameter.  So, if the input
    argument is an array of 3 by 2 elements, then the return value is
    an array of 6 by 3 by 2 elements.

    A hash sign (`#`) at the end of the dimensions specification means
    that the element count of an input parameter must either be equal
    to 1 or else to the element count of the reference parameter.

    \verbatim
    i3,3,3;i#;r&
    \endverbatim

    says that the second input parameter must either have exactly one
    element or else must have the same number of elements as the
    reference parameter (the first parameter), i.e., 27.  The
    dimensions do not need to be the same, as long as the element
    count matches, so it is OK if the second input parameter has a
    single dimension equal to 27, or is a 9 by 3 array, or a 3 by 3 by
    1 by 3 array.

    \par Axis Parameters

    Some LUX functions and subroutines specify an <em>axis
    parameter</em>, which says along which dimensions of the main data
    to apply the operation.  If the operation produces one value
    (e.g., the minimum value) when running along the indicated axes,
    then the result should have the same dimensions as the main data
    except that the dimensions specified in the axis parameter should
    be omitted.  This is achieved by specifying the axis parameter's
    index between curly braces (`{}`) just before the specification of
    the dimensions, and just after the specification of the reference
    parameter, if any.

    \verbatim
    iD*;iL*;rD{1}
    \endverbatim

    says that parameter 1 (i.e., the 2nd parameter) is the axis
    parameter for the return value.  If the function is called like `y
    = foo(x,[1,2])` and `x` is an array of 4 by 100 by 200 by 3
    elements, then `y` is an array of 4 by 3 elements.

    \par Complete Syntax

    All in all, the standard format is schematically as follows, where
    something between quotes (<tt>''</tt>) stands for that literal
    character, something between non-literal square brackets (`[]`) is
    optional, something between non-literal curly braces (`{}`) is a
    group of alternatives, a non-literal pipe symbol (`|`) separates
    alternatives, and a non-literal asterisk (`*`) indicates
    repetition zero or more times:

    \verbatim
      <format> = <param-spec>[;<param-spec>]*
      <param-spec> = {'i'|'o'|'r'}[<type-spec>][<dims-spec>]['?']
      <type-spec> = {{['>']{'B'|'W'|'L'|'Q'|'F'|'D'}}|'S'}['^']
      <dims-spec> = ['@']['['<ref-par>']']['{'<axis-par>'}']
                    <dim-spec>[,<dim-spec>]*['*'|'&'|'#']
      <dim-spec> = [{['+'|'-'|'=']NUMBER|'-'|'='|':'}]*
    \endverbatim

    In words,
    - the format consists of one or more parameter specifications
      separated by semicolons `;`.
    - each parameter specification begins with an `i`, `o`, or `r`,
      followed by a type specification and a dimensions specification,
      optionally followed by a question mark `?`.
    - a type specification consists of an `S`, or else of an optional
      greater-than sign `>` followed by one of `B`, `W`, `L`, `Q`,
      `F`, or `D`.  Optionally, a `^` follows.
    - a dimensions specification consists of an optional at sign `@`,
      an a optional reference parameter number between square brackets
      `[]`, followed by an optional axis parameter number between
      curly braces `{}`, followed by one or more dimension
      specifications separated by commas `,`, optionally followed by
      an asterisk `*` or ampersand `&` or hash symbol `#`.
    - a dimension specification consists of a hyphen `-`, an equals
      sign `=`, a colon `:`, or a number preceded by a plus sign `+`,
      a hyphen `-`, or an equals sign `=`; followed by any number of
      additional instances of the preceding.

    Some of the characteristics of a parameter may depend on those of
    a reference parameter.  That reference parameter is the very first
    parameter (parameter \c ps[0]) unless a different reference
    parameters is specified at the beginning of the dimension
    specification.

    For the parameter specification \c param-spec:
    - `i` = input parameter.
    - `o` = output parameter.  An error is declared if this is not a
      named parameter.
    - `r` = return value.  For functions, there must be exactly one of
      these in the parameters specification.  Subroutines must not
      have one of these.
    - `?` = optional parameter.

    For the type specification \c type-spec:
    - `>` = the type should be at least equal to the indicated type.
    - `B` = LUX_INT8
    - `W` = LUX_INT16
    - `L` = LUX_INT32
    - `Q` = LUX_INT64
    - `F` = LUX_FLOAT
    - `D` = LUX_DOUBLE
    - `S` = LUX_STRING
    - `^` = all numerical parameters marked like this get the same
      data type, which is the greatest numerical data type among them
      that would have applied if no `^` had been specified.

    For input parameters, a copy is created with the indicated
    (minimum) type if the input parameter does not meet the condition,
    and further processing is based on that copy.  For output
    parameters, an array is created with the indicate type, unless `>`
    is specified and the reference parameter has a greater type, in
    which case that type is used.  If no explicit type is specified
    for an output parameter, then it gets the type of the reference
    parameter.

    For the reference parameter \c ref-par:
    - If absent, then 0 is taken for it (i.e., the first parameter).
    - If a number, then the indicated parameter is taken for it.
    - If `-`, then the previous parameter is taken for it.

    For the axis parameter \c axis-par:
    - The specified parameter is expected to indicate one or more
      unique axes to remove from the current parameter, which must be
      of type `r` or `o`.
    - If a number, then the indicated parameter is taken for it.
    - If `-`, then the previous parameter is taken for it.

    An at sign `@` at the beginning of the list of dimension
    specifications indicates that dimensions equal to 1 are omitted.
    For an input parameter such dimensions are omitted before
    considering the dimension specifications.  For an output or a
    return parameter such dimensions are omitted just before adjusting
    or creating the symbol.

    For the dimension specification \c dim-spec:
    - NUMBER = the current dimension has the specified size.  For
      input parameters, an error is declared if the dimension does not
      have the specified size.
    - `>`NUMBER = the current dimension has at least the specified
      size.  For input parameters, an error is declared if the
      dimension does not have at least the specified size.
    - `+`NUMBER = for output or return parameters, a new dimension with
      the specified size is inserted here.
    - `=` = for output or return parameters, the current dimension is
      taken from the reference parameter.
    - `=`NUMBER = for output or return parameters, the current dimension
      is taken from the reference parameter, and must be equal to the
      specified number.  An error is declared if the reference
      parameter's dimension does not have the indicated size
    - `-` = the corresponding dimension from the reference parameter is
      skipped.
    - `:` = for input parameters, accept the current dimension.
    - `&` = the remaining dimensions must be equal to those of the
      reference parameter.
    - `#` = the element count must be equal to 1 or to that of the
      reference parameter.
    - `*` = the remaining dimensions are unrestricted.

    Both a `+`NUMBER and a `-`NUMBER may be given in the same
    dimension specification \c dim_spec.

  */
int32_t standard_args(int32_t narg, int32_t ps[], char const *fmt,
                      Pointer **ptrs, LoopInfo **infos, size_t* out_size)
{
  int32_t lux_convert(int32_t, int32_t [], Symboltype, int32_t);

  int32_t returnSym = LUX_ONE;
  param_spec_list* psl = parse_standard_arg_fmt(fmt);
  if (!psl) {
    if (ptrs)
      *ptrs = NULL;
    if (infos)
      *infos = NULL;
    if (out_size)
      *out_size = 0;
    return luxerror("Illegal standard arguments specification %s", 0, fmt);
  }
  // the number of parameters except for the return parameter, if any
  int32_t num_in_out_params = psl->num_param_specs
    - (psl->return_param_index >= 0);
  // determine mininum and maximum required number of arguments
  int32_t nmin;
  for (nmin = num_in_out_params; nmin > 0; nmin--)
    if (!psl->param_specs[nmin - 1].is_optional)
      break;
  if (narg < nmin || narg > num_in_out_params) {
    if (ptrs)
      *ptrs = NULL;
    if (infos)
      *infos = NULL;
    if (out_size)
      *out_size = 0;
    return luxerror("Standard arguments specification asks for between "
                    "%d and %d input/output arguments but %d are specified"
                    " (%s)", 0, nmin, num_in_out_params, narg, fmt);
  } // end if (narg < nmin || narg > num_in_out_params)
  if (ptrs)
    *ptrs = (Pointer*) malloc(psl->num_param_specs*sizeof(Pointer));
  if (infos)
    *infos = (LoopInfo*) malloc(psl->num_param_specs*sizeof(LoopInfo));
  if (out_size)
    *out_size = psl->num_param_specs;

  // the final parameter values; they may be converted copies of the
  // original values.
  auto final = reinterpret_cast<int32_t*>
    (calloc(psl->num_param_specs, sizeof(int32_t)));

  Symboltype common_type
    = standard_args_common_symboltype(psl->num_param_specs,
                                      psl->param_specs,
                                      narg, ps);

  obstack o;
  obstack_init(&o);
  /* now we treat the parameters. */
  int32_t prev_ref_param = -1; // < 0 indicates no reference parameter set yet

  NumericDataDescriptor refDescr;

  for (int32_t param_ix = 0; param_ix < psl->num_param_specs; param_ix++) {
    int32_t pspec_dims_ix; /* parameter dimension specification index */
    int32_t ref_dims_ix;   /* reference dimension index */
    int32_t src_dims_ix;   /* input dimension index */
    int32_t iq, d;
    std::vector<DimensionSize_tp> tgt_dims;

    NumericDataDescriptor srcDescr;

    Param_spec* pspec = &psl->param_specs[param_ix];
    Dims_spec* dspec = pspec->dims_spec;
    if (param_ix == num_in_out_params || param_ix >= narg || !ps[param_ix]
        || !srcDescr.set_from(ps[param_ix])) {
      srcDescr.reset();
    } else if (pspec->omit_dimensions_equal_to_one && srcDescr.is_valid()) {
      srcDescr.omit_dimensions_equal_to_one();
    } // end if (param_ix == num_in_out_params || ...) else

    int32_t ref_param = pspec->ref_par;
    if (ref_param < 0)
      ref_param = (param_ix? param_ix - 1: 0);
    if (param_ix > 0             // first parameter has no reference
        && (!refDescr.is_valid() // no reference yet
            || ref_param != prev_ref_param)) { // or different from
                                               // before
      // get reference parameter's information.  If the reference
      // parameter is an output parameter, then we must get the
      // information from its *final* value
      switch (psl->param_specs[ref_param].logical_type) {
      case PS_INPUT:
        if (refDescr.set_from(ps[ref_param])) {
          if (psl->param_specs[ref_param].omit_dimensions_equal_to_one) {
            refDescr.omit_dimensions_equal_to_one();
          }
        } else {
          returnSym = luxerror("Reference parameter %d must be an array",
                               ps[param_ix], ref_param + 1);
          goto error;
        } // end if (refDescr.valid()) else
        break;
      case PS_OUTPUT: case PS_RETURN:
        if (!final[ref_param]) {
          returnSym = luxerror("Illegal forward output/return reference "
                               "parameter %d for parameter %d", 0,
                               ref_param + 1, param_ix + 1);
          goto error;
        } // end if (!final[ref_param])
        if (refDescr.set_from(final[ref_param])) {
          refDescr.omit_dimensions_equal_to_one();
        } else {
          returnSym = luxerror("Reference parameter %d must be an array",
                               final[param_ix], ref_param + 1);
          goto error;
        } // end if (refDescr.set_from(final[ref_param])) else
        break;
      } // end switch (psl->param_specs[ref_param].logical_type)
      prev_ref_param = ref_param;
    } else if (!param_ix) {
      refDescr.reset();
    } // end if (param_ix > 0 ...) else if (!param_ix)

    if (!pspec->is_optional || param_ix == num_in_out_params
        || (param_ix < narg && ps[param_ix])) {
      for (pspec_dims_ix = 0, src_dims_ix = 0, ref_dims_ix = 0;
           pspec_dims_ix < pspec->num_dims_spec; pspec_dims_ix++) {
        int src_dim_size = srcDescr.dimension(src_dims_ix);
        switch (dspec[pspec_dims_ix].type) {
        case DS_EXACT: /* an input parameter must have the exact
                          specified dimension */
        case DS_ATLEAST: // or at least the specified size
          if (pspec->logical_type == PS_INPUT) {
            if (dspec[pspec_dims_ix].type == DS_EXACT
                && src_dim_size != dspec[pspec_dims_ix].size_add) {
              returnSym = luxerror("Expected size %d for dimension %d "
                                   "but found %d", ps[param_ix],
                                   dspec[pspec_dims_ix].size_add,
                                   src_dims_ix, src_dim_size);
              goto error;
            }
            else if (dspec[pspec_dims_ix].type == DS_ATLEAST
                     && src_dim_size < dspec[pspec_dims_ix].size_add) {
              returnSym = luxerror("Expected at least size %d for dimension %d "
                                   "but found %d", ps[param_ix],
                                   dspec[pspec_dims_ix].size_add,
                                   src_dims_ix, src_dim_size);
              goto error;
            } // end if (dspec[pspec_dims_ix].type == DS_EXACT ...) else if
          } // end if (pspec->logical_type == PS_INPUT)
          /* the target gets the exact specified dimension */
          tgt_dims.push_back(dspec[pspec_dims_ix].size_add);
          ++src_dims_ix;
          ++ref_dims_ix;
          break;
        case DS_COPY_REF:       /* copy from reference */
          if (src_dims_ix >= refDescr.dimensions_count()) {
            returnSym = luxerror("Requested copying dimension %d from the "
                                 "reference parameter which has only %d "
                                 "dimensions", ps[param_ix], src_dims_ix,
                                 refDescr.dimensions_count());
            goto error;
          } // end if (src_dims_ix >= refDescr.dimensions_count())
          tgt_dims.push_back(refDescr.dimension(ref_dims_ix++));
          ++src_dims_ix;
          break;
        case DS_ADD:
          d = dspec[pspec_dims_ix].size_add;
          switch (pspec->logical_type) {
          case PS_INPUT:
            if (src_dim_size != d) {
              returnSym = luxerror("Expected size %d for dimension %d "
                                   "but found %d", ps[param_ix],
                                   d, src_dims_ix, src_dim_size);
              goto error;
            } // end if (src_dim_size != d)
            ++src_dims_ix;
            tgt_dims.push_back(d);
            break;
          case PS_OUTPUT: case PS_RETURN:
            tgt_dims.push_back(d);
            break;
          } // end switch (pspec->logical_type)
          break;
        case DS_REMOVE: case DS_ADD_REMOVE:
          switch (pspec->logical_type) {
          case PS_INPUT:
            {
              int32_t d = dspec[pspec_dims_ix].size_remove;
              if (d && refDescr.dimension(ref_dims_ix) != d) {
                returnSym = luxerror("Expected size %d for dimension %d "
                                     "but found %d", ps[param_ix],
                                     d, ref_dims_ix,
                                     refDescr.dimension(ref_dims_ix));
                goto error;
              } // end if (d && refDescr.dimension(ref_dims_ix) != d)
            }
            break;
          case PS_OUTPUT: case PS_RETURN:
            {
              int32_t d = dspec[pspec_dims_ix].size_remove;
              if (d && refDescr.dimension(ref_dims_ix) != d) {
                returnSym = luxerror("Expected size %d for dimension %d "
                                     "but found %d", ps[param_ix],
                                     d, ref_dims_ix,
                                     refDescr.dimension(ref_dims_ix));
                goto error;
              } // end if (d && ref_dims[ref_dims_ix] != d)
            }
            if (dspec[pspec_dims_ix].type == DS_ADD_REMOVE)
              tgt_dims.push_back(dspec[pspec_dims_ix].size_add);
            break;
          } // end switch (pspec->logical_type)
          ref_dims_ix++;
          break;
        case DS_ACCEPT:         /* copy from input */
          if (src_dims_ix >= srcDescr.dimensions_count()) {
            returnSym = luxerror("Cannot copy non-existent dimension %d",
                                 ps[param_ix], src_dims_ix);
            goto error;
          } else {
            tgt_dims.push_back(srcDescr.dimension(src_dims_ix++));
            ++ref_dims_ix;
          } // end if (src_dims_ix >= srcDescr.dimensions_count()) else
          break;
        default:
          returnSym = luxerror("Dimension specification type %d "
                               "not implemented yet", ps[param_ix],
                               dspec[pspec_dims_ix].type);
          goto error;
          break;
        } // end switch (dspec[pspec_dims_ix].type)
      } // end for (pspec_dims_ix = 0, tgt_dims_ix = 0, src_dims_ix = 0,...)

      Symboltype type;
      switch (pspec->logical_type) {
      case PS_INPUT:
        switch (pspec->remaining_dims) {
        case PS_EQUAL_TO_REFERENCE:
          if (refDescr.is_valid()
              && ref_dims_ix < refDescr.dimensions_count()) {
            int32_t expect = refDescr.dimensions_count()
              + src_dims_ix - ref_dims_ix;
            if (expect != srcDescr.dimensions_count()) {
              returnSym = luxerror("Expected %d dimensions but found %d",
                                   ps[param_ix],
                                   refDescr.dimensions_count()
                                   + src_dims_ix - ref_dims_ix,
                                   srcDescr.dimensions_count());
              goto error;
            } // end if (expect != num_src_dims)
            int32_t i, j;
            for (i = ref_dims_ix, j = src_dims_ix;
                 i < refDescr.dimensions_count(); i++, j++)
              if (refDescr.dimension(i) != srcDescr.dimension(j)) {
                returnSym = luxerror("Expected dimension %d equal to %d "
                                     "but found %d", ps[param_ix], i + 1,
                                     refDescr.dimension(i),
                                     srcDescr.dimension(j));
                goto error;
              } // end if (refDescr.dimension(i) != srcDescr.dimension(j))
          } else {
            returnSym = luxerror("Dimensions of parameter %d required to be "
                                 "equal to those of the reference, but no "
                                 "reference is available",
                                 ps[param_ix], param_ix + 1);
            goto error;
          } // end if (refDescr.is_valid() && ref_dims_ix <
            // refDescr.dimensions_count()) else
          break;
        case PS_ONE_OR_EQUAL_TO_REFERENCE:
          if (refDescr.is_valid()
              && ref_dims_ix < refDescr.dimensions_count()) {
            int32_t expect = refDescr.dimensions_count()
              + src_dims_ix - ref_dims_ix;
            if (expect != srcDescr.dimensions_count()) {
              returnSym = luxerror("Expected %d dimensions but found %d",
                                   ps[param_ix],
                                   refDescr.dimensions_count()
                                   + src_dims_ix - ref_dims_ix,
                                   srcDescr.dimensions_count());
              goto error;
            } // end if (expect != srcDescr.dimensions_count())
            int32_t i, j;
            for (i = ref_dims_ix, j = src_dims_ix;
                 i < refDescr.dimensions_count(); i++, j++)
              if (srcDescr.dimension(j) != 1
                  && refDescr.dimension(i) != srcDescr.dimension(j)) {
                if (refDescr.dimension(i) == 1)
                  returnSym = luxerror("Expected dimension %d equal to %d "
                                       "but found %d", ps[param_ix], i + 1,
                                       refDescr.dimension(i),
                                       srcDescr.dimension(j));
                else
                  returnSym = luxerror("Expected dimension %d equal to 1 or "
                                       "%d but found %d", ps[param_ix], i + 1,
                                       refDescr.dimension(i),
                                       srcDescr.dimension(j));
                goto error;
              } // end if (srcDescr.dimension(j) != 1 &&
                // refDescr.dimension(i) != srcDescr.dimension(j))
          } else {
            returnSym = luxerror("Dimensions of parameter %d required to be "
                                 "equal to those of the reference, but no "
                                 "reference is available",
                                 ps[param_ix], param_ix + 1);
            goto error;
          } // end if (ref_dims && ref_dims_ix <
            // refDescr.dimensions_count()) else
          break;
        case PS_ARBITRARY:
          break;
        case PS_ABSENT:
          if (!pspec_dims_ix) {     /* had no dimensions */
            /* assume dimension equal to 1 */
            if (srcDescr.dimension(src_dims_ix) != 1) {
              returnSym = luxerror("Expected dimension %d equal to 1 "
                                   "but found %d", ps[param_ix],
                                   src_dims_ix + 1,
                                   srcDescr.dimension(src_dims_ix));
              goto error;
            } else
              src_dims_ix++;
          } // end if (!pspec_dims_ix)
          if (src_dims_ix < srcDescr.dimensions_count()) {
            returnSym = luxerror("Specification (parameter %d) says %d "
                                 "dimensions but source has %d dimensions",
                                 ps[param_ix], param_ix, src_dims_ix,
                                 srcDescr.dimensions_count());
            goto error;
          } // end if (src_dims_ix < srcDescr.dimensions_count())
          break;
        } // end switch (pspec->remaining_dims)
        iq = ps[param_ix];
        type = symbol_type(iq);
        if (pspec->common_type)
          type = common_type;
        else if ((pspec->data_type_limit == PS_LOWER_LIMIT
                  && type < pspec->data_type)
                 || (pspec->data_type_limit == PS_EXACT
                     && type != pspec->data_type
                     && pspec->data_type != LUX_NO_SYMBOLTYPE))
          type = pspec->data_type;
        iq = lux_convert(1, &iq, type, 1);
        break;
      case PS_OUTPUT: case PS_RETURN:
        switch (pspec->remaining_dims) {
        case PS_ABSENT:
          break;
        case PS_EQUAL_TO_REFERENCE:
          if (ref_dims_ix < refDescr.dimensions_count()) {
            /* append remaining dimensions from reference parameter*/
            while (ref_dims_ix < refDescr.dimensions_count()) {
              tgt_dims.push_back(refDescr.dimension(ref_dims_ix));
              ++src_dims_ix;
              ++ref_dims_ix;
            }
          } // end if (ref_dims_ix < refDescr.dimensions_count())
          break;
        case PS_ARBITRARY:
          returnSym = luxerror("'Arbitrary' remaining dimensions makes no "
                               "sense for an output or return parameter "
                               " (number %d)", 0, param_ix + 1);
          goto error;
        } // end switch (pspec->remaining_dims)
        if (pspec->axis_par > -2) {
          /* We have an axis parameter specified for this one. */
          int32_t axis_param = pspec->axis_par;
          if (axis_param == -1) /* points at previous parameter */
            axis_param = param_ix - 1;

          if (axis_param < 0 || axis_param >= num_in_out_params) {
            returnSym = luxerror("Axis parameter %d for parameter %d is "
                                 "out of bounds", 0, axis_param, param_ix + 1);
            goto error;
          } // end if (axis_param < 0 || axis_param >= narg)
          if (axis_param == param_ix) {
            returnSym = luxerror("Parameter %d cannot be its own axis"
                                 " parameter", 0, param_ix + 1);
            goto error;
          } // end if (axis_param == param_ix)
          if (final[axis_param]) { // axis parameter exists
            // the axis parameter describes which axes of the
            // reference parameter to process.  The output or return
            // value gets the same dimensions as the reference
            // parameter except that the dimensions mentioned in the
            // axis parameter are omitted.
            tgt_dims = refDescr.dimensions();
            int32_t aq = ps[axis_param];
            if (!symbolIsNumerical(aq)) {
              returnSym = luxerror("Axis parameter %d is not numerical for"
                                   " parameter %d", 0,
                                   axis_param + 1, param_ix + 1);
              goto error;
            } // end if (!symbolIsNumerical(aq))
            aq = lux_long(1, &aq);
            int32_t nAxes;
            Pointer axes;
            numerical(aq, NULL, NULL, &nAxes, &axes);
            for (int32_t j = 0; j < nAxes; j++) {
              if (axes.l[j] < 0 || axes.l[j] >= tgt_dims.size()) {
                returnSym = luxerror("Axis %d out of bounds for"
                                     " parameter %d", 0,
                                     axes.l[j], param_ix + 1);
                goto error;
              } // end if (axes.l[j] < 0 || axes.l[j] >= tgt_dims_ix)
              tgt_dims[axes.l[j]] = 0; // flags removal.  Note: no check
                                       // for duplicate axes
            } // end for (j = 0; j < nAxes; j++)
            int32_t k;
            /* remove flagged dimensions */
            tgt_dims.erase(std::remove(tgt_dims.begin(), tgt_dims.end(), 0),
                           tgt_dims.end());
          } else {
            // axis parameter does not exist; treat as 1D
            tgt_dims.clear();   // remove all target dimensions
            // going to produce a scalar
          }
        } // end if (pspec->axis_par > -2)
        if (pspec->omit_dimensions_equal_to_one) {
          if (tgt_dims.size() > 0) {
            tgt_dims.erase(std::remove(tgt_dims.begin(),
                                       tgt_dims.end(),
                                       1),
                           tgt_dims.end());
            if (!tgt_dims.size()) {
              tgt_dims.push_back(1);
            }
          }
        }
        if (param_ix == num_in_out_params) {      /* a return parameter */
          if (ref_param >= 0) {
            type = symbol_type(ps[ref_param]);
          } else if (pspec->data_type != LUX_NO_SYMBOLTYPE) {
            type = pspec->data_type;
          } // end if (ref_param >= 0) else
          if (pspec->common_type
              && common_type != LUX_NO_SYMBOLTYPE) {
            type = common_type;
          } else if (pspec->data_type_limit == PS_EXACT
                     && pspec->data_type != LUX_NO_SYMBOLTYPE) {
            type = pspec->data_type;
          } else if (pspec->data_type_limit == PS_LOWER_LIMIT
                     && type < pspec->data_type) {
            type = pspec->data_type;
          }
          if (tgt_dims.size())
            iq = returnSym = array_scratch(type, tgt_dims.size(),
                                           tgt_dims.data());
          else
            iq = returnSym = scalar_scratch(type);
        } else { // if (param_ix == num_in_out_params) else
          // not a return parameter, so an output parameter
          iq = ps[param_ix];
          type = symbol_type(iq);
          if (symbol_class(iq) == LUX_UNUSED
              || ((pspec->data_type_limit == PS_LOWER_LIMIT
                   && type < pspec->data_type)
                  || (pspec->data_type_limit == PS_EXACT
                      && pspec->data_type != LUX_NO_SYMBOLTYPE
                      && type != pspec->data_type)))
            type = pspec->data_type;
          if (tgt_dims.size())
            redef_array(iq, type, tgt_dims.size(), tgt_dims.data());
          else
            redef_scalar(iq, type, NULL);
        } // end if (param_ix == num_in_out_params) else
        break;
      } // end switch (pspec->logical_type)
      final[param_ix] = iq;
      {
        LoopInfo li;
        Pointer p;
        standardLoop(iq, 0, SL_ALLAXES, symbol_type(iq), &li, &p, NULL,
                     NULL, NULL);
        if (infos)
          (*infos)[param_ix] = li;
        if (ptrs)
          (*ptrs)[param_ix] = p;
      }
    } else { // if (!pspec->is_optional || ...) else
      if (infos)
        memset(&(*infos)[param_ix], 0, sizeof(LoopInfo));
      if (ptrs)
        (*ptrs)[param_ix].v = NULL;
    } // end if (!pspec->is_optional || ...) else

  } // end for (param_ix = 0; param_ix < psl->num_param_specs; param_ix++)

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
  if (out_size)
    *out_size = 0;
  return returnSym;
}

int32_t standard_args(int32_t narg, int32_t ps[], const std::string& fmt,
                      std::vector<Pointer>& ptrs, std::vector<LoopInfo>& infos)
{
  Pointer* these_ptrs;
  LoopInfo* these_infos;
  size_t out_size;
  int32_t result = standard_args(narg, ps, fmt.c_str(), &these_ptrs,
                                 &these_infos, &out_size);
  ptrs.clear();
  if (these_ptrs)
    ptrs.insert(ptrs.begin(), &these_ptrs[0], &these_ptrs[out_size - 1]);
  infos.clear();
  if (these_infos)
    infos.insert(infos.begin(), &these_infos[0], &these_infos[out_size - 1]);
  return result;
}

/// The input is assumed to be valid!
void
LoopInfo::setAxes(int32_t nAxes, int32_t* axes, int32_t mode)
{
  memcpy(this->axes, axes, nAxes*sizeof(*this->axes));
  setAxisMode(this, mode);
}

void
LoopInfo::setOneDimensional()
{
  dims[0] = nelem;
  ndim = 1;
  naxes = 1;
  axes[0] = 0;
  rearrangeDimensionLoop(this);
}
