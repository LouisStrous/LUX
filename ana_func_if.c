#include "ana_func_if.h"

typedef struct {
  int func_sym;
  size_t num_params;
  word *param_syms;
  pointer param_data;
} ana_func_if;

ana_func_if * ana_func_if_alloc(char const * const name, size_t num_params)
{
  ana_func_if *afif = calloc(1, sizeof(ana_func_if));
  if (!afif)
    return NULL;
  afif->func_sym = stringpointer(name, SP_USER_FUNC);
  if (afif->func_sym < 0) {
    errno = EDOM;
    goto error;
  }
  afif->num_param = num_params;
  if (num_params) {
    afif->param_sym = calloc(num_params, sizeof(word));
    afif->param_data = calloc(num_params, sizeof(pointer));
    if (!afif->param_sym || !afif->param_data)
      goto error;
  }
  return afif;
 error:
  ana_func_if_free(afif);
  return NULL;
}

void ana_func_if_free(ana_func_if * afif)
{
  if (afif) {
    free(afif->param_data);
    free(afif->param_sym);
    free(afif->name);
    free(afif);
  }
}

int ana_func_if_set_param(ana_func_if *afif, size_t index, int param)
{
  if (!afif || index >= afif->num_param
      || !symbolIsNumerical(param)) {
    errno = EDOM;
    return 1;
  }
  afif->param_sym[index] = param;
  numerical(param, NULL, NULL, NULL, &afif->param_data[index]);
  return 0;
}

int ana_func_if_get_param_sym(ana_func_if *afif, size_t index)
{
  if (index >= afif->num_param) {
    errno = EDOM;
    return -1;
  }
  return afif->param_sym[index];
}

pointer ana_func_if_get_param_data(ana_func_if *afif, size_t index)
{
  if (index >= afif->num_param) {
    errno = EDOM;
    pointer p;
    p.v = NULL;
    return p;
  }
  return afif->param_data[index];
}
