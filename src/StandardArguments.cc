/* This is file StandardArguments.cc.

Copyright 2017 Louis Strous

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

#include <algorithm>            // for std::remove
#include <cstring>              // for strchr
#include <vector>
#include "NumericDataDescriptor.hh"
#include "action.hh"

/// Represents the parameter type from a standard_args() format
/// specification.
enum param_spec_type {
  /// For an input parameter.  Corresponds to `i` in a standard_args()
  /// format specification.
  PS_INPUT,

  /// For an output parameter.  Corresponds to `o` in a format
  /// specification.
  PS_OUTPUT,

  /// For a return parameter.  Corresponds to `r` in a format
  /// specification.
  PS_RETURN
};

/// Selects how to handle a particular dimension, for use in
/// standard_args().  Each constant is a bit flag that can be combined
/// with others through the logical or operation.  Not all
/// combinations are logical.
enum dim_spec_type {
  /// No selection.
  DS_NONE = 0,

  /// Accept the dimension as is.  Corresponds to `:` in the dimension
  /// part of a standard_args() format specification.
  DS_ACCEPT = (1<<0),

  /// Insert this dimension.  Corresponds to `+` in a format
  /// specification.
  DS_ADD = (1<<1),

  /// Remove this dimension.  Corresponds to `-` in a format
  /// specification.
  DS_REMOVE = (1<<2),

  /// Add or remove this dimension.
  DS_ADD_REMOVE = (DS_ADD | DS_REMOVE),

  /// Copy this dimension from the reference symbol.  Corresponds to
  /// `&` in a format specification.
  DS_COPY_REF = (1<<3),

  /// This dimension must have exactly the specified size.
  DS_EXACT = (1<<4),

  /// This dimension must have at least the specified size.
  /// Corresponds to `>` in a format specification.
  DS_ATLEAST = (1<<5),
};

/// Represents a single dimension specification from a standard_args()
/// format specification.
struct Dims_spec {
  /// How to handle the dimension.
  enum dim_spec_type type = DS_NONE;

  /// The size to add, or 0 if not specified.
  size_t size_add = 0;

  /// The size to remove, or 0 if not specified.
  size_t size_remove = 0;
};

/// Represents how a data type specification in a standard_args()
/// format specification should be interpreted.
enum type_spec_limit_type {
  /// The data type is exact.
  PS_EXACT,

  /// The data type is a lower limit: If an input argument has a data
  /// type lower than this, then a copy converted to this type is used
  /// instead.  Corresponds to `>` in the data type part of a
  /// standard_args() format specification.
  PS_LOWER_LIMIT
};


/// Represents how the remaining dimensions should be handled
/// according to a standard_args() format specification.
enum remaining_dims_type {
  /// Only the explicitly mentioned dimensions are present.
  PS_ABSENT,

  /// Remaining dimensions are equal to the corresponding ones from
  /// the reference parameter.  Corresponds to `&` in a
  /// standard_args() format specification.
  PS_EQUAL_TO_REFERENCE,

  /// Remaining dimensions are equal to 1 or to the corresponding ones
  /// from the reference parameter.  Corresponds to `#` in a format
  /// specification.
  PS_ONE_OR_EQUAL_TO_REFERENCE,

  /// Remaining dimensions may have arbitrary sizes.  Corresponds to
  /// `*` in a format specification.
  PS_ARBITRARY
};

/// Represents the specification of a single parameter in a
/// standard_args() format specification.
struct Param_spec {
  /// The parameter type.
  enum param_spec_type logical_type = PS_INPUT;

  /// `true` if the parameter is optional, `false` if is is mandatory.
  bool is_optional = false;

  /// The data type limitation.
  enum type_spec_limit_type data_type_limit = PS_EXACT;

  /// The data type.
  Symboltype data_type = LUX_NO_SYMBOLTYPE;

  /// The count of dimension specifications in \p Dims_spec.
  size_t num_dims_spec = 0;

  /// A pointer to the beginning of the array of dimension
  /// specifications.
  Dims_spec *dims_spec = 0;

  /// The reference parameter index in \p Dims_spec.
  int32_t ref_par = 0;

  /// The axis parameter index in \p Dims_spec.
  int32_t axis_par = 0;

  /// How to handle dimensions that aren't explicitly specified.
  enum remaining_dims_type remaining_dims = PS_ABSENT;

  /// `true` if the current dimension has the "common type" flag (`^`
  /// in a standard_args() format specification), `false` otherwise.
  bool common_type = false;

  /// `true` if dimensions equal to one should be suppressed as far as
  /// possible, `false` otherwise.
  bool omit_dimensions_equal_to_one = false;
};

/// Represents a list of parameter specifications corresponding to a
/// standard_args() format specification.
struct param_spec_list {
  /// The count of parameter specifications in \p param_specs.
  size_t num_param_specs;

  /// A pointer to the beginning of the list of parameter
  /// specifications.
  Param_spec *param_specs;

  /// The index of the return parameter in the list of parameter
  /// specifications.
  int32_t return_param_index;
};

/*--------------------------------------------------------------------*/
/** Free a param_spec_list.

    \param[in] psl points to the param_spec_list for which to free the
    allocated memory.
 */
void free_param_spec_list(struct param_spec_list *psl)
{
  if (psl) {
    size_t i;
    for (i = 0; i < psl->num_param_specs; i++) {
      Param_spec *p = &psl->param_specs[i];
      free(p->dims_spec);
    }
    free(psl->param_specs);
    free(psl);
  }
}

/** Parse a standard arguments format.

    \param [in] fmt points at the format to parse.

    \returns a list of parsed parameter specifications.
 */
struct param_spec_list *parse_standard_arg_fmt(char const *fmt)
{
  struct param_spec_list *psl = NULL;
  size_t i;
  int32_t return_param_index = -1;
  int32_t param_index;
  char const *fmt0 = fmt;

  if (!fmt || !*fmt)
    return NULL;

