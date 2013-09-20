/* This is file axis.h.

Copyright 2013 Louis Strous

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

struct dims_spec {
  enum dim_spec_type { DS_NONE = 0, DS_ACCEPT = (1<<0), DS_ADD = (1<<1),
                       DS_REMOVE = (1<<2), DS_ADD_REMOVE = (DS_ADD | DS_REMOVE),
                       DS_COPY_REF = (1<<3), DS_EXACT = (1<<4) } type;
  size_t size_add;
  size_t size_remove;
};

struct param_spec {
  enum param_spec_type { PS_INPUT, PS_OUTPUT, PS_RETURN } logical_type;
  Int is_optional;
  enum type_spec_limit_type { PS_EXACT, PS_LOWER_LIMIT } data_type_limit;
  enum Symboltype data_type;
  size_t num_dims_spec;
  struct dims_spec *dims_spec;
  Int ref_par;
  Int axis_par;
  enum remaining_dims_type { PS_ABSENT, PS_EQUAL_TO_REFERENCE, PS_ARBITRARY } remaining_dims;
  Int remaining_dims_equal_to_reference;
};

struct param_spec_list {
  size_t num_param_specs;
  struct param_spec *param_specs;
  Int return_param_index;
};

#endif