/* This is file site.cc.

Copyright 2013-2016 Louis Strous

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
// File site.c
// LUX site and version identification routine.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "luxdefs.hh"
#include "dmalloc.hh"
#include "editor.hh"                // for BUFSIZE

extern int32_t        internalMode;
extern void printw(char const* string), printwf(char const* fmt, ...);
extern char        *c_left, *k_left, *c_right, *k_right,
        *c_up, *k_up, *c_down, *k_down, *k_delete,
        *k_backspace, *k_insert, *cl_eos, *c_save, *c_restore;

void printnice(char *p)
{
  if (!p) {
    printf("<none>");
    return;
  }
  while (*p) {
    if (isprint((uint8_t) *p))
      putchar(*p);
    else
      printf("\\%03o", *p);
    p++;
  }
}

int32_t site(ArgumentCount narg, Symbol ps[])
/* Prints greeting message.  More info available when arguments are used.
   Be sure to delete  site.o  before every compilation, because otherwise the
   compilation time etc. aren't updated!   LS 10/5/92 */
{
  char        fmt[] = " %22s %6d %6d\n", hasInclude = 0;
  void        setPager(int32_t), resetPager(void);

  setPager(0);
  if (!internalMode || internalMode == 255) {
    printw("*** Welcome to " PACKAGE_STRING "\n");
    printw("Copyright 2013-2024 Louis Strous.\n"
           "This program comes with ABSOLUTELY NO WARRANTY; "
           "for details type ‘info,/warranty’.  "
           "This is free software, and you are welcome to redistribute "
           "it under certain conditions; type ‘info,/copy’ for details.\n"
           "Type ‘help’ for assistance.\n");
#if DEBUG
    printw("Warning: This version of LUX was compiled with the DEBUG option - This may make the program very slow!\n");
#endif
  }

  if (internalMode & 16)        // /WARRANTY
    printw("****WARRANTY\n"
           "This program is distributed in the hope that it will be useful, "
           "but WITHOUT ANY WARRANTY; without even the implied warranty of "
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
           "GNU General Public License for more details.\n\n"
           "You should have received a copy of the GNU General Public License\n "
           "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
  if (internalMode & 32)        // /COPY
    printw("****COPYING\n"
           "This program is free software: you can redistribute it and/or modify "
           "it under the terms of the GNU General Public License as published by "
           "the Free Software Foundation, either version 3 of the License, or "
           "(at your option) any later version.\n");
  if (internalMode & 64)        // /BUGS
    printw("****BUGS\n"
           "If you think you have found a bug in LUX, then please report "
           "it to lux@quae.nl.\n");
#ifdef __STDC__
  if (internalMode & 2) {        // /TIME
    printw("****COMPILATION TIME\n");
    printw("This version was compiled on " __DATE__ ", " __TIME__ "\n");
  }
#endif
  if (internalMode & 4) {        // /PLATFORM
    printw("****PLATFORM\n");
    printw("Platform: " PLATFORM "\n");
  }
  if (internalMode & 8)                // /PACKAGES
  {
    printw("****PACKAGES\n");
#ifdef JPEG
    if (!hasInclude) printw("Packages: ");
    printw("JPEG ");
    hasInclude = 1;
#endif
    if (hasInclude) printw("\n");
  }

  if (internalMode & 1)        {        // give tables info
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
  }
  if (internalMode & 128) {        // /KEYS
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
