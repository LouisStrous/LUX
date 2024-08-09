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
#include <errno.h>
#include <ctype.h>
#include "axis.hh"

//-------------------------------------------------------------------------
/** Adjusts LoopInfo for axis treatment.

    \param[in,out] info is a pointer to the \c LoopInfo structure to
    adjust.

    \param[in] nAxes is the number of axes.

    \param[in] axes points to the array of axes.

    \param[in] mode says how to treat the axes.

    \returns `LUX_OK` for success, `LUX_ERROR` for failure.
 */
Symbol
LoopInfo::setAxes(int32_t nAxes, const int32_t *axes, int32_t mode)
{
  int32_t i;
  int32_t temp[MAX_DIMS];

  if (mode & SL_TAKEONED)       // take data as 1D
    nAxes = 0;                  // treat as 1D
  else if (mode & SL_ALLAXES) { // select all axes
    nAxes = this->ndim;
    axes = NULL;                // treat as if all axes were specified
  } else if ((mode & SL_NEGONED)        // negative-axis treatment
             && nAxes == 1              // one axis specified 
            && *axes < 0)      // and it is negative
    nAxes = 0;

  if ((mode & SL_ONEAXIS)       // only one axis allowed
      && nAxes > 1)             // and more than one selected
    return luxerror("Only one axis allowed", -1);

  // check the specified axes for legality
  if (nAxes && axes) {
    for (i = 0; i < nAxes; i++) // check all specified axes
      if (axes[i] < 0           // axis is negative
          || axes[i] >= this->ndim)     // or too great
        return luxerror("Illegal axis %1d", -1, axes[i]);
    if (mode & SL_UNIQUEAXES) {         // no axis must occur more than once
      zerobytes(temp, this->ndim*sizeof(int32_t));
      for (i = 0; i < nAxes; i++)
        if (temp[axes[i]]++)
          return luxerror("Axis %1d illegally specified more than once",
                          -1, axes[i]);
    }
  }
  this->naxes = nAxes;
  if (nAxes && axes)
    memcpy(this->axes, axes, nAxes*sizeof(*axes));
  this->setAxisMode(mode);
  return 0;
}

Symbol
LoopInfo::setAxes(const std::vector<int32_t>& axes, int32_t mode)
{
  return setAxes(static_cast<int32_t>(axes.size()), &axes[0], mode);
}
//-------------------------------------------------------------------------
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
int
LoopInfo::setupDimensionLoop(int32_t ndim, int32_t const *dims,
                             Symboltype type, int32_t naxes,
                             int32_t const *axes,
                             Pointer *data, int32_t mode)
{
  int32_t       i;
  size_t        size;

  /* first copy the arguments into the structure
     NOTE: no checking to see if the values are legal. */

  if (dims && ndim >= 0) {
    this->ndim = ndim;          // the dimensions count
    memmove(this->dims, dims, ndim*sizeof(int32_t)); // the dimensions
  } else {                   // a scalar
    // treat as single-element array
    this->ndim = 1;
    this->dims[0] = 1;
  }
  this->naxes = naxes;
  if (naxes > 0) {
    if (axes)
      memmove(this->axes, axes, naxes*sizeof(*axes));
    else                        // do all axes
      for (i = 0; i < this->naxes; i++)
        this->axes[i] = i;
  }

  // calculate the number of elements in the array.
  size = 1;
  size_t oldsize;
  for (i = 0; i < this->ndim; i++) {
    oldsize = size;
    size *= this->dims[i];
    if (size < oldsize) {       // overflow occurred
      return EDOM;
    }
  }
  if (size >= INT64_MAX/2)
    return EDOM;        // element count may be too large for an off_t

  this->nelem = size;
  // the type of data: LUX_INT8, ..., LUX_DOUBLE
  this->type = type;
  // a pointer to a pointer to the data
  this->data = data;
  this->data0 = data->v;

  // now derive auxiliary data
  /* the step size per dimension (measured in elements), i.e. by how many
   elements one has to advance a suitable pointer to point at the next
   element in the selected dimension: */
  this->singlestep[0] = 1;
  for (i = 0; i < this->ndim - 1; i++)
    this->singlestep[i + 1] = this->singlestep[i]*this->dims[i];
  // the number of bytes per data element:
  this->stride = lux_type_size[type];

  this->setAxisMode(mode);
  return 0;
}

