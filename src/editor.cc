/* This is file editor.cc.

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
// File editor.c
// Command line editor.
#include "config.h"
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "action.hh"
#include <limits.h>
#include <unistd.h>                // for usleep()
#include "editor.hh"

#include <stdio.h>
#include "readline.hh"
#include "history.hh"

char        line[BUFSIZE], tLine[BUFSIZE], recording = 0;
FILE        *inputStream;
char        *inputString;
int32_t        curLineNumber = 0, compileLevel = 0;
int32_t        echo = 0;  // flag: 1 -> echo lines even if not gotten from stdin
int32_t        getStreamChar(void), getStringChar(void);
int32_t     (*getChar)(void) = getStreamChar, termCol, termRow, uTermCol, page;
int32_t        noPrompt = 0;
extern int32_t        scrat[];
//----------------------------------------------------
void getTerminalSize(void)
{
  int rows, cols;
  rl_initialize();
  rl_get_screen_size(&rows, &cols);
  uTermCol = termCol = cols;
  page = termRow = rows;
}
//----------------------------------------------------
int getStreamChar(void)
{
  return rl_getc(inputStream);
}
//----------------------------------------------------
int getSingleStdinChar(void)
{
  rl_prep_terminal(0);          // prepare for input one char at a time
  int c = rl_getc(stdin);
  rl_deprep_terminal();         // restore old situation
  return c;
}
//----------------------------------------------------
int getStringChar(void)
// gets the next input char from inputString, no control sequences
{
  int32_t        c;

  c = *inputString++;
  if (!c)
  { c = EOF;
    inputString--; }
  return c;
}
//----------------------------------------------------
int32_t readHistory(void)
// reads history from a history file (~/.lux-history)
{
  int result = read_history(expand_name("~/.lux-history", NULL));
  if (result                    // a problem
      && result != ENOENT)      // except "no such file"
    luxerror("A problem reading command line history from ~/.lux-history: %s",
             0, strerror(result));
  return result;
}
//----------------------------------------------------
int32_t saveHistory(void)
// saves history in a history file (~/.lux-history)
{
  int result = write_history(expand_name("~/.lux-history", NULL));
  if (result)
    luxerror("A problem writing command line history to ~/.lux-history: %s",
             0, strerror(result));
  return result;
}
//----------------------------------------------------
int32_t getNewLine(char *buffer, size_t bufsize, char const* prompt, char historyFlag)
/* reads new line from keyboard or file into buffer; returns length
   (terminating null isn't included in count).
   includes history buffer, Word-by-Word movement, search in
   the history buffer, search & replace.
   historyFlag determines whether the history buffer is enabled.
   If End-Of-File is reached on the input stream, then -1 is
   returned. */
{
  static char *line = 0;
  if (line) {
    free(line);
    line = 0;
  }
  int n;
  if (inputStream == stdin) {
    line = readline(prompt);
    if (historyFlag && line && *line)
      add_history(line);
    n = strlen(line);
  } else {                      // read from a file
    size_t nbuf = 0;
    n = getline(&line, &nbuf, inputStream);
    if (n > 0)
      --n; /* don't count terminating null, so the outcome is the same
              as for strlen() */
  }
  if (n < 0)                    // end of file
    return n;
  strncpy(buffer, line, bufsize);
  if (n >= bufsize) {
    buffer[bufsize - 1] = '\0';
    n = bufsize - 1;
  }
  return n;
}
//----------------------------------------------------
