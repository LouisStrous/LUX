#ifndef HISTORY_HH
# define HISTORY_HH
# include "config.hh"
# if READLINE_INCLUDE
#  include <stdio.h>
#  include <readline/history.h>
# else
// provide dummy versions of relevant history functions

void add_history(char* line);
int  read_history(char* path);
void stifle_history(int x);
int  write_history(char* path);

# endif
#endif
