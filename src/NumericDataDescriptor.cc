/* This is file NumericDataDescriptor.cc.

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

#include <algorithm>            // for std::remove
#include <cstring>              // for std::memset
#include <functional>           // for std::multiplies
#include <numeric>              // for std::accumulate
#include <vector>

#include "NumericDataDescriptor.hh"

#include "action.hh"            // for sym[]

NumericDataDescriptor::NumericDataDescriptor()
{
  reset();
}

NumericDataDescriptor::NumericDataDescriptor(SymbolProxy_tp symbol)
{
  set_from(symbol);
}

DimensionSize_tp
NumericDataDescriptor::dimension(int index) const
{
  if (index >= 0 && index < m_dimensions.size()) {
    return m_dimensions[index];
  } else {
    return 0;
  }
}

Pointer
NumericDataDescriptor::data() const
{
  return m_data;
}

bool
NumericDataDescriptor::set_from(SymbolProxy_tp symbol)
{
  switch (symbol_class(symbol)) {
  case LUX_SCALAR:
    m_dimensions.push_back(1);
    m_data.b = &scalar_value(symbol).b;
    break;
  case LUX_ARRAY:
    m_dimensions.resize(array_num_dims(symbol));
    std::copy(array_dims(symbol),
              array_dims(symbol) + array_num_dims(symbol),
              m_dimensions.begin());
    m_data.v = array_data(symbol);
    break;
  default:
    reset();
    return false;
  }

  return true;
}

void
NumericDataDescriptor::reset()
{
  std::memset(&m_data, 0, sizeof(m_data));
  m_dimensions.clear();
}

size_t
NumericDataDescriptor::dimensions_count() const
{
  return m_dimensions.size();
}

size_t
NumericDataDescriptor::element_count() const
{
  return m_dimensions.size()?
    std::accumulate(m_dimensions.cbegin(), m_dimensions.cend(),
                    1, std::multiplies<>())
    : 0;
}

bool
NumericDataDescriptor::is_valid() const
{
  return m_dimensions.size() > 0;
}

void
NumericDataDescriptor::omit_dimensions_equal_to_one()
{
  if (m_dimensions.size()) {
    m_dimensions.erase(
                       std::remove(m_dimensions.begin(),
                                   m_dimensions.end(), 1),
                       m_dimensions.end());
    if (!m_dimensions.size()) {
      m_dimensions.push_back(1);
    }
  }
}
