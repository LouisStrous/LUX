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