  std::vector<Param_spec> ps;
  std::vector<Dims_spec> ds;
  param_index = 0;
  size_t prev_ds_num_elem = 0;
  int bad = 0;
  while (*fmt) {
    Param_spec p_spec;

    while (*fmt && *fmt != ';') { /* every parameter specification */
      /* required parameter kind specification */
      switch (*fmt) {
      case 'i':
        p_spec.logical_type = PS_INPUT;
        break;
      case 'o':
        p_spec.logical_type = PS_OUTPUT;
        break;
      case 'r':
        p_spec.logical_type = PS_RETURN;
        if (return_param_index >= 0) {
          /* already had a return parameter */
          luxerror("Specified multiple return parameters", 0);
          errno = EINVAL;
          bad = 1;
          break;
        } else
          return_param_index = param_index;
        break;
      default:
        /* illegal parameter kind specification */
        luxerror("Illegal parameter kind %d specified", 0, *fmt);
        errno = EINVAL;
        bad = 1;
        break;
      } /* end of switch (*fmt) */
      fmt++;

      /* optional data type limit specification */
      switch (*fmt) {
      case '>':
        p_spec.data_type_limit = PS_LOWER_LIMIT;
        fmt++;
        break;
      default:
        p_spec.data_type_limit = PS_EXACT;
        break;
      } /* end of switch (*fmt) */

      /* optional data type specification */
      switch (*fmt) {
      case 'B':
        p_spec.data_type = LUX_INT8;
        fmt++;
        break;
      case 'W':
        p_spec.data_type = LUX_INT16;
        fmt++;
        break;
      case 'L':
        p_spec.data_type = LUX_INT32;
        fmt++;
        break;
      case 'Q':
        p_spec.data_type = LUX_INT64;
        fmt++;
        break;
      case 'F':
        p_spec.data_type = LUX_FLOAT;
        fmt++;
        break;
      case 'D':
        p_spec.data_type = LUX_DOUBLE;
        fmt++;
        break;
      case 'S':
        p_spec.data_type = LUX_TEMP_STRING;
        fmt++;
        break;
      default:
        p_spec.data_type = LUX_NO_SYMBOLTYPE;
        break;
      } /* end of switch (*fmt) */

      if (*fmt == '^') {
        p_spec.common_type = true;
        ++fmt;
      } else {
        p_spec.common_type = false;
      }

      /* optional dims-specs */
      Dims_spec d_spec;
      if (*fmt == '[') {       /* reference parameter specification */
        fmt++;
        if (*fmt == '-') {
          p_spec.ref_par = -1;  /* point at previous parameter */
          ++fmt;
        } else if (isdigit(*fmt)) { /* a specific parameter */
          char *p;
          p_spec.ref_par = strtol(fmt, &p, 10);
          fmt = p;
        } else {
          luxerror("Expected a digit or hyphen after [ in"
                   " reference parameter specification but found %c", 0, *fmt);
          errno = EINVAL;
          bad = 1;
          break;
        }
        if (*fmt == ']')
          fmt++;
        else {
          luxerror("Expected ] instead of %c at end of reference "
                   "parameter specification", 0, *fmt);
          errno = EINVAL;
          bad = 1;
          break;
        }
      }
      if (*fmt == '{') {   /* optional axis parameter specification */
        // if (p_spec.logical_type == PS_INPUT) {
        //   luxerror("Axis parameter illegally specified for input parameter",
        //         0, fmt);
        //   errno = EINVAL;
        //   bad = 1;
        //   break;
        // }
        fmt++;
        if (*fmt == '-') {
          p_spec.axis_par = -1;  /* point at previous parameter */
          ++fmt;
        } else if (isdigit(*fmt)) { /* a specific parameter */
          char *p;
          p_spec.axis_par = strtol(fmt, &p, 10);
          fmt = p;
        } else {
          luxerror("Expected a digit or hyphen after { in"
                   " reference parameter specification but found %c", 0, *fmt);
          errno = EINVAL;
          bad = 1;
          break;
        }
        // TODO: parse axis modes
        if (*fmt == '}')
          fmt++;
        else {
          luxerror("Expected } instead of %c at end of reference "
                   "parameter specification", 0, *fmt);
          errno = EINVAL;
          bad = 1;
          break;
        }
      } else
        p_spec.axis_par = -2;                /* indicates "none" */
      if (bad)
        break;
      if (*fmt == '@') {
        p_spec.omit_dimensions_equal_to_one = true;
        ++fmt;
      } else {
        p_spec.omit_dimensions_equal_to_one = false;
      }
      while (*fmt && !strchr("*?;&#", *fmt)) { /* all dims */
        d_spec = Dims_spec();
        while (*fmt && !strchr(",?*;&#", *fmt)) { /* every dim */
          dim_spec_type type = (dim_spec_type) 0;
          size_t size = 0;
          switch (*fmt) {
          case '+':
            type = DS_ADD;
            fmt++;
            break;
          case '-':
            type = DS_REMOVE;
            fmt++;
            break;
          case '=':
            type = DS_COPY_REF;
            fmt++;
            break;
          case ':':
            type = DS_ACCEPT;
            fmt++;
            break;
          case '>':
            type = DS_ATLEAST;
            fmt++;
            break;
          default:
            type = DS_EXACT;
            break;
          } /* end of switch (*fmt) */
          if (isdigit(*fmt)) {
            char *p;
            size = strtol(fmt, &p, 10);
            fmt = p;
          }
          switch (type) {
          case DS_ADD:
            if (d_spec.type == DS_NONE || d_spec.type == DS_REMOVE) {
              d_spec.size_add = size;
              d_spec.type = (dim_spec_type) (d_spec.type | type);
            } else {
              luxerror("Illegal combination of multiple types for dimension;"
                       " parameter specification #%d: %s", 0, param_index + 1,
                       fmt0);
              errno = EINVAL;
              bad = 1;
              break;
            }
            break;
          case DS_REMOVE:
            if (d_spec.type == DS_NONE || d_spec.type == DS_ADD) {
              d_spec.size_remove = size;
              d_spec.type = (dim_spec_type) (d_spec.type | type);
            } else {
              luxerror("Illegal combination of multiple types for dimension;"
                       " parameter specification #%d: %s", 0, param_index + 1,
                       fmt0);
              errno = EINVAL;
              bad = 1;
              break;
            }
            break;
          default:
            if (d_spec.type == DS_NONE || d_spec.type == DS_ATLEAST) {
              d_spec.size_add = size;
              d_spec.type = type;
            } else {
              luxerror("Illegal combination of multiple types for dimension;"
                       " parameter specification #%d: %s", 0, param_index + 1,
                       fmt0);
              errno = EINVAL;
              bad = 1;
              break;
            }
            break;
          } /* end switch type */
          if (bad)
            break;
        } /* end of while *fmt && !strchr(",*;&") */
        if (bad)
          break;
        ds.push_back(d_spec);
        if (*fmt == ',')
          ++fmt;
      } /* end of while *fmt && !strchr("*;&", *fmt) */
      if (bad)
        break;
      switch (*fmt) {
      case '*':
        p_spec.remaining_dims = PS_ARBITRARY;
        ++fmt;
        break;
      case '&':
        p_spec.remaining_dims = PS_EQUAL_TO_REFERENCE;
        ++fmt;
        break;
      case '#':
        p_spec.remaining_dims = PS_ONE_OR_EQUAL_TO_REFERENCE;
        ++fmt;
        break;
        /* default is PS_ABSENT */
      }

      if (*fmt == '?') {        /* optional argument */
        if (p_spec.logical_type == PS_RETURN) {
          /* return parameter cannot be optional */
          luxerror("Return parameter was illegally specified as optional", 0);
          errno = EINVAL;
          bad = 1;
          break;
        } else
          p_spec.is_optional = 1;
        fmt++;
      } else
        p_spec.is_optional = 0;
      if (bad)
        break;
      if (*fmt && *fmt != ';') {
        luxerror("Expected ; instead of %c at end of parameter "
                 "specification", 0, *fmt);
        errno = EINVAL;
        bad = 1;
        break;
      }
      if (bad)
        break;
      /* determine the number of dims_specs added to the list for this
         parameter */
      size_t n = ds.size() - prev_ds_num_elem;
      p_spec.num_dims_spec = n;
      p_spec.dims_spec = NULL;                     /* will be filled in later */
      ps.push_back(p_spec);
      prev_ds_num_elem += n;
    } /* end of while (*fmt && *fmt != ';') */
    if (bad)
      break;
    if (*fmt == ';')
      fmt++;
    else if (*fmt) {
      /* unexpected character */
      luxerror("Expected ; instead of %c at end of parameter specification",
               0, *fmt);
      errno = EINVAL;
      bad = 1;
      break;
    }
    param_index++;
  }   /* end of while (*fmt) */
  if (!bad) {
    /* now we copy the information into the final allocated memory */
    psl = (param_spec_list*) malloc(sizeof(struct param_spec_list));
    if (!psl) {                   /* malloc sets errno */
      cerror(ALLOC_ERR, 0);
      bad = 1;
    }
  }
  if (!bad) {
    psl->param_specs = (Param_spec*) calloc(param_index, sizeof(Param_spec));
    if (!psl->param_specs) {        /* malloc sets errno */
      cerror(ALLOC_ERR, 0);
      bad = 1;
    }
  }
  if (!bad) {
    /* the return parameter, if any, gets moved to the end */
    psl->num_param_specs = param_index;
    psl->return_param_index = -1; /* default, may be updated later */

    Param_spec *pstgt;
    size_t ds_ix, j;

    pstgt = psl->param_specs;
    ds_ix = 0;
    for (i = j = 0; i < psl->num_param_specs; i++) {
      size_t j0;
      if (i == return_param_index) {
        j0 = j;
        j = psl->return_param_index = psl->num_param_specs - 1;
      }
      memcpy(pstgt + j, &ps[i], sizeof(Param_spec));
      if (pstgt[j].num_dims_spec) {
        size_t size = pstgt[j].num_dims_spec*sizeof(Dims_spec);
        pstgt[j].dims_spec = (Dims_spec*) malloc(size);
        if (!pstgt[j].dims_spec) {
          cerror(ALLOC_ERR, 0);
          bad = 1;
          break;
        }
        memcpy(pstgt[j].dims_spec, &ds[ds_ix], size);
        ds_ix += pstgt[j].num_dims_spec;
      } /* else pstgt[j].dims_spec == NULL */
      if (i == return_param_index)
        j = j0;
      else
        j++;
    }
  }
  if (!bad) {
    /* check that the reference parameter does not point outside the list */
    int32_t n = psl->num_param_specs - (psl->return_param_index >= 0);
    if (n) {
      for (i = 0; i < psl->num_param_specs; i++) {
        if (psl->param_specs[i].ref_par >= n) {
          errno = EINVAL;
          luxerror("Reference parameter %d for parameter %d points outside"
                   " of the list (size %d)", 0, psl->param_specs[i].ref_par + 1,
                   i + 1, n);
          bad = 1;
          break;
        }
      }
    }
  }
  if (bad) {
    free_param_spec_list(psl);
    psl = NULL;
  }
  return psl;
}

