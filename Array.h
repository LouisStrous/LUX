#ifndef ARRAY_H
#define ARRAY_H

struct {
  size_t num_dims;
  size_t num_elem;
  size_t *dims;
  Symboltype type;
  pointer *data;
} Array;

#endif
