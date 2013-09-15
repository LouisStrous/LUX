/* This is file rawio.c.

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
/* File rawio.c */
/* LUX routines for (de)selecting raw keyboard input. */
/* Author: Louis Strous */
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include "action.h"
#include <stdio.h>
#include <termios.h>		/* for struct termios, tcgetattr(), */
				/* tcsetattr() */
#include <unistd.h>

Int	buffering = 1;
static struct termios	entry_io_params; /* to save entry state */

Int rawIo(void)
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
Int cookedIo(void)
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
Int resetIo(void)
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