int32_t
LoopInfo::set(size_t index)
{
  if (index > this->nelem)
    return EDOM;
  this->data->ui8 = static_cast<uint8_t*>(this->data0) + index*this->stride;
  for (int i = 0; i < this->rndim; ++i)
  {
    this->coords[i] = index % this->rdims[i];
    index /= this->rdims[i];
  }
  return 0;
}

//-----------------------------------------------------------------------

/// Advance along a loop.  The coordinates and the pointer to the data are
/// advanced, taking into account the dimensional structure of the data and the
/// configured axes.
///
/// \param[in,out] ptr points at the current data element.
///
/// \returns the index of the first loop axis that is not yet completely
/// traversed.
///
/// For example, if the array has 4 by 5 by 6 elements, then advancement from
/// element (2,0,0) (to element (3,0,0)) yields 0, (3,0,0) yields 1, (3,4,0)
/// yields 2, and (3,4,5) yields 3.
int32_t
LoopInfo::advanceLoop(const void* ptr)
{
  int32_t i, done;

  if (this->advanceaxis >= this->rndim) // already done
    done = this->rndim;
  else {
    // advance pointer
    auto p = static_cast<char**>(const_cast<void*>(ptr));
    *p += this->step[this->advanceaxis]*this->stride;

    done = this->advanceaxis;   // default: not done yet

    // update coordinates
    for (i = this->advanceaxis; i < this->rndim; i++) {
      if (++(this->coords[i]) < this->rdims[i])
        break;                  // not yet at end of this dimension
      // if we get here, we are at the end of a dimension
      this->coords[i] = 0;      // back to start
      done = i + 1;             // keep track of last advanced dimension
      if (done < this->rndim)
        *p += this->step[i + 1]*this->stride;
    }
  }
  return done;
}
//-----------------------------------------------------------------------
/** Are we at the start of the loop?

    \param[in] info is the LoopInfo to query.

    \returns 1 if all coordinates are equal to 0, and 0 otherwise.
    Can be used to test if the loop is back at the start again --
    i.e., has completed.  See also advanceLoop().
*/
int32_t
LoopInfo::loopIsAtStart() const
{
  int32_t state = 1, i;

  for (i = 0; i < this->rndim; i++)
    state &= (this->coords[i] == 0);
  return state;
}
//-----------------------------------------------------------------------
/** Rearranges dimensions suitable for walking along the selected
    axes.

    \param[in,out] info points at the LoopInfo that gets adjusted.
*/
void
LoopInfo::rearrangeDimensionLoop()
{
  int32_t       axis, i, temp[MAX_DIMS], j, axisIndex, mode, axis2;

  /*
   <this->axisindex>: index to the position in this->axes where the axis is
      stored along which is to be traveled
   <this->mode>: flags that indicate the desired treatment of the axes
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

  axisIndex = this->axisindex;
  mode = this->mode;
  if (axisIndex < this->naxes) {
    // actually have an axis to put first
    axis = this->axes[axisIndex];
    switch (mode & (SL_EACHCOORD | SL_AXISCOORD | SL_AXESBLOCK)) {
      case SL_EACHCOORD: default:
        // put desired axis at front; leave order of remaining axes
        // get rearranged step sizes
        this->rsinglestep[0] = this->singlestep[axis];
        memcpy(&this->rsinglestep[1], this->singlestep, axis*sizeof(int32_t));
        memcpy(&this->rsinglestep[axis + 1], &this->singlestep[axis + 1],
               (this->ndim - axis - 1)*sizeof(int32_t));

        // get rearranged dimensions
        this->rndim = this->ndim;
        this->rdims[0] = this->dims[axis];
        memcpy(&this->rdims[1], this->dims, axis*sizeof(int32_t));
        memcpy(&this->rdims[axis + 1], &this->dims[axis + 1],
               (this->ndim - axis - 1)*sizeof(int32_t));

        // get mappings between original and rearranged axes
        this->raxes[0] = axis;
        for (i = 0; i < axis; i++)
          this->raxes[i + 1] = i;
        for (i = axis + 1; i < this->ndim; i++)
          this->raxes[i] = i;
        for (i = 0; i < this->ndim; i++)
          this->iraxes[this->raxes[i]] = i;
        break;
      case SL_AXISCOORD:
        /* only need the coordinate in the desired axis; we can lump the
           axes preceding the selected one together into one pseudo-dimension,
           and likewise for the axes following the selected one: this may
           speed up the loop traversal.  We can only lump together contiguous
           blocks of axes, so we cannot (in general) lump all remaining
           dimensions together. */
        this->rdims[0] = this->dims[axis]; // selected axis goes first
        this->raxes[0] = axis;
        this->iraxes[axis] = 0;
        if (axis) {             // selected axis was not the first one
          // lump earlier axes together
          this->rdims[1] = 1;
          for (i = 0; i < axis; i++) {
            this->rdims[1] *= this->dims[i];
            this->iraxes[i] = 1;
          }
          this->raxes[1] = 0;   // smallest axis in this lump
          j = 2;
        } else
          j = 1;
        if (axis < this->ndim - 1) { // selected axis is not the last one
          // lump later axes together
          this->rdims[j] = 1;
          for (i = axis + 1; i < this->ndim; i++) {
            this->rdims[j] *= this->dims[i];
            this->iraxes[i] = j;
          }
          this->raxes[j++] = axis + 1; // smallest axis in this lump
        }
        this->rndim = j;

        // get step sizes
        for (i = 0; i < this->rndim; i++)
          this->rsinglestep[i] = this->singlestep[this->raxes[i]];
        break;
      case SL_AXESBLOCK:
        // the active axis goes first, then the remaining selected axes,
        // and then the ones that were not selected; in ascending order
        // within each group
        zerobytes(temp, this->ndim*sizeof(int32_t));

        this->rdims[0] = this->dims[axis];
        temp[axis] = 1;
        this->raxes[0] = axis;
        this->rsinglestep[0] = this->singlestep[axis];

        // treat the remaining selected axes
        j = 1;
        for (i = 0; i < this->naxes; i++) {
          axis2 = this->axes[i];
          if (axis2 == axis)
            continue;           // already have the active one
          this->rdims[j] = this->dims[axis2];
          temp[axis2] = 1;
          this->raxes[j] = axis2;
          this->rsinglestep[j++] = this->singlestep[axis2];
        }
        // now all selected axes have 1 in temp
        for (i = 0; j < this->ndim; i++)
          if (!temp[i]) {       // this axis not yet included
            this->rdims[j] = this->dims[i];
            this->raxes[j] = i;
            this->rsinglestep[j++] = this->singlestep[i];
          }
        // fix this->iraxes
        for (i = 0; i < this->ndim; i++)
          this->iraxes[this->raxes[i]] = i;

        this->rndim = this->ndim;
        break;
    }
  } else {
    if (this->naxes) {          // do have axes
      // just keep the original order
      memcpy(this->rsinglestep, this->singlestep, this->ndim*sizeof(int32_t));
      memcpy(this->rdims, this->dims, this->ndim*sizeof(int32_t));
      this->rndim = this->ndim;
      for (i = 0; i < this->ndim; i++)
        this->raxes[i] = this->iraxes[i] = i;
    } else {                    // treat as 1D
      this->rdims[0] = this->dims[0];
      for (i = 1; i < this->ndim; i++)
        this->rdims[0] *= this->dims[i];
      this->rndim = this->ndim? 1: 0; // 0 means scalar
      this->rsinglestep[0] = 1;
      this->raxes[0] = 0;
      for (i = 0; i < this->ndim; i++)
        this->iraxes[i] = 0;
    }
  }

  // prepare step sizes for use in advanceLoop()
  memcpy(this->step, this->rsinglestep, this->rndim*sizeof(int32_t));
  if (this->rndim > 0)
    for (i = this->rndim - 1; i; i--)
      this->step[i] -= this->step[i - 1]*this->rdims[i - 1];

  for (i = 0; i < this->rndim; i++)
    this->coords[i] = 0;        // initialize coordinates
  this->data->v = this->data0;  // initialize pointer
}
//-------------------------------------------------------------------------
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
void
LoopInfo::setAxisMode(int32_t mode) {
  if ((mode & SL_EACHBLOCK) == SL_EACHBLOCK)
    this->advanceaxis = this->naxes;
  else if (mode & SL_EACHROW)
    this->advanceaxis = 1;
  else
    this->advanceaxis = 0;
  this->axisindex = 0;
  this->mode = mode;

  // rearrange the dimensions for the first pass
  this->rearrangeDimensionLoop();
}
//-----------------------------------------------------------------------
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
int32_t
LoopInfo::dimensionLoopResult1(int32_t tmode, Symboltype ttype,
                               int32_t nMore, int32_t const * more,
                               int32_t nLess, int32_t const * less,
                               LoopInfo *tinfo, Pointer *tptr)
{
  int32_t       target, n, i, nOmitAxes = 0, omitAxes[MAX_DIMS];
  Pointer       ptr;

  std::vector<int32_t> dims {this->dims, this->dims + this->ndim};
  std::vector<int32_t> axes {this->axes, this->axes + this->naxes};
  for (i = 0; i < ndim; i++)
    omitAxes[i] = 0;
  // it is assumed that 0 <= axes[i] < ndim for i = 0..naxes-1

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
      if (dims[axes[i]] == 1 && less[i] > 1) // this dimension becomes 1
        omitAxes[nOmitAxes++] = i;
    }
    if (!(tmode & SL_ONEDIMS) && nOmitAxes) {
      // remove dimensions corresponding to axes mentioned in omit[]
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
      // update dims[]
      std::vector<int32_t> newvalues;
      if (j) {
        for (i = 0; i < j; i++)
          newvalues.push_back(dims[newIndexToOld[i]]);
        dims = newvalues;
      }
      // update axes[] and naxes
      for (i = 0; i < naxes; i++)
        retain[i] = 1;
      for (i = 0; i < nOmitAxes; i++)
        retain[omitAxes[i]] = 0;
      // now retain[i] says whether axis i is to be retained
      for (i = j = 0; i < naxes; i++)
        if (retain[i])
          newIndexToOld[j++] = i;
      if (j) {
        newvalues.clear();
        for (i = 0; i < j; i++)
          newvalues.push_back(axes[newIndexToOld[i]]);
        axes = newvalues;
      }
    }
  }

  /* do things work out OK if nMore and nLess are nonzero at the same
     time? */

  if (nMore && more) {
    if (nMore < 1)
      return luxerror("Illegal number %d of dimensions to add", -1, nMore);
    if (nMore + dims.size() > MAX_DIMS)
      return luxerror("Requested total number of dimensions %d "
                      "exceeds allowed maximum %d", -1, nMore + dims.size(),
                      MAX_DIMS);
    if (nMore + axes.size() > MAX_DIMS)
      return luxerror("Total number of axes %d after growing "
                      "exceeds allowed maximum %d", nMore + axes.size(),
                      MAX_DIMS);
    for (i = 0; i < nMore; i++)
      if (more[i] < 1)
        return luxerror("Illegal size %d requested for new dimension %d", -1,
                      more[i], i);
    dims.insert(dims.begin(), more, more + nMore);
    for (i = 0; i < naxes; i++) // adjust axes for new dimensions
      axes[i] += nMore;
    axes.insert(axes.begin(), nMore, 0);
    for (i = 0; i < nMore; i++)
      axes[i] = i;
  }

  if (!less && !more) {
    switch (tmode & (SL_COMPRESS | SL_COMPRESSALL)) {
    case SL_COMPRESS:           /* output has same dimensions as source,
                                   except that first selected axis dimension
                                   is omitted -- or set to 1 */
    case SL_COMPRESSALL:        /* output has same dimensions as source,
                                   except that all selected axis dimensions
                                   are omitted -- or set to 1 */
      if ((tmode & (SL_COMPRESS | SL_COMPRESSALL)) == SL_COMPRESS)
        n = 1;                  // omit one axis only
      else
        n = this->naxes;        // omit all axes

      if (this->axes) {        // have specific axes
        if (tmode & SL_ONEDIMS)  // replace by dimension of 1
          for (i = 0; i < n; i++)
            dims[this->axes[i]] = 1;
        else {                  // really omit
          if (this->naxes      // no fake 1D
              && dims.size() > n) { // and no dimensions left either
            for (i = 0; i < n; i++)
              dims[this->axes[i]] = 0; // set omitted dims to 0
            // remove elements equal to 0
            auto e = std::remove_if(dims.begin(), dims.end(),
                                    [](auto x){return (x == 0);});
            dims.erase(e, dims.end());
          } else                // it yields a single number -> scalar
            dims.clear();       // scalar
        } // end of if (mode & ... )
      } else {                  // assume all axes were specified
        dims.erase(dims.begin(), dims.begin() + n);
      }
      axes.erase(axes.begin(), axes.begin() + n);
      break;
    }
  }

  // create the output symbol
  if (dims.size() > 1
      || (dims.size() == 1 && dims[0] > 1))
  {            // get an array
    target = array_scratch(ttype, dims.size(), &dims[0]);
    ptr.i32 = (int32_t *) array_data(target);
  } else {                      // get a scalar
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
        ptr.i32 = &scalar_value(target).i32;
    }
  }

  *tptr = ptr;                  // store pointer to output data
  /* we must do this first so the correct value is stored in the data0
   element of tinfo. */

  // fill loop structure for output symbol
  tinfo->setupDimensionLoop(dims.size(), &dims[0], ttype, axes.size(), &axes[0],
                            tptr, tmode);
  return target;
}
//-----------------------------------------------------------------------
/** Create an appropriate result symbol without adjusting dimensions.
    The arguments and return value are the same as the corresponding
    ones of dimensionLoopResult1(), except that no `more` and `less`
    arguments are specified.
 */
