#include <stdio.h>
#include <stdlib.h>

void main(void)
{
  Double	d;
  Float		f;

  d = 2443259.9;
  f = (Float) d;
  printf("%20.15g %20.15g\n", d, f);
}
