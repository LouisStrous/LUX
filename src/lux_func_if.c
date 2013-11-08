/* This is file lux_func_if.c.

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
#include <errno.h>
#include <math.h>
#include "lux_func_if.h"

/*
  An interface to allow a user-defined LUX function to be called as a
  C function.  Example for a user-defined function called MYFUNC that
  takes 2 parameters:
  
  lux_func_if *afif = lux_func_if_alloc("MYFUNC", 2);
  lux_func_if_set_param(afif, 0, par0);
  lux_func_if_set_param(afif, 1, par1);
  pointer p0 = lux_func_if_get_param_data(afif, 0);
  pointer p1 = lux_func_if_get_param_data(afif, 1);
  
  lux_func_if_call(afif);

  lux_func_if_free(afif);
*/

lux_func_if * lux_func_if_alloc(char const * const name, size_t num_params)
{
  lux_func_if *afif = calloc(1, sizeof(lux_func_if));
  if (!afif)
    return NULL;
  Int func_sym = stringpointer((char *) name, SP_USER_FUNC);
  if (func_sym < 0) {
    errno = EDOM;
    goto error;
  }
  afif->num_params = num_params;
  if (num_params) {
    afif->param_syms = calloc(num_params, sizeof(Word));
    afif->param_data = calloc(num_params, sizeof(pointer));
    if (!afif->param_syms || !afif->param_data)
      goto error;
  }
  afif->func_sym = nextFreeTempExecutable();
  symbol_class(afif->func_sym) = LUX_USR_FUNC;
  usr_func_arguments(afif->func_sym) = afif->param_syms;
  symbol_memory(afif->func_sym) = num_params*sizeof(Word);
  usr_func_number(afif->func_sym) = func_sym;
  return afif;
 error:
  lux_func_if_free(afif);
  return NULL;
}

void lux_func_if_free(lux_func_if * afif)
{
  if (afif) {
    if (afif->func_sym)
      zapTemp(afif->func_sym); /* make it available */
    free(afif->param_data);
    free(afif->param_syms);
    free(afif);
  }
}

Int lux_func_if_set_param(lux_func_if *afif, size_t index, Int param)
{
  if (!afif || index >= afif->num_params
      || !symbolIsNumerical(param)) {
    errno = EDOM;
    return 1;
  }
  afif->param_syms[index] = param;
  numerical(param, NULL, NULL, NULL, &afif->param_data[index]);
  return 0;
}

Int lux_func_if_get_param_sym(lux_func_if *afif, size_t index)
{
  if (index >= afif->num_params) {
    errno = EDOM;
    return -1;
  }
  return afif->param_syms[index];
}

pointer lux_func_if_get_param_data(lux_func_if *afif, size_t index)
{
  if (index >= afif->num_params) {
    errno = EDOM;
    pointer p;
    p.v = NULL;
    return p;
  }
  return afif->param_data[index];
}

double lux_func_if_call(lux_func_if *afif)
{
  Int result_sym = eval(afif->func_sym);
  double result = (result_sym < 0? NAN: double_arg(result_sym));
  zap(result_sym);
  return result;
}
