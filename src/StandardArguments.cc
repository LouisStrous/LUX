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

/// \file

#include <algorithm>            // for std::remove
#include <cstring>              // for strchr
#include <vector>
#include "NumericDataDescriptor.hh"
#include "action.hh"

namespace StandardArgs {

  /// Represents the parameter type from a StandardArguments format
  /// specification.  The default value is #PS_INPUT.
  enum Param_spec_type {
    /// For an input parameter.  Corresponds to `i` in a StandardArguments
    /// format specification.
    PS_INPUT,

    /// For an output parameter.  Corresponds to `o` in a format specification.
    PS_OUTPUT,

    /// For a return parameter.  Corresponds to `r` in a format specification.
    PS_RETURN
  };

  /// Selects how to handle a particular dimension, for use in
  /// StandardArguments.  Each constant is a bit flag that can be combined with
  /// others through the logical "or" operation.  Not all combinations are
  /// logical.  The default value is `DS_NONE`.
  enum Dim_spec_type {
    /// No selection.
    DS_NONE = 0,

    /// Accept the dimension as is.  Corresponds to `:` in the dimension part of
    /// a StandardArguments format specification.
    DS_ACCEPT = (1<<0),

    /// Insert this dimension.  Corresponds to `+` in a format specification.
    DS_ADD = (1<<1),

    /// Remove this dimension.  Corresponds to `-` in a format specification.
    DS_REMOVE = (1<<2),

    /// Add or remove this dimension.
    DS_ADD_REMOVE = (DS_ADD | DS_REMOVE),

    /// Copy this dimension from the reference symbol.  Corresponds to `&` in a
    /// format specification.
    DS_COPY_REF = (1<<3),

    /// This dimension must have exactly the specified size.
    DS_EXACT = (1<<4),

    /// This dimension must have at least the specified size.  Corresponds to
    /// `>` in a format specification.
    DS_ATLEAST = (1<<5),
  };

  /// Represents a single dimension specification from a StandardArguments
  /// format specification.  The default represents an empty dimension
  /// specification.
  struct Dims_spec {
    /// How to handle the dimension.
    Dim_spec_type type = DS_NONE;

    /// The size to add, or 0 if not specified.
    size_t size_add = 0;

    /// The size to remove, or 0 if not specified.
    size_t size_remove = 0;
  };

  /// Represents how a data type specification in a StandardArguments format
  /// specification should be interpreted.  The default value is `PS_EXACT`.
  enum Type_spec_limit_type {
    /// The data type is exact.
    PS_EXACT,

    /// The data type is a lower limit: If an input argument has a data type
    /// lower than this, then a copy converted to this type is used instead.
    /// Corresponds to `>` in the data type part of a StandardArguments format
    /// specification.
    PS_LOWER_LIMIT,

    /// The data type is an upper limit: If an input argument has a data type
    /// higher than this, then a copy converted to this type is used instead.
    /// Corresponds to `<` in the data type part of a StandardArguments format
    /// specification.
    PS_UPPER_LIMIT,

    /// The data type is integer.  If an input argument has a floating-point
    /// data type, then a copy converted to the corresponding integer type
    /// (`float` to `int32_t`, `double` to `int64_t`) is used instead.
    /// Corresponds to `~` in the data type part of a StandardArguments format
    /// specification.
    PS_FORCE_INTEGER,
  };

  /// Represents how the remaining dimensions should be handled according to a
  /// StandardArguments format specification.  The default value is `PS_ABSENT`.
  enum Remaining_dims_type {
    /// Only the explicitly mentioned dimensions are present.
    PS_ABSENT,

    /// Remaining dimensions are equal to the corresponding ones from the
    /// reference parameter.  Corresponds to `&` in a StandardArguments format
    /// specification.
    PS_EQUAL_TO_REFERENCE,

    /// Remaining dimensions are equal to 1 or to the corresponding ones
    /// from the reference parameter.  Corresponds to `#` in a format
    /// specification.
    PS_ONE_OR_EQUAL_TO_REFERENCE,

    /// Remaining dimensions may have arbitrary sizes.  Corresponds to
    /// `*` in a format specification.
    PS_ARBITRARY
  };

  /// Represents the specification of a single parameter in a StandardArguments
  /// format specification.  The default represents a mandatory input parameter
  /// with no further specifications.
  struct Param_spec {
    /// The parameter type.
    Param_spec_type logical_type = PS_INPUT;

    /// `true` if the parameter is optional, `false` if is is mandatory.
    bool is_optional = false;

    /// The data type limitation.
    Type_spec_limit_type data_type_limit = PS_EXACT;

    /// The data type.
    Symboltype data_type = LUX_NO_SYMBOLTYPE;

    /// The dimension specifications.
    std::vector<Dims_spec> dims_spec;

