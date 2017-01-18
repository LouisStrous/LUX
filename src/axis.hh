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

/// Selects how to handle a particular dimension, for use in
/// standard_args.
enum dim_spec_type {
  /// No selection.
  DS_NONE = 0,

  /// Accept the dimension as is.
  DS_ACCEPT = (1<<0),

  /// Insert this dimension.
  DS_ADD = (1<<1),

  /// Remove this dimension.
  DS_REMOVE = (1<<2),

  /// Add or remove this dimension.
  DS_ADD_REMOVE = (DS_ADD | DS_REMOVE),

  /// Copy this dimension from the reference symbol.
  DS_COPY_REF = (1<<3),

  /// This dimension must have exactly the specified size.
  DS_EXACT = (1<<4),

  /// This dimension must have at least the specified size.
  DS_ATLEAST = (1<<5),
};

struct dims_spec {
  enum dim_spec_type type;
  size_t size_add;
  size_t size_remove;
};

enum param_spec_type { PS_INPUT, PS_OUTPUT, PS_RETURN };
enum type_spec_limit_type { PS_EXACT, PS_LOWER_LIMIT };
enum remaining_dims_type { PS_ABSENT, PS_EQUAL_TO_REFERENCE, PS_ONE_OR_EQUAL_TO_REFERENCE, PS_ARBITRARY };

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

#endif