/** Determines the common symbol type for standard arguments.

    \param[in] num_param_specs is the count of Param_spec in \p
    param_specs.

    \param[in] param_specs points at the parameter specifications to
    inspect.

    \param[in] narg is the count of arguments in \p ps.

    \param[in] ps points at the arguments.

    \returns the highest Symboltype among the arguments for which the
    common symbol type is requested, or `LUX_NO_SYMBOLTYPE` if there
    aren't any.
 */
Symboltype standard_args_common_symboltype(int32_t num_param_specs,
                                           Param_spec* param_specs,
                                           int32_t narg,
                                           int32_t* ps)
{
  Symboltype common_type = LUX_NO_SYMBOLTYPE;

  for (int param_ix = 0; param_ix < num_param_specs; ++param_ix) {
    Param_spec* pspec = &param_specs[param_ix];
    if (pspec->common_type) {
      int32_t iq = ps[param_ix];
      Symboltype type;
      if (pspec->logical_type == PS_INPUT)
        type = symbol_type(iq);
      else
        type = LUX_NO_SYMBOLTYPE;
      if ((pspec->data_type_limit == PS_LOWER_LIMIT
           && (type == LUX_NO_SYMBOLTYPE || type < pspec->data_type))
          || (pspec->data_type_limit == PS_EXACT
              && pspec->data_type != LUX_NO_SYMBOLTYPE
              && type != pspec->data_type))
        type = pspec->data_type;
      if (type != LUX_NO_SYMBOLTYPE
          && (common_type == LUX_NO_SYMBOLTYPE
              || type > common_type))
        common_type = type;
    }
  }
  return common_type;
}

