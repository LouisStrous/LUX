#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <string.h>
#include "action.h"

char	*c_left, *c_right, *c_up, *c_down, *cl_eos, *k_backspace,
	*k_delete, *k_insert, *k_up, *k_down, *k_right, *k_left, *c_save,
	*c_restore, *special[7], isSpecial[256];

/* dummyterm assumes vt100-like defaults for the special key codes. */
/* The delete and backspace keys on PCs are often different from those */
/* on workstations, so we have a "PC" mode and a "workstation" mode. */
/* The PC mode is triggered by certain TERM specifications; currently */
/* only "linux".  */
void dummyTerm(void)
/* Use this if terminfo doesn't work. LS 12jul96 31jul97 */
{
  char	*termName[] = {
    "linux"
  };
  int	nTerms = 1, i;
  char	*name, isPCterm = 0;

  name = getenv("TERM");
  for (i = 0; i < nTerms; i++)
    if (!strcmp(name, termName[i]))
    { isPCterm = 1;
      break; }

  /* we use strsave() because manualTerm() uses realloc() on these */
  /* strings */
  if (isPCterm)
  { k_backspace = strsave("\177");
    k_delete = strsave("\033[3~"); }
  else
  { k_backspace = strsave("\010");
    k_delete = strsave("\177"); }
  k_right = strsave("\033[C");
  k_left = strsave("\033[D");
  k_up = strsave("\033[A");
  k_down = strsave("\033[B");
  k_insert = strsave("\033[2~");
  c_right = strsave("\033[C");
  c_left = strsave("\b");
  c_up = strsave("\033[A");
  c_down = strsave("\033[B");
  c_save = strsave("\0337");
  c_restore = strsave("\0338");
  cl_eos = strsave("\033[J");
  special[0] = k_backspace;
  special[1] = k_delete;
  special[2] = k_right;
  special[3] = k_left;
  special[4] = k_up;
  special[5] = k_down;
  special[6] = k_insert;
  for (i = 0; i < 256; i++)
    isSpecial[i] = 0;
  for (i = 0; i < 7; i++)
    isSpecial[(unsigned char) special[i][0]] = 1;
}
/*--------------------------------------------------------------------*/
/* This routine reads the specifications from a user-generated file. */
#define	nNames  (sizeof(names)/sizeof(char *))
void manualTerm(int number)
{
  char *names[] = {
    "k_backspace", "k_delete", "k_right", "k_left", "k_up", "k_down",
    "k_insert", "c_right", "c_left", "c_up", "c_down", "c_save", "c_restore",
    "cl_eos"
  };
  char **codes[] = {
    &k_backspace, &k_delete, &k_right, &k_left, &k_up, &k_down,
    &k_insert, &c_right, &c_left, &c_up, &c_down, &c_save, &c_restore,
    &cl_eos
  };
    
  extern char	*curScrat;
  char	*p, *name;
  FILE	*fp;
  int	n, i;

  dummyTerm();			/* default; */
  p = getenv("ANADIR");
  if (p) {
    strcpy(curScrat, p);
    strcat(curScrat, "/");
  } else
    *curScrat = '\0';
  name = curScrat + strlen(curScrat);
  strcat(name, "terminal");
  if (number) {
    p = curScrat + strlen(curScrat);
    sprintf(p, "%1ud", number);
  }
  fp = fopen(curScrat, "r");
  if (!fp)
    return;			/* not found; using dummyterm instead */
  n = scratSize();
  do {
    if (fgets(curScrat, n, fp)) { /* get line from file */
      p = strtok(curScrat, "="); /* find code name */
      for (i = 0; i < nNames; i++)
	if (!strcmp(names[i], p)) /* found code name in list */
	  break;
      p = strtok(NULL, "\n");
      if (i < nNames) {		/* found code name in list */
	translateEscapes(p);
	*codes[i] = realloc(*codes[i], strlen(p) + 1);
	memcpy(*codes[i], p, strlen(p) + 1);
      }
    }
  } while (!feof(fp));
  fclose(fp);
  special[0] = k_backspace;
  special[1] = k_delete;
  special[2] = k_right;
  special[3] = k_left;
  special[4] = k_up;
  special[5] = k_down;
  special[6] = k_insert;
  for (i = 0; i < 256; i++)
    isSpecial[i] = 0;
  for (i = 0; i < 7; i++)
    isSpecial[(unsigned char) special[i][0]] = 1;
}
/*--------------------------------------------------------------------*/
int ana_manualterm(int narg, int ps[])
/* select a manual terminal specification */
{
  int	i;

  i = int_arg(ps[0]);
  if (i < 0)
    i = 0;
  manualTerm(i);
  return ANA_OK;
}
/*--------------------------------------------------------------------*/

