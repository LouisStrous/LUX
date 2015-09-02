/* This is file TestArray.hh.

   Copyright 2015 Louis Strous

   This file is part of LUX.

   LUX is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   LUX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with LUX.  If not, see <http://www.gnu.org/licenses/>.
*/

/// \file
/// A file that provides template struct TestArray.

#include <cmath>                // for fabs
#include <cstdarg>              // for va_list &c
#include <cstring>              // for memset, memcpy

/// A template struct that encapsulates a simple array of type T
/// holding N elements.
template<typename T, unsigned int N>
struct TestArray {
  /// The value type of the TestArray
  typedef T value_type;

  /// The data values
  value_type data[N];

  /// A simple constructor that provides an array with all elements
  /// equal to the default (zero) element.
  TestArray()
  {
    for (int i = 0; i < sizeof(data)/sizeof(*data); ++i)
      data[i] = value_type();
  }

  /// A constructor that copies values from a regular array.
  ///
  /// \param[in] values is a non-NULL pointer to an array of at least
  /// N values that are copied bitwise.
  TestArray(value_type const* values)
  {
    assert(values);
    memcpy(data, values, sizeof(data));
  }

  /// A constructor that accepts multiple individual values.  The
  /// total number of specified values must be equal to at least N;
  /// only the first N values are used.
  ///
  /// \param[in] value1 is the first value for the newly constructed
  /// TestArray.
  ///
  /// \param[in] ... are further values as needed to make up N values
  /// in total.
  ///
  /// Because of the use of variadic arguments, the second and further
  /// values in any call of this constructor must be cast to the
  /// correct type, otherwise the behavior may be undefined.  It is
  /// not wise to rely on automatic promotion of value types here.
  TestArray(value_type value1, ...)
  {
    data[0] = value1;

    va_list ap;

    va_start(ap, value1);
    for (int i = 1; i < sizeof(data)/sizeof(*data); ++i) {
      value_type value = va_arg(ap, value_type);
      data[i] = value;
    }
    va_end(ap);
  }

  /// Does the current TestArray contain the same values as the other
  /// TestArray, to within a small margin of error?
  ///
  /// \param[in] other is the other TestArray to compare with.
  ///
  /// \return true if the TestArray are equivalent, false otherwise.
  bool operator==(TestArray const& other)
  {
    for (int i = 0; i < sizeof(data)/sizeof(*data); ++i) {
      if (fabs(data[i] - other.data[i]) > 1e-10)
        return false;
    }
    return true;
  }

  /// Is the current TestArray not equivalent to the other TestArray?
  /// Opposite of TestArray::operator==.
  ///
  /// \param[in] other is the other TestArray to compare with.
  ///
  /// \return true if the two TestArray are not equivalent, false
  /// otherwise.
  bool operator!=(TestArray const &b)
  {
    return !operator==(b);
  }

  TestArray operator*(value_type value)
  {
    TestArray result(*this);
    for (int i = 0; i < sizeof(data)/sizeof(*data); ++i) {
      result.data[i] *= value;
    }
    return result;
  }

  friend TestArray operator*(value_type value, TestArray const& array)
  {
    TestArray result;
    for (int i = 0; i < sizeof(array.data)/sizeof(*array.data); ++i) {
      result.data[i] = array.data[i] * value;
    }
    return result;
  }
};
