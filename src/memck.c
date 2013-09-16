/* This is file memck.c.

Copyright 2013 Louis Strous

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
/* LUX routines about memory use statistics. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <stdio.h>
#include "action.h"

#ifdef SBRK_H
#include SBRK_H
#else
void	*sbrk(Int);
#endif

/*-------------------------------------------------------------------*/
Int lux_memstat(Int narg, Int ps[])
/* returns some info on memory usage */
{
 char		*p;
 extern char	*firstbreak;

 p = sbrk(0);
 printf("Break at %p, allocated %u bytes\n",
  p, p - firstbreak);
 return 1;
}
/*-------------------------------------------------------------------*/
Int lux_memory(Int narg, Int ps[])
     /* returns size of allocated memory */
{
  Int	result;
  char	*p;
  extern char	*firstbreak;

  p = sbrk(0);
  result = scalar_scratch(LUX_LONG);
  sym[result].spec.scalar.l = p - firstbreak;
  return result;
}
/*-------------------------------------------------------------------*/
