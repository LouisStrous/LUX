/* File rawio.c */
/* ANA routines for (de)selecting raw keyboard input. */
/* Author: Louis Strous */
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <termios.h>		/* for struct termios, tcgetattr(), */
				/* tcsetattr() */
#include <unistd.h>

static char rcsid[] __attribute__ ((unused)) =
 "$Id: rawio.c,v 4.0 2001/02/07 20:37:04 strous Exp $";

int	buffering = 1;
static struct termios	entry_io_params; /* to save entry state */

int rawIo(void)
/* sets input to raw (unbuffered) */
{
  struct termios	io_params;

	/* ensure raw (unbuffered) input */
  if (tcgetattr(1, &io_params) < 0) {/* file descriptor 1 = standard input */
    puts("I could not get the terminal parameters.  You may not get full command line editing capabilities.");
    perror("The system response was:");
    return 0;
  } else {
    entry_io_params = io_params;	/* save for later */
    io_params.c_cc[VMIN] = 1;	/* wait for at least this many chars */
    io_params.c_cc[VTIME] = 0;	/* inter-char timer (unit 0.1 sec) */
    io_params.c_lflag &= ~(ICANON|ECHO); /* unbuffered input & no echoing */
    if (tcsetattr(1, TCSANOW, &io_params) < 0) {
      puts("I could not modify the terminal parameters.  You may not get full command line editing capabilities.");
      return 0;
    } else
      buffering = 0;		/* no line buffering */
  }
  return 1;
}
/*----------------------------------------------------------------*/
int cookedIo(void)
/* sets input to buffered */
{
  struct termios	io_params;

	/* ensure raw (unbuffered) input */
  if (tcgetattr(1, &io_params) < 0) {/* file descriptor 1 = standard input? */
    puts("I could not get the terminal parameters.");
    perror("The system response was:");
    return 0;
  } else {
    io_params.c_lflag |= (ICANON|ECHO); /* buffered input & echoing */
    if (tcsetattr(1, TCSANOW, &io_params) < 0) {
      puts("I could not modify the terminal parameters.");
      return 0;
    } else
      buffering = 1;		/* line buffering */
  }
  return 1;
}
/*----------------------------------------------------------------*/
int resetIo(void)
/* resets input to entry state */
{
  if (tcsetattr(1, TCSANOW, &entry_io_params) < 0) {
    puts("I could not reset the terminal parameters.");
    return 0;
  }
  buffering = 1;			/* buffered input */
  return 1;
}
/*----------------------------------------------------------------*/