/** Prepares for looping through input and output variables based on a
    standard arguments specification.

    \param [in] narg is the number of arguments in `ps`

    \param [in] ps is the array of arguments

    \param [in] fmt is the arguments specification in standard format
    (see below)

    \param [out] ptrs is the address of a pointer in which is returned
    `NULL` (in case of a problem) or a pointer to a freshly allocated
    list of pointers, one for each argument (whether mandatory or
    optional) and possibly one for the return symbol.  Memory for the
    list is allocated using malloc().  The user is responsible for
    releasing the memory (using free()) when it is no longer needed.
    See class StandardArguments for a way to release the memory
    automatically.

    \param [out] infos is the address of a pointer in which is
    returned `NULL` (in case of a problem) or a pointer to a list of
    loop information structures, one for each argument (mandatory or
    optional) and possibly one for the return symbol.  Memory for the
    list is allocated using malloc().  The user is responsible for
    releasing the memory (using free()) when it is no longer needed.
    See class StandardArguments for a way to release the memory
    automatically.

    \param[out] out_size points at a location where the element count
    of \a ptrs and \a infos is returned, if the pointer is not `NULL`.

    \return the return symbol, or -1 if an error occurred.

    \par Introduction

    The standard format can look rather daunting.  It has several
    tasks:

    1. Specify which of the parameters are input parameters, output
       parameters, or return parameters.

    2. Specify which of the parameters are optional.

    3. Specify the data types of the input, output, and return
       parameter that are made available to the back-end (through the
       `ptrs` parameter of standard_args()).  These data types may
       depend on the type of an earlier parameter.

    4. Specify the expectations for the dimensions of the input
       parameters, and specify the dimensions of the output parameters
       and the return value.  These may depend on the dimensions of an
       earlier parameter, but may have dimensions added or removed
       relative to that other parameter.  The removed dimensions may
       be explicit ones or may be identified by the contents (not the
       dimensions) of another parameter (the axis parameter).

    \par Parameter Types

    The specification parts for different parameters are separated by
    semicolons (`;`).  The specification for each parameter begins
    with a letter that identifies the parameter type.

    An input parameter (`i`) is an existing named or unnamed LUX
    symbol whose contents are going to be processed.

    An output parameter (`o`) is an existing named LUX symbol that
    will be reshaped to receive output.  Its previous contents are
    lost.  It must be a named symbol, because otherwise the user
    couldn't access the contents afterwards.

    A return parameter (`r`) is a new unnamed LUX symbol that gets
    created to act as the return value of a LUX function.  There can
    be at most one return parameter in a specification.

    \verbatim
    i;r
    \endverbatim

    The above specification says that the first parameter is an input
    parameter and the second one is a return parameter.

    No dimensions are specified for the input parameter, so it must
    contain exactly one value (be either a scalar or an array with one
    element).  No dimensions or type are specified for the return
    parameter, so it gets the same as its <em>reference
    parameter</em>, which by default is the first parameter.  A
    corresponding call to a fictitous LUX function `foo` might be `y =
    foo(3)`.

    \verbatim
    i;i;o;o
    \endverbatim

    The above specification says that the first two parameters are
    single-element input parameters, and the next two are output
    parameters with the same data type and dimensions as the first
    parameter.  An example call is `foo,3,5,x,y`.

    \par Reference Parameter

    A reference parameter can be indicated for all but the first
    parameter.  Some missing information (like a data type or a
    dimension) may be copied from the reference parameter.  The
    reference parameter is indicated by a number or a hyphen (`-`)
    between square brackets (`[]`) just after the parameter data type
    (described later), which itself follows the parameter type.  A
    number indicates a particular parameter (0 indicates the first
    one), and a hyphen indicates the parameter preceding the current
    one.  If no reference parameter is explicitly given, then the
    first parameter is the reference parameter.  The reference
    parameter must have a smaller index than the current parameter.

    \verbatim
    i;i;o[1]
    \endverbatim

    says that the output parameter's reference parameter is the one
    with index 1 (i.e., the 2nd parameter).  The output parameter gets
    the same type and dimensions as the second parameter.

    \verbatim
    i;i;o[-]
    \endverbatim

    has the same effect as the previous specification.  Now the output
    parameter's reference parameter is the parameter preceding the
    output parameter, which is the 2nd parameter as before.

    \par Optional Parameters

    An input or output parameter specification that has a question
    mark (`?`) at its very end means that that parameter is optional.
    A return parameter cannot be optional.

    \verbatim
    i;i?;r
    \endverbatim

    says that the second parameter is optional, so does not have to be
    specified.  Example calls are `y = foo(3,5)` but also `y =
    foo(3)`.

    \par Parameter Data Types

    Parameter data types may be specified for any parameter,
    immediately after the parameter type.  Explicit parameter data
    types are indicated by one of the letters `B W L Q F D S`
    corresponding to `int8` through `int64`, `float`, `double`, and
    `string`, respectively.

    An output or return parameter for which an explicit data type is
    specified gets set to that data type.

    An explicit data type for an input parameter does't say what data
    type the argument must have, but defines what data type is made
    available to the back-end.  If an input argument's data type is
    equal to the corresponding explicit input parameter's data type,
    then a pointer to that argument's data is made available.  If an
    input argument's data type differs from the corresponding explicit
    input parameter's data type, then a copy of the argument is
    created and converted to the explicit data type, and a pointer to
    that copy's data is made available instead.

    \verbatim
    iF;rL
    \endverbatim

    says that the first argument must be a single-element argument and
    that a `float` copy is made available for processing, if the
    argument isn't `float` already.  Also, a single-element `int32`
    return value is created and made available for receiving output.

    If the explicit data type is numeric (i.e., not `S`) and is
    preceded by a greater-than sign (`>`), then the data type is a
    minimum.  If the data type that would have applied if no explicit
    data type were given is less than the minimum, then that minimum
    is used instead.

    \verbatim
    i>L;r
    \endverbatim

    says that if the data type of the first argument is less than
    `int32`, then an `int32` copy is made available instead.  No
    explicit data type is given for the return parameter, so it gets
    the same as its reference parameter, which by default is the first
    parameter.  So the data type of the return parameter is equal to
    that of the first parameter, which is at least `int32`.

    \verbatim
    i>L;r>F
    \endverbatim

    is like the previous case, but now the return parameter has a
    minimum data type of `float`.  If the input parameter type is at
    least `float`, then the return parameter gets the same type as the
    input parameter.  If the input parameter type is less than
    `float`, then an `int32` version of the input is made available,
    and the return parameter is of type `float`.  If the input
    parameter type is at least `float`, then the return value gets the
    same type as the input parameter.

    If the data type specifications for more than one numerical
    parameter are followed by a caret (`^`), then all of those
    parameters get the same data type, which is equal to the greatest
    data type among them that would have applied if no carets had been
    specified.

    \verbatim
    i>L^;i>L^;iW;r^
    \endverbatim

    says that the first two input parameters and the return value get
    the same data type applied, which is the greatest one among them
    that would have applied if there were no carets.  So, if the first
    parameter is an `int64` and the second one is a `float`, then the
    parameters made available to the back-end have data types `float`,
    `float`, `int16`, and `float`, respectively.

    \par Parameter Dimensions

    Expectations for the dimensions of input parameters can be
    specified, and also how to determine the dimensions of output and
    return parameters.

    At its simplest, the dimensions are specified in a comma-separated
    list after the data type.

    \verbatim
    iF3,6;rD3
    \endverbatim

    says that the input parameter must be an array of 3 by 6 elements,
    of which a `float` version is made available to the back-end, and
    that the return value is a one-dimensional `double` array of 3
    elements.

    A greater-than sign (`>`) before a dimension number means that the
    dimension must be at least as great as the number.

    \verbatim
    i>7
    \endverbatim

    says that the first (and only) dimension must be at least 7.

    For input parameters, a colon (`:`) means to accept the current
    dimension.

    \verbatim
    i:,4,:
    \endverbatim

    says that the input parameter must have 3 dimensions of which the
    2nd one is equal to 4.

    An at sign (`@`) at the beginning of the dimensions specification
    means that dimensions equal to 1 are ignored, as far as possible.
    If omitting all dimensions equal to 1 would mean that there are no
    dimensions left, then a single dimension equal to 1 is retained.

    \verbatim
    i@:,:
    \endverbatim

    says that the input parameter must have two dimensions after
    dimensions equal to 1 are omitted.

    Dimensions for output parameters and the return value can be
    copied from the reference parameter.  An equals sign (`=`) means
    that the corresponding dimension of the reference parameter is
    copied.  If a number follows the equals sign immediately, then it
    says what the dimension of the reference parameter must be.  A
    hyphen (`-`) means that the corresponding dimension of the
    reference parameter is skipped.  A plus sign (`+`) followed by a
    number means that, relative to the reference parameter, a
    dimension equal to that number is inserted.

    \verbatim
    i7,3,2;o=,=
    \endverbatim

    says that the output parameter is an array of 7 by 3 elements.

    \verbatim
    i7,3,2;o=,-,=
    \endverbatim

    says that the output parameter is an array of 7 by 2 elements,
    because the 3 was skipped.

    \verbatim
    i7,3,2;o=,+5,=
    \endverbatim

    says that the output parameter is an array of 7 by 5 by 3
    elements.

    \verbatim
    i7,3,2;o=,5,=
    \endverbatim

    says that the output parameter is an array of 7 by 5 by 2
    elements.

    \verbatim
    i7,3,2;o=2
    \endverbatim

    produces an error because the output parameter's specification
    says that the first dimension of its reference parameter (which is
    the first parameter) should be equal to 2, but the first
    parameter's specification says that its first dimension should be
    equal to 7, and those cannot both be true.

    An asterisk (`*`) at the end of the dimensions list for an input
    parameter says that the remaining dimensions are unrestricted.

    \verbatim
    iF3*;i*
    \endverbatim

    says that the first dimension of the first input parameter must be
    equal to 3 but that any following dimensions are unrestricted, so,
    for example, a one-dimensional array of 3 elements is accepted,
    and also an array of 3 by 5 elements, or 3 by 1 by 17 elements.
    The second input parameter has no restrictions on its dimensions,
    so a scalar is acceptable, and also any array.

    An ampersand (`&`) at the end of the dimensions list for any
    parameter says that the remaining dimensions must be equal to the
    dimensions of the reference parameter.

    \verbatim
    i*;rD6&
    \endverbatim

    says that the input parameter may have any data type and
    dimensions and that the return value is a `double` array with
    dimension 6 followed by the dimensions of the reference parameter,
    which by default is the first parameter.  So, if the input
    argument is an array of 3 by 2 elements, then the return value is
    an array of 6 by 3 by 2 elements.

    A hash sign (`#`) at the end of the dimensions specification means
    that the element count of an input parameter must either be equal
    to 1 or else to the element count of the reference parameter.

    \verbatim
    i3,3,3;i#;r&
    \endverbatim

    says that the second input parameter must either have exactly one
    element or else must have the same number of elements as the
    reference parameter (the first parameter), i.e., 27.  The
    dimensions do not need to be the same, as long as the element
    count matches, so it is OK if the second input parameter has a
    single dimension equal to 27, or is a 9 by 3 array, or a 3 by 3 by
    1 by 3 array.

    \par Axis Parameters

    Some LUX functions and subroutines specify an <em>axis
    parameter</em>, which says along which dimensions of the main data
    to apply the operation.  If the operation produces one value
    (e.g., the minimum value) when running along the indicated axes,
    then the result should have the same dimensions as the main data
    except that the dimensions specified in the axis parameter should
    be omitted.  This is achieved by specifying the axis parameter's
    index between curly braces (`{}`) just before the specification of
    the dimensions, and just after the specification of the reference
    parameter, if any.

    \verbatim
    iD*;iL*;rD{1}
    \endverbatim

    says that parameter 1 (i.e., the 2nd parameter) is the axis
    parameter for the return value.  If the function is called like `y
    = foo(x,[1,2])` and `x` is an array of 4 by 100 by 200 by 3
    elements, then `y` is an array of 4 by 3 elements.

    \par Complete Syntax

    All in all, the standard format is schematically as follows, where
    something between quotes (<tt>''</tt>) stands for that literal
    character, something between non-literal square brackets (`[]`) is
    optional, something between non-literal curly braces (`{}`) is a
    group of alternatives, a non-literal pipe symbol (`|`) separates
    alternatives, and a non-literal asterisk (`*`) indicates
    repetition zero or more times:

    \verbatim
      <format> = <param-spec>[;<param-spec>]*
      <param-spec> = {'i'|'o'|'r'}[<type-spec>][<dims-spec>]['?']
      <type-spec> = {{['>']{'B'|'W'|'L'|'Q'|'F'|'D'}}|'S'}['^']
      <dims-spec> = ['@']['['<ref-par>']']['{'<axis-par>'}']
                    <dim-spec>[,<dim-spec>]*['*'|'&'|'#']
      <dim-spec> = [{['+'|'-'|'=']NUMBER|'-'|'='|':'}]*
    \endverbatim

    In words,
    - the format consists of one or more parameter specifications
      separated by semicolons `;`.
    - each parameter specification begins with an `i`, `o`, or `r`,
      followed by a type specification and a dimensions specification,
      optionally followed by a question mark `?`.
    - a type specification consists of an `S`, or else of an optional
      greater-than sign `>` followed by one of `B`, `W`, `L`, `Q`,
      `F`, or `D`.  Optionally, a `^` follows.
    - a dimensions specification consists of an optional at sign `@`,
      an a optional reference parameter number between square brackets
      `[]`, followed by an optional axis parameter number between
      curly braces `{}`, followed by one or more dimension
      specifications separated by commas `,`, optionally followed by
      an asterisk `*` or ampersand `&` or hash symbol `#`.
    - a dimension specification consists of a hyphen `-`, an equals
      sign `=`, a colon `:`, or a number preceded by a plus sign `+`,
      a hyphen `-`, or an equals sign `=`; followed by any number of
      additional instances of the preceding.

    Some of the characteristics of a parameter may depend on those of
    a reference parameter.  That reference parameter is the very first
    parameter (parameter \c ps[0]) unless a different reference
    parameters is specified at the beginning of the dimension
    specification.

    For the parameter specification \c param-spec:
    - `i` = input parameter.
    - `o` = output parameter.  An error is declared if this is not a
      named parameter.
    - `r` = return value.  For functions, there must be exactly one of
      these in the parameters specification.  Subroutines must not
      have one of these.
    - `?` = optional parameter.

    For the type specification \c type-spec:
    - `>` = the type should be at least equal to the indicated type.
    - `B` = LUX_INT8
    - `W` = LUX_INT16
    - `L` = LUX_INT32
    - `Q` = LUX_INT64
    - `F` = LUX_FLOAT
    - `D` = LUX_DOUBLE
    - `S` = LUX_STRING
    - `^` = all numerical parameters marked like this get the same
      data type, which is the greatest numerical data type among them
      that would have applied if no `^` had been specified.

    For input parameters, a copy is created with the indicated
    (minimum) type if the input parameter does not meet the condition,
    and further processing is based on that copy.  For output
    parameters, an array is created with the indicate type, unless `>`
    is specified and the reference parameter has a greater type, in
    which case that type is used.  If no explicit type is specified
    for an output parameter, then it gets the type of the reference
    parameter.

    For the reference parameter \c ref-par:
    - If absent, then 0 is taken for it (i.e., the first parameter).
    - If a number, then the indicated parameter is taken for it.
    - If `-`, then the previous parameter is taken for it.

    For the axis parameter \c axis-par:
    - The specified parameter is expected to indicate one or more
      unique axes to remove from the current parameter, which must be
      of type `r` or `o`.
    - If a number, then the indicated parameter is taken for it.
    - If `-`, then the previous parameter is taken for it.

    An at sign `@` at the beginning of the list of dimension
    specifications indicates that dimensions equal to 1 are omitted.
    For an input parameter such dimensions are omitted before
    considering the dimension specifications.  For an output or a
    return parameter such dimensions are omitted just before adjusting
    or creating the symbol.

    For the dimension specification \c dim-spec:
    - NUMBER = the current dimension has the specified size.  For
      input parameters, an error is declared if the dimension does not
      have the specified size.
    - `>`NUMBER = the current dimension has at least the specified
      size.  For input parameters, an error is declared if the
      dimension does not have at least the specified size.
    - `+`NUMBER = for output or return parameters, a new dimension with
      the specified size is inserted here.
    - `=` = for output or return parameters, the current dimension is
      taken from the reference parameter.
    - `=`NUMBER = for output or return parameters, the current dimension
      is taken from the reference parameter, and must be equal to the
      specified number.  An error is declared if the reference
      parameter's dimension does not have the indicated size
    - `-` = the corresponding dimension from the reference parameter is
      skipped.
    - `:` = for input parameters, accept the current dimension.
    - `&` = the remaining dimensions must be equal to those of the
      reference parameter.
    - `#` = the element count must be equal to 1 or to that of the
      reference parameter.
    - `*` = the remaining dimensions are unrestricted.

    Both a `+`NUMBER and a `-`NUMBER may be given in the same
    dimension specification \c dim_spec.

  */
