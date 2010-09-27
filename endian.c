#include <stdio.h>

int main(void)
{
  union { unsigned char b; int l; } x ;

  x.l = 1;
  printf(x.b? "1\n": "0\n");
  return 0;
}