int32_t
LoopInfo::dimensionLoopResult(LoopInfo *tinfo, Symboltype ttype, Pointer *tptr)
{
  return this->dimensionLoopResult1(this->mode, ttype,
                                    0, NULL, 0, NULL, tinfo, tptr);
}
//-----------------------------------------------------------------------
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

  if (axisSym > 0) {            // <axisSym> is a regular symbol
    if (!symbolIsNumerical(axisSym))
      return luxerror("Need a numerical argument", axisSym);
    i = lux_long(1, &axisSym);  // get a LONG copy
    numerical(i, NULL, NULL, &nAxes, &axes); // get info
  } else {
    nAxes = 0;
    axes.i32 = NULL;
  }
  return standardLoop0(data, nAxes, axes.i32, mode, outType,
                       src, srcptr, output, trgt, trgtptr);

}
//-----------------------------------------------------------------------
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

  if (axisSym > 0) {            // <axisSym> is a regular symbol
    if (!symbolIsNumerical(axisSym))
      return luxerror("Need a numerical argument", axisSym);
    i = lux_long(1, &axisSym);  // get a LONG copy
    numerical(i, NULL, NULL, &nAxes, &axes); // get info
  } else {
    nAxes = 0;
    axes.i32 = NULL;
  }
  int32_t result = standardLoop1(source, nAxes, axes.i32, srcMode,
                                 srcinf, srcptr, nMore, more, nLess, less,
                                 tgtType, tgtMode, target, tgtinf, tgtptr);
  return result;
}
//-----------------------------------------------------------------------
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
  int32_t       *dims, ndim, i, temp[MAX_DIMS];
  int32_t lux_convert(int32_t, int32_t [], Symboltype, int32_t);

  // check if <data> is of proper class, and get some info about it
  if (numerical(data, &dims, &ndim, NULL, srcptr) == LUX_ERROR)
    return LUX_ERROR;           // some error

  if (mode & SL_TAKEONED)       // take data as 1D
    nAxes = 0;                  // treat as 1D
  else if (mode & SL_ALLAXES) { // select all axes
    nAxes = ndim;
    axes = NULL;                // treat as if all axes were specified
  } else if ((mode & SL_NEGONED)        // negative-axis treatment
             && nAxes == 1              // one axis specified
             && *axes < 0)      // and it is negative
    nAxes = 0;

  if ((mode & SL_ONEAXIS)       // only one axis allowed
      && nAxes > 1)             // and more than one selected
    return luxerror("Only one axis allowed", -1);

  // check the specified axes for legality
  if (nAxes && axes) {
    for (i = 0; i < nAxes; i++) // check all specified axes
      if (axes[i] < 0           // axis is negative
          || axes[i] >= ndim)   // or too great
        return luxerror("Illegal axis %1d", -1, axes[i]);
    if (mode & SL_UNIQUEAXES) {         // no axis must occur more than once
      zerobytes(temp, ndim*sizeof(int32_t));
      for (i = 0; i < nAxes; i++)
        if (temp[axes[i]]++)
          return luxerror("Axis %1d illegally specified more than once",
                          -1, axes[i]);
    }
  }

  // The input is of legal classes and types.
  int original_data = data;
  if ((mode & SL_SRCUPGRADE)    // upgrade source if necessary
      && (symbol_type(data) < outType))         { // source needs upgrading
    data = lux_convert(1, &data, outType, 1); // upgrade
    // need new data's srcptr.  Also, dims may now be stale
    numerical(data, &dims, &ndim, NULL, srcptr);
  }

  int status;
  if (status = src->setupDimensionLoop(ndim, dims, symbol_type(data), nAxes,
                                       axes, srcptr, mode))
    return luxerror("Error %d\n", original_data, status);

  if (output) {                         // user wants an output symbol
    if (((mode & SL_UPGRADE)
         && outType < src->type)
        || (mode & SL_KEEPTYPE))
      trgt->type = src->type;   // output type equal to source type
    else
      trgt->type = outType;     // take specified output type

    *output = src->dimensionLoopResult(trgt, trgt->type, trgtptr);
    if (*output == LUX_ERROR)
      // but didn't get one
      return LUX_ERROR;
  }

  if (mode & SL_TAKEONED) { // mimic 1D array
    src->dims[0] = src->rdims[0] = src->nelem;
    src->ndim = src->rndim = 1;
  }

  return LUX_OK;
}
//-----------------------------------------------------------------------
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

  // check if <source> is of proper class, and get some info about it
  if (numerical_or_string(source, &dims, &ndim, NULL, srcptr) == LUX_ERROR)
    return LUX_ERROR;           // some error

  if (srcMode & SL_TAKEONED)    // take data as 1D
    nAxes = 0;                  // treat as 1D
  else if (srcMode & SL_ALLAXES) { // select all axes
    nAxes = ndim;
    axes = NULL;                // treat as if all axes were specified
  } else if ((srcMode & SL_NEGONED)     // negative-axis treatment
             && nAxes == 1              // one axis specified
             && *axes < 0)      // and it is negative
    nAxes = 0;

  if ((srcMode & SL_ONEAXIS)    // only one axis allowed
      && nAxes > 1)             // and more than one selected
    return luxerror("Only one axis allowed", -1);

  // check the specified axes for legality
  if (nAxes && axes) {
    for (i = 0; i < nAxes; i++) // check all specified axes
      if (axes[i] < 0           // axis is negative
          || axes[i] >= ndim)   // or too great
        return luxerror("Illegal axis %1d", -1, axes[i]);
    if (srcMode & SL_UNIQUEAXES) { // no axis must occur more than once
      zerobytes(temp, ndim*sizeof(int32_t));
      for (i = 0; i < nAxes; i++)
        if (temp[axes[i]]++)
          return luxerror("Axis %1d illegally specified more than once",
                          -1, axes[i]);
    }
  }

  // The input is of legal classes and types.

  if ((srcMode & SL_SRCUPGRADE)         // upgrade source if necessary
      && (symbol_type(source) < tgtType))       { // source needs upgrading
    source = lux_convert(1, &source, tgtType, 1); // upgrade
    numerical_or_string(source, NULL, NULL, NULL, srcptr);
  }

  srcinf->setupDimensionLoop(ndim, dims, symbol_type(source), nAxes,
                             axes, srcptr, srcMode);

  if (target) {                         // user wants an output symbol
    if (((tgtMode & SL_UPGRADE)
         && tgtType < srcinf->type)
        || (tgtMode & SL_KEEPTYPE))
      tgtinf->type = srcinf->type; // output type equal to source type
    else
      tgtinf->type = tgtType;   // take specified output type

    *target = srcinf->dimensionLoopResult1(tgtMode, tgtinf->type,
                                           nMore, more, nLess, less, tgtinf,
                                           tgtptr);
    if (*target == LUX_ERROR)
      // but didn't get one
      return LUX_ERROR;
  }

  if (srcMode & SL_TAKEONED) { // mimic 1D array
    srcinf->dims[0] = srcinf->rdims[0] = srcinf->nelem;
    srcinf->ndim = srcinf->rndim = 1;
  }

  return LUX_OK;
}
//-----------------------------------------------------------------------
/** Rearranges dimensions for a next loop through the array.

    \param[in,out] info is the LoopInfo to adjust.

    \returns 1 if OK, 0 if we're beyond the last specified axis.
*/
int32_t
LoopInfo::nextLoop()
{
  if (++(this->axisindex) >= this->naxes)
    return 0;
  this->rearrangeDimensionLoop();
  return LUX_OK;
}
//-----------------------------------------------------------------------
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
  info1->rearrangeDimensionLoop();
  info2->rearrangeDimensionLoop();
  return 1;
}
//-----------------------------------------------------------------------
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
void
LoopInfo::subdataLoop(int32_t *range)
{
  // LS 30apr98
   int32_t      i, offset;

  offset = 0;
  for (i = 0; i < this->ndim; i++) {
    offset += range[2*i]*this->singlestep[i];
    this->rdims[this->raxes[i]] = range[2*i + 1] - range[2*i] + 1;
  }

  memcpy(this->step, this->rsinglestep, this->ndim*sizeof(int32_t));

  for (i = this->ndim - 1; i > 0; i--)
    this->step[i] -= this->step[i - 1]*this->rdims[i - 1];

  (*this->data).ui8 = (uint8_t *) this->data0 + offset*this->stride;
}
//--------------------------------------------------------------------
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
void
LoopInfo::rearrangeEdgeLoop(LoopInfo *trgt, int32_t index)
{
  // LS 23oct98 LS 2feb99
  uint8_t       back;
  int32_t       axis, trgtstride, i;
  Pointer       *trgtdata;
  void  *trgtdata0;

  back = index % 2;             // 0 -> front edge, 1 -> back edge
  index /= 2;                   // the target axis
  axis = this->axes[index];

  // put target dimension in the back; get rearranged step sizes
  memcpy(this->rsinglestep, this->singlestep, axis*sizeof(int32_t));
  memcpy(this->rsinglestep + axis, this->singlestep + axis + 1,
         (this->ndim - axis - 1)*sizeof(int32_t));
  this->rsinglestep[this->ndim - 1] = this->singlestep[axis];

  // get rearranged dimensions
  this->rndim = this->ndim;
  memcpy(this->rdims, this->dims, axis*sizeof(int32_t));
  memcpy(this->rdims + axis, this->dims + axis + 1,
         (this->ndim - axis - 1)*sizeof(int32_t));
  this->rdims[this->ndim - 1] = this->dims[axis];

  // get mappings between original and rearranged axes
  for (i = 0; i < axis; i++)
    this->raxes[i] = i;
  for (i = axis; i < this->ndim - 1; i++)
    this->raxes[i] = i + 1;
  this->raxes[this->ndim - 1] = axis;
  for (i = 0; i < this->ndim; i++)
    this->iraxes[this->raxes[i]] = i;

  // prepare step sizes for use in advanceLoop()
  memcpy(this->step, this->rsinglestep, this->rndim*sizeof(int32_t));
  for (i = this->rndim - 1; i; i--)
    this->step[i] -= this->step[i - 1]*this->rdims[i - 1];

  zerobytes(this->coords, this->ndim*sizeof(int32_t));
  this->data->ui8 = (uint8_t*) this->data0;

  if (back) {
    // adjust coordinate and pointer to point at back side
    index = this->ndim - 1;
    this->coords[index] = this->rdims[index] - 1;
    this->data->ui8 += this->coords[index]*this->rsinglestep[index]*this->stride;
  }

  if (trgt) {
    trgtstride = trgt->stride;
    trgtdata = trgt->data;
    trgtdata0 = trgt->data0;
    *trgt = *this;
    trgt->stride = trgtstride;
    trgt->data = trgtdata;
    trgt->data0 = trgtdata0;
    trgt->data->ui8 = (uint8_t*) trgt->data0;
    if (back) {
      // adjust coordinate and pointer to point at back side
      trgt->coords[index] = trgt->rdims[index] - 1;
      trgt->data->ui8 += trgt->coords[index]*trgt->rsinglestep[index]*trgt->stride;
    }
  }
}
//--------------------------------------------------------------------
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
  int32_t       i, j, *d, nDiagonal, nDoDim, n, n0, n1, n2, k;

  if (symbol) {                         // have <diagonal>
    if (symbol_class(symbol) != LUX_ARRAY)
      return cerror(NEED_ARR, symbol);
    if (array_size(symbol) != info->ndim)
      return cerror(INCMP_ARG, symbol);
    i = lux_long(1, &symbol);   // ensure LONG
    d = (int32_t*) array_data(i);
    nDiagonal = nDoDim = 0;
    for (i = 0; i < info->ndim; i++)
      if (d[i]) {
        nDoDim++;               /* # dimensions that are considered
                                 at all */
        if (d[i] != 1)
          nDiagonal++;          /* # dimensions that allow diagonal
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

  // calculate offsets to elements to be investigated
  // we need to treat n directions
  for (i = 0; i < info->ndim; i++)
    info->coords[i] = 0;
  info->coords[0] = 1;

  n0 = n1 = 0;
  n2 = 1;                       // defaults for when diagonal == 0
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
      // OK: treat this direction
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
    // go to next direction.
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

  // now that we have everything we need, we clean up
  zerobytes(info->coords, info->ndim*sizeof(int32_t));
  return n;
}
//--------------------------------------------------------------------
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
int32_t
LoopInfo::moveLoop(int32_t index, int32_t distance)
{
  // LS 9apr99
  int32_t       i;

  if (index < 0)                // illegal axis; don't do anything
    return this->rndim;
  if (index >= this->rndim)     // illegal axis; don't do anything
    return index + 1;
  // adjust data pointer
  this->data->ui8 += distance*this->rsinglestep[index]*this->stride;
  // adjust the coordinate
  this->coords[index] += distance;
  // the new coordinate may be out of range; either negative or greater
  // than the dimension
  if (this->coords[index] >= this->rdims[index]) {// too great
    i = this->coords[index];
    do {
      i /= this->rdims[index];  // carry
      this->coords[index] -= i*this->rdims[index]; // carry over
      if (++index == this->rndim)
        break;
      this->coords[index] += i;
      this->data->ui8 += i*this->step[index]*this->stride;
    } while (i && index < this->rndim);
  } else if (this->coords[index] < 0) {         // negative
    i = this->coords[index];
    do {
      i = (i + 1)/this->rdims[index] - 1;
      this->coords[index] -= i*this->rdims[index];
      if (++index == this->rndim)
        break;
      this->coords[index] += i;
      this->data->ui8 += i*this->step[index]*this->stride;
    } while (this->coords[index] < 0);
  }
  return index;
}
//--------------------------------------------------------------------
/** Move to the start of the rearranged axis.

    \param[in,out] info points to the LoopInfo to adjust.

    \param[in,out] ptr points to the Pointer to adjust.

    \param[in] index is the index of the rearranged axis.

    Moves to the start of the rearranged axis indicated by \p index,
    zeroing all rearranged coordinates up to and including that one,
    and adjusting the pointer accordingly.
*/
void
LoopInfo::returnLoop(Pointer *ptr, int32_t index)
{
  // LS 9apr99
  int32_t       i;

  for (i = 0; i <= index; i++) {
    ptr->ui8 -= this->coords[index]*this->rsinglestep[index]*this->stride;
    this->coords[index] = 0;
  }
}
//--------------------------------------------------------------------
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
  static int32_t        one = 1;

  if (symbolIsString(data)
      && !string_is_ok)
    return LUX_ERROR;

  switch (symbol_class(data)) {
  default:
    return LUX_ERROR;         // no message, because not always wanted
  case LUX_SCAL_PTR:
    data = dereferenceScalPointer(data);
    // fall-thru
  case LUX_SCALAR:
    if (dims)
      *dims = &one;
    if (nDim)
      *nDim = 1;
    if (size)
      *size = 1;
    if (src)
      (*src).i32 = &scalar_value(data).i32;
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
      (*src).i32 = (int32_t*) array_data(data);
    break;
  }
  return 1;
}

//---------------------------------------------------------------------
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
//---------------------------------------------------------------------
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
//--------------------------------------------------------------------
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

    \param[in] clear says whether to clear (i.e., set to 0) the contents.
 */
void
standard_redef_array(Symbol iq, Symboltype type,
                     int32_t num_dims, int32_t *dims,
                     int32_t naxes, int32_t *axes,
                     int32_t mode, Pointer *ptr, LoopInfo *info,
                     bool clear)
{
  redef_array(iq, type, num_dims, dims);
  ptr->v = array_data(iq);
  if (clear)
    memset(ptr->ui8, 0, array_size(iq)*lux_type_size[type]);
  info->setupDimensionLoop(num_dims, dims, type, naxes, axes, ptr, mode);
}
