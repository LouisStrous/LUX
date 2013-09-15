/* This is file ana_func_if.c.

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
#include "ana_func_if.h"

/*
  An interface to allow a user-defined LUX function to be called as a
  C function.  Example for a user-defined function called MYFUNC that
  takes 2 parameters:
  
  ana_func_if *afif = ana_func_if_alloc("MYFUNC", 2);
  ana_func_if_set_param(afif, 0, par0);
  ana_func_if_set_param(afif, 1, par1);
  pointer p0 = ana_func_if_get_param_data(afif, 0);
  pointer p1 = ana_func_if_get_param_data(afif, 1);
  
  ana_func_if_call(afif);

  ana_func_if_free(afif);
*/

ana_func_if * ana_func_if_alloc(char const * const name, size_t num_params)
{
  ana_func_if *afif = calloc(1, sizeof(ana_func_if));
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
  symbol_class(afif->func_sym) = ANA_USR_FUNC;
  usr_func_arguments(afif->func_sym) = afif->param_syms;
  symbol_memory(afif->func_sym) = num_params*sizeof(Word);
  usr_func_number(afif->func_sym) = func_sym;
  return afif;
 error:
  ana_func_if_free(afif);
  return NULL;
}

void ana_func_if_free(ana_func_if * afif)
{
  if (afif) {
    free(afif->param_data);
    free(afif->param_syms);
    free(afif);
  }
}

Int ana_func_if_set_param(ana_func_if *afif, size_t index, Int param)
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

Int ana_func_if_get_param_sym(ana_func_if *afif, size_t index)
{
  if (index >= afif->num_params) {
    errno = EDOM;
    return -1;
  }
  return afif->param_syms[index];
}

pointer ana_func_if_get_param_data(ana_func_if *afif, size_t index)
{
  if (index >= afif->num_params) {
    errno = EDOM;
    pointer p;
    p.v = NULL;
    return p;
  }
  return afif->param_data[index];
}

Double ana_func_if_call(ana_func_if *afif)
{
  Int result_sym = eval(afif->func_sym);
  Double result = (result_sym < 0? NAN: double_arg(result_sym));
  zap(result_sym);
  return result;
}
