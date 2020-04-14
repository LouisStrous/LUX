/* This is file termcap.cc.

Copyright 2013-2014 Louis Strous

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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <termcap.h>

char	*c_left, *c_right, *c_up, *c_down, *cl_eos,
	*c_save, *c_restore, *k_backspace, *k_delete, *k_insert,
	*k_left, *k_right, *k_up, *k_down, *special[7], bs = '\010';
static char	termcaps[1024];
extern int32_t	scrat[];
//----------------------------------------------------
void getTermCaps(void)
// reads terminal capabilities of terminal type TERM (environment variable)
// from file /etc/termcap. Uses vt100 defaults for capabilities that
// could not be found.
{
  /*  int32_t	tgetent(char *buffer, char *name);
      char	*tgetstr(char *cap, char **buf); */
  char	*term, *ptr, *cscrat = (char *) scrat;
  int32_t	n, keycode(char *);

  term = getenv("TERM");
  if (!term)
  { puts("getTermCaps - environment variable TERM is not set.");
    puts("Trying 'dumb' terminal.");
    term = "dumb"; }
  n = tgetent(cscrat, term);
  if (n < 1)
  { if (n == -1)
      puts("getTermCaps - Could not locate termcap file.");
    else
      printf("getTermCaps - No termcap entry for '%s' terminal.", term);
    n = 2; }
  ptr = termcaps;
  if (n == 1)			// termcaps found
  { c_left = tgetstr("le", &ptr);
    k_left = tgetstr("kl", &ptr);
    c_down = tgetstr("do", &ptr);
    k_down = tgetstr("kd", &ptr);
    c_right = tgetstr("nd", &ptr);
    k_right = tgetstr("kr", &ptr);
    c_up = tgetstr("up", &ptr);
    k_up = tgetstr("ku", &ptr);
    cl_eos = tgetstr("cd", &ptr);
    c_restore = tgetstr("rc", &ptr);
    c_save = tgetstr("sc", &ptr);
    k_backspace = tgetstr("kb", &ptr);
    k_delete = tgetstr("kD", &ptr);
    k_insert = tgetstr("kI", &ptr);
    if (!c_left)
      puts("No 'cursor left' (le) capability - trying vt100 default");
    if (!c_right)
      puts("No 'cursor right' (nd) capability - trying vt100 default");
    if (!c_down)
      puts("No 'cursor down' (do) capability - trying vt100 default");
    if (!c_up)
      puts("No 'cursor up' (up) capability - trying vt100 default");
    if (!cl_eos)
      puts("No 'clear down' (cd) capability - trying vt100 default");
    if (!c_save)
      puts("No 'save cursor' (sc) capability - trying vt100 default");
    if (!c_restore)
      puts("No 'restore cursor' (rc) capability - trying vt100 default");
    if (!k_backspace)
    { if (tgetflag("bs"))
	k_backspace = &bs;
      else
	puts("No 'backspace key' (kb,bs) capability - trying vt100 default"); }
    if (!k_delete)
      puts("No 'delete key' (kD) capability - trying default");
    if (!k_insert)
      puts("No 'insert key' (kI) capability - trying default"); }
  if (!c_right)
    c_right = "\033[C";
  if (!k_right)
    k_right = c_right;
  if (!c_left)
    c_left = "\033[D";
  if (!k_left)
    k_left = c_left;
  if (!c_up)
    c_up = "\033[A";
  if (!k_up)
    k_up = c_up;
  if (!c_down)
    c_down = "\033[B";
  if (!k_down)
    k_down = c_down;
  if (!cl_eos)
    cl_eos = "\033[J";
  if (!c_save)
    c_save = "\0337";
  if (!c_restore)
    c_restore = "\0338";
  if (!k_backspace)
    k_backspace = "\010";
  if (!k_delete)
    k_delete = "\033[3~";
  if (!k_insert)
    k_insert = "\033[2~";
  special[0] = k_backspace;
  special[1] = k_delete;
  special[2] = k_right;
  special[3] = k_left;
  special[4] = k_up;
  special[5] = k_down;
  special[6] = k_insert;
}
//----------------------------------------------------
