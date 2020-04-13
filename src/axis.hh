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
#ifndef HAVE_AXIS_H
#define HAVE_AXIS_H

#include <vector>
#include "types.hh"

/// \file
/// Declares supporting data types and functions for standard_args().

/// Selects how to handle a particular dimension, for use in
/// standard_args().  Each constant is a bit flag that can be combined
/// with others through the logical or operation.  Not all
/// combinations are logical.
enum dim_spec_type {
  /// No selection.
  DS_NONE = 0,

  /// Accept the dimension as is.  Corresponds to `:` in the dimension
  /// part of a standard_args() format specification.
  DS_ACCEPT = (1<<0),

  /// Insert this dimension.  Corresponds to `+` in a format
  /// specification.
  DS_ADD = (1<<1),

  /// Remove this dimension.  Corresponds to `-` in a format
  /// specification.
  DS_REMOVE = (1<<2),

  /// Add or remove this dimension.
  DS_ADD_REMOVE = (DS_ADD | DS_REMOVE),

  /// Copy this dimension from the reference symbol.  Corresponds to
  /// `&` in a format specification.
  DS_COPY_REF = (1<<3),

  /// This dimension must have exactly the specified size.
  DS_EXACT = (1<<4),

  /// This dimension must have at least the specified size.
  /// Corresponds to `>` in a format specification.
  DS_ATLEAST = (1<<5),
};

/// Represents a single dimension specification from a standard_args()
/// format specification.
struct dims_spec {
  /// How to handle the dimension.
  enum dim_spec_type type;

  /// The size to add, or 0 if not specified.
  size_t size_add;

  /// The size to remove, or 0 if not specified.
  size_t size_remove;
};

/// Represents the parameter type from a standard_args() format
/// specification.
enum param_spec_type {
  /// For an input parameter.  Corresponds to `i` in a standard_args()
  /// format specification.
  PS_INPUT,

  /// For an output parameter.  Corresponds to `o` in a format
  /// specification.
  PS_OUTPUT,

  /// For a return parameter.  Corresponds to `r` in a format
  /// specification.
  PS_RETURN
};

/// Represents how a data type specification in a standard_args()
/// format specification should be interpreted.
enum type_spec_limit_type {
  /// The data type is exact.
  PS_EXACT,

  /// The data type is a lower limit: If an input argument has a data
  /// type lower than this, then a copy converted to this type is used
  /// instead.  Corresponds to `>` in the data type part of a
  /// standard_args() format specification.
  PS_LOWER_LIMIT
};


/// Represents how the remaining dimensions should be handled
/// according to a standard_args() format specification.
enum remaining_dims_type {
  /// Only the explicitly mentioned dimensions are present.
  PS_ABSENT,

  /// Remaining dimensions are equal to the corresponding ones from
  /// the reference parameter.  Corresponds to `&` in a
  /// standard_args() format specification.
  PS_EQUAL_TO_REFERENCE,

  /// Remaining dimensions are equal to 1 or to the corresponding ones
  /// from the reference parameter.  Corresponds to `#` in a format
  /// specification.
  PS_ONE_OR_EQUAL_TO_REFERENCE,

  /// Remaining dimensions may have arbitrary sizes.  Corresponds to
  /// `*` in a format specification.
  PS_ARBITRARY
};

/// Represents the specification of a single parameter in a
/// standard_args() format specification.
struct Param_spec {
  /// The parameter type.
  enum param_spec_type logical_type;

  /// `true` if the parameter is optional, `false` if is is mandatory.
  bool is_optional;

  /// The data type limitation.
  enum type_spec_limit_type data_type_limit;

  /// The data type.
  Symboltype data_type;

  /// The count of dimension specifications in \p dims_spec.
  size_t num_dims_spec;

  /// A pointer to the beginning of the array of dimension
  /// specifications.
  struct dims_spec *dims_spec;

  /// The reference parameter index in \p dims_spec.
  int32_t ref_par;

  /// The axis parameter index in \p dims_spec.
  int32_t axis_par;

  /// How to handle dimensions that aren't explicitly specified.
  enum remaining_dims_type remaining_dims;

  /// `true` if the current dimension has the "common type" flag (`^`
  /// in a standard_args() format specification), `false` otherwise.
  bool common_type;

  /// `true` if dimensions equal to one should be suppressed as far as
  /// possible, `false` otherwise.
  bool omit_dimensions_equal_to_one;
};

/// Represents a list of parameter specifications corresponding to a
/// standard_args() format specification.
struct param_spec_list {
  /// The count of parameter specifications in \p param_specs.
  size_t num_param_specs;

  /// A pointer to the beginning of the list of parameter
  /// specifications.
  Param_spec *param_specs;

  /// The index of the return parameter in the list of parameter
  /// specifications.
  int32_t return_param_index;
};

#endif
