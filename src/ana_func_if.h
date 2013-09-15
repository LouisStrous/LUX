/* This is file ana_func_if.h.

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
#include <stdio.h>
#include "action.h"

typedef struct {
  Int func_sym;
  size_t num_params;
  Word *param_syms;
  pointer *param_data;
} ana_func_if;

ana_func_if * ana_func_if_alloc(char const * const name, size_t num_params);
void ana_func_if_free(ana_func_if *);
Int ana_func_if_set_param(ana_func_if *, size_t index, Int param);
Int ana_func_if_get_param_sym(ana_func_if *, size_t index);
pointer ana_func_if_get_param_data(ana_func_if *, size_t index);
Double ana_func_if_call(ana_func_if *);
