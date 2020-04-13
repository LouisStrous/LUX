/* This is file NumericDataDescriptor.hh.

Copyright 2019 Louis Strous

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

#ifndef NUMERICDATADESCRIPTOR_HH_
#define NUMERICDATADESCRIPTOR_HH_

#include <vector>
#include "types.hh"

/// A class that describes the dimensional structure and location of
/// numeric data.
class NumericDataDescriptor
{
public:
  // typedef

  // constructors

  /// Default constructor.  Leaves is_valid() equal to `false`.
  NumericDataDescriptor();

  /// Constructor that fills the members based on the contents of the
  /// symbol.
  ///
  /// \param[in] symbol is the number of the symbol.
  explicit NumericDataDescriptor(SymbolProxy_tp symbol);

  // accessors

  /// Returns a Pointer to the data.
  Pointer data() const;

  /// Returns the size of a dimension.
  ///
  /// \param[in] index is the zero-based index of the dimension.
  ///
  /// \returns the size of the requested dimension, or 0 if the
  /// dimension does not exist.
  DimensionSize_tp dimension(int index) const;

  /// Returns the number of dimensions.
  size_t dimensions_count() const;

  /// Returns the collection of dimensions.
  std::vector<DimensionSize_tp> dimensions() const;

  /// Returns the number of elements, obtained by multiplying all of
  /// the dimensions together, or 0 if the instance wasn't
  /// successfully set based on a symbol yet.
  size_t element_count() const;

  /// Returns `true` if the descriptor is valid, which means it was
  /// successfully set based on a symbol.
  bool is_valid() const;

  // modifiers

  /// Omits dimensions equal to 1.  If that removes all dimensions,
  /// then retain a single dimension equal to 1.
  void omit_dimensions_equal_to_one();

  /// Resets the current instance so it is as if it were just
  /// constructed and wasn't successfully set based on a symbol yet.
  void reset();

  /// (Re)sets the current instance based on the specified symbol.
  ///
  /// \param[in] symbols is the number of the symbol.
  ///
  /// \returns `true` for success, `false` for failure.
  bool set_from(SymbolProxy_tp symbol);

private:

  /// A vector of dimensions.
  std::vector<DimensionSize_tp> m_dimensions;

  /// A Pointer to the beginning of the data.
  Pointer m_data;
};

#endif
