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
  int32_t advanceLoop(LoopInfo*, Pointer*);
  return advanceLoop(&m_loop_infos[index], &m_pointers[index]);
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
