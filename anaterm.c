/*----------------------------------------------------*/
void getTermCaps(void)
/* sets terminal capabilities.  I've had trouble with both termcap and */
/* terminfo (specifically with returned key sequences - e.g. for the */
/* right arrow key - which did not correspond to what it said in the */
/* termcap or terminfo databases, and finally decided to do it my own */
/* way.  Now, I read an "anatermcap" file depending on the TERM */
/* environment variable.  LS 12jul96 */
{
  char	*term, *cscrat = (char *) scrat;
  extern char	expname[];
  FILE	*fp;
  int	i, n;
  
  term = getenv("TERM");
  if (!term)
  { puts("getTermCaps - No translation for environment variable TERM.");
    puts("Using default terminal key codes."); }
  else
  { expand_name("$ANADIR/term/", NULL);
    strcat(expname, term);
    fp = fopen(expname, "r");
    if (!fp)
    { printf("getTermCaps - Cannot open file %s.\n", expname);
      puts("Using default terminal key codes."); }
    else
    { i = 0;
      while (fgets(cscrat, 80, fp))
      { if (*cscrat == '#')	/* a comment line */
	  continue;
	sprintf(cscrat + 80, cscrat);
	n = strlen(cscrat + 80);
	if (cscrat[n + 79] == '\n')
	  cscrat[--n + 80] = '\0';
	special[i] = (char *) malloc(n + 1);
	strcpy(special[i], cscrat + 80);
	i++;
	if (i == 14)
	  break; }
      Fclose(fp); }
  }
  if (!special[0])
    special[0] = "\010";
  if (!special[1])
    special[1] = "\033[3~";
  if (!special[6])
    special[6] = "\033[2~";
  if (!special[7])
    special[7] = "\033[C";
  if (!special[8])
    special[8] = "\033[D";
  if (!special[9])
    special[9] = "\033[A";
  if (!special[10])
    special[10] = "\033[B";
  if (!special[11])
    special[11] = "\0337";
  if (!special[12])
    special[12] = "\0338";
  if (!special[13])
    special[13] = "\033[J";
  if (!special[2])
    special[2] = special[7];
  if (!special[3])
    special[3] = special[8];
  if (!special[4])
    special[4] = special[9];
  if (!special[5])
    special[5] = special[10];
  k_backspace = special[0];
  k_delete = special[1];
  k_right = special[2];
  k_left = special[3];
  k_up = special[4];
  k_down = special[5];
  k_insert = special[6];
  c_right = special[7];
  c_left = special[8];
  c_up = special[9];
  c_down = special[10];
  c_save = special[11];
  c_restore = special[12];
  cl_eos = special[13];
}
