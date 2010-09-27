#include <stdio.h>
#include <ctype.h>
void getTermCaps(void);
int	scrat[8192];

char *pp(char *p)
{
  char	*q = (char *) scrat;

  while (*p) {
    if (isprint(*p))
      sprintf(q, "%c", *p++);
    else
      sprintf(q, "[%1d]", *p++);
    q += strlen(q); }
  return (char *) scrat;
}

void main(void)
{
  extern char	*c_left, *c_right, *c_up, *c_down, *cl_eos,
	*c_save, *c_restore, *k_backspace, *k_delete, *k_insert,
	*k_left, *k_right, *k_up, *k_down;

  getTermCaps();
  printf("c_left:      %s\n", pp(c_left));
  printf("c_right:     %s\n", pp(c_right));
  printf("c_up:        %s\n", pp(c_up));
  printf("c_down:      %s\n", pp(c_down));
  printf("cl_eos:      %s\n", pp(cl_eos));
  printf("c_save:      %s\n", pp(c_save));
  printf("c_restore:   %s\n", pp(c_restore));
  printf("k_backspace: %s\n", pp(k_backspace));
  printf("k_delete:    %s\n", pp(k_delete));
  printf("k_insert:    %s\n", pp(k_insert));
  printf("k_left:      %s\n", pp(k_left));
  printf("k_right:     %s\n", pp(k_right));
  printf("k_up:        %s\n", pp(k_up));
  printf("k_down:      %s\n", pp(k_down));
}