int32_t standard_args(int32_t narg, int32_t ps[], char const *fmt,
                      Pointer **ptrs, LoopInfo **infos, size_t* out_size)
{
  int32_t lux_convert(int32_t, int32_t [], Symboltype, int32_t);

  int32_t returnSym = LUX_ONE;
  param_spec_list* psl = parse_standard_arg_fmt(fmt);
  if (!psl) {
    if (ptrs)
      *ptrs = NULL;
    if (infos)
      *infos = NULL;
    if (out_size)
      *out_size = 0;
    return luxerror("Illegal standard arguments specification %s", 0, fmt);
  }
  // the number of parameters except for the return parameter, if any
  int32_t num_in_out_params = psl->num_param_specs
    - (psl->return_param_index >= 0);
  // determine mininum and maximum required number of arguments
  int32_t nmin;
  for (nmin = num_in_out_params; nmin > 0; nmin--)
    if (!psl->param_specs[nmin - 1].is_optional)
      break;
  if (narg < nmin || narg > num_in_out_params) {
    if (ptrs)
      *ptrs = NULL;
    if (infos)
      *infos = NULL;
    if (out_size)
      *out_size = 0;
    return luxerror("Standard arguments specification asks for between "
                    "%d and %d input/output arguments but %d are specified"
                    " (%s)", 0, nmin, num_in_out_params, narg, fmt);
  } // end if (narg < nmin || narg > num_in_out_params)
  if (ptrs)
    *ptrs = (Pointer*) malloc(psl->num_param_specs*sizeof(Pointer));
  if (infos)
    *infos = (LoopInfo*) malloc(psl->num_param_specs*sizeof(LoopInfo));
  if (out_size)
    *out_size = psl->num_param_specs;

  // the final parameter values; they may be converted copies of the
  // original values.
  auto final = reinterpret_cast<int32_t*>
    (calloc(psl->num_param_specs, sizeof(int32_t)));

  Symboltype common_type
    = standard_args_common_symboltype(psl->num_param_specs,
                                      psl->param_specs,
                                      narg, ps);
  /* now we treat the parameters. */
  int32_t prev_ref_param = -1; // < 0 indicates no reference parameter set yet

  NumericDataDescriptor refDescr;

  for (int32_t param_ix = 0; param_ix < psl->num_param_specs; param_ix++) {
    int32_t pspec_dims_ix; /* parameter dimension specification index */
    int32_t ref_dims_ix;   /* reference dimension index */
    int32_t src_dims_ix;   /* input dimension index */
    int32_t iq, d;
    std::vector<DimensionSize_tp> tgt_dims;

    NumericDataDescriptor srcDescr;

    Param_spec* pspec = &psl->param_specs[param_ix];
    Dims_spec* dspec = pspec->dims_spec;
    if (param_ix == num_in_out_params || param_ix >= narg || !ps[param_ix]
        || !srcDescr.set_from(ps[param_ix])) {
      srcDescr.reset();
    } else if (pspec->omit_dimensions_equal_to_one && srcDescr.is_valid()) {
      srcDescr.omit_dimensions_equal_to_one();
    } // end if (param_ix == num_in_out_params || ...) else

    int32_t ref_param = pspec->ref_par;
    if (ref_param < 0)
      ref_param = (param_ix? param_ix - 1: 0);
    if (param_ix > 0             // first parameter has no reference
        && (!refDescr.is_valid() // no reference yet
            || ref_param != prev_ref_param)) { // or different from
                                               // before
      // get reference parameter's information.  If the reference
      // parameter is an output parameter, then we must get the
      // information from its *final* value
      switch (psl->param_specs[ref_param].logical_type) {
      case PS_INPUT:
        if (refDescr.set_from(ps[ref_param])) {
          if (psl->param_specs[ref_param].omit_dimensions_equal_to_one) {
            refDescr.omit_dimensions_equal_to_one();
          }
        } else {
          returnSym = luxerror("Reference parameter %d must be an array",
                               ps[param_ix], ref_param + 1);
          goto error;
        } // end if (refDescr.valid()) else
        break;
      case PS_OUTPUT: case PS_RETURN:
        if (!final[ref_param]) {
          returnSym = luxerror("Illegal forward output/return reference "
                               "parameter %d for parameter %d", 0,
                               ref_param + 1, param_ix + 1);
          goto error;
        } // end if (!final[ref_param])
        if (refDescr.set_from(final[ref_param])) {
          refDescr.omit_dimensions_equal_to_one();
        } else {
          returnSym = luxerror("Reference parameter %d must be an array",
                               final[param_ix], ref_param + 1);
          goto error;
        } // end if (refDescr.set_from(final[ref_param])) else
        break;
      } // end switch (psl->param_specs[ref_param].logical_type)
      prev_ref_param = ref_param;
    } else if (!param_ix) {
      refDescr.reset();
    } // end if (param_ix > 0 ...) else if (!param_ix)

    if (!pspec->is_optional || param_ix == num_in_out_params
        || (param_ix < narg && ps[param_ix])) {
      for (pspec_dims_ix = 0, src_dims_ix = 0, ref_dims_ix = 0;
           pspec_dims_ix < pspec->num_dims_spec; pspec_dims_ix++) {
        int src_dim_size = srcDescr.dimension(src_dims_ix);
        switch (dspec[pspec_dims_ix].type) {
        case DS_EXACT: /* an input parameter must have the exact
                          specified dimension */
        case DS_ATLEAST: // or at least the specified size
          if (pspec->logical_type == PS_INPUT) {
            if (dspec[pspec_dims_ix].type == DS_EXACT
                && src_dim_size != dspec[pspec_dims_ix].size_add) {
              returnSym = luxerror("Expected size %d for dimension %d "
                                   "but found %d", ps[param_ix],
                                   dspec[pspec_dims_ix].size_add,
                                   src_dims_ix, src_dim_size);
              goto error;
            }
            else if (dspec[pspec_dims_ix].type == DS_ATLEAST
                     && src_dim_size < dspec[pspec_dims_ix].size_add) {
              returnSym = luxerror("Expected at least size %d for dimension %d "
                                   "but found %d", ps[param_ix],
                                   dspec[pspec_dims_ix].size_add,
                                   src_dims_ix, src_dim_size);
              goto error;
            } // end if (dspec[pspec_dims_ix].type == DS_EXACT ...) else if
          } // end if (pspec->logical_type == PS_INPUT)
          /* the target gets the exact specified dimension */
          tgt_dims.push_back(dspec[pspec_dims_ix].size_add);
          ++src_dims_ix;
          ++ref_dims_ix;
          break;
        case DS_COPY_REF:       /* copy from reference */
          if (src_dims_ix >= refDescr.dimensions_count()) {
            returnSym = luxerror("Requested copying dimension %d from the "
                                 "reference parameter which has only %d "
                                 "dimensions", ps[param_ix], src_dims_ix,
                                 refDescr.dimensions_count());
            goto error;
          } // end if (src_dims_ix >= refDescr.dimensions_count())
          tgt_dims.push_back(refDescr.dimension(ref_dims_ix++));
          ++src_dims_ix;
          break;
        case DS_ADD:
          d = dspec[pspec_dims_ix].size_add;
          switch (pspec->logical_type) {
          case PS_INPUT:
            if (src_dim_size != d) {
              returnSym = luxerror("Expected size %d for dimension %d "
                                   "but found %d", ps[param_ix],
                                   d, src_dims_ix, src_dim_size);
              goto error;
            } // end if (src_dim_size != d)
            ++src_dims_ix;
            tgt_dims.push_back(d);
            break;
          case PS_OUTPUT: case PS_RETURN:
            tgt_dims.push_back(d);
            break;
          } // end switch (pspec->logical_type)
          break;
        case DS_REMOVE: case DS_ADD_REMOVE:
          switch (pspec->logical_type) {
          case PS_INPUT:
            {
              int32_t d = dspec[pspec_dims_ix].size_remove;
              if (d && refDescr.dimension(ref_dims_ix) != d) {
                returnSym = luxerror("Expected size %d for dimension %d "
                                     "but found %d", ps[param_ix],
                                     d, ref_dims_ix,
                                     refDescr.dimension(ref_dims_ix));
                goto error;
              } // end if (d && refDescr.dimension(ref_dims_ix) != d)
            }
            break;
          case PS_OUTPUT: case PS_RETURN:
            {
              int32_t d = dspec[pspec_dims_ix].size_remove;
              if (d && refDescr.dimension(ref_dims_ix) != d) {
                returnSym = luxerror("Expected size %d for dimension %d "
                                     "but found %d", ps[param_ix],
                                     d, ref_dims_ix,
                                     refDescr.dimension(ref_dims_ix));
                goto error;
              } // end if (d && ref_dims[ref_dims_ix] != d)
            }
            if (dspec[pspec_dims_ix].type == DS_ADD_REMOVE)
              tgt_dims.push_back(dspec[pspec_dims_ix].size_add);
            break;
          } // end switch (pspec->logical_type)
          ref_dims_ix++;
          break;
        case DS_ACCEPT:         /* copy from input */
          if (src_dims_ix >= srcDescr.dimensions_count()) {
            returnSym = luxerror("Cannot copy non-existent dimension %d",
                                 ps[param_ix], src_dims_ix);
            goto error;
          } else {
            tgt_dims.push_back(srcDescr.dimension(src_dims_ix++));
            ++ref_dims_ix;
          } // end if (src_dims_ix >= srcDescr.dimensions_count()) else
          break;
        default:
          returnSym = luxerror("Dimension specification type %d "
                               "not implemented yet", ps[param_ix],
                               dspec[pspec_dims_ix].type);
          goto error;
          break;
        } // end switch (dspec[pspec_dims_ix].type)
      } // end for (pspec_dims_ix = 0, tgt_dims_ix = 0, src_dims_ix = 0,...)

      Symboltype type;
      switch (pspec->logical_type) {
      case PS_INPUT:
        switch (pspec->remaining_dims) {
        case PS_EQUAL_TO_REFERENCE:
          if (refDescr.is_valid()
              && ref_dims_ix < refDescr.dimensions_count()) {
            int32_t expect = refDescr.dimensions_count()
              + src_dims_ix - ref_dims_ix;
            if (expect != srcDescr.dimensions_count()) {
              returnSym = luxerror("Expected %d dimensions but found %d",
                                   ps[param_ix],
                                   refDescr.dimensions_count()
                                   + src_dims_ix - ref_dims_ix,
                                   srcDescr.dimensions_count());
              goto error;
            } // end if (expect != num_src_dims)
            int32_t i, j;
            for (i = ref_dims_ix, j = src_dims_ix;
                 i < refDescr.dimensions_count(); i++, j++)
              if (refDescr.dimension(i) != srcDescr.dimension(j)) {
                returnSym = luxerror("Expected dimension %d equal to %d "
                                     "but found %d", ps[param_ix], i + 1,
                                     refDescr.dimension(i),
                                     srcDescr.dimension(j));
                goto error;
              } // end if (refDescr.dimension(i) != srcDescr.dimension(j))
          } else {
            returnSym = luxerror("Dimensions of parameter %d required to be "
                                 "equal to those of the reference, but no "
                                 "reference is available",
                                 ps[param_ix], param_ix + 1);
            goto error;
          } // end if (refDescr.is_valid() && ref_dims_ix <
            // refDescr.dimensions_count()) else
          break;
        case PS_ONE_OR_EQUAL_TO_REFERENCE:
          if (refDescr.is_valid()
              && ref_dims_ix < refDescr.dimensions_count()) {
            int32_t expect = refDescr.dimensions_count()
              + src_dims_ix - ref_dims_ix;
            if (expect != srcDescr.dimensions_count()) {
              returnSym = luxerror("Expected %d dimensions but found %d",
                                   ps[param_ix],
                                   refDescr.dimensions_count()
                                   + src_dims_ix - ref_dims_ix,
                                   srcDescr.dimensions_count());
              goto error;
            } // end if (expect != srcDescr.dimensions_count())
            int32_t i, j;
            for (i = ref_dims_ix, j = src_dims_ix;
                 i < refDescr.dimensions_count(); i++, j++)
              if (srcDescr.dimension(j) != 1
                  && refDescr.dimension(i) != srcDescr.dimension(j)) {
                if (refDescr.dimension(i) == 1)
                  returnSym = luxerror("Expected dimension %d equal to %d "
                                       "but found %d", ps[param_ix], i + 1,
                                       refDescr.dimension(i),
                                       srcDescr.dimension(j));
                else
                  returnSym = luxerror("Expected dimension %d equal to 1 or "
                                       "%d but found %d", ps[param_ix], i + 1,
                                       refDescr.dimension(i),
                                       srcDescr.dimension(j));
                goto error;
              } // end if (srcDescr.dimension(j) != 1 &&
                // refDescr.dimension(i) != srcDescr.dimension(j))
          } else {
            returnSym = luxerror("Dimensions of parameter %d required to be "
                                 "equal to those of the reference, but no "
                                 "reference is available",
                                 ps[param_ix], param_ix + 1);
            goto error;
          } // end if (ref_dims && ref_dims_ix <
            // refDescr.dimensions_count()) else
          break;
        case PS_ARBITRARY:
          break;
        case PS_ABSENT:
          if (!pspec_dims_ix) {     /* had no dimensions */
            /* assume dimension equal to 1 */
            if (srcDescr.dimension(src_dims_ix) != 1) {
              returnSym = luxerror("Expected dimension %d equal to 1 "
                                   "but found %d", ps[param_ix],
                                   src_dims_ix + 1,
                                   srcDescr.dimension(src_dims_ix));
              goto error;
            } else
              src_dims_ix++;
          } // end if (!pspec_dims_ix)
          if (src_dims_ix < srcDescr.dimensions_count()) {
            returnSym = luxerror("Specification (parameter %d) says %d "
                                 "dimensions but source has %d dimensions",
                                 ps[param_ix], param_ix, src_dims_ix,
                                 srcDescr.dimensions_count());
            goto error;
          } // end if (src_dims_ix < srcDescr.dimensions_count())
          break;
        } // end switch (pspec->remaining_dims)
        iq = ps[param_ix];
        type = symbol_type(iq);
        if (pspec->common_type)
          type = common_type;
        else if ((pspec->data_type_limit == PS_LOWER_LIMIT
                  && type < pspec->data_type)
                 || (pspec->data_type_limit == PS_EXACT
                     && type != pspec->data_type
                     && pspec->data_type != LUX_NO_SYMBOLTYPE))
          type = pspec->data_type;
        iq = lux_convert(1, &iq, type, 1);
        break;
      case PS_OUTPUT: case PS_RETURN:
        switch (pspec->remaining_dims) {
        case PS_ABSENT:
          break;
        case PS_EQUAL_TO_REFERENCE:
          if (ref_dims_ix < refDescr.dimensions_count()) {
            /* append remaining dimensions from reference parameter*/
            while (ref_dims_ix < refDescr.dimensions_count()) {
              tgt_dims.push_back(refDescr.dimension(ref_dims_ix));
              ++src_dims_ix;
              ++ref_dims_ix;
            }
          } // end if (ref_dims_ix < refDescr.dimensions_count())
          break;
        case PS_ARBITRARY:
          returnSym = luxerror("'Arbitrary' remaining dimensions makes no "
                               "sense for an output or return parameter "
                               " (number %d)", 0, param_ix + 1);
          goto error;
        } // end switch (pspec->remaining_dims)
        if (pspec->axis_par > -2) {
          /* We have an axis parameter specified for this one. */
          int32_t axis_param = pspec->axis_par;
          if (axis_param == -1) /* points at previous parameter */
            axis_param = param_ix - 1;

          if (axis_param < 0 || axis_param >= num_in_out_params) {
            returnSym = luxerror("Axis parameter %d for parameter %d is "
                                 "out of bounds", 0, axis_param, param_ix + 1);
            goto error;
          } // end if (axis_param < 0 || axis_param >= narg)
          if (axis_param == param_ix) {
            returnSym = luxerror("Parameter %d cannot be its own axis"
                                 " parameter", 0, param_ix + 1);
            goto error;
          } // end if (axis_param == param_ix)
          if (final[axis_param]) { // axis parameter exists
            // the axis parameter describes which axes of the
            // reference parameter to process.  The output or return
            // value gets the same dimensions as the reference
            // parameter except that the dimensions mentioned in the
            // axis parameter are omitted.
            tgt_dims = refDescr.dimensions();
            int32_t aq = ps[axis_param];
            if (!symbolIsNumerical(aq)) {
              returnSym = luxerror("Axis parameter %d is not numerical for"
                                   " parameter %d", 0,
                                   axis_param + 1, param_ix + 1);
              goto error;
            } // end if (!symbolIsNumerical(aq))
            aq = lux_long(1, &aq);
            int32_t nAxes;
            Pointer axes;
            numerical(aq, NULL, NULL, &nAxes, &axes);
            for (int32_t j = 0; j < nAxes; j++) {
              if (axes.i32[j] < 0 || axes.i32[j] >= tgt_dims.size()) {
                returnSym = luxerror("Axis %d out of bounds for"
                                     " parameter %d", 0,
                                     axes.i32[j], param_ix + 1);
                goto error;
              } // end if (axes.i32[j] < 0 || axes.i32[j] >= tgt_dims_ix)
              tgt_dims[axes.i32[j]] = 0; // flags removal.  Note: no check
                                       // for duplicate axes
            } // end for (j = 0; j < nAxes; j++)
            int32_t k;
            /* remove flagged dimensions */
            tgt_dims.erase(std::remove(tgt_dims.begin(), tgt_dims.end(), 0),
                           tgt_dims.end());
          } else {
            // axis parameter does not exist; treat as 1D
            tgt_dims.clear();   // remove all target dimensions
            // going to produce a scalar
          }
        } // end if (pspec->axis_par > -2)
        if (pspec->omit_dimensions_equal_to_one) {
          if (tgt_dims.size() > 0) {
            tgt_dims.erase(std::remove(tgt_dims.begin(),
                                       tgt_dims.end(),
                                       1),
                           tgt_dims.end());
            if (!tgt_dims.size()) {
              tgt_dims.push_back(1);
            }
          }
        }
        if (param_ix == num_in_out_params) {      /* a return parameter */
          if (ref_param >= 0) {
            type = symbol_type(ps[ref_param]);
          } else if (pspec->data_type != LUX_NO_SYMBOLTYPE) {
            type = pspec->data_type;
          } // end if (ref_param >= 0) else
          if (pspec->common_type
              && common_type != LUX_NO_SYMBOLTYPE) {
            type = common_type;
          } else if (pspec->data_type_limit == PS_EXACT
                     && pspec->data_type != LUX_NO_SYMBOLTYPE) {
            type = pspec->data_type;
          } else if (pspec->data_type_limit == PS_LOWER_LIMIT
                     && type < pspec->data_type) {
            type = pspec->data_type;
          }
          if (tgt_dims.size())
            iq = returnSym = array_scratch(type, tgt_dims.size(),
                                           tgt_dims.data());
          else
            iq = returnSym = scalar_scratch(type);
        } else { // if (param_ix == num_in_out_params) else
          // not a return parameter, so an output parameter
          iq = ps[param_ix];
          type = symbol_type(iq);
          if (symbol_class(iq) == LUX_UNUSED
              || ((pspec->data_type_limit == PS_LOWER_LIMIT
                   && type < pspec->data_type)
                  || (pspec->data_type_limit == PS_EXACT
                      && pspec->data_type != LUX_NO_SYMBOLTYPE
                      && type != pspec->data_type)))
            type = pspec->data_type;
          if (tgt_dims.size())
            redef_array(iq, type, tgt_dims.size(), tgt_dims.data());
          else
            redef_scalar(iq, type, NULL);
        } // end if (param_ix == num_in_out_params) else
        break;
      } // end switch (pspec->logical_type)
      final[param_ix] = iq;
      {
        LoopInfo li;
        Pointer p;
        standardLoop(iq, 0, SL_ALLAXES, symbol_type(iq), &li, &p, NULL,
                     NULL, NULL);
        if (infos)
          (*infos)[param_ix] = li;
        if (ptrs)
          (*ptrs)[param_ix] = p;
      }
    } else { // if (!pspec->is_optional || ...) else
      if (infos)
        memset(&(*infos)[param_ix], 0, sizeof(LoopInfo));
      if (ptrs)
        (*ptrs)[param_ix].v = NULL;
    } // end if (!pspec->is_optional || ...) else

  } // end for (param_ix = 0; param_ix < psl->num_param_specs; param_ix++)

  free_param_spec_list(psl);
  return returnSym;

 error:
  if (ptrs) {
    free(*ptrs);
    *ptrs = NULL;
  }
  if (infos) {
    free(*infos);
    *infos = NULL;
  }
  if (out_size)
    *out_size = 0;
  return returnSym;
}

