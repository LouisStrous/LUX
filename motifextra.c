/* routines that Strous needs to run Shine's Motif stuff */
#include "action.h"
#include <string.h>
#include <stdlib.h>

int	motif_input_flag = 0;

/*-------------------------------------------*/
int ana_zeroifnotdefined(int narg, int ps[])
/* assigns zero to the argument if the argument is undefined.
 (should replace by DEFAULT,arg,0 instead) */
{
  while (narg--) {
    if (symbol_class(*ps) == ANA_UNDEFINED) {
      symbol_class(*ps) = ANA_SCALAR;
      scalar_type(*ps) = ANA_LONG;
      scalar_value(*ps).l = 0;
    }
    ps++;
  }
  return 1;
}
/*-------------------------------------------*/
int ana_compile_file(int narg, int ps[])
/* COMPILE_FILE compiles the contents of a file at the top level */
{
  FILE	*fp;
  int	result, nextCompileLevel(FILE *, char *);
  
  if (symbol_class(ps[0]) != ANA_STRING)
    return cerror(NEED_STR, ps[0]);

  fp = openPathFile(string_value(ps[0]), FIND_SUBR);
  if (!fp)
    return cerror(ERR_OPEN, ps[0]);
  
  result = nextCompileLevel(fp, curScrat);
  fclose(fp);
  return result;
}
/*-------------------------------------------*/
