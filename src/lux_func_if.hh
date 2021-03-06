/* This is file lux_func_if.hh.

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
#include <stdio.h>
#include "action.hh"

typedef struct {
  int32_t func_sym;
  size_t num_params;
  int16_t *param_syms;
  Pointer *param_data;
} lux_func_if;

lux_func_if * lux_func_if_alloc(char const * const name, size_t num_params);
void lux_func_if_free(lux_func_if *);
int32_t lux_func_if_set_param(lux_func_if *, size_t index, int32_t param);
int32_t lux_func_if_get_param_sym(lux_func_if *, size_t index);
Pointer lux_func_if_get_param_data(lux_func_if *, size_t index);
double lux_func_if_call(lux_func_if *);
