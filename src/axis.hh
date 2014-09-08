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
#ifndef HAVE_AXIS_H_
#define HAVE_AXIS_H_

enum dim_spec_type { DS_NONE = 0, DS_ACCEPT = (1<<0), DS_ADD = (1<<1),
                     DS_REMOVE = (1<<2), DS_ADD_REMOVE = (DS_ADD | DS_REMOVE),
                     DS_COPY_REF = (1<<3), DS_EXACT = (1<<4) };

struct dims_spec {
  enum dim_spec_type type;
  size_t size_add;
  size_t size_remove;
};

enum param_spec_type { PS_INPUT, PS_OUTPUT, PS_RETURN };
enum type_spec_limit_type { PS_EXACT, PS_LOWER_LIMIT };
enum remaining_dims_type { PS_ABSENT, PS_EQUAL_TO_REFERENCE, PS_ARBITRARY };

struct param_spec {
  enum param_spec_type logical_type;
  int32_t is_optional;
  enum type_spec_limit_type data_type_limit;
  Symboltype data_type;
  size_t num_dims_spec;
  struct dims_spec *dims_spec;
  int32_t ref_par;
  int32_t axis_par;
  enum remaining_dims_type remaining_dims;
  int32_t remaining_dims_equal_to_reference;
};

struct param_spec_list {
  size_t num_param_specs;
  struct param_spec *param_specs;
  int32_t return_param_index;
};

/// A class representing axis loop information.
class LoopInfo
{
public:
  /// A pointer to a ::pointer to the current data element.  For
  /// example, if the data type is LUX_DOUBLE, then the current
  /// data element is at `*data->d`.  Gets updated as appropriate
  /// when the axes are traversed.
  pointer *data_;

  /// The start of the data.  Remains constant when the axes are
  /// traversed.
  void *data0_;

  /// The current (rearranged) coordinates, taking into account which
  /// axes are traversed (and in what order), and taking into account
  /// axis compression, if any.  \code{coords[i]} indicates the
  /// position along axis `axes[i]` (for `i < naxes`).
  int32_t coords_[MAX_DIMS];

  /// The step size (elements) per original dimension.  You have to
  /// move the \c data pointer forward by `coords[i]` elements
  /// when original dimension \c i increases by 1.
  int32_t singlestep_[MAX_DIMS];

  int32_t step_[MAX_DIMS];           //< combined step size for loop transfer
  int32_t dims_[MAX_DIMS];           //< original dimensions
  int32_t nelem_;                    //< number of elements
  int32_t ndim_;                     //< number of original dimensions
  int32_t axes_[MAX_DIMS];           //< selected axes
  int32_t naxes_;                    //< selected number of axes
  int32_t rdims_[MAX_DIMS];          //< compressed rearranged dimensions
  int32_t rndim_;                    //< number of compressed rearranged dims
  int32_t rsinglestep_[MAX_DIMS];    //< step size per rearranged coordinate
  int32_t axisindex_;                //< index to current axis (in axes[])
  int32_t mode_;                     //< desired treatment modes
  int32_t stride_;                   //< bytes per data element
  Symboltype type_;                  //< data type
  int32_t advanceaxis_;              //< how many axes not to advance (from start)
  int32_t raxes_[MAX_DIMS];          //< from rearranged to old axes
  int32_t iraxes_[MAX_DIMS];         //< from old to rearranged axes
};

#endif
