/* This is file terminfo.cc.

Copyright 2013-2014 Louis Strous

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
#include "config.hh"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "terminfo.hh"
#include "action.hh"

#define MAGIC        0432
#define DVI(x)                (((uint8_t)(x)[0]) + 256*((uint8_t)(x)[1]))
#define IS_MINUS1(x)        ((uint8_t)((x)[0]) == 0377 && (uint8_t)((x)[1]) == 0377)
#define IS_MINUS2(x)        ((uint8_t)((x)[0]) == 0376 && (uint8_t)((x)[1]) == 0377)

extern char const *c_left, *c_right, *c_up, *c_down, *cl_eos, *k_backspace,
        *k_delete, *k_insert, *k_up, *k_down, *k_right, *k_left, *c_save,
  *c_restore, *special[7];
extern char  isSpecial[256];
extern int32_t        scrat[];
static int32_t        str_count;
static char        *cap_strings;
static uint8_t        *cap_offsets;
char        *termEntry(int32_t);
//----------------------------------------------------
void getTermCaps(void)
// reads terminal capabilities of terminal type TERM (environment variable)
// through terminfo or (if terminfo is not available) termcap.
{
  char        *terminfo, buf[256], txt[12];
  char const* term;
  int32_t        n, i, name_size, bool_count, num_count, str_size, ncap;
  FILE        *fp;
  // order: c_left, k_left, c_down, k_down, c_up, k_up, c_right, k_right,
  // cl_eos, k_backspace, k_delete, k_insert, c_save, c_restore
  int32_t        terminfos[] = {
    STR_CAP_cub1,               // move left one space
    STR_CAP_kcub1,              // left-arrow key
    STR_CAP_cud1,               // down one line
    STR_CAP_kcud1,              // down-arrow key
    STR_CAP_cuu1,               // up one line
    STR_CAP_kcuu1,              // up-arrow key
    STR_CAP_cuf1,   // non-destructive space (move right one space)
    STR_CAP_kcuf1,              // right-arrow key
    STR_CAP_ed,                 // clear to end of screen
    STR_CAP_kbs,                // backspace key
    STR_CAP_kdch1,              // delete-character key
    STR_CAP_kich1,              // insert-character key
    STR_CAP_sc,                 // save current cursor position
    STR_CAP_rc    // restore cursor to position of last save_cursor
  };
  char const **capabilities[] = {
    &c_left, &k_left, &c_down, &k_down, &c_up, &k_up, &c_right, &k_right,
    &cl_eos, &k_backspace, &k_delete, &k_insert, &c_save, &c_restore
  };

  // determine the terminal type
  term = getenv("TERM");
  if (!term) {
    puts("getTermCaps - environment variable TERM is not set.");
    puts("Trying 'vt100' terminal.");
    term = "vt100";
  }

  ncap = sizeof(capabilities)/sizeof(char *);
  for (i = 0; i < ncap; i++)
    *capabilities[i] = NULL;        // default: unknown

  fp = NULL;

  // where are the terminfo files?
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
      cap_strings = (char*) malloc(str_size);
      cap_offsets = (uint8_t*) malloc(str_count*2);
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
        fseek(fp, n, SEEK_CUR);        // skip
        fread(cap_offsets, 1, str_count*2, fp); // string offset table
        fread(cap_strings, 1, str_size, fp); // string capabilities

        /* in the following list of capabilities, those names starting
           with a k_ indicate Byte sequences generated by the corresponding
           keys, and the names starting with a c_ indicate Byte
           sequences to be sent to get the corresponding result. */

        for (i = 0; i < ncap; i++)
          *capabilities[i] = termEntry(terminfos[i]);
        fclose(fp);
      }
    }
  }
  if (!fp) {
    printf("WARNING - could not open terminfo file for terminal \"%s\"",
           term);
    puts("(Check TERM and TERMINFO environment variables and file permissions.)");
    puts("Assuming vt100-like PC terminal.");
  }

  if (!c_left) {
    printf("terminfo 'auto_left_margin' not set for %s; using default\n", term);
    c_left = "\033[D";
  }
  if (!k_left) {
    printf("terminfo 'key_left' not set for %s; using default\n", term);
    k_left = "\033[D";
  }
  if (!c_down) {
    printf("terminfo 'cursor_down' not set for %s; using default\n", term);
    c_down = "\033[B";
  }
  if (!k_down) {
    printf("terminfo 'key_down' not set for %s; using default\n", term);
    k_down = "\033[B";
  }
  if (!c_up) {
    printf("terminfo 'cursor_up' not set for %s; using default\n", term);
    c_up = "\033[A";
  }
  if (!k_up) {
    printf("terminfo 'key_up' not set for %s; using default\n", term);
    k_up = "\033[A";
  }
  if (!c_right) {
    printf("terminfo 'cursor_right' not set for %s; using default\n", term);
    c_right = "\033[C";
  }
  if (!k_right) {
    printf("terminfo 'key_right' not set for %s; using default\n", term);
    k_right = "\033[C";
  }
  if (!cl_eos) {
    printf("terminfo 'clr_eos' not set for %s; using default\n", term);
    cl_eos = "\033[J";
  }
  if (!k_backspace) {
    printf("terminfo 'key_backspace' not set for %s; using default\n", term);
    k_backspace = "\177";
  }
  if (!k_delete) {
    printf("terminfo 'key_dc' not set for %s; using default\n", term);
    k_delete = "\033[3~";
  }
  if (!k_insert) {
    printf("terminfo 'key_ic' not set for %s; using default\n", term);
    k_insert = "\033[2~";
  }
  if (!c_save) {
    printf("terminfo 'save_cursor' not set for %s; using default\n", term);
    c_save = "\0337";
  }
  if (!c_restore) {
    printf("terminfo 'restore_cursor' not set for %s; using default\n", term);
    c_restore = "\0338";
  }

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

  printf("\nA");
  printf("%s%sB", c_right, c_right);
  printf("%sC", c_up);
  printf("%s%sD", c_left, c_left);
  printf("%s%sE", c_down, c_right);
  printf("\n\n\n");
}
//----------------------------------------------------
/* ensure that regular malloc is used and not our debug malloc,
 because memory allocated here is not associated with any particular
 symbol.  LS 21sep98 */
#undef malloc
char *termEntry(int32_t index)
{
  char        *p, *q;

  if (index >= str_count)
    return NULL;
  if (IS_MINUS1(cap_offsets + 2*index) || IS_MINUS2(cap_offsets + 2*index))
    return NULL;
  p = cap_strings + DVI(cap_offsets + 2*index);
  q = (char*) malloc(strlen(p) + 1);
  if (q)
    strcpy(q, p);
  return q;
}
//----------------------------------------------------