int32_t standard_args(int32_t narg, int32_t ps[], const std::string& fmt,
                      std::vector<Pointer>& ptrs, std::vector<LoopInfo>& infos)
{
  Pointer* these_ptrs;
  LoopInfo* these_infos;
  size_t out_size;
  int32_t result = standard_args(narg, ps, fmt.c_str(), &these_ptrs,
                                 &these_infos, &out_size);
  ptrs.clear();
  if (these_ptrs)
    ptrs.insert(ptrs.begin(), &these_ptrs[0], &these_ptrs[out_size - 1]);
  infos.clear();
  if (these_infos)
    infos.insert(infos.begin(), &these_infos[0], &these_infos[out_size - 1]);
  return result;
}

/// Construct an object representing standard arguments for a LUX
/// subroutine or function.
///
/// \param[in] narg is the number of arguments to the LUX subroutine
/// or function.
///
/// \param[in] ps points to the beginning of the array of arguments to
/// the LUX subroutine or function.
///
/// \param[in] fmt is the format string that describes what arguments
/// are expected for the LUX subroutine or function.  See at
/// standard_args() for a detailed description.
StandardArguments::StandardArguments(int32_t narg, int32_t ps[],
                                     const std::string& fmt)
{
  int32_t standard_args(int32_t narg, int32_t ps[], const std::string& fmt,
                        std::vector<Pointer>& ptrs, std::vector<LoopInfo>&
                        infos);
  m_return_symbol = standard_args(narg, ps, fmt.c_str(), m_pointers,
                                  m_loop_infos);
}

