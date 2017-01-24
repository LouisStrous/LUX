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

#include "action.hh"

StandardArguments::StandardArguments()
  : m_pointer(0), m_loop_info(0)
{
}

int32_t
StandardArguments::set(int32_t narg, int32_t ps[], char const* fmt,
                       Pointer** ptrs, loopInfo** infos)
{
  int32_t standard_args(int32_t narg, int32_t ps[], char const* fmt,
                        Pointer** ptrs, loopInfo** infos);

  // when all other calls to standard_args have been refactored into
  // calls to this "set" method, then we can make m_pointer and
  // m_loop_info dynamically allocated variables rather than malloc'd
  // variables.

  int32_t result = standard_args(narg, ps, fmt, &m_pointer, &m_loop_info);
  if (ptrs)
    *ptrs = m_pointer;
  if (infos)
    *infos = m_loop_info;
  return result;
}

StandardArguments::~StandardArguments()
{
  free(m_pointer);
  free(m_loop_info);
}

Pointer*
StandardArguments::pointer()
{
  return m_pointer;
}

loopInfo*
StandardArguments::loop_info()
{
  return m_loop_info;
}
