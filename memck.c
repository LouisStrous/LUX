/* File memck.c */
/* ANA routines about memory use statistics. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <stdio.h>
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: memck.c,v 4.0 2001/02/07 20:37:03 strous Exp $";

#ifdef SBRK_H
#include SBRK_H
#else
void	*sbrk(Int);
#endif

/*-------------------------------------------------------------------*/
Int ana_memstat(Int narg, Int ps[])
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
Int ana_memory(Int narg, Int ps[])
     /* returns size of allocated memory */
{
  Int	result;
  char	*p;
  extern char	*firstbreak;

  p = sbrk(0);
  result = scalar_scratch(ANA_LONG);
  sym[result].spec.scalar.l = p - firstbreak;
  return result;
}
/*-------------------------------------------------------------------*/