    /// The reference parameter index in #dims_spec.  It defaults to 0.
    int32_t ref_par = 0;

    /// The axis parameter index in #dims_spec.  It defaults to 0.
    int32_t axis_par = 0;

    /// How to handle dimensions that aren't explicitly specified.
    Remaining_dims_type remaining_dims = PS_ABSENT;

    /// `true` if the current dimension has the "common type" flag (`^` in a
    /// StandardArguments format specification), `false` otherwise.
    bool common_type = false;

    /// `true` if dimensions equal to one should be suppressed as far as
    /// possible, `false` otherwise.
    bool omit_dimensions_equal_to_one = false;
  };

  /// Represents a list of parameter specifications corresponding to a
  /// StandardArguments format specification.  The single return parameter, if
  /// any, comes last.  The default has no parameter specifications.
  struct Param_spec_list
    : std::vector<Param_spec>
  {
    // const methods

    /// \returns the index of the return parameter, or `-1` if there is none.
    int
    return_param_index() const {
      return has_return_param? size() - 1: -1;
    }

    /// \returns the count of input and output parameters.
    int
    num_in_out_parameters() const {
      return has_return_param? size() - 1: size();
    }

    // fields

    /// Is there a return parameter?  If so then it is stored at the end of
    /// #param_specs.
    bool has_return_param = false;
  };
}

using namespace StandardArgs;

