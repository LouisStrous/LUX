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

class StandardArguments {
public:
  StandardArguments();
  ~StandardArguments();
  int32_t set(int32_t narg, int32_t ps[], char const* fmt, Pointer** ptrs,
              loopInfo** infos);
  Pointer* pointer();
  loopInfo* loop_info();
private:
  /// Points to the beginning of a list of Pointer for all arguments.
  Pointer* m_pointer;

  /// Points to the beginning of a list of loopInfo for all arguments.
  loopInfo* m_loop_info;
};

#endif