/// Returns the Pointer for the argument with the given index.
///
/// \param[in] index is the 0-based index of the argument for which to
/// return the Pointer.
///
/// \returns a reference to the Pointer, or a std::out_of_range
/// exception if the \a index is out of range.
Pointer&
StandardArguments::datapointer(size_t index)
{
  return m_pointers.at(index);
}

/// Returns the LoopInfo for the argument with the given index.
///
/// \param[in] index is the 0-based index of the argument for which to
/// return the LoopInfo.
///
/// \returns a reference to the LoopInfo, or a std::out_of_range
/// exception if the \a index is out of range.
LoopInfo&
StandardArguments::datainfo(size_t index)
{
  return m_loop_infos.at(index);
}

/// Returns the return symbol, the LUX symbol intended to represent
/// the result of the call of the LUX subroutine or function.
int32_t
StandardArguments::result() const
{
  return m_return_symbol;
}

int
StandardArguments::advanceLoop(size_t index)
{
  return m_loop_infos[index].advanceLoop(m_pointers[index].ui8);
}

/// Creates an object to which the instances of Pointer and LoopInfo
/// created by a standard_args() function call can be tied.  When this
/// object goes out of scope, then those instances of Pointer and
/// LoopInfo are properly cleaned up.  Otherwise the user has to
/// remember to call free() on the memory storing the arrays of
/// instances of Pointer and LoopInfo.  Call set() to make the tie.
StandardArguments_RAII::StandardArguments_RAII()
  : m_pointers(), m_loopInfos(), m_result_symbol()
{ }

