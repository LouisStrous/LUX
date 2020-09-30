#ifndef READLINE_HH
# define READLINE_HH
# include "config.h"
# if HAVE_LIBREADLINE
#  include <stdio.h>
#  include <readline/readline.h>
# else
// provide dummy versions of relevant readline functions

#  include <stdio.h>            // for FILE

char* readline(const char* prompt);
void  rl_deprep_terminal();
void  rl_get_screen_size(int* rows, int* cols);
int   rl_getc(FILE* fin);
void  rl_initialize();
void  rl_prep_terminal(int x);

# endif
#endif
