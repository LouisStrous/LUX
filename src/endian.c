#include <stdio.h>

Int main(void)
{
  union { unsigned char b; Int l; } x ;

  x.l = 1;
  printf(x.b? "1\n": "0\n");
  return 0;
}
