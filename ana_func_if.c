#include "ana_func_if.h"

typedef struct {
  int func_sym;
  size_t num_params;
  word *param_syms;
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
  afif->num_params = num_params;
  if (num_params) {
    afif->param_syms = calloc(num_params, sizeof(word));
    if (!afif->param_syms)
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
    free(afif->name);
    free(afif);
  }
}

int ana_func_if_set_param(ana_func_if *afif, size_t index, int param)
{
  if (!afif || index >= afif->num_params) {
    errno = EDOM;
    return 1;
  }
  afif->param_syms[index] = param;
  return 0;
}
