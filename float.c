#include <stdio.h>
#include <stdlib.h>

void main(void)
{
  double	d;
  float		f;

  d = 2443259.9;
  f = (float) d;
  printf("%20.15g %20.15g\n", d, f);
}
