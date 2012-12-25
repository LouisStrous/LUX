#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "terminfo.h"
#include "action.h"

#define MAGIC	0432
#define DVI(x)		(((Byte)(x)[0]) + 256*((Byte)(x)[1]))
#define IS_MINUS1(x)	((Byte)((x)[0]) == 0377 && (Byte)((x)[1]) == 0377)
#define IS_MINUS2(x)	((Byte)((x)[0]) == 0376 && (Byte)((x)[1]) == 0377)

extern char	*c_left, *c_right, *c_up, *c_down, *cl_eos, *k_backspace,
	*k_delete, *k_insert, *k_up, *k_down, *k_right, *k_left, *c_save,
	*c_restore, *special[7], isSpecial[256];
extern Int	scrat[];
static Int	str_count;
static char	*cap_strings;
static Byte	*cap_offsets;
char	*termEntry(Int);
/*----------------------------------------------------*/
void getTermCaps(void)
/* reads terminal capabilities of terminal type TERM (environment variable) */
/* through terminfo or (if terminfo is not available) termcap. */
{
  char	*term, *terminfo, buf[256], txt[12], *p, *q, found;
  Int	n, i, name_size, bool_count, num_count, str_size, ncap;
  FILE	*fp;
  /* order: c_left, k_left, c_down, k_down, c_up, k_up, c_right, k_right, */
  /* cl_eos, k_backspace, k_delete, k_insert, c_save, c_restore */
  Int	terminfos[] = {
    STR_CAP_cub1, STR_CAP_kcub1, STR_CAP_cud1, STR_CAP_kcud1,
    STR_CAP_cuu1, STR_CAP_kcuu1, STR_CAP_cuf1, STR_CAP_kcuf1,
    STR_CAP_ed, STR_CAP_kbs, STR_CAP_kdch1, STR_CAP_kich1,
    STR_CAP_sc, STR_CAP_rc
  };
  char	**capabilities[] = {
    &c_left, &k_left, &c_down, &k_down, &c_up, &k_up, &c_right, &k_right,
    &cl_eos, &k_backspace, &k_delete, &k_insert, &c_save, &c_restore
  };
  char	termcaps[] =
    "le:kI:do:kd:up:ku:nd:kr:cd:kb:kD:kI:sc:rc";
  
  /* determine the terminal type */
  term = getenv("TERM");
  if (!term) {
    puts("getTermCaps - environment variable TERM is not set.");
    puts("Trying 'vt100' terminal.");
    term = "vt100";
  }

  ncap = sizeof(capabilities)/sizeof(char *);
  for (i = 0; i < ncap; i++)
    *capabilities[i] = NULL;	/* default: unknown */

  fp = NULL;

  /* where are the terminfo files? */
  terminfo = getenv("TERMINFO");
  if (terminfo) {
    sprintf(buf, "%s/%c/%s", terminfo, term[0], term);
    fp = fopen(buf, "r");
  }

  if (!fp) {
    sprintf(buf, "/usr/lib/terminfo/%c/%s", term[0], term);
    fp = fopen(buf, "r");
  }

  if (!fp) {
    sprintf(buf, "/usr/share/lib/terminfo/%c/%s", term[0], term);
    fp = fopen(buf, "r");
  }

  if (!fp) {
    sprintf(buf, "/usr/share/terminfo/%c/%s", term[0], term);
    fp = fopen(buf, "r");
  }

  if (fp) {
    fread(txt, 1, 12, fp);
    if (DVI(txt) != MAGIC) {
      printf("WARNING - terminfo file \"%s\" has wrong magic number (%1d)\n",
	     buf, DVI(txt));
      fclose(fp);
      fp = NULL;
    } else {
      name_size = DVI(txt + 2);
      bool_count = DVI(txt + 4);
      num_count = DVI(txt + 6);
      str_count = DVI(txt + 8);
      str_size = DVI(txt + 10);
      cap_strings = malloc(str_size);
      cap_offsets = malloc(str_count*2);
      if (!cap_strings || !cap_offsets) {
	printf("WARNING - could not allocate memory in getTermCaps\n");
	puts("Aborting.");
	fclose(fp);
	abort();
      } else {
	/* we skip the terminal name and the boolean and numerical
	   capabilities */
	n = name_size + bool_count;
	n += (n % 2);
	n += num_count*2;
	fseek(fp, n, SEEK_CUR);	/* skip */
	fread(cap_offsets, 1, str_count*2, fp); /* string offset table */
	fread(cap_strings, 1, str_size, fp); /* string capabilities */

	/* in the following list of capabilities, those names starting
	   with a k_ indicate Byte sequences generated by the corresponding
	   keys, and the names starting with a c_ indicate Byte
	   sequences to be sent to get the corresponding result. */

	for (i = 0; i < ncap; i++)
	  *capabilities[i] = termEntry(terminfos[i]);
	fclose(fp);
      }
    }
  } else if ((fp = fopen("/etc/termcap", "r"))) {
    /* didn't find terminfo file, but did find termcap file */
    /* we assume that each entry in the termcap file starts with */
    /* a line that has no tab at its beginning.  Items on such a */
    /* first line are separated by | and the last one ends with a :. */
    /* All items except the last one are terminal names with which */
    /* TERM is to be compared. */
    /* subsequent lines start with a tab and a colon : and have */
    /* capabilities listed separated by colons :.  Backslashes at */
    /* the end of a line are used as a continuation character */
    /* it is assumed that no line exceeds 256 characters! */

    found = 0;
    while (!found) {
      if (fgets(buf, 256, fp) == NULL) {
	fclose(fp);
	fp = NULL;
	break;
      }
      switch (buf[0]) {
	case '#': case '\t':	/* comment or continuation line; skip */
	  continue;
	default:		/* first line of entry */
	  p = strtok(buf, "|");
	  while (p) {
	    n = strlen(p);
	    if (p[n - 1] == '\\') /* continuation character */
	      p[--n] = '\0';
	    if (p[n - 1] == ':') /* this item is the verbose description;
				    skip */
	      p = NULL;
	    else if (strcmp(p, term))
	      p = strtok(NULL, "|");
	    else {		/* we found it */
	      found = 1;
	      break;
	    }
	  }
	  break;
      }	/* end of switch (buf[0]) */
    } /* end of while (!found) */

    if (found)			/* found it */
      while (1) {
	if (fgets(buf, 256, fp) == NULL
	    || buf[0] != '\t') {	/* no more lines in this entry */
	  fclose(fp);
	  fp = NULL;
	  break;
	}
	p = strtok(buf + 1, ":");
	while (p) {
	  if (p[2] == '=') {	/* capability is not a boolean */
	    p[2] = '\0';
	    if ((q = strstr(termcaps, p))) { /* found one */
	      i = (q - termcaps)/3;
	      *capabilities[i] = strsave(p + 2);
	    }
	  } else if (strcmp(p, "bs"))
	    *capabilities[9] = "\010"; /* standard backspace */
	  p = strtok(NULL, ":");
	}
      }
    
  } /* end of if (fp) else */
  
  if (!fp) {
    printf("WARNING - could not open terminfo file for terminal \"%s\"",
	   term);
    printf("and did not find appropriate entry in termcap file \"/etc/termcap\".\n");
    puts("(Check TERM and TERMINFO environment variables and file permissions.)");
    puts("Assuming vt100-like PC terminal.");
  } 
  
  if (!c_left)
    c_left = "\033[D";
  if (!k_left)
    k_left = "\033[D";
  if (!c_down)
    c_down = "\033[B";
  if (!k_down)
    k_down = "\033[B";
  if (!c_up)
    c_up = "\033[A";
  if (!k_up)
    k_up = "\033[A";
  if (!c_right)
    c_right = "\033[C";
  if (!k_right)
    k_right = "\033[C";
  if (!cl_eos)
    cl_eos = "\033[J";
  if (!k_backspace)
    k_backspace = "\177";
  if (!k_delete)
    k_delete = "\033[3~";
  if (!k_insert)
    k_insert = "\033[2~";
  if (!c_save)
    c_save = "\0337";
  if (!c_restore)
    c_restore = "\0338";

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
    if (special[i])
      isSpecial[(uint32_t) *special[i]] = 1;

  free(cap_strings);
  free(cap_offsets);
}
/*----------------------------------------------------*/
/* ensure that regular malloc is used and not our debug malloc,
 because memory allocated here is not associated with any particular
 symbol.  LS 21sep98 */
#undef malloc
char *termEntry(Int index)
{
  char	*p, *q;

  if (index >= str_count)
    return NULL;
  if (IS_MINUS1(cap_offsets + 2*index) || IS_MINUS2(cap_offsets + 2*index))
    return NULL;
  p = cap_strings + DVI(cap_offsets + 2*index);
  q = malloc(strlen(p) + 1);
  if (q)
    strcpy(q, p);
  return q;
}
/*----------------------------------------------------*/
