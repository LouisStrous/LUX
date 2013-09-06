/* File site.c */
/* ANA site and version identification routine. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "anaparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "editor.h"		/* for BUFSIZE */
#include "dmalloc.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: site.c,v 4.0 2001/02/07 20:37:04 strous Exp $";

extern Int	internalMode;
void	printw(char *), printwf(char *, ...);
extern char	*c_left, *k_left, *c_right, *k_right,
	*c_up, *k_up, *c_down, *k_down, *k_delete,
	*k_backspace, *k_insert, *cl_eos, *c_save, *c_restore;

void printnice(char *p)
{
  if (!p) {
    printf("<none>");
    return;
  }
  while (*p) {
    if (isprint((Byte) *p))
      putchar(*p);
    else
      printf("\\%03o", *p);
    p++;
  }
}

Int site(Int narg, Int ps[])
/* Prints greeting message.  More info available when arguments are used.
   Be sure to delete  site.o  before every compilation, because otherwise the
   compilation time etc. aren't updated!   LS 10/5/92 */
{
  char	fmt[] = " %22s %6d %6d\n", hasInclude = 0, *p, *p2;
  void	setPager(Int), resetPager(void);
  
  setPager(0);
  if (!internalMode || internalMode == 255) {
    printw("*** Welcome to " PACKAGE_STRING "\n");
    printw("Type \"HELP\" for assistance.\n");
#if DEBUG
    printw("Warning: This version of ANA was compiled with the DEBUG option - This may make the program very slow!\n");
#endif
  }

  if (internalMode & 16)	/* /WARRANTY */
    printw("****WARRANTY\n"
	   "This program is distributed in the hope that it will be "
	   "useful, but WITHOUT ANY WARRANTY; without even the implied "
	   "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR "
	   "PURPOSE.  You use it COMPLETELY AT YOUR OWN RISK.\n\n");
  if (internalMode & 32)	/* /COPY */
    printw("****COPYING\n"
	   "This program was written by Louis Strous and Richard Shine. "
	   "This version (c) 1995-1999. "
	   "You may make and redistribute as many copies as you like. "
	   "If you modify the program before redistributing, then you "
	   "must clearly mark the modified parts as such and include "
	   "the name of the author of the modified parts.\n");
  if (internalMode & 64)	/* /BUGS */
    printw("****BUGS\n"
	   "If you think you have found a bug in ANA, then please act as follows:\n"
	   "1. Ensure that you have really found a bug, i.e. some part "
	   "of ANA is not behaving as advertised in the manual, or causes "
	   "fatal errors.\n"
	   "2. If you don't have the latest version of ANA, then obtain "
	   "the latest version from your original source or from the "
	   "original authors (http://ana.lmsal.com), and check that the same"
	   "bug is present in the latest version, too.\n"
	   "3. Write a short ANA program that reproduces the bug.  The "
	   "shorter the program is, the easier it is to fix the bug.\n"
	   "4. Send a description of the bug, a copy of your ANA program "
	   "(i.e., code written in the ANA language) that reproduces it,"
	   " and the information about "
	   "your version of ANA (displayed when ANA is started) "
	   "to the author of the malfunctioning part "
	   "of ANA, or to the original authors.  If you have a suggestion "
	   "how the bug may be fixed, then send that, too.\n\n"
	   "Useful additions to ANA (written either in C source code, or "
	   "in the ANA language) may also be directed to the original "
	   "authors of ANA for incorporation in future releases, at "
	   "their discretion, under the same rules as this version of ANA.\n\n"
	   "NOTE that the authors of ANA do not guarantee fixing any bugs "
	   "(see INFO,/WARRANTY).  The speed of their reply (if any) "
	   "depends on their mood, and on how much time they have "
	   "available.\n\n"
	   "You can reach the authors of ANA at email address"
	   " ana@lmsal.com\n\n");
#ifdef __STDC__
  if (internalMode & 2) {	/* /TIME */
    printw("****COMPILATION TIME\n");
    printw("This version was compiled on " __DATE__ ", " __TIME__ "\n");
  }
#endif
#if 0
  if (internalMode & 4) {	/* /PLATFORM */
    printw("****PLATFORM\n");
    printw("Platform: " PLATFORM "\n");
  }
#endif
  if (internalMode & 8)		/* /PACKAGES */
  {
    printw("****PACKAGES\n");
#ifdef X11
    if (!hasInclude) printw("Packages: ");
    printw("X11 ");
    hasInclude = 1;
#endif
#ifdef JPEG
    if (!hasInclude) printw("Packages: ");
    printw("JPEG ");
    hasInclude = 1;
#endif
    if (hasInclude) printw("\n");
  }

  if (internalMode & 1)	{	/* give tables info */
    printw("****SYMBOL TABLES\n");
    printwf("Symbol Table:\n %22s %6s %6s\n", "Type", "Start", "Number");
    printwf(fmt, "named variables", NAMED_START, N_NAMED);
    printwf(fmt, "temporary variables", TEMPS_START, N_TEMPS);
    printwf(fmt, "executables", EXE_START, N_EXE);
    printwf(fmt, "temporary executables", TEMP_EXE_START, N_TEMP_EXE);
    printwf("Symbol stack: %d symbols\n", SYMBOLSTACKSIZE);
    printwf("Hash size: %d\n", HASHSIZE);
    printwf("Scratch space: %d bytes\n", NSCRAT);
    printwf("List stack: %d lists\n", NLIST);
    printwf("Maximum history buffer line: %d characters\n", BUFSIZE);
    printwf("Logical file units: %d\n", MAXFILES);
    printwf("Array dimensions: max %d\n", MAX_DIMS);
    printwf("User stack size (#STACK): %d variables\n", STACKSIZE);
    printwf("Debugging breakpoints: max %d\n", MAXDEBUG);
#ifdef X11
    printwf("X ports: %d\n", MAXWINDOWS);
    printwf("X pixmaps: %d\n", MAXPIXMAPS);
    printwf("X color cells: max %d\n", MAXCOLORS);
    printwf("X menus: %d\n", MAXMENU);
#endif
  }
  if (internalMode & 128) {	/* /KEYS */
    puts("****TERMINAL KEYS\n");
    printf(" Right Arrow: ");
    printnice(k_right);
    putchar('\n');
    printf(" Left Arrow : ");
    printnice(k_left);
    putchar('\n');
    printf(" Up Arrow   : ");
    printnice(k_up);
    putchar('\n');
    printf(" Down Arrow : ");
    printnice(k_down);
    putchar('\n');
    printf(" Backspace  : ");
    printnice(k_backspace);
    putchar('\n');
    printf(" Delete     : ");
    printnice(k_delete);
    putchar('\n');
    printf(" Insert     : ");
    printnice(k_insert);
    putchar('\n');
    puts("Actions:");
    printf(" Right      : ");
    printnice(c_right);
    putchar('\n');
    printf(" Left       : ");
    printnice(c_left);
    putchar('\n');
    printf(" Up         : ");
    printnice(c_up);
    putchar('\n');
    printf(" Down       : ");
    printnice(c_down);
    putchar('\n');
    printf(" Clear Down : ");
    printnice(cl_eos); 
    putchar('\n'); 
    printf(" Save Position   : ");
    printnice(c_save);
    putchar('\n');
    printf(" Restore Position: ");
    printnice(c_restore);
    putchar('\n');
  }
  resetPager();
  return 1;
}