/// Parse a StandardArguments format string.
///
/// \param[in] format is the string to parse.
///
/// \param[out] psl receives the parsed information.  Its previous contents are
/// lost.
///
/// \returns `true` if successful, `false` otherwise.
static bool
parse_standard_arg_fmt(const std::string& format, Param_spec_list& psl)
{
  psl.clear();

  if (!format.size())
    return false;

  int32_t param_index = 0;
  bool bad = false;
  int32_t return_param_index = -1;
  auto fmt = format.c_str();
  while (*fmt) {
    while (*fmt && *fmt != ';') { // every parameter specification
      Param_spec p_spec;

      // required parameter type specification
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
          // already had a return parameter
          luxerror("Specified multiple return parameters", 0);
          errno = EINVAL;
          bad = true;
          break;
        } else
          return_param_index = param_index;
        break;
      default:
        // illegal parameter kind specification
        luxerror("Illegal parameter kind %d specified", 0, *fmt);
        errno = EINVAL;
        bad = true;
        break;
      } // end of switch (*fmt)
      ++fmt;

      // optional data type limit specification
      switch (*fmt) {
      case '>':
        p_spec.data_type_limit = PS_LOWER_LIMIT;
        ++fmt;
        break;
      case '<':
        p_spec.data_type_limit = PS_UPPER_LIMIT;
        ++fmt;
        break;
      case '~':
        p_spec.data_type_limit = PS_FORCE_INTEGER;
        ++fmt;
        break;
      } // end of switch (*fmt)

      // optional data type specification
      switch (*fmt) {
      case 'B':
        p_spec.data_type = LUX_INT8;
        ++fmt;
        break;
      case 'W':
        p_spec.data_type = LUX_INT16;
        ++fmt;
        break;
      case 'L':
        p_spec.data_type = LUX_INT32;
        ++fmt;
        break;
      case 'Q':
        p_spec.data_type = LUX_INT64;
        ++fmt;
        break;
      case 'F':
        p_spec.data_type = LUX_FLOAT;
        ++fmt;
        break;
      case 'D':
        p_spec.data_type = LUX_DOUBLE;
        ++fmt;
        break;
      case 'S':
        p_spec.data_type = LUX_TEMP_STRING;
        ++fmt;
        break;
      } // end of switch (*fmt)

      if (*fmt == '^') {
        p_spec.common_type = true;
        ++fmt;
      }

      // optional dims-specs
      if (*fmt == '[') {        // reference parameter specification
        ++fmt;
        if (*fmt == '-') {
          p_spec.ref_par = -1;  // previous parameter is reference
          ++fmt;
        } else if (isdigit(*fmt)) { // a specific parameter is reference
          p_spec.ref_par = strtol(fmt, &fmt, 10);
        } else {
          luxerror("Expected a digit or hyphen after [ in"
                   " reference parameter specification but found %c", 0, *fmt);
          errno = EINVAL;
          bad = true;
          break;
        }
        if (*fmt == ']')
          ++fmt;
        else {
          luxerror("Expected ] instead of %c at end of reference "
                   "parameter specification", 0, *fmt);
          errno = EINVAL;
          bad = true;
          break;
        }
      }
      if (*fmt == '{') {        // optional axis parameter specification
        // if (p_spec.logical_type == PS_INPUT) {
        //   luxerror("Axis parameter illegally specified for input parameter",
        //         0, fmt);
        //   errno = EINVAL;
        //   bad = true;
        //   break;
        // }
        ++fmt;
        if (*fmt == '-') {
          p_spec.axis_par = -1;  // previous parameter is axis parameter
          ++fmt;
        } else if (isdigit(*fmt)) { // specific parameter is axis parameter
          p_spec.axis_par = strtol(fmt, &fmt, 10);
        } else {
          luxerror("Expected a digit or hyphen after { in"
                   " reference parameter specification but found %c", 0, *fmt);
          errno = EINVAL;
          bad = true;
          break;
        }
        // TODO: parse axis modes
        if (*fmt == '}')
          ++fmt;
        else {
          luxerror("Expected } instead of %c at end of reference "
                   "parameter specification", 0, *fmt);
          errno = EINVAL;
          bad = true;
          break;
        }
      } else
        p_spec.axis_par = -2;   // indicates "none"
      if (bad)
        break;
      if (*fmt == '@') {
        p_spec.omit_dimensions_equal_to_one = true;
        ++fmt;
      }
      while (*fmt && !strchr("*?;&#", *fmt)) { // all dims
        Dims_spec d_spec;
        while (*fmt && !strchr(",?*;&#", *fmt)) { // every dim
          Dim_spec_type type;
          size_t size = 0;
          switch (*fmt) {
          case '+':
            type = DS_ADD;
            ++fmt;
            break;
          case '-':
            type = DS_REMOVE;
            ++fmt;
            break;
          case '=':
            type = DS_COPY_REF;
            ++fmt;
            break;
          case ':':
            type = DS_ACCEPT;
            ++fmt;
            break;
          case '>':
            type = DS_ATLEAST;
            ++fmt;
            break;
          default:
            type = DS_EXACT;
            break;
          } // end of switch (*fmt)
          if (isdigit(*fmt)) {
            size = strtol(fmt, &fmt, 10);
          }
          switch (type) {
          case DS_ADD:
            if (d_spec.type == DS_NONE || d_spec.type == DS_REMOVE) {
              d_spec.size_add = size;
              d_spec.type = static_cast<Dim_spec_type>(d_spec.type | type);
            } else {
              luxerror("Illegal combination of multiple types for dimension;"
                       " parameter specification #%d: %s", 0, param_index + 1,
                       format.c_str());
              errno = EINVAL;
              bad = true;
              break;
            }
            break;
          case DS_REMOVE:
            if (d_spec.type == DS_NONE || d_spec.type == DS_ADD) {
              d_spec.size_remove = size;
              d_spec.type = static_cast<Dim_spec_type>(d_spec.type | type);
            } else {
              luxerror("Illegal combination of multiple types for dimension;"
                       " parameter specification #%d: %s", 0, param_index + 1,
                       format.c_str());
              errno = EINVAL;
              bad = true;
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
                       format.c_str());
              errno = EINVAL;
              bad = true;
              break;
            }
            break;
          } // end switch type
          if (bad)
            break;
        } // end of while *fmt && !strchr(",*;&")
        if (bad)
          break;
        p_spec.dims_spec.push_back(d_spec);
        if (*fmt == ',')
          ++fmt;
      } // end of while *fmt && !strchr("*;&", *fmt)
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
      }

      if (*fmt == '?') {        /* optional argument */
        if (p_spec.logical_type == PS_RETURN) {
          /* return parameter cannot be optional */
          luxerror("Return parameter was illegally specified as optional", 0);
          errno = EINVAL;
          bad = true;
          break;
        } else
          p_spec.is_optional = 1;
        ++fmt;
      }
      if (bad)
        break;
      if (*fmt && *fmt != ';') {
        luxerror("Expected ; instead of %c at end of parameter "
                 "specification", 0, *fmt);
        errno = EINVAL;
        bad = true;
        break;
      }
      if (bad)
        break;
      psl.push_back(p_spec);
    } /* end of while (*fmt && *fmt != ';') */
    if (bad)
      break;
    if (*fmt == ';')
      ++fmt;
    else if (*fmt) {
      /* unexpected character */
      luxerror("Expected ; instead of %c at end of parameter specification",
               0, *fmt);
      errno = EINVAL;
      bad = true;
      break;
    }
    param_index++;
  }   /* end of while (*fmt) */
  if (!bad) {
    if (return_param_index >= 0) {
      auto last = psl.size() - 1;
      if (return_param_index != last) {
        // move the return parameter to the end
        std::swap(psl[return_param_index],
                  psl[last]);
      }
      psl.has_return_param = true;
    }
  }
  if (!bad) {
    /* check that the reference parameter does not point outside the list */
    int32_t n = psl.num_in_out_parameters();
    if (n) {
      for (int i = 0; i < psl.size(); i++) {
        if (psl[i].ref_par >= n) {
          errno = EINVAL;
          luxerror("Reference parameter %d for parameter %d points outside"
                   " of the list (size %d)", 0, psl[i].ref_par + 1,
                   i + 1, n);
          bad = true;
          break;
        }
      }
    }
  }
  return !bad;
}

