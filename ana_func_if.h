typedef void *ana_func_if;

ana_func_if * ana_func_if_alloc(char const * const name, size_t num_params);
void ana_func_if_free(ana_func_if *);
int ana_func_if_set_param(ana_func_if *, size_t index, int param);
double call_ana_func_if(ana_func_if *);
