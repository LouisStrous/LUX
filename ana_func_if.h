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
