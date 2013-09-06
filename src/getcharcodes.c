#include <stdio.h>
#include <ctype.h>
#include <termio.h>
#include <unistd.h>

void main(void)
{
  struct termio	ioParams;
  char	input;

	/* ensure raw (unbuffered) input */
  if (ioctl(1, TCGETA, &ioParams) < 0)
  { perror("IOCTL get failed");
    exit(); }
  else
  { ioParams.c_cc[VMIN] = 1;
    ioParams.c_cc[VTIME] = 0;
    ioParams.c_lflag &= ~ICANON; /* direct read -> no wait for NL */
    ioParams.c_lflag &= ~ECHO;	/* no direct echo of input */
    if (ioctl(1, TCSETA, &ioParams) < 0)
    { perror("IOCTL set failed");
      exit(); }
  }
  while (input = getchar())
  { if (isprint(input))
      putchar(input);
    else
      printf("\\x%02x", input); }
  putchar('\n');
}
