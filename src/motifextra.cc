/* This is file motifextra.c.

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
/* routines that Strous needs to run Shine's Motif stuff */
#include "action.hh"
#include <string.h>
#include <stdlib.h>

int32_t	motif_input_flag = 0;

/*-------------------------------------------*/
int32_t lux_zeroifnotdefined(int32_t narg, int32_t ps[])
/* assigns zero to the argument if the argument is undefined.
 (should replace by DEFAULT,arg,0 instead) */
{
  while (narg--) {
    if (symbol_class(*ps) == LUX_UNDEFINED) {
      symbol_class(*ps) = LUX_SCALAR;
      scalar_type(*ps) = LUX_INT32;
      scalar_value(*ps).l = 0;
    }
    ps++;
  }
  return 1;
}
/*-------------------------------------------*/
int32_t lux_compile_file(int32_t narg, int32_t ps[])
/* COMPILE_FILE compiles the contents of a file at the top level */
{
  FILE	*fp;
  int32_t	result, nextCompileLevel(FILE *, char const*);

  if (symbol_class(ps[0]) != LUX_STRING)
    return cerror(NEED_STR, ps[0]);

  fp = openPathFile(string_value(ps[0]), FIND_SUBR);
  if (!fp)
    return cerror(ERR_OPEN, ps[0]);

  result = nextCompileLevel(fp, curScrat);
  fclose(fp);
  return result;
}
/*-------------------------------------------*/
