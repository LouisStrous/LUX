#include <stdio.h>
#include "action.h"

typedef struct {
  int func_sym;
  size_t num_params;
  word *param_syms;
  pointer *param_data;
} ana_func_if;

ana_func_if * ana_func_if_alloc(char const * const name, size_t num_params);
void ana_func_if_free(ana_func_if *);
int ana_func_if_set_param(ana_func_if *, size_t index, int param);
int ana_func_if_get_param_sym(ana_func_if *, size_t index);
pointer ana_func_if_get_param_data(ana_func_if *, size_t index);
double ana_func_if_call(ana_func_if *);
