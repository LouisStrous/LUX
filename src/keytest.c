#include <stdio.h>
#include <ctype.h>

char	batch='\0';

Int main(void)
{
  Int	c;

  rawIo();
  do {
    c = getchar();
    if (isprint(c))
      printf("%c", c);
    else
      printf("\\%03o", c);
  } while (c != 'q');
  cookedIo();
  return 0;
}