/// Determines the final symbol type.
///
/// \param[in] actual is the symbol type of the Symbol.
///
/// \param[in] limit is the limiting symbol type to compare with.
///
/// \param[in] limit_type identifies the nature of the limit.
///
/// \returns the symbol type that corresponds to the \a actual one with the
/// limit applied.
static Symboltype
standard_args_final_symboltype(Symboltype actual, Symboltype limit,
                               Type_spec_limit_type limit_type)
{
  switch (limit_type) {
  case PS_EXACT:
    if (limit != LUX_NO_SYMBOLTYPE
        && actual != limit) {
      actual = limit;
    }
    break;
  case PS_LOWER_LIMIT:
    if (actual == LUX_NO_SYMBOLTYPE
        || (actual < limit && limit != LUX_NO_SYMBOLTYPE)) {
      actual = limit;
    }
    break;
  case PS_UPPER_LIMIT:
    if (actual == LUX_NO_SYMBOLTYPE
        || (actual > limit && limit != LUX_NO_SYMBOLTYPE)) {
      actual = limit;
    }
    break;
  case PS_FORCE_INTEGER:
    switch (actual) {
    case LUX_FLOAT: case LUX_CFLOAT: case LUX_NO_SYMBOLTYPE:
      actual = LUX_INT32;
      break;
    case LUX_DOUBLE: case LUX_CDOUBLE:
      actual = LUX_INT64;
      break;
    default:
      if (actual < limit && limit != LUX_NO_SYMBOLTYPE) {
        actual = limit;
      }
      break;
    }
    break;
  }
  return actual;
}

/// Determines the common symbol type for standard arguments.
///
/// \param[in] param_specs points at the parameter specifications to inspect.
///
/// \param[in] narg is the count of arguments in \a ps.
///
/// \param[in] ps points at the arguments.
///
/// \returns the highest #Symboltype among the arguments for which the common
/// symbol type is requested, or #LUX_NO_SYMBOLTYPE if there aren't any.
static Symboltype
standard_args_common_symboltype(const Param_spec_list& param_specs,
                                ArgumentCount narg, Symbol ps[])
{
  Symboltype common_type = LUX_NO_SYMBOLTYPE;

  for (int ix = 0; ix < param_specs.size(); ++ix) {
    auto& pspec = param_specs[ix];
    if (pspec.common_type) {
      int32_t iq = ps[ix];
      Symboltype type;
      if (pspec.logical_type == PS_INPUT)
        type = symbol_type(iq);
      else
        type = LUX_NO_SYMBOLTYPE;

      type = standard_args_final_symboltype(type, pspec.data_type,
                                            pspec.data_type_limit);
      if (type != LUX_NO_SYMBOLTYPE
          && (common_type == LUX_NO_SYMBOLTYPE
              || type > common_type))
        common_type = type;
    }
  }
  return common_type;
}

StandardArguments::StandardArguments()
  : m_return_symbol()
{ }

StandardArguments::StandardArguments(ArgumentCount narg, Symbol ps[],
                                     const std::string& fmt,
                                     Pointer** ptrs, LoopInfo** infos)
{
  set(narg, ps, fmt, ptrs, infos);
}

