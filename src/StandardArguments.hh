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

#include <string>
#include <vector>

int32_t standard_args(int32_t, int32_t [], char const *, Pointer **,
                      LoopInfo **, size_t* = 0);

/// A class to facilitate applying functions to a collection of LUX
/// symbols.
class StandardArguments {
public:
  StandardArguments(int32_t narg, int32_t ps[], const std::string& fmt);
  Pointer& datapointer(size_t index);
  LoopInfo& datainfo(size_t index);
  int32_t result() const;
  int32_t advanceLoop(size_t index);
private:
  /// A collection with a Pointer for each argument.
  std::vector<Pointer> m_pointers;

  /// A collection with a LoopInfo for each argument.
  std::vector<LoopInfo> m_loop_infos;

  /// The return symbol -- the symbol representing the result of the
  /// call of the LUX subroutine or function.
  int32_t m_return_symbol;
};

/// A class to prepare for looping through input and output variables
/// based on a standard arguments specification.  Does the same as
/// standard_args() but frees dynamically allocated memory
/// automatically when the object goes out of scope.
///
/// The intention is to replace all standard_args() calls by
/// corresponding uses of class StandardArguments.  That requires more
/// work, so an interim solution is to use the current class.  Replace
/// every call
///
///     {
///       iq = standard_args(narg, ps, fmt, ptrs, infos);
///       ...
///       free(ptrs);
///       free(infos);
///     }
///
/// with
///
///     {
///       StandardArguments_RAII sa(narg, ps, fmt, ptrs, infos);
///       iq = sa.result();
///       ...
///     }
///
/// The problem with the old code is that the free() calls are often
/// forgotten.  With the new code, when the StandardArguments_RAII
/// object goes out of scope, then the dynamically allocated memory
/// associated with `ptrs` and `infos` is released automatically.
class StandardArguments_RAII {
public:
  StandardArguments_RAII();
  StandardArguments_RAII(int32_t narg, int32_t ps[], const std::string& fmt,
                         Pointer** ptrs, LoopInfo** infos);
  ~StandardArguments_RAII();
  int32_t set(int32_t narg, int32_t ps[], const std::string& fmt,
              Pointer** ptrs, LoopInfo** infos);
  int32_t result() const;
private:
  /// Points at a sequence of instances of Pointer, one for each argument.
  Pointer* m_pointers;

  /// Points at a sequence of instances of LoopInfo, one for each
  /// argument.
  LoopInfo* m_loopInfos;

  /// The LUX symbol to return.
  int32_t m_result_symbol;
};

#endif