/// Creates an object to which the instances of Pointer and LoopInfo
/// created by a standard_args() function call are tied.  The
/// constructor does the standard_args() call.
///
/// When this object goes out of scope, then those instances of
/// Pointer and LoopInfo are properly cleaned up.  Otherwise the user
/// has to remember to call free() on the memory storing the arrays of
/// instances of Pointer and LoopInfo.
///
/// The parameters are the same as those of standard_args().
StandardArguments_RAII::StandardArguments_RAII(int32_t narg, int32_t ps[],
                                               const std::string& fmt,
                                               Pointer** ptrs,
                                               LoopInfo** infos)
  : m_pointers(), m_loopInfos(), m_result_symbol()
{
  set(narg, ps, fmt, ptrs, infos);
}

/// Calls standard_args() with the given arguments and ties the
/// resulting instances of Pointer and LoopInfo to the current
/// instance.  free() is called on the memory storing the previous
/// instances of Pointer and LoopInfo, if any.
int32_t
StandardArguments_RAII::set(int32_t narg, int32_t ps[], const std::string& fmt,
                            Pointer** ptrs, LoopInfo** infos)
{
  if (m_pointers) {
    free(m_pointers);
    m_pointers = NULL;
  }
  if (m_loopInfos) {
    free(m_loopInfos);
    m_loopInfos = NULL;
  }
  m_result_symbol = standard_args(narg, ps, fmt.c_str(), &m_pointers,
                                  &m_loopInfos);
  if (ptrs)
    *ptrs = m_pointers;
  if (infos)
    *infos = m_loopInfos;
  return m_result_symbol;
}

/// Destructor.  Releases the dynamic memory associated with the
/// instances of Pointer and LoopInfo.
StandardArguments_RAII::~StandardArguments_RAII()
{
  free(m_pointers);
  free(m_loopInfos);
}

int32_t
StandardArguments_RAII::result() const
{
  return m_result_symbol;
}
