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
#include "action.h"
#include <string.h>
#include <stdlib.h>

Int	motif_input_flag = 0;

/*-------------------------------------------*/
Int lux_zeroifnotdefined(Int narg, Int ps[])
/* assigns zero to the argument if the argument is undefined.
 (should replace by DEFAULT,arg,0 instead) */
{
  while (narg--) {
    if (symbol_class(*ps) == LUX_UNDEFINED) {
      symbol_class(*ps) = LUX_SCALAR;
      scalar_type(*ps) = LUX_LONG;
      scalar_value(*ps).l = 0;
    }
    ps++;
  }
  return 1;
}
/*-------------------------------------------*/
Int lux_compile_file(Int narg, Int ps[])
/* COMPILE_FILE compiles the contents of a file at the top level */
{
  FILE	*fp;
  Int	result, nextCompileLevel(FILE *, char *);
  
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
