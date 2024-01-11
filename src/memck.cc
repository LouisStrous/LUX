/* This is file memck.cc.

Copyright 2013-2014 Louis Strous

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
// LUX routines about memory use statistics.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include "action.hh"

//-------------------------------------------------------------------
int32_t lux_memstat(ArgumentCount narg, Symbol ps[])
// returns some info on memory usage
{
 char                *p;
 extern char        *firstbreak;

 p = (char*) sbrk(0);
 printf("Break at %p, allocated %u bytes\n",
  p, p - firstbreak);
 return 1;
}
//-------------------------------------------------------------------
int32_t lux_memory(ArgumentCount narg, Symbol ps[])
     // returns size of allocated memory
{
  int32_t        result;
  char        *p;
  extern char        *firstbreak;

  p = (char*) sbrk(0);
  result = scalar_scratch(LUX_INT32);
  sym[result].spec.scalar.i32 = p - firstbreak;
  return result;
}
//-------------------------------------------------------------------
