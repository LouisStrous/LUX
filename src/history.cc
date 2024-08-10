/// This file provides dummy stubs for history support, in case the readline
/// library is not available
#include "history.hh"

#if !READLINE_INCLUDE

int read_history(char* path)
{
  return 0;
}

int write_history(char* path)
{
  return 0;
}

void add_history(char* line) { }

void stifle_history(int x) { }

#endif
