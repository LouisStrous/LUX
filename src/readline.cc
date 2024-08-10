/// This file provides dummy stubs for readline support, in case the readline
/// library is not available
#include "readline.hh"

#if !HAVE_LIBREADLINE

# include <stdio.h>
# include <malloc.h>
# include <string.h>            /* for strchr */

void rl_initialize() { }

void rl_get_screen_size(int* rows, int* cols) {
  // assume an 80 by 25 terminal
  if (rows)
    *rows = 25;
  if (cols)
    *cols = 80;
}

int rl_getc(FILE* fin)
{
  return getc(fin);
}

void rl_prep_terminal(int x) { }

void rl_deprep_terminal() { }

char* readline(const char* prompt)
{
  fputs(prompt, stdout);

  size_t size = 0;
  size_t offset = 0;
  char* buf = NULL;
  char* result;
  char* p;
  do {
    size += 3;
    buf = (char*) realloc(buf, size);
    size_t o = offset - (offset? 1: 0);
    result = fgets(buf + o, size - o, stdin);
    p = strchr(buf + o, '\n');
    offset += 3;
  } while (result != NULL && p == NULL);
  return buf;
}

#endif
