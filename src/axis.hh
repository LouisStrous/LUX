#pragma once
/* This is file axis.hh.

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
#include <vector>
#include "types.hh"

/// \file

/// axis loop information
struct LoopInfo
{
  /// A pointer to a `pointer` to the current data element.  For
  /// example, if the data type is LUX_DOUBLE, then the current data
  /// element is at `data->d`.  Gets updated as appropriate when the
  /// axes are traversed.
  Pointer *data;

  /// The start of the data.  Remains constant when the axes are
  /// traversed.
  void *data0;

  /// The current (rearranged) coordinates, taking into account which
  /// axes are traversed (and in what order), and taking into account
  /// axis compression, if any.  `coords[i]` indicates the position
  /// along axis `axes[i]` (for `i < naxes`).
  int32_t coords[MAX_DIMS];

  /// The step size (elements) per original dimension.  You have to
  /// move the `data` pointer forward by `coords[i]` elements when
  /// original dimension `i` increases by 1.
  int32_t singlestep[MAX_DIMS];

  int32_t step[MAX_DIMS];           //!< combined step size for loop transfer
  int32_t dims[MAX_DIMS];           //!< original dimensions
  int32_t nelem;                    //!< number of elements
  int32_t ndim;                     //!< number of original dimensions
  int32_t axes[MAX_DIMS];           //!< selected axes
  int32_t naxes;                    //!< selected number of axes
  int32_t rdims[MAX_DIMS];          //!< compressed rearranged dimensions
  int32_t rndim;                    //!< number of compressed rearranged dims
  int32_t rsinglestep[MAX_DIMS];    //!< step size per rearranged coordinate
  int32_t axisindex;                //!< index to current axis (in axes[])
  int32_t mode;                     //!< desired treatment modes
  int32_t stride;                   //!< bytes per data element
  Symboltype type;                  //!< data type
  int32_t advanceaxis;          //!< how many axes not to advance (from start)
  int32_t raxes[MAX_DIMS];          //!< from rearranged to old axes
  int32_t iraxes[MAX_DIMS];         //!< from old to rearranged axes

  Symbol setAxes(int32_t nAxes, const int32_t* axes, int32_t mode);
  Symbol setAxes(const std::vector<int32_t>& axes, int32_t mode);

  int setupDimensionLoop(int32_t ndim, int32_t const *dims,
                         Symboltype type, int32_t naxes, int32_t const *axes,
                         Pointer *data, int32_t mode);
  int32_t advanceLoop(const void* ptr);
  int32_t loopIsAtStart() const;
  void rearrangeDimensionLoop();
  void setAxisMode(int32_t mode);
  int32_t dimensionLoopResult1(int32_t tmode, Symboltype ttype,
                               int32_t nMore, int32_t const * more,
                               int32_t nLess, int32_t const * less,
                               LoopInfo *tinfo, Pointer *tptr);
  int32_t dimensionLoopResult(LoopInfo *tinfo, Symboltype ttype, Pointer *tptr);
  int32_t nextLoop();
  void subdataLoop(int32_t *range);
  void rearrangeEdgeLoop(LoopInfo *trgt, int32_t index);
  int32_t moveLoop(int32_t index, int32_t distance);
  void returnLoop( Pointer *ptr, int32_t index);
  int32_t set(size_t index);
};