/// Configure symbols based on the passed symbols and a StandardArguments format
/// string.  The configured symbols can be used to handle a call of a LUX
/// function or subroutine.
///
/// \param[in] narg is the number of symbols passed in \a ps.
///
/// \param[in] ps points at the symbols passed into this call.
///
/// \param[in] fmt is the StandardArguments format string that says what input,
/// output, and return symbols the LUX function or subroutine expects.
///
/// \param[in,out] ptrs points (if it is not null) at the address where to store
/// a pointer to one Pointer per configured symbol, that points at the first
/// data element of each symbol.
///
/// \param[in,out] infos points (if it is not null) at the address where to
/// store a pointer to one LoopInfo per configured symbol, that holds
/// information about the coordinates, dimensional structure, and way to
/// traverse the elements.
///
/// \returns the configured return symbol.  For a LUX subroutine this is
/// LUX_ONE.
Symbol
StandardArguments::set(ArgumentCount narg, Symbol ps[], const std::string& fmt,
                       Pointer** ptrs, LoopInfo** infos)
{
  int32_t lux_convert(int32_t, int32_t [], Symboltype, int32_t);

  m_return_symbol = LUX_ONE;
  Param_spec_list psl;
  if (!parse_standard_arg_fmt(fmt, psl)) {
    m_return_symbol = luxerror("Illegal standard arguments specification %s", 0,
                               fmt.c_str());
    return m_return_symbol;
  }

  // the number of parameters except for the return parameter, if any
  int32_t num_in_out_params = psl.num_in_out_parameters();

  // determine mininum and maximum required number of arguments
  int32_t nmin;
  for (nmin = num_in_out_params; nmin > 0; nmin--)
    if (!psl[nmin - 1].is_optional)
      break;
  if (narg < nmin || narg > num_in_out_params) {
    m_return_symbol =
      luxerror("Standard arguments specification asks for between "
               "%d and %d input/output arguments but %d are specified"
               " (%s)", 0, nmin, num_in_out_params, narg, fmt);
    return m_return_symbol;
  } // end if (narg < nmin || narg > num_in_out_params)
  m_pointers.resize(psl.size());
  m_loop_infos.resize(psl.size());

  // the final parameter values; they may be converted copies of the
  // original values.
  auto final = std::vector<Symbol>(psl.size());

  Symboltype common_type
    = standard_args_common_symboltype(psl, narg, ps);

  // now we treat the parameters
  int32_t prev_ref_param = -1; // < 0 indicates no reference parameter set yet

  NumericDataDescriptor refDescr;

  for (int32_t param_ix = 0; param_ix < psl.size(); param_ix++) {
    int32_t pspec_dims_ix; /* parameter dimension specification index */
    int32_t ref_dims_ix;   /* reference dimension index */
    int32_t src_dims_ix;   /* input dimension index */
    int32_t iq, d;
    std::vector<Dimension> tgt_dims;

    NumericDataDescriptor srcDescr;

    auto& pspec = psl[param_ix];
    auto& dspec = pspec.dims_spec;
    if (param_ix == num_in_out_params || param_ix >= narg || !ps[param_ix]
        || !srcDescr.set_from(ps[param_ix])) {
      srcDescr.reset();
    } else if (pspec.omit_dimensions_equal_to_one && srcDescr.is_valid()) {
      srcDescr.omit_dimensions_equal_to_one();
    } // end if (param_ix == num_in_out_params || ...) else

    int32_t ref_param = pspec.ref_par;
    if (ref_param < 0)
      ref_param = (param_ix? param_ix - 1: 0);
    if (param_ix > 0             // first parameter has no reference
        && (!refDescr.is_valid() // no reference yet
            || ref_param != prev_ref_param)) { // or different from
                                               // before
      // get reference parameter's information.  If the reference
      // parameter is an output parameter, then we must get the
      // information from its *final* value
      switch (psl[ref_param].logical_type) {
      case PS_INPUT:
        if (refDescr.set_from(ps[ref_param])) {
          if (psl[ref_param].omit_dimensions_equal_to_one) {
            refDescr.omit_dimensions_equal_to_one();
          }
        } else {
          m_return_symbol = luxerror("Reference parameter %d must be an array",
                               ps[param_ix], ref_param + 1);
          return m_return_symbol;
        } // end if (refDescr.valid()) else
        break;
      case PS_OUTPUT: case PS_RETURN:
        if (!final[ref_param]) {
          m_return_symbol = luxerror("Illegal forward output/return reference "
                               "parameter %d for parameter %d", 0,
                               ref_param + 1, param_ix + 1);
          return m_return_symbol;
        } // end if (!final[ref_param])
        if (refDescr.set_from(final[ref_param])) {
          refDescr.omit_dimensions_equal_to_one();
        } else {
          m_return_symbol = luxerror("Reference parameter %d must be an array",
                               final[param_ix], ref_param + 1);
          return m_return_symbol;
        } // end if (refDescr.set_from(final[ref_param])) else
        break;
      } // end switch (psl.param_specs[ref_param].logical_type)
      prev_ref_param = ref_param;
    } else if (!param_ix) {
      refDescr.reset();
    } // end if (param_ix > 0 ...) else if (!param_ix)

    if (!pspec.is_optional || param_ix == num_in_out_params
        || (param_ix < narg && ps[param_ix])) {
      for (pspec_dims_ix = 0, src_dims_ix = 0, ref_dims_ix = 0;
           pspec_dims_ix < pspec.dims_spec.size(); pspec_dims_ix++) {
        int src_dim_size = srcDescr.dimension(src_dims_ix);
        switch (dspec[pspec_dims_ix].type) {
        case DS_EXACT: /* an input parameter must have the exact
                          specified dimension */
        case DS_ATLEAST: // or at least the specified size
          if (pspec.logical_type == PS_INPUT) {
            if (dspec[pspec_dims_ix].type == DS_EXACT
                && src_dim_size != dspec[pspec_dims_ix].size_add) {
              m_return_symbol = luxerror("Expected size %d for dimension %d "
                                   "but found %d", ps[param_ix],
                                   dspec[pspec_dims_ix].size_add,
                                   src_dims_ix, src_dim_size);
              return m_return_symbol;
            }
            else if (dspec[pspec_dims_ix].type == DS_ATLEAST
                     && src_dim_size < dspec[pspec_dims_ix].size_add) {
              m_return_symbol =
                luxerror("Expected at least size %d for dimension %d "
                         "but found %d", ps[param_ix],
                                   dspec[pspec_dims_ix].size_add,
                                   src_dims_ix, src_dim_size);
              return m_return_symbol;
            } // end if (dspec[pspec_dims_ix].type == DS_EXACT ...) else if
          } // end if (pspec.logical_type == PS_INPUT)
          // the target gets the exact specified dimension
          tgt_dims.push_back(dspec[pspec_dims_ix].size_add);
          ++src_dims_ix;
          ++ref_dims_ix;
          break;
        case DS_COPY_REF:       /* copy from reference */
          if (src_dims_ix >= refDescr.dimensions_count()) {
            m_return_symbol =
              luxerror("Requested copying dimension %d from "
                       "the reference parameter which has only %d "
                       "dimensions", ps[param_ix], src_dims_ix,
                       refDescr.dimensions_count());
            return m_return_symbol;
          } // end if (src_dims_ix >= refDescr.dimensions_count())
          tgt_dims.push_back(refDescr.dimension(ref_dims_ix++));
          ++src_dims_ix;
          break;
        case DS_ADD:
          d = dspec[pspec_dims_ix].size_add;
          switch (pspec.logical_type) {
          case PS_INPUT:
            if (src_dim_size != d) {
              m_return_symbol = luxerror("Expected size %d for dimension %d "
                                         "but found %d", ps[param_ix],
                                         d, src_dims_ix, src_dim_size);
              return m_return_symbol;
            } // end if (src_dim_size != d)
            ++src_dims_ix;
            tgt_dims.push_back(d);
            break;
          case PS_OUTPUT: case PS_RETURN:
            tgt_dims.push_back(d);
            break;
          } // end switch (pspec.logical_type)
          break;
        case DS_REMOVE: case DS_ADD_REMOVE:
          switch (pspec.logical_type) {
          case PS_INPUT:
            {
              int32_t d = dspec[pspec_dims_ix].size_remove;
              if (d && refDescr.dimension(ref_dims_ix) != d) {
                m_return_symbol = luxerror("Expected size %d for dimension %d "
                                           "but found %d", ps[param_ix],
                                           d, ref_dims_ix,
                                           refDescr.dimension(ref_dims_ix));
                return m_return_symbol;
              } // end if (d && refDescr.dimension(ref_dims_ix) != d)
            }
            break;
          case PS_OUTPUT: case PS_RETURN:
            {
              int32_t d = dspec[pspec_dims_ix].size_remove;
              if (d && refDescr.dimension(ref_dims_ix) != d) {
                m_return_symbol = luxerror("Expected size %d for dimension %d "
                                           "but found %d", ps[param_ix],
                                           d, ref_dims_ix,
                                           refDescr.dimension(ref_dims_ix));
                return m_return_symbol;
              } // end if (d && ref_dims[ref_dims_ix] != d)
            }
            if (dspec[pspec_dims_ix].type == DS_ADD_REMOVE)
              tgt_dims.push_back(dspec[pspec_dims_ix].size_add);
            break;
          } // end switch (pspec.logical_type)
          ref_dims_ix++;
          break;
        case DS_ACCEPT:         /* copy from input */
          if (src_dims_ix >= srcDescr.dimensions_count()) {
            m_return_symbol = luxerror("Cannot copy non-existent dimension %d",
                                       ps[param_ix], src_dims_ix);
            return m_return_symbol;
          } else {
            tgt_dims.push_back(srcDescr.dimension(src_dims_ix++));
            ++ref_dims_ix;
          } // end if (src_dims_ix >= srcDescr.dimensions_count()) else
          break;
        default:
          m_return_symbol = luxerror("Dimension specification type %d "
                                     "not implemented yet", ps[param_ix],
                                     dspec[pspec_dims_ix].type);
          return m_return_symbol;
        } // end switch (dspec[pspec_dims_ix].type)
      } // end for (pspec_dims_ix = 0, tgt_dims_ix = 0, src_dims_ix = 0,...)

      Symboltype type;
      switch (pspec.logical_type) {
      case PS_INPUT:
        switch (pspec.remaining_dims) {
        case PS_EQUAL_TO_REFERENCE:
          if (refDescr.is_valid()
              && ref_dims_ix < refDescr.dimensions_count()) {
            int32_t expect = refDescr.dimensions_count()
              + src_dims_ix - ref_dims_ix;
            if (expect != srcDescr.dimensions_count()) {
              m_return_symbol = luxerror("Expected %d dimensions but found %d",
                                         ps[param_ix],
                                         refDescr.dimensions_count()
                                         + src_dims_ix - ref_dims_ix,
                                         srcDescr.dimensions_count());
              return m_return_symbol;
            } // end if (expect != num_src_dims)
            int32_t i, j;
            for (i = ref_dims_ix, j = src_dims_ix;
                 i < refDescr.dimensions_count(); i++, j++)
              if (refDescr.dimension(i) != srcDescr.dimension(j)) {
                m_return_symbol = luxerror("Expected dimension %d equal to %d "
                                           "but found %d", ps[param_ix], i + 1,
                                           refDescr.dimension(i),
                                           srcDescr.dimension(j));
                return m_return_symbol;
              } // end if (refDescr.dimension(i) != srcDescr.dimension(j))
          } else {
            m_return_symbol =
              luxerror("Dimensions of parameter %d required to be "
                       "equal to those of the reference, but no "
                       "reference is available",
                       ps[param_ix], param_ix + 1);
            return m_return_symbol;
          } // end if (refDescr.is_valid() && ref_dims_ix <
            // refDescr.dimensions_count()) else
          break;
        case PS_ONE_OR_EQUAL_TO_REFERENCE:
          if (refDescr.is_valid()
              && ref_dims_ix < refDescr.dimensions_count()) {
            int32_t expect = refDescr.dimensions_count()
              + src_dims_ix - ref_dims_ix;
            if (expect != srcDescr.dimensions_count()) {
              m_return_symbol = luxerror("Expected %d dimensions but found %d",
                                         ps[param_ix],
                                         refDescr.dimensions_count()
                                         + src_dims_ix - ref_dims_ix,
                                         srcDescr.dimensions_count());
              return m_return_symbol;
            } // end if (expect != srcDescr.dimensions_count())
            int32_t i, j;
            for (i = ref_dims_ix, j = src_dims_ix;
                 i < refDescr.dimensions_count(); i++, j++)
              if (srcDescr.dimension(j) != 1
                  && refDescr.dimension(i) != srcDescr.dimension(j)) {
                if (refDescr.dimension(i) == 1)
                  m_return_symbol =
                    luxerror("Expected dimension %d equal to %d "
                             "but found %d", ps[param_ix], i + 1,
                             refDescr.dimension(i),
                             srcDescr.dimension(j));
                else
                  m_return_symbol =
                    luxerror("Expected dimension %d equal to 1 or "
                             "%d but found %d", ps[param_ix], i + 1,
                             refDescr.dimension(i),
                             srcDescr.dimension(j));
                return m_return_symbol;
              } // end if (srcDescr.dimension(j) != 1 &&
                // refDescr.dimension(i) != srcDescr.dimension(j))
          } else {
            m_return_symbol =
              luxerror("Dimensions of parameter %d required to be "
                       "equal to those of the reference, but no "
                       "reference is available",
                       ps[param_ix], param_ix + 1);
            return m_return_symbol;
          } // end if (ref_dims && ref_dims_ix <
            // refDescr.dimensions_count()) else
          break;
        case PS_ARBITRARY:
          break;
        case PS_ABSENT:
          if (!pspec_dims_ix) {     /* had no dimensions */
            /* assume dimension equal to 1 */
            if (srcDescr.dimension(src_dims_ix) != 1) {
              m_return_symbol = luxerror("Expected dimension %d equal to 1 "
                                         "but found %d", ps[param_ix],
                                         src_dims_ix + 1,
                                         srcDescr.dimension(src_dims_ix));
              return m_return_symbol;
            } else
              src_dims_ix++;
          } // end if (!pspec_dims_ix)
          if (src_dims_ix < srcDescr.dimensions_count()) {
            m_return_symbol =
              luxerror("Specification (parameter %d) says %d "
                       "dimensions but source has %d dimensions",
                       ps[param_ix], param_ix, src_dims_ix,
                       srcDescr.dimensions_count());
            return m_return_symbol;
          } // end if (src_dims_ix < srcDescr.dimensions_count())
          break;
        } // end switch (pspec.remaining_dims)
        iq = ps[param_ix];
        type = symbol_type(iq);
        if (pspec.common_type)
          type = common_type;
        else
          type = standard_args_final_symboltype(type, pspec.data_type,
                                                pspec.data_type_limit);
        iq = lux_convert(1, &iq, type, 1);
        break;
      case PS_OUTPUT: case PS_RETURN:
        switch (pspec.remaining_dims) {
        case PS_ABSENT:
          break;
        case PS_EQUAL_TO_REFERENCE:
          if (ref_dims_ix < refDescr.dimensions_count()) {
            // append remaining dimensions from reference parameter
            while (ref_dims_ix < refDescr.dimensions_count()) {
              tgt_dims.push_back(refDescr.dimension(ref_dims_ix));
              ++src_dims_ix;
              ++ref_dims_ix;
            }
          } // end if (ref_dims_ix < refDescr.dimensions_count())
          break;
        case PS_ARBITRARY:
          m_return_symbol =
            luxerror("'Arbitrary' remaining dimensions makes no "
                     "sense for an output or return parameter "
                     " (number %d)", 0, param_ix + 1);
          return m_return_symbol;
        } // end switch (pspec.remaining_dims)
        if (pspec.axis_par > -2) {
          /* We have an axis parameter specified for this one. */
          int32_t axis_param = pspec.axis_par;
          if (axis_param == -1) /* points at previous parameter */
            axis_param = param_ix - 1;

          if (axis_param < 0 || axis_param >= num_in_out_params) {
            m_return_symbol = luxerror("Axis parameter %d for parameter %d is "
                                       "out of bounds", 0,
                                       axis_param, param_ix + 1);
            return m_return_symbol;
          } // end if (axis_param < 0 || axis_param >= narg)
          if (axis_param == param_ix) {
            m_return_symbol = luxerror("Parameter %d cannot be its own axis"
                                       " parameter", 0, param_ix + 1);
            return m_return_symbol;
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
              m_return_symbol =
                luxerror("Axis parameter %d is not numerical for"
                         " parameter %d", 0,
                         axis_param + 1, param_ix + 1);
              return m_return_symbol;
            } // end if (!symbolIsNumerical(aq))
            aq = lux_long(1, &aq);
            int32_t nAxes;
            Pointer axes;
            numerical(aq, NULL, NULL, &nAxes, &axes);
            for (int32_t j = 0; j < nAxes; j++) {
              if (axes.i32[j] < 0 || axes.i32[j] >= tgt_dims.size()) {
                m_return_symbol = luxerror("Axis %d out of bounds for"
                                           " parameter %d", 0,
                                           axes.i32[j], param_ix + 1);
                return m_return_symbol;
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
        } // end if (pspec.axis_par > -2)
        if (pspec.omit_dimensions_equal_to_one) {
          if (tgt_dims.size() > 0) {
            tgt_dims.erase(std::remove(tgt_dims.begin(),
                                       tgt_dims.end(),
                                       1),
                           tgt_dims.end());
          }
        }
        if (param_ix == num_in_out_params) {      /* a return parameter */
          if (ref_param >= 0) {
            type = symbol_type(ps[ref_param]);
          } else if (pspec.data_type != LUX_NO_SYMBOLTYPE) {
            type = pspec.data_type;
          } // end if (ref_param >= 0) else
          if (pspec.common_type
              && common_type != LUX_NO_SYMBOLTYPE) {
            type = common_type;
          } else {
            type = standard_args_final_symboltype(type, pspec.data_type,
                                                  pspec.data_type_limit);
          }
          if (tgt_dims.size())
            iq = m_return_symbol = array_scratch(type, tgt_dims.size(),
                                                 tgt_dims.data());
          else
            iq = m_return_symbol = scalar_scratch(type);
        } else { // if (param_ix == num_in_out_params) else
          // not a return parameter, so an output parameter
          iq = ps[param_ix];
          type = symbol_type(iq);
          if (symbol_class(iq) == LUX_UNUSED) {
            type = pspec.data_type;
          } else {
            switch (pspec.data_type_limit) {
            case PS_LOWER_LIMIT:
              if (type < pspec.data_type) {
                type = pspec.data_type;
              }
              break;
            case PS_UPPER_LIMIT:
              if (type > pspec.data_type) {
                type = pspec.data_type;
              }
              break;
            case PS_EXACT:
              if (pspec.data_type != LUX_NO_SYMBOLTYPE
                  && type != pspec.data_type) {
                type = pspec.data_type;
              }
              break;
            }
          }
          if (tgt_dims.size())
            redef_array(iq, type, tgt_dims.size(), tgt_dims.data());
          else
            redef_scalar(iq, type, NULL);
        } // end if (param_ix == num_in_out_params) else
        break;
      } // end switch (pspec.logical_type)
      final[param_ix] = iq;
      {
        LoopInfo li;
        Pointer p;
        standardLoop(iq, 0, SL_ALLAXES, symbol_type(iq), &li, &p, NULL,
                     NULL, NULL);
        m_loop_infos[param_ix] = li;
        m_pointers[param_ix] = p;
      }
    } // end if (!pspec.is_optional || ...) else

  } // end for (param_ix = 0; param_ix < psl.num_param_specs; param_ix++)
  if (ptrs) {
    *ptrs = &m_pointers[0];
  }
  if (infos) {
    *infos = &m_loop_infos[0];
  }
  return m_return_symbol;
}

size_t
StandardArguments::size() const
{
  return m_pointers.size();
}

Pointer&
StandardArguments::datapointer(size_t index)
{
  return m_pointers.at(index);
}

LoopInfo&
StandardArguments::datainfo(size_t index)
{
  return m_loop_infos.at(index);
}

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
