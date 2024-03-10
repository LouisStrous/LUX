/* This is file StandardArguments.hh.

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
/// Declares StandardArguments.

#ifndef STANDARDARGUMENTS_HH_
#define STANDARDARGUMENTS_HH_

#include <stdexcept>
#include <string>
#include <vector>

/// A class to facilitate applying LUX functions.
class StandardArguments
{
public:
  // enums, nested structs

  /// Represents the parameter type from a StandardArguments format
  /// specification.  The default value is #PS_INPUT.
  enum Param_spec_type
  {
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
  enum Dim_spec_type
  {
    /// No selection.
    DS_NONE = 0,

    /// Accepts the dimension as is.  Corresponds to `:` in the dimension part
    /// of a StandardArguments format specification.
    DS_ACCEPT = (1<<0),

    /// Inserts this dimension.  Corresponds to `+` in a format specification.
    DS_ADD = (1<<1),

    /// Removes this dimension.  Corresponds to `-` in a format specification.
    DS_REMOVE = (1<<2),

    /// Adds or removes this dimension.
    DS_ADD_REMOVE = (DS_ADD | DS_REMOVE),

    /// Copies this dimension from the reference symbol.  Corresponds to `&` in
    /// a format specification.
    DS_COPY_REF = (1<<3),

    /// This dimension must have exactly the specified size.
    DS_EXACT = (1<<4),

    /// This dimension must have at least the specified size.  Corresponds to
    /// `>` in a format specification.
    DS_ATLEAST = (1<<5),
  };

  /// Logical or operator for Dim_spec_type.
  ///
  /// \param lhs is the left hand side value.
  ///
  /// \param rhs is the right hand side value.
  ///
  /// \returns the combined value
  friend Dim_spec_type operator|(const Dim_spec_type lhs,
                                 const Dim_spec_type rhs);

  /// Logical or-and-assign operator for Dim_spec_type.
  ///
  /// \param lhs is a reference to the left hand side value.
  ///
  /// \param rhs is the right hand side value.
  ///
  /// \returns a reference to the left hand side value after merging in the
  /// right hand side value.
  friend Dim_spec_type& operator|=(const Dim_spec_type& lhs,
                                   const Dim_spec_type rhs);

  /// Represents a single dimension specification from a StandardArguments
  /// format specification.  The default represents an empty dimension
  /// specification.
  struct Dims_spec
  {
    /// How to handle the dimension.
    Dim_spec_type type = DS_NONE;

    /// The size to add, or 0 if not specified.
    size_t size_add = 0;

    /// The size to remove, or 0 if not specified.
    size_t size_remove = 0;
  };

  /// Represents how a data type specification in a StandardArguments format
  /// specification should be interpreted.  The default value is `PS_EXACT`.
  enum Type_spec_limit_type
  {
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
  enum Remaining_dims_type
  {
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

  /// Represents the specification of a single parameter in a Param_spec_list
  /// that is a StandardArguments format specification.  The default represents
  /// a mandatory input parameter that contains exactly one value (i.e., is
  /// either a scalar or an array with one element).
  struct Param_spec
  {
    // non-const methods

    /// Sets the optionality of the parameter.
    ///
    /// \param is says whether the parameter is optional (`true`) or mandatory
    /// (`false`).
    ///
    /// \returns a reference to the current object.
    Param_spec& set_optional(bool is = true);

    /// Sets a constraint on the data type of the parameter.
    ///
    /// \param t is the constraint to apply, replacing any such constraint that
    /// was specified earlier.
    ///
    /// \returns a reference to the current object.
    Param_spec& set_data_type_limit(Type_spec_limit_type t);

    /// Sets how dimensions are handled beyond the ones for which explicit
    /// specifications are provided.
    ///
    /// \param t says how to handle the excess dimensions.
    ///
    /// \returns a reference to the current object.
    Param_spec& set_remaining_dims_action(Remaining_dims_type t);

    /// Says whether the current parameter contributes to and takes the common
    /// data type.  The common data type is the greatest data type among all
    /// parameters that are marked as contributing to the common data type.
    ///
    /// An output or return parameter gets the common type.  If this parameter
    /// is an input parameter and if the Symbol specified as argument for this
    /// parameter does not have this type, then a copy is created and converted
    /// to the specified type and then the copy's data gets processed instead.
    ///
    /// \param is says whether the current parameter does (`true`) or does not
    /// (`false`) contribute to and take the common type.
    ///
    /// \returns a reference to the current object.
    Param_spec& set_use_common_type(bool is = true);

    /// Says whether to omit any dimensions with a size equal to one.
    ///
    /// \param is says whether to omit (`true`) or keep (`false`) dimensions
    /// equal to one.
    ///
    /// \returns a reference to the current object.
    Param_spec& set_omit_dimension_equal_to_one(bool is = true);

    // fields

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
    Symbol ref_par = 0;

    /// The axis parameter index in #dims_spec.  It defaults to -2, which
    /// indicates "none".
    Symbol axis_par = -2;

    /// How to handle dimensions that aren't explicitly specified.
    Remaining_dims_type remaining_dims = PS_ABSENT;

    /// `true` if the current dimension has the "common type" flag (`^` in a
    /// StandardArguments format specification), `false` otherwise.
    bool common_type = false;

    /// `true` if dimensions equal to one should be suppressed as far as
    /// possible, `false` otherwise.
    bool omit_dimensions_equal_to_one = false;

    /// `true` if the base data type should be taken from the reference
    /// parameter, `false` otherwise.
    bool data_type_from_ref = false;
  };

  /// Represents a vector of parameter specifications (of type Param_spec) for
  /// use with StandardArguments.  When a LUX function or subroutine gets called
  /// that uses the StandardArguments interface, then the Param_spec_list
  ///
  /// - can specify conditions on the dimensions of the input arguments.
  ///
  /// - can specify how to convert the input arguments (of type #Symbol) to a
  ///   data type more suitable for the back end C++ function.
  ///
  /// - specifies the data type and dimensions of the output arguments and (for
  ///   a LUX function) return #Symbol.  Those dimensions can be specified
  ///   explicitly and/or be taken from a reference parameter.  And that data
  ///   type can be specified explicitly and/or be based on the data type of one
  ///   or more other parameters.
  ///
  /// The default is empty, contains no parameter specifications yet.  Use
  /// new_input_param(), new_output_param(), or new_return_param() to add the
  /// specification of an input, output, or return parameter.  Specify a single
  /// return parameter for a Param_spec_list for use with a LUX function. and no
  /// return parameter for use with a LUX subroutine.
  ///
  /// The methods that specify other attributes of a parameter always apply to
  /// the most recently added parameter.  When you're done with the current
  /// parameter then use new_input_param(), new_output_param(), or
  /// new_return_param() to add the next parameter, and so on until you're done.
  struct Param_spec_list
    : std::vector<Param_spec>
  {
    // const methods

    /// \returns the index of the return parameter specification, or `-1` if
    /// there is none.
    int return_param_index() const;

    /// \returns the count of input and output parameters specifications.
    size_t in_out_param_count() const;

    // fields

    /// Is there a return parameter?  If so then it is stored at the end of
    /// #param_specs.
    bool has_return_param = false;
  };

  // public constructors

  /// Default constructor.
  StandardArguments();

  /// Construct an object representing standard arguments for a LUX subroutine
  /// or function.
  ///
  /// \param[in] narg is the number of arguments to the LUX subroutine or
  /// function.
  ///
  /// \param[in] ps points to the beginning of the array of arguments to the LUX
  /// subroutine or function.
  ///
  /// \param[in] fmt is the format string that describes what arguments are
  /// expected for the LUX subroutine or function.  See below for a detailed
  /// description.
  ///
  /// \param[in,out] ptrs is the address, if it is not null, where a pointer to
  /// the instances of Pointer corresponding to the arguments and the return
  /// value, if any (which comes at the end) is written.
  ///
  /// \param[in,out] infos is the address, if it is not null, where a pointer to
  /// the instances of Loopinfo corresponding to the arguments and the return
  /// value, if any (which comes at the end) is written.
  ///
  /// \par Introduction
  ///
  /// The standard format can look rather daunting.  It has several tasks:
  ///
  /// 1. Specify which of the parameters are input parameters, output
  ///    parameters, or return parameters.
  ///
  /// 2. Specify which of the parameters are optional.
  ///
  /// 3. Specify the data types of the input, output, and return parameter that
  ///    are made available to the back-end.  These data types may depend on the
  ///    type of an earlier parameter.
  ///
  /// 4. Specify the expectations for the dimensions of the input parameters,
  ///    and specify the dimensions of the output parameters and the return
  ///    value.  These may depend on the dimensions of an earlier parameter, but
  ///    may have dimensions added or removed relative to that other parameter.
  ///    The removed dimensions may be explicit ones or may be identified by the
  ///    contents (not the dimensions) of another parameter (the axis
  ///    parameter).
  ///
  /// \par Parameter Types
  ///
  /// The specification parts for different parameters are separated by
  /// semicolons (`;`).  The specification for each parameter begins with a
  /// letter that identifies the parameter type.
  ///
  /// An input parameter (`i`) is an existing named or unnamed LUX symbol whose
  /// contents are going to be processed.
  ///
  /// An output parameter (`o`) is an existing named LUX symbol that will be
  /// reshaped to receive output.  Its previous contents are lost.  It must be a
  /// named symbol, because otherwise the user couldn't access the contents
  /// afterwards.
  ///
  /// A return parameter (`r`) is a new unnamed LUX symbol that gets created to
  /// act as the return value of a LUX function.  There can be at most one
  /// return parameter in a specification.
  ///
  /// \verbatim i;r \endverbatim
  ///
  /// The above specification says that the first parameter is an input
  /// parameter and the second one is a return parameter.
  ///
  /// No dimensions are specified for the input parameter, so it must contain
  /// exactly one value (be either a scalar or an array with one element).  No
  /// dimensions or type are specified for the return parameter, so it gets the
  /// same as its <em>reference parameter</em>, which by default is the first
  /// parameter.  A corresponding call to a fictitous LUX function `foo` might
  /// be `y = foo(3)`.
  ///
  /// \verbatim i;i;o;o \endverbatim
  ///
  /// The above specification says that the first two parameters are
  /// single-element input parameters of arbitrary type, and the next two are
  /// output parameters with the same data type and dimensions as the first
  /// parameter.  An example call is `foo,3,5,x,y`.
  ///
  /// \par Reference Parameter
  ///
  /// Some missing information (like a data type or a dimension) may be copied
  /// from the reference parameter.  The reference parameter is indicated by a
  /// number or a hyphen (`-`) between square brackets (`[]`) just after the
  /// parameter data type (described later), which itself follows the parameter
  /// type.  A number indicates a particular parameter (0 indicates the first
  /// one), and a hyphen indicates the parameter preceding the current one.  If
  /// no reference parameter is explicitly given, then the first parameter is
  /// the reference parameter.  The reference parameter must have a smaller
  /// index than the current parameter.
  ///
  /// \verbatim i;i;o[1] \endverbatim
  ///
  /// says that the output parameter's reference parameter is the one with index
  /// 1 (i.e., the 2nd parameter).  The output parameter gets the same type and
  /// dimensions as the second parameter.
  ///
  /// \verbatim i;i;o[-] \endverbatim
  ///
  /// has the same effect as the previous specification.  Now the output
  /// parameter's reference parameter is the parameter preceding the output
  /// parameter, which is the 2nd parameter as before.
  ///
  /// \par Optional Parameters
  ///
  /// An input or output parameter specification that has a question mark (`?`)
  /// at its very end means that that parameter is optional.  A return parameter
  /// cannot be optional.
  ///
  /// \verbatim i;i?;r \endverbatim
  ///
  /// says that the second parameter is optional, so does not have to be
  /// specified.  Example calls are `y = foo(3,5)` but also `y = foo(3)`.
  ///
  /// \par Parameter Data Types
  ///
  /// Parameter data types may be specified for any parameter, immediately after
  /// the parameter type.  Explicit parameter data types are indicated by one of
  /// the letters `B W L Q F D S` corresponding to `int8` through `int64`,
  /// `float`, `double`, and `string`, respectively.  In addition, the special
  /// value `Z` indicates that the data type should be copied from the reference
  /// parameter.  This behavior is standard for output and return parameters but
  /// not for input parameters.
  ///
  /// An output or return parameter for which an explicit data type is specified
  /// gets set to that data type.
  ///
  /// An explicit data type for an input parameter does't say what data type the
  /// argument must have, but defines what data type is made available to the
  /// back-end.  If an input argument's data type is equal to the corresponding
  /// explicit input parameter's data type, then a pointer to that argument's
  /// data is made available.  If an input argument's data type differs from the
  /// corresponding explicit input parameter's data type, then a copy of the
  /// argument is created and converted to the explicit data type, and a pointer
  /// to that copy's data is made available instead.
  ///
  /// \verbatim iF;rL \endverbatim
  ///
  /// says that the first parameter is an input parameter, the corresponding
  /// argument must have a single-element, and that a `float` copy is made
  /// available for processing, if the argument isn't `float` already.  Also, a
  /// single-element `int32` return value is created and made available for
  /// receiving output.
  ///
  /// \verbatim i;iZ \endverbatim
  ///
  /// says that the first parameter is an input parameter and that the
  /// corresponding argument must have a single data value.  The second
  /// parameter is an input parameter, too, and a version of the corresponding
  /// argument that has the same data type as the reference parameter (which by
  /// default is the first parameter) is made available to the back-end.
  ///
  /// If the explicit data type is numeric (i.e., not `S`) and is preceded by a
  /// greater-than sign (`>`), then the data type is a minimum.  If the data
  /// type that would have applied if no explicit data type were given is less
  /// than the minimum, then that minimum is used instead.
  ///
  /// \verbatim i>L;r \endverbatim
  ///
  /// says that if the data type of the first argument is less than `int32`,
  /// then an `int32` copy is made available instead.  No explicit data type is
  /// given for the return parameter, so it gets the same as its reference
  /// parameter, which by default is the first parameter.  So the data type of
  /// the return parameter is equal to that of the first parameter, which is at
  /// least `int32`.
  ///
  /// \verbatim i>L;r>F \endverbatim
  ///
  /// is like the previous case, but now the return parameter has a minimum data
  /// type of `float`.  If the input parameter type is at least `float`, then
  /// the return parameter gets the same type as the input parameter.  If the
  /// input parameter type is less than `float`, then an `int32` version of the
  /// input is made available, and the return parameter is of type `float`.
  ///
  /// Similarly, a less-than sign (`<`) introduces a maximum data type.
  ///
  /// A tilde (`~`) is like greater-than (`>`) but with the additional condition
  /// that the data type must be integer.  If the data type would be `float`
  /// then instead it becomes `int32`, and if the data type would be `double`
  /// then it instead becomes `int64`.
  ///
  /// If the data type specifications for more than one numerical parameter are
  /// followed by a caret (`^`), then all of those parameters get the same data
  /// type, which is equal to the greatest data type among them that would have
  /// applied if no carets had been specified.
  ///
  /// \verbatim i>L^;i>L^;iW;r^ \endverbatim
  ///
  /// says that the first two input parameters and the return value get the same
  /// data type applied, which is the greatest one among them that would have
  /// applied if there were no carets.  So, if the first parameter is an `int64`
  /// and the second one is a `float`, then the parameters made available to the
  /// back-end have data types `float`, `float`, `int16`, and `float`,
  /// respectively.
  ///
  /// \par Parameter Dimensions
  ///
  /// Expectations for the dimensions of input parameters can be specified, and
  /// also how to determine the dimensions of output and return parameters.
  ///
  /// At its simplest, the dimensions are specified in a comma-separated list
  /// after the data type.
  ///
  /// \verbatim iF3,6;rD3 \endverbatim
  ///
  /// says that the input parameter must be an array of 3 by 6 elements, of
  /// which a `float` version is made available to the back-end, and that the
  /// return value is a one-dimensional `double` array of 3 elements.
  ///
  /// A greater-than sign (`>`) before a dimension number means that the
  /// dimension must be at least as great as the number.
  ///
  /// \verbatim i>7 \endverbatim
  ///
  /// says that the first (and only) dimension must be at least 7.
  ///
  /// For input parameters, a colon (`:`) means to accept the current dimension.
  ///
  /// \verbatim i:,4,: \endverbatim
  ///
  /// says that the input parameter must have 3 dimensions of which the 2nd one
  /// is equal to 4.
  ///
  /// An at sign (`@`) at the beginning of the dimensions specification means
  /// that dimensions equal to 1 are ignored, as far as possible.  If omitting
  /// all dimensions equal to 1 would mean that there are no dimensions left,
  /// then a single dimension equal to 1 is retained.
  ///
  /// \verbatim i@:,: \endverbatim
  ///
  /// says that the input parameter must have two dimensions after dimensions
  /// equal to 1 are omitted.
  ///
  /// Dimensions for output parameters and the return value can be copied from
  /// the reference parameter.  An equals sign (`=`) means that the
  /// corresponding dimension of the reference parameter is copied.  If a number
  /// follows the equals sign immediately, then it says what the dimension of
  /// the reference parameter must be.  A hyphen (`-`) means that the
  /// corresponding dimension of the reference parameter is skipped.  A plus
  /// sign (`+`) followed by a number means that, relative to the reference
  /// parameter, a dimension equal to that number is inserted.
  ///
  /// \verbatim i7,3,2;o=,= \endverbatim
  ///
  /// says that the output parameter is an array of 7 by 3 elements.
  ///
  /// \verbatim i7,3,2;o=,-,= \endverbatim
  ///
  /// says that the output parameter is an array of 7 by 2 elements, because the
  /// 3 was skipped.
  ///
  /// \verbatim i7,3,2;o=,+5,= \endverbatim
  ///
  /// says that the output parameter is an array of 7 by 5 by 3 elements.
  ///
  /// \verbatim i7,3,2;o=,5,= \endverbatim
  ///
  /// says that the output parameter is an array of 7 by 5 by 2 elements.
  ///
  /// \verbatim i7,3,2;o=2 \endverbatim
  ///
  /// produces an error because the output parameter's specification says that
  /// the first dimension of its reference parameter (which is the first
  /// parameter) should be equal to 2, but the first parameter's specification
  /// says that its first dimension should be equal to 7, and those cannot both
  /// be true.
  ///
  /// An asterisk (`*`) at the end of the dimensions list for an input parameter
  /// says that the remaining dimensions are unrestricted.
  ///
  /// \verbatim iF3*;i* \endverbatim
  ///
  /// says that the first dimension of the first input parameter must be equal
  /// to 3 but that any following dimensions are unrestricted, so, for example,
  /// a one-dimensional array of 3 elements is accepted, and also an array of 3
  /// by 5 elements, or 3 by 1 by 17 elements.  The second input parameter has
  /// no restrictions on its dimensions, so a scalar is acceptable, and also any
  /// array.
  ///
  /// An ampersand (`&`) at the end of the dimensions list for any parameter
  /// says that the remaining dimensions must be equal to the dimensions of the
  /// reference parameter.
  ///
  /// \verbatim i*;rD6& \endverbatim
  ///
  /// says that the input parameter may have any data type and dimensions and
  /// that the return value is a `double` array with dimension 6 followed by the
  /// dimensions of the reference parameter, which by default is the first
  /// parameter.  So, if the input argument is an array of 3 by 2 elements, then
  /// the return value is an array of 6 by 3 by 2 elements.
  ///
  /// A hash sign (`#`) at the end of the dimensions specification means that
  /// the element count of an input parameter must either be equal to 1 or else
  /// to the element count of the reference parameter.
  ///
  /// \verbatim i3,3,3;i#;r& \endverbatim
  ///
  /// says that the second input parameter must either have exactly one element
  /// or else must have the same number of elements as the reference parameter
  /// (the first parameter), i.e., 27.  The dimensions do not need to be the
  /// same, as long as the element count matches, so it is OK if the second
  /// input parameter has a single dimension equal to 27, or is a 9 by 3 array,
  /// or a 3 by 3 by 1 by 3 array.
  ///
  /// \par Axis Parameters
  ///
  /// Some LUX functions and subroutines specify an <em>axis parameter</em>,
  /// which says along which dimensions of the main data to apply the operation.
  /// If the operation produces one value (e.g., the minimum value) when running
  /// along the indicated axes, then the result should have the same dimensions
  /// as the main data except that the dimensions specified in the axis
  /// parameter should be omitted.  This is achieved by specifying the axis
  /// parameter's index between curly braces (`{}`) just before the
  /// specification of the dimensions, and just after the specification of the
  /// reference parameter, if any.
  ///
  /// \verbatim iD*;iL*;rD{1} \endverbatim
  ///
  /// says that parameter 1 (i.e., the 2nd parameter) is the axis parameter for
  /// the return value.  If the function is called like `y = foo(x,[1,2])` and
  /// `x` is an array of 4 by 100 by 200 by 3 elements, then `y` is an array of
  /// 4 by 3 elements.
  ///
  /// \par Complete Syntax
  ///
  /// All in all, the standard format is schematically as follows, where
  /// something between quotes (<tt>''</tt>) stands for that literal character,
  /// something between non-literal square brackets (`[]`) is optional,
  /// something between non-literal curly braces (`{}`) is a group of
  /// alternatives, a non-literal pipe symbol (`|`) separates alternatives, and
  /// a non-literal asterisk (`*`) indicates repetition zero or more times:
  ///
  /// \code
  ///   <format> = <param-spec>[;<param-spec>]*
  ///   <param-spec> = {'i'|'o'|'r'}[<type-spec>][<dims-spec>]['?']
  ///   <type-spec> = {{['>'|'<'|'~']{'B'|'W'|'L'|'Q'|'F'|'D'}}|'S'}['^']
  ///   <dims-spec> = ['@']['['<ref-par>']']['{'<axis-par>'}']
  ///                 <dim-spec>[,<dim-spec>]*['*'|'&'|'#']
  ///   <dim-spec> = [{['+'|'-'|'=']NUMBER|'-'|'='|':'}]*
  /// \endcode
  ///
  /// In words,
  /// - the format consists of one or more parameter specifications separated by
  ///   semicolons `;`.
  /// - each parameter specification begins with an `i`, `o`, or `r`, followed
  ///   by a type specification and a dimensions specification, optionally
  ///   followed by a question mark `?`.
  /// - a type specification consists of an `S`, or else of an optional
  ///   greater-than sign `>` or less-than sign `<` or tilde `~` followed by one
  ///   of `B`, `W`, `L`, `Q`, `F`, or `D`.  Optionally, a `^` follows.
  /// - a dimensions specification consists of an optional at sign `@`, an a
  ///   optional reference parameter number between square brackets `[]`,
  ///   followed by an optional axis parameter number between curly braces `{}`,
  ///   followed by one or more dimension specifications separated by commas
  ///   `,`, optionally followed by an asterisk `*` or ampersand `&` or hash
  ///   symbol `#`.
  /// - a dimension specification consists of a hyphen `-`, an equals sign `=`,
  ///   a colon `:`, or a number preceded by a plus sign `+`, a hyphen `-`, or
  ///   an equals sign `=`; followed by any number of additional instances of
  ///   the preceding.
  ///
  /// Some of the characteristics of a parameter may depend on those of a
  /// reference parameter.  That reference parameter is the very first parameter
  /// (parameter \c ps[0]) unless a different reference parameters is specified
  /// at the beginning of the dimension specification.
  ///
  /// For the parameter specification \c param-spec:
  /// - `i` = input parameter.
  /// - `o` = output parameter.  An error is declared if this is not a named
  ///   parameter.
  /// - `r` = return value.  For functions, there must be exactly one of these
  ///   in the parameters specification.  Subroutines must not have one of
  ///   these.
  /// - `?` = optional parameter.
  ///
  /// For the type specification \c type-spec:
  /// - `>` = the type should be at least equal to the indicated type.
  /// - `<` = the type should be at most equal to the indicated type.
  /// - '~' = the type should be at least equal to the indicated type but must
  ///   be integer: float becomes `int32_t` and double becomes `int64_t`.
  /// - `B` = #LUX_INT8
  /// - `W` = #LUX_INT16
  /// - `L` = #LUX_INT32
  /// - `Q` = #LUX_INT64
  /// - `F` = #LUX_FLOAT
  /// - `D` = #LUX_DOUBLE
  /// - `S` = #LUX_STRING
  /// - `^` = all numerical parameters marked like this get the same data type,
  ///   which is the greatest numerical data type among them that would have
  ///   applied if no `^` had been specified.
  ///
  /// For input parameters, a copy is created with the indicated
  /// (minimum/maximum) type if the input parameter does not meet the condition,
  /// and further processing is based on that copy.  For output parameters, an
  /// array is created with the indicate type, except that
  /// - if `>` is specified and the reference parameter has a greater type, then
  ///   that type is used.
  /// - if `~` is specified and the reference parameter has a greater type, then
  ///   that type is used, except that `float` becomes `int32_t` and `double`
  ///   becomes `int64_t`.
  /// - if `<` is specified and the reference parameter has a lesser type, then
  ///   that type is used.
  ///
  /// If no explicit type is
  /// specified for an output parameter, then it gets the type of the reference
  /// parameter.
  ///
  /// For the reference parameter \c ref-par:
  /// - If absent, then 0 is taken for it (i.e., the first parameter).
  /// - If a number, then the indicated parameter is taken for it.
  /// - If `-`, then the previous parameter is taken for it.
  ///
  /// For the axis parameter \c axis-par:
  /// - The specified parameter is expected to indicate one or more unique axes
  ///   to remove from the current parameter, which must be of type `r` or `o`.
  /// - If a number, then the indicated parameter is taken for it.
  /// - If `-`, then the previous parameter is taken for it.
  ///
  /// An at sign `@` at the beginning of the list of dimension specifications
  /// indicates that dimensions equal to 1 are omitted.  For an input parameter
  /// such dimensions are omitted before considering the dimension
  /// specifications.  For an output or a return parameter such dimensions are
  /// omitted just before adjusting or creating the symbol.
  ///
  /// For the dimension specification \c dim-spec:
  /// - NUMBER = the current dimension has the specified size.  For input
  ///   parameters, an error is declared if the dimension does not have the
  ///   specified size.
  /// - `>`NUMBER = the current dimension has at least the specified size.  For
  ///   input parameters, an error is declared if the dimension does not have at
  ///   least the specified size.
  /// - `+`NUMBER = for output or return parameters, a new dimension with the
  ///   specified size is inserted here.
  /// - `=` = for output or return parameters, the current dimension is taken
  ///   from the reference parameter.
  /// - `=`NUMBER = for output or return parameters, the current dimension is
  ///   taken from the reference parameter, and must be equal to the specified
  ///   number.  An error is declared if the reference parameter's dimension
  ///   does not have the indicated size
  /// - `-` = the corresponding dimension from the reference parameter is
  ///   skipped.
  /// - `:` = for input parameters, accept the current dimension.
  /// - `&` = the remaining dimensions must be equal to those of the reference
  ///   parameter.
  /// - `#` = the element count must be equal to 1 or to that of the reference
  ///   parameter.
  /// - `*` = the remaining dimensions are unrestricted.
  ///
  /// Both a `+`NUMBER and a `-`NUMBER may be given in the same dimension
  /// specification \c dim_spec.
  StandardArguments(ArgumentCount narg, Symbol ps[], const std::string& fmt,
                    Pointer** ptrs = nullptr, LoopInfo** infos = nullptr);

  StandardArguments(ArgumentCount narg, Symbol ps[], const Param_spec_list& psl,
                    Pointer** ptrs = nullptr, LoopInfo** infos = nullptr);

  // const methods

  /// \returns the count of LUX symbols for which this instance contains
  /// information.
  size_t size() const;

  /// Returns the return symbol, the LUX symbol intended to represent the result
  /// of the call of the LUX subroutine or function.
  Symbol result() const;

  // non-const methods

  /// Configure symbols based on the passed symbols and a StandardArguments
  /// format string.  The configured symbols can be used to handle a call of a
  /// LUX function or subroutine.
  ///
  /// \param[in] narg is the number of symbols passed in \a ps.
  ///
  /// \param[in] ps points at the symbols passed into this call.
  ///
  /// \param[in] fmt is the StandardArguments format string that says what
  /// input, output, and return symbols the LUX function or subroutine expects.
  ///
  /// \param[in,out] ptrs points (if it is not null) at the address where to
  /// store a pointer to one Pointer per configured symbol, that points at the
  /// first data element of each symbol.
  ///
  /// \param[in,out] infos points (if it is not null) at the address where to
  /// store a pointer to one LoopInfo per configured symbol, that holds
  /// information about the coordinates, dimensional structure, and way to
  /// traverse the elements.
  ///
  /// \returns the configured return symbol.  For a LUX subroutine this is
  /// LUX_ONE.
  Symbol set(ArgumentCount narg, Symbol ps[], const std::string& fmt,
             Pointer** ptrs = nullptr, LoopInfo** infos = nullptr);

  /// Configure symbols based on the passed symbols and a parameter
  /// specification list.  The configured symbols can be used to handle a call
  /// of a LUX function or subroutine.
  ///
  /// \param[in] narg is the number of symbols passed in \a ps.
  ///
  /// \param[in] ps points at the symbols passed into this call.
  ///
  /// \param[in] psl is the parameter specification list that says what input,
  /// output, and return symbols the LUX function or subroutine expects.
  ///
  /// \param[in,out] ptrs points (if it is not null) at the address where to
  /// store a pointer to one Pointer per configured symbol, that points at the
  /// first data element of each symbol.
  ///
  /// \param[in,out] infos points (if it is not null) at the address where to
  /// store a pointer to one LoopInfo per configured symbol, that holds
  /// information about the coordinates, dimensional structure, and way to
  /// traverse the elements.
  ///
  /// \returns the configured return symbol.  For a LUX subroutine this is
  /// LUX_ONE.
  Symbol set(ArgumentCount narg, Symbol ps[], const Param_spec_list& fmt,
             Pointer** ptrs = nullptr, LoopInfo** infos = nullptr);

  /// Returns the Pointer for the argument with the given index.
  ///
  /// \param[in] index is the 0-based index of the argument for which to return
  /// the Pointer.  It must be less than size().
  ///
  /// \returns a reference to the Pointer, or a std::out_of_range exception if
  /// the \a index is out of range.
  ///
  /// \todo: can we make this const?
  Pointer& datapointer(size_t index);

  /// Returns the LoopInfo for the argument with the given index.
  ///
  /// \param[in] index is the 0-based index of the argument for which to return
  /// the LoopInfo.  It must be less than size().
  ///
  /// \returns a reference to the LoopInfo, or a std::out_of_range exception if
  /// the \a index is out of range.
  ///
  /// \todo: can we make this const?
  LoopInfo& datainfo(size_t index);

  /// Advances the specified argument to its next element according to the
  /// configured path through the elements.
  ///
  /// \param[in] index is the 0-based index of the argument for which to advance
  /// to the next element.  It must be less than size().
  int32_t advanceLoop(size_t index);  //!< \todo: make const?

private:
  /// A collection with a Pointer for each argument.
  std::vector<Pointer> m_pointers;

  /// A collection with a LoopInfo for each argument.
  std::vector<LoopInfo> m_loop_infos;

  /// The return symbol -- the symbol representing the result of the
  /// call of the LUX subroutine or function.
  int32_t m_return_symbol;
};

/// An iterator-like object for iterating over data elements using
/// StandardArguments.
///
/// \note This iterator-like object does not work exactly like standard
/// iterators do.
///
/// \tparam T is the data type of the data elements.
template<typename T>
class StandardLoopIterator
{
public:
  // constructors, destructor

  /// Constructs an instance for one standard argument.
  ///
  /// \param[in] as represents the relevant StandardArguments.
  ///
  /// \param[in] index is the 0-based index of the standard argument.  It must
  /// be nonnegative and less than sa.size().  Throws a std::domain_error if the
  /// index is out of range.
  StandardLoopIterator(StandardArguments& sa, int index)
    : m_status()
  {
    if (index < 0 || index >= sa.size()) {
      throw std::domain_error("StandardArguments index out of range");
    }
    m_data = reinterpret_cast<T*>(sa.datapointer(index).v);
    m_loop_info = sa.datainfo(index);
  }

  /// Copy constructor.
  ///
  /// \param other is the instance whose contents get copied.
  StandardLoopIterator(const StandardLoopIterator& other)
    : m_data(other.m_data), m_loop_info(other.m_loop_info),
      m_status(other.m_status)
  { }

  ~StandardLoopIterator() = default;

  // operators

  /// Assignment operator.
  ///
  /// \param other is the instance whose contents get assigned to the current
  /// instance.
  ///
  /// \returns a reference to the current instance.
  StandardLoopIterator&
  operator=(const StandardLoopIterator& other)
  {
    m_data = other.m_data;
    m_loop_info = other.m_loop_info;
    m_status = other.m_status;
    return *this;
  }

  /// Prefix increment.
  StandardLoopIterator&
  operator++()
  {
    m_status = m_loop_info.advanceLoop(&m_data);
    return *this;
  }

  /// postfix increment.
  StandardLoopIterator
  operator++(int)
  {
    auto old = *this;
    m_status = m_loop_info.advanceLoop(&m_data);
    return old;
  }

  // const methods

  StandardLoopIterator
  begin() const
  {
    auto result = *this;
    result.set(0);
    return result;
  }

  StandardLoopIterator
  end() const
  {
    auto result = *this;
    result.set(result.nelem);
    return result;
  }

  /// Returns a reference to the data value currently pointed at.
  ///
  /// \returns the value.
  ///
  /// \todo what if we're past the end?
  T&
  operator*() const
  {
    return *m_data;
  }

  /// Returns a pointer to the address of the data value currently pointed at.
  ///
  /// \returns the pointer.
  ///
  /// \todo what if we're past the end?
  T*
  pointer() const
  {
    return m_data;
  }

  /// Returns the count of data elements.
  ///
  /// \returns the count.
  size_t
  element_count() const
  {
    return m_loop_info.nelem;
  }

  /// Returns the status of the iterator, expressed as the 1-based index of the
  /// highest rearranged dimension whose coordinate was incremented just before
  /// the current element.  The initial status, when the iterator points at the
  /// very first element, is 0.
  ///
  /// For example, if the rearranged dimensions are 4 and 3, then the status is
  /// as follows for each of the elements:
  ///
  ///      0 0 0 0
  ///      1 0 0 0
  ///      1 0 0 0
  ///     (2)
  ///
  /// so if you need to detect when the first rearranged dimension has been
  /// completed (and the 2nd coordinate is the highest one to have been
  /// incremented) then look for status() == 1.  And if you need to detect when
  /// the second rearranged dimension has been completed, then look for status()
  /// == 2.
  ///
  /// If you're only interested in detecting when you've passed beyond the last
  /// data element, then you can look at done() instead.
  ///
  /// \returns the status.
  int32_t
  status() const
  {
    return m_status;
  }

  /// Did the iterator just pass beyond the last element?
  ///
  /// \returns `true` if the iterator just passed beyond the last element,
  /// `false` otherwise.
  bool
  done() const
  {
    return m_status == m_loop_info.rndim;
  }

private:
  T* m_data;                    //!< points at the current data element
  LoopInfo m_loop_info;         //!< the dimensional information
  int32_t m_status;             //!< the status
};

#endif
