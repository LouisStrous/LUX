/* This is file editor.c.

Copyright 2013 Louis Strous

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
/* File editor.c */
/* Command line editor. */
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "action.h"
#include <limits.h>
#include <termios.h>		/* for tcgetattr(), tcsetattr() */
#include <unistd.h>		/* for usleep() */
#include "editor.h"
#include "editorcharclass.h"
#include "install.h"
/* ioctl() is generally defined in sys/ioctl.h or in unistd.h */
#include <sys/ioctl.h>
#include <sys/termios.h>	/* for struct winsize */

char	line[BUFSIZE], tLine[BUFSIZE], recording = 0;
FILE	*inputStream;
char	*inputString;
Int	curLineNumber = 0, compileLevel = 0;
static Int	promptLength, show = 1;
Int	echo = 0;  /* flag: 1 -> echo lines even if not gotten from stdin */
Int	getStreamChar(void), getStringChar(void);
Int     (*getChar)(void) = getStreamChar, termCol, termRow, uTermCol, page;
void	rawIo(void), cookedIo(void);
Int	noPrompt = 0;
static Int	col = 0, row = 0, textWidth;
static char	*thePrompt;
extern Int	scrat[];
extern char	*c_left, *c_right, *c_up, *c_down,
	*cl_eos, *c_save, *c_restore, *k_backspace, *k_delete,
	*k_insert, *k_left, *k_right, *k_up, *k_down;
extern char	*special[7], isSpecial[256];
/*----------------------------------------------------*/
void getTerminalSize(void)
{
  struct winsize	terminal;

  if (ioctl(1, TIOCGWINSZ, &terminal) < 0) {
    termCol = uTermCol = 80;		/* default values */
    termRow = page = 24;
  } else {
    termCol = terminal.ws_col;	/* # columns on terminal */
    if (!termCol) {
      puts("Cannot determine the number of columns on your terminal: Assuming infinite.");
      uTermCol = INT32_MAX;
    } else
      uTermCol = termCol - 1;	/* minus one so we don't get into trouble
				   typing \n at the right edge of the screen */
    termRow = terminal.ws_row; /* # rows on terminal */
    if (!termRow) {
      puts("Cannot determine the number of lines on your terminal: Assuming infinite.");
      page = INT32_MAX;
    } else
      page = termRow;
  }
  /*  printf("terminal size: %1dx%1d\n", termCol, termRow); */
}
/*----------------------------------------------------*/
void setPrompt(char *s)
/* saves current cursor position, goes to start of line, prints <s> */
/* in a field of width <promptLength>, then restores cursor position */
{
  printf(c_save);
  printf("\r%*.*s", promptLength, promptLength, s);
  printf(c_restore);
}
/*----------------------------------------------------*/
Int getStringChar(void)
/* gets the next input char from inputString, no control sequences */
{
  Int	c;

  c = *inputString++;
  if (!c)
  { c = EOF;
    inputString--; }
  return c;
}
/*----------------------------------------------------*/
static Int	code[] = { BKS, DEL, RAR, LAR, UAR, DAR, INS };
static char	streamBuffer[10];
static Int	streamBufferDepth = 0, streamBufferIndex = 0;
#define isSpecialChar(c)	((c < 0 || c >= 256)? 0: isSpecial[c])
Int getStreamChar(void)
/* gets the next input char from inputStream; translates control sequences */
/* ASSUMES THAT ALL MULTI-BYTE KEY RETURNS (eg. for arrow keys) START */
/* WITH ESC */
/* return values:
     char		char
     ESC char		char + 0x200
     ESC chars		check for arrow keys, backspace, delete, insert
     C-X char		char + 0x400 */
{
 Int	c, n, n2, i, os, c3;
 char	*q;

 if (streamBufferIndex == streamBufferDepth) {
   if (!inputStream)
     return EOF;
   streamBuffer[streamBufferDepth++] = c = c3 = getc(inputStream);
 } else
   c = c3 = streamBuffer[streamBufferIndex];

 n = 1;
 if (isSpecialChar(c))		/* need to check agains keystroke sequences */
   for (i = 0; i < 7; i++) {	/* seven of them */
     q = special[i];		/* keystroke sequence */
     if (!q)
       continue;
     n2 = strlen(q);
     while (!strncmp(streamBuffer + streamBufferIndex, q, n) && n < n2) {
				/* partial match but read text is still
				   too short */
       os = streamBufferIndex;
       streamBufferIndex = streamBufferDepth;
       c3 = getStreamChar();		/* get more characters */
       streamBufferIndex = os;
       n++;
     }
     if (!strncmp(streamBuffer + streamBufferIndex, q, n)) { /* exact match */
       /* setPrompt(row? "->": thePrompt); */
       if (!streamBufferIndex)
	 streamBufferDepth = 0;
       else {
	 streamBuffer[streamBufferIndex] = code[i];
	 streamBufferDepth -= n - 1;
       }
       return code[i];
     }     
   }

 /* no match with special keystroke sequences */
 switch (c) {
   case '\030':			/* C-X */
     /* setPrompt("C-X>"); */
     streamBufferIndex++;
     c = getStreamChar();	/* get one more char */
     streamBufferIndex--;
     c = (c < 256? toupper(c): c) | 0x400;
     if (streamBufferDepth == 2)  /* last one */
       streamBufferDepth = streamBufferIndex = 0;
     else {
       streamBufferDepth--;
       streamBuffer[streamBufferIndex] = c;
     }
     /* setPrompt(row? "->": thePrompt); */
     return c;
   case ESC:			/* ESC */
     /* setPrompt("ESC>"); */
     if (n == 1) {		/* get next char */
       streamBufferIndex++;
       c = getStreamChar();
     } else if (n > 2)
       c = streamBuffer[streamBufferIndex + 1];
     else
       c = c3;
     c = (c < 256? toupper(c): c) | 0x200;
     if (streamBufferDepth == 2)
       streamBufferDepth = streamBufferIndex = 0;
     else if (!streamBufferIndex) { /* last one */
       streamBufferDepth -= 2;
       if (n - 2)
	 memcpy(streamBuffer, streamBuffer + 2, n - 2);
     } else {
       streamBufferDepth--;
       streamBuffer[streamBufferIndex] = c;
       if (n - 2)
	 memcpy(streamBuffer + streamBufferIndex + 1,
		streamBuffer + streamBufferIndex + 2,
		n - 2);
     }
     /* setPrompt(row? "->": thePrompt); */
     return c;
   default:
     if (streamBufferDepth == 1)
       streamBufferDepth = streamBufferIndex = 0;
     return c;
   }
}
/*----------------------------------------------------*/
void putChar(Int ch)
/* reciprocal of getChar(); outputs the characters that go with code <ch>.
   No complaints about weird codes. */
{
 Int	n;

 if (!show)
   return;
 n = ch/0x100;
 switch (n)
 { case 0:
     putchar(ch);
     return;
   case 1:
     switch (ch)
     { case INS:
	 printf(k_insert);
	 break;
       case DEL:
	 printf(k_delete);
	 break;
       case BKS:
	 printf(k_backspace);
	 break;
       case UAR:
	 printf(c_up);
	 break;
       case DAR:
	 printf(c_down);
	 break;
       case LAR:
	 printf(c_left);
	 break;
       case RAR:
	 printf(c_right);
	 break; }
     return;
   case 2:
     putchar(ESC);
     putchar((char) (ch & 0x100));
     return;
   case 4:
     putchar('\030');
     putchar((char) (ch & 0x100));
     return; }
}
/*----------------------------------------------------*/
Int keycode(char *p)
/* returns the key sequence code for the string at <p> (same code as */
/* in getStreamChar() ) */
{
 Int	n, n2, i;
 char	*temp = p, *q;

 n = 1;
 for (i = 0; i < 7; i++)
 { q = special[i];
   n2 = strlen(q);
   while (!strncmp(temp, q, n) && n < n2)
     n++;
   if (!strncmp(temp, q, n))
     return code[i];
 }
 switch (*temp)
 { case '\030':			/* C-X */
     return toupper(temp[1]) + 0x400;
   case ESC:			/* ESC */
     return toupper(temp[1]) + 0x200;
   default:
     return *temp;
   }
}
/*----------------------------------------------------*/
void cursor_goto(Int rcol, Int rrow)
/* cursor is at (col, row) and goes to (rcol, rrow).  col and row */
/* are updated  */
{
  if (row != rrow) {
    if (row < rrow)
      while (row++ < rrow)
	printf(c_down);
    else
      while (row-- > rrow)
	printf(c_up);
  }
  if (col != rcol) {
    if (col < rcol)
      while (col++ < rcol)
	printf(c_right);
    else
      while (col-- > rcol)
	printf(c_left);
  }
  col = rcol;
  row = rrow;
}
/*----------------------------------------------------*/
void cursor_gofrom(Int rcol, Int rrow)
/* cursor is at (rcol, rrow) and goes to (col, row). */
{
  if (col != rcol) {
    if (rcol < col)
      while (rcol++ < col) 
	printf(c_right);
    else
      while (rcol-- > col)
	printf(c_left);
  }
  if (row != rrow) {
    if (rrow < row)
      while (rrow++ < row)
	printf(c_down);
    else
      while (rrow-- > row)
	printf(c_up);
  }
}
/*---------------------------------------------*/
void out(char *p, Int blank)
/* redraw the line from position <p>.  If <black> is non-zero, then
   that many blanks are printed after the text. */
{
  Int	n, size, newCol, newRow;

  newCol = col;
  newRow = row;
  n = size = strlen(p);
  newCol += size;
  if (newCol >= textWidth)
    n = textWidth - col;
  size -= n;
  while (n--)
    putchar(*p++);
  while (size) {
    printf("-\n%*.*s", promptLength, promptLength, "->");
    newRow++;
    n = (size >= textWidth)? textWidth: size;
    newCol = n;
    size -= n;
    while (n--)
      putchar(*p++);
  }
  if (blank) {
    if (newCol + blank >= textWidth)
      n = textWidth - newCol;
    else
      n = blank;
    blank -= n;
    newCol += n;		/* position after blanks are printed */
    while (n--)
      putchar(' ');
    while (blank) {
      printf(" \n%*.*s", promptLength, promptLength, " ");
      newRow++;
      n = (blank >= textWidth)? textWidth: blank;
      newCol = n;
      blank -= n;
      while (n--)
	putchar(' ');
    }
  }
  cursor_gofrom(newCol, newRow);
}
/*----------------------------------------------------*/
void outChar(Int c)
{
  if (col++ == textWidth) {
    printf("-\n%*.*s", promptLength, promptLength, "->");
    row++;
    col = 1;
  }
  putchar(c);
}
/*----------------------------------------------------*/
void advance(Int num)
/* move the cursor <num> positions forward in the line */
{
  Int	oldCol, oldRow;

  oldCol = col;
  oldRow = row;
  col += num;
  if (col < 0) {
    row -= 1 + col/textWidth;
    col = col%textWidth + textWidth;
  } else if (col >= textWidth) {
    row += col/textWidth;
    col = col%textWidth;
  }
  cursor_gofrom(oldCol, oldRow);
}
/*----------------------------------------------------*/
char *nextEOW(char *p)
/* returns pointer to next end-of-Word */
/* a Word consists of a contiguous set of characters from the set
 ! # $ 0-9 A-Z _ a-z */
{
  if (*p)
  { while (*p && !isOrdinaryChar((Byte) *p))
      p++;
    while (isOrdinaryChar((Byte )*p))
      p++;
  }
  return p;
}
/*----------------------------------------------------*/
char *prevBOW(char *p, char *buf)
{
  if (p > buf)
  { while (p > buf && !isOrdinaryChar((Byte) p[-1]))
      p--;
    while (p > buf && isOrdinaryChar((Byte) p[-1]))
      p--;
  }
  return p;
}
/*----------------------------------------------------*/
char *nextEOE(char *p, char *buf)
/* returns a pointer to the next end-of-expression */ 
{
  char	*q, *q2, inString = (char) 0;
  Int	n;

  if (!*p)
    return p;
  q = buf;
  while (q < p)			/* determine if we're inside a string */
  { if (*q == '\'')
    { if (inString && q[1] == '\'') /* quote in string */
	q++;
      else
      { inString = inString ^ (char) 1;
	q2 = q; }		/* q2 = start of current string */
    }
    q++; }
  if (!inString)
    while (isWhiteSpace((Byte) *p) ||
	   (isSeparatorChar((Byte) *p) && !strchr("\'([{}])", *p)))
      p++;
  if (*p == '\'')
  { inString = (char) 1;
    q2 = p++; }
  if (inString)
  { p = q2 + 1;			/* start at beginning of current string */
    while (inString)
    { switch (*p)
      { case '\0':		/* end of command line */
	  inString = (char) 0;
	  break;
	case '\'':
	  if (p[1] == '\'')	/* quote in string */
	    p++;
	  else
	    inString = (char) 0;
	  break; }
      p++; }
    return p; }
  if (isNextChar(toupper(*p)) || isFirstChar(toupper(*p))) /* identifier */
  { while (isNextChar(toupper(p[1])))
      p++;
    return p + 1; }
  if (isNumberChar(toupper(*p))		/* number */)
  { while (isNumberChar(toupper(p[1])))
      p++;
    return p + 1; }
  if (*p == '(')
  { n = 1;
    while (n)
    { switch (p[1])
      { case '(':
	  n++;
	  break;
	case ')':
	  n--;
	  break;
	case '\0':
	  n = 0;
	  break; }
      p++;
    }
    return p + 1; }
  if (*p == '[')
  { n = 1;
    while (n)
    { switch (p[1])
      { case '[':
	  n++;
	  break;
	case ']':
	  n--;
	  break;
	case '\0':
	  n = 0;
	  break; }
      p++;
    }
    return p + 1; }
  if (*p == '{')
  { n = 1;
    while (n)
    { switch (p[1])
      { case '{':
	  n++;
	  break;
	case '}':
	  n--;
	  break;
	case '\0':
	  n = 0;
	  break; }
      p++;
    }
    return p + 1; }
  return p + 1;
}
/*----------------------------------------------------*/
char *prevBOE(char *p, char *buf)
/* returns a pointer to the previous beginning-of-expression */
{
  char	*q, *q2, inString = (char) 0;
  Int	n;

  if (p == buf)			/* at start, so return right away */
    return p;
  if (p > buf)
    p--;
  q = buf;
  while (q < p)			/* determine if we're inside a string */
  { if (*q == '\'')
    { if (inString && q[1] == '\'') /* quote in string */
	q++;
      else
      { inString = inString ^ (char) 1;
        q2 = q; }
    }
    q++; }
  if (!inString)		/* not inside a string; skip separators */
  { q2 = p;
    while (p > buf && (isWhiteSpace((Byte) *p)
		       || (isSeparatorChar((Byte) *p) && *p != '\'')))
      p--;
    if (q2 != p && *p == '\'')	/* and end of string: find beginning */
    { if (p > buf)
	p--;
      q = buf;
      while (q < p)		
      { if (*q == '\'')
	{ if (inString && q[1] == '\'') /* quote in string */
	    q++;
	else
	{ inString = inString ^ (char) 1;
	  q2 = q; }
	}
	q++; }
    }
  }
  if (inString)
    return q2;
  if (isNextChar(toupper(*p)) || isFirstChar(toupper(*p))) /* identifier */
  { while (p > buf && isNextChar(toupper(p[-1])))
      p--;
    if (p > buf && isFirstChar(toupper(p[-1])))
      p--;
    return p; }
  if (isNumberChar(toupper(*p))		/* number */)
  { while (p > buf && isNumberChar(toupper(p[-1])))
      p--;
    return p; }
  if (*p == ')')
  { n = 1;
    while (n && p > buf)
    { switch (*p)
      { case '(':
	  n--;
	  break;
	case ')':
	  n++;
	  break; }
      p--; }
    return p; }
  if (*p == ']')
  { n = 1;
    while (n && p > buf)
    { switch (*p)
      { case '[':
	  n--;
	  break;
	case ']':
	  n++;
	  break; }
      p--; }
    return p; }
  if (*p == '}')
  { n = 1;
    while (n && p > buf)
    { switch (*p)
      { case '{':
	  n--;
	  break;
	case '}':
	  n++;
	  break; }
      p--; }
    return p; }
  return p;
}
/*----------------------------------------------------*/
void close_groups(char *buf)
/* close all open expressions */
{
  char	*q, *end, inString = (char) 0;
  Int	npar = 0, nbrack = 0, nbrace = 0;

  q = buf;
  while (*q)			/* determine if we're inside a string */
				/* at the end of the command line */
  { if (*q == '\'')
    { if (inString && q[1] == '\'') /* quote in string */
	q++;
      else
      { inString = inString ^ (char) 1; }
    }
    q++; }
  if (q == buf)			/* nothing to do */
    return;
  end = q--;			/* in text */
  if (inString)
  { *end++ = '\'';		/* close string */
    *end = '\0'; }
  while (q > buf)
  { switch (*q)
    { case '\'':
	if (inString)
	{ if (q > buf && q[-1] == '\'')	/* quote in string */
	    q--;
	  else
	    inString = (char) 0; }
	else
	  inString = (char) 1;
	break;
      case ')':
	if (!inString)
	  npar++;
	break;
      case '(':
	if (!inString)
	{ npar--;
	  if (npar < 0)
	  { *end++ = ')';
	    *end = '\0';
	    npar++; }
	}
	break;
      case ']':
	if (!inString)
	  nbrack++;
	break;
      case '[':
	if (!inString)
	{ nbrack--;
	  if (nbrack < 0)
	  { *end++ = ']';
	    *end = '\0';
	    nbrack++; }
	}
	break;
      case '}':
	if (!inString)
	  nbrace++;
	break;
      case '{':
	if (!inString)
	{ nbrace--;
	  if (nbrace < 0)
	  { *end++ = '}';
	    *end = '\0';
	    nbrace++; }
	}
	break; }
    q--; }
}
/*----------------------------------------------------*/
Int strsame(char *str1, char *str2)
/* returns the number of initial characters that are the same in */
/* strings <str1> and <str2> */
{
  Int	n = 0;

  while (*str1++ == *str2++)
    n++;
  return n;
}
/*----------------------------------------------------------------*/
#define SEVERAL	1
void completeWord(char *p, char *buffer, Int show)
/* tries to complete the Word at *p depending on existing names */
/* of variables and routines */
{
  char	*p1, *p2, *p3, *q, *complete = NULL;
  Int	size, n, nRep, completeSize;
  internalRoutine	*r;
  hashTableEntry	*he, **h, ***ht;
  static hashTableEntry	**hts[4] =
  { varHashTable, subrHashTable, funcHashTable, blockHashTable };
  static char *marker[4] =
  { "(b)", "(f)", "(s)", "" };
  Int	matchInternalName(char *name, internalRoutine *table, Int size,
			  Int hi);
  void	printwf(char *fmt, ...);

  if (!*buffer)			/* nothing to complete */
    return;
  p1 = prevBOW(p, buffer);	/* previous beginning-of-Word */
  p2 = nextEOW(p1);		/* next end-of-Word after that */
  size = p2 - p1;		/* size of the Word */
  q = (char *) scrat;
  p3 = p1;
  for (n = 0; n < size; n++)	/* make uppercase for comparison */
    *q++ = toupper(*p3++);
  q = (char *) scrat;
  if (show) {
    cursor_goto(0,0);
    putchar('\r');
    printf(cl_eos);
    printw("E-w>\n");
  }
  /* now seek the name */
  /* first compare with internal subroutines */
  n = matchInternalName(q, subroutine, size, nSubroutine);
  if (n >= 0) {
    r = &subroutine[n];
    nRep = nSubroutine - n;
    while (nRep--) {
      if (!strncmp(r->name, q, size)) {
	if (show)
	  printwf("%s, ", r->name);
	else if (!complete) {
	  complete = r->name;
	  completeSize = strlen(complete);
	} else
	  completeSize = strsame(r->name, complete);
	r++;
      }
      else
	break;
    }
  }
  /* compare with internal functions */
  n = matchInternalName(q, function, size, nFunction);
  if (n >= 0) {
    r = &function[n];
    nRep = nFunction - n;
    while (nRep--) {
      if (!strncmp(r->name, q, size)) {
	if (show)
	  printwf("%s() ", r->name);
	else if (!complete) {
	  complete = r->name;
	  completeSize = strlen(complete);
	} else
	  completeSize = strsame(r->name, complete);
	r++;
      }
      else
	break;
    }
  }
  /* compare with user-defined stuff */
  n = 4;
  ht = hts;
  while (n--) {
    h = *ht++;
    nRep = HASHSIZE;
    while (nRep--) {
      he = *h++;
      if (he) {
	while (he) {
	  if (!strncmp(he->name, q, size)) {
	    if (show)
	      printf("%s%s ", he->name, marker[n]);
	    else if (!complete) {
	      complete = he->name;
	      completeSize = strlen(complete);
	    } else
	      completeSize = strsame(he->name, complete); 
	    he = he->next;
	  }
	  else
	    break;
	}
      }
    }
  }
  if (show) {
    if (!col)
      printw("\n");
    printf(thePrompt);
    out(buffer, 0);
    advance(p - buffer);
  } else {
    if (completeSize != size)
      memcpy(p1 + completeSize, p1 + size,
	     strlen(p1 + size) + 1);
    strncpy(p1 += size, complete + size, completeSize - size);
    if (islower((Byte) p1[-1])) { /* make completion lowercase, too */
      for (n = 0; n < completeSize - size; n++) {
	*p1 = tolower(*p1);
	p1++;
      }
    }
  }
}
/*----------------------------------------------------------------*/
void help(void)
{
  char	*fmt = "%-16s %-7s %-7s %-7s %-11s %-7s\n";

  putchar('\n');
  printf(fmt, "", "char", "Word", "expr", "line", "command");
  printf(fmt, "Move forward", "C-f", "ESC-f", "ESC C-f", "ESC-e, DOWN", "C-e");
  printf(fmt, "Move backward", "C-b", "ESC-b", "ESC C-b", "ESC-a, UP", "C-a");
  printf(fmt, "Delete forward", "C-d,DEL", "ESC-d", "ESC C-k", "ESC-k", "C-k");
  printf(fmt, "Delete backward", "BKS", "ESC-DEL", "ESC-BKS", "C-x DEL",
	 "C-x k");
  printf(fmt, "Transpose", "C-t", "ESC-t", "ESC C-t", "", "");
  puts("ESC-( inserts parentheses; ESC-[ brackets; ESC-{ braces.");
  puts("ESC-) close open groups; ESC-w complete Word; C-x RET quit.");
  puts("ESC-h keyboard help; C-r find; ESC-% replace.");
  puts("C-n, C-p, UP, DOWN retrieve.");
  puts("INS insert/overwrite toggle; C-g join/replace toggle.");
  printf("hlp>");
}
/*----------------------------------------------------*/
void transpose(char *p1, char *p2, char *p3, char *p4)
/* switch the text between p1 and p2 (exclusive) with the text between */
/* p3 and p4 (exclusive), shifting the text between p2 and */
/* p3 (exclusive) accordingly. */
{
  Int	n;
  char	*cscrat = (char *) scrat;

  n = p4 - p3;
  memcpy(cscrat, p3, n);
  memcpy(cscrat + n, p2, p3 - p2);
  n += p3 - p2;
  memcpy(cscrat + n, p1, p2 - p1);
  memcpy(p1, cscrat, p4 - p1);
  return;
}
/*----------------------------------------------------*/
static char	historyLine[HISTORYSIZE][BUFSIZE];
static Int	historyIndex = 0;
/*----------------------------------------------------*/
void inHistory(char *text)
{
  Int prev = historyIndex - 1;
  if (prev < 0)
    prev = HISTORYSIZE - 1;
  if (!historyLine[prev] || strcmp(historyLine[prev], text)) {
    /* only store if different from previous one */
    strcpy(historyLine[historyIndex], text);
    historyIndex++;
    if (historyIndex == HISTORYSIZE)
      historyIndex = 0;		/* wrap-around */
  }
}
/*----------------------------------------------------*/
Int readHistory(void)
/* reads history from a history file (~/.lux-history) */
{
  FILE	*fp;
  Int	i;
  char	*p;

  fp = Fopen(expand_name("~/.lux-history", NULL), "r");
  if (!fp)			/* no such file */
    return 0;
  fread(&historyIndex, sizeof(Int), 1, fp);
  for (i = 0; i < HISTORYSIZE; i++) {
    fgets(p = historyLine[i], BUFSIZE - 1, fp);
    p += strlen(p);
    if (p[-1] == '\n')
      p[-1] = '\0';
  }
  Fclose(fp);
  return 1;
}
/*----------------------------------------------------*/
Int saveHistory(void)
/* saves history in a history file (~/.lux-history) */
{
  FILE	*fp;
  Int	i;
  char	*expand_name(char *, char *);

  fp = Fopen(expand_name("~/.lux-history", NULL), "w");
  if (!fp)
    return anaerror("Cannot open file ~/.lux-history to save input line history",
		 0);
  fwrite(&historyIndex, sizeof(Int), 1, fp);
  for (i = 0; i < HISTORYSIZE; i++)
    fprintf(fp, historyLine[i]? "%s\n": "\n", historyLine[i]);
  Fclose(fp);
  return 1;
}
/*----------------------------------------------------*/
Int charpending(void)
/* returns 0 if the inputStream is stdin and no character is pending on */
/* the input stream, and 1 otherwise. */
{
  struct termios	io_params, old_io_params;
  Int	c;

  if (inputStream != stdin	/* not from standard input */
      || tcgetattr(1, &io_params) < 0) /* or couldn't get terminal */
				       /* attributes */
    return 1;
  old_io_params = io_params;
  io_params.c_cc[VMIN] = 0;
  io_params.c_cc[VTIME] = 0;
  if (tcsetattr(1, TCSANOW, &io_params) < 0)
    return 1;
  c = getc(stdin);		/* attempt to read a character */
  if (c >= 0) 			/* we got one */
    ungetc(c, stdin);		/* push it back onto the input stream */
  tcsetattr(1, TCSANOW, &old_io_params); /* restore previous state */
  return (c >= 0)? 1: 0;
}
/*----------------------------------------------------*/
Int getNewLine(char *buffer, char *prompt, char historyFlag)
/* reads new line from keyboard into buffer; returns length
   (terminating null isn't included in count).
   includes history buffer, Word-by-Word movement, search in
   the history buffer, search & replace.
   historyFlag determines whether the history buffer is enabled.
   If End-Of-File is reached on the input stream, then -1 is
   returned. */
{
  static char	find[BUFSIZE], oldLine[BUFSIZE], openers[] = "([{",
    closers[] = ")]}";
  char	*p, insertFlag = 1, joinFlag = 0, cc, changed = '\0',
	*p2, *b1, *b2, *b3, *b4, finding = '\0';
  Int	size, c, n, indx, joinSize = 0, oldC, findIndex, n2,
	prevC;
  void	Quit(Int);

  if (noPrompt
      || (inputStream != stdin
	  && !echo))
    show = 0;
  else
    show = 1;
  promptLength = strlen(prompt);
  textWidth = uTermCol - promptLength - 1;
  if (textWidth < 1) {
    printwf("WARNING - screen width !COL = %1d is too narrow\nwidening to %1d\n", uTermCol, promptLength + 2);
    uTermCol = promptLength + 2;
    textWidth = uTermCol - promptLength - 1;
  }
  if (show)
    printf("\r%s", prompt);
  thePrompt = prompt;		/* make available to other routines */
  p = buffer;
  *p = '\0';
  indx = historyIndex;
  size = col = row = 0;
  c = 0;
  while (size < BUFSIZE) {
    /*    FOR DEBUGGING
    printf("%s\033[23;70H", c_save);
    printf("%3d%3d%s", col, row, c_restore);
    fflush(stdout);
    */
    prevC = c;
    c = getChar();
    if (c != FIND)
      finding = '\0';
    do {
      oldC = c;

      switch (c) {
	/* INSERTIONS */
	case PAREN:		/* insert parentheses */
	  if (inputStream == stdin) { /* only insert multiple if reading */
				/* from keyboard */
	    if (insertFlag) {
	      if (size + 2 > BUFSIZE) {
		if (show)
		  setPrompt("FUL>");
		break;
	      }
	      memmove(p + 2, p, strlen(p) + 1);
	      memcpy(p, "()", 2);
	      if (show) {
		out(p, 0);
		advance(1);
	      }
	      p++;
	      size += 2;
	    } else {
	      if (!*p) {
		if (size + 2 > BUFSIZE) {
		  if (show)
		    setPrompt("FUL>");
		  break;
		}
		p[2] = '\0';
		size += 2;
	      } else if (!p[1]) {
		size++;
		p[2] = '\0';
	      }
	      memcpy(p, "()", 2);
	      if (show)
		printf("()");
	      p += 2;
	    }
	  } else {		/* when reading from a file only insert one */
	    if (insertFlag) {
	      memmove(p + 1, p, strlen(p) + 1);
	      *p++ = '(';
	      size++;
	    } else {
	      if (!*p) {
		p[1] = '\0';
		size++;
	      }
	      *p++ = '(';
	    }
	  }
	  break;
	case BRACK:		/* insert brackets */
	  if (inputStream == stdin) { /* only insert multiple if reading */
				/* from keyboard */
	    if (insertFlag) {
	      if (size + 2 > BUFSIZE) {
		if (show)
		  setPrompt("FUL>");
		break;
	      }
	      memmove(p + 2, p, strlen(p) + 1);
	      memcpy(p, "[]", 2);
	      if (show) {
		out(p, 0);
		advance(1);
	      }
	      p++;
	      size += 2;
	    } else {
	      if (!*p) {
		if (size + 2 > BUFSIZE) {
		  if (show)
		    setPrompt("FUL>");
		  break;
		}
		p[2] = '\0';
		size += 2;
	      } else if (!p[1]) {
		size++;
		p[2] = '\0';
	      }
	      memcpy(p, "[]", 2);
	      if (show)
		printf("[]");
	      p += 2;
	    }
	  } else {		/* when reading from a file only insert one */
	    if (insertFlag) {
	      memmove(p + 1, p, strlen(p) + 1);
	      *p++ = '[';
	      size++;
	    } else {
	      if (!*p) {
		p[1] = '\0';
		size++;
	      }
	      *p++ = '[';
	    }
	  }
	  break;
	case BRACE:	/* insert braces */
	  if (inputStream == stdin) { /* only insert multiple if reading */
				/* from keyboard */
	    if (insertFlag) {
	      if (size + 2 > BUFSIZE) {
		if (show)
		  setPrompt("FUL>");
		break;
	      }
	      memmove(p + 2, p, strlen(p) + 1);
	      memcpy(p, "{}", 2);
	      if (show) {
		out(p, 0);
		advance(1);
	      }
	      p++;
	      size += 2;
	    } else {
	      if (!*p) {
		if (size + 2 > BUFSIZE) {
		  if (show)
		    setPrompt("FUL>");
		  break;
		}
		p[2] = '\0';
		size += 2;
	      } else if (!p[1]) {
		size++;
		p[2] = '\0';
	      }
	      memcpy(p, "{}", 2);
	      if (show)
		printf("{}");
	      p += 2;
	    }
	  } else {		/* when reading from a file only insert one */
	    if (insertFlag) {
	      memmove(p + 1, p, strlen(p) + 1);
	      *p++ = '{';
	      size++;
	    } else {
	      if (!*p) {
		p[1] = '\0';
		size++;
	      }
	      *p++ = '{';
	    }
	  }
	  break;
	case CGR:		/* complete groups */
	  close_groups(buffer);
	  if (size != strlen(buffer)) {
	    size = strlen(buffer);
	    changed = 1;
	  }
	  cursor_goto(0,0);
	  out(buffer, 0);
	  advance(p - buffer);
	  break;
	case CPW:		/* complete Word */
	  if (prevC == CPW)
	    completeWord(p, buffer, 1);
	  else {
	    completeWord(p, buffer, 0);
	    if (size != strlen(buffer)) {
	      size = strlen(buffer);
	      changed = 1;
	    }
	  }
	  out(p, 0);
	  p2 = nextEOW(p);
	  advance(p2 - p);
	  p = p2;
	  break;

      /* DELETIONS */
	case DEL: case DLC:	/* delete char (forward) */
	  if (*p) {
	    memmove(p, p + 1, strlen(p));
	    size--;
	    changed = 1;
	    if (show)
	      out(p, 1);
	  }
	  break;
	case BKS:		/* backspace char (backward) */
	  if (p > buffer) {
	    memmove(p - 1, p, strlen(p - 1));
	    size--;
	    p--;
	    changed = 1;
	    if (show) {
	      advance(-1);
	      out(p, 1);
	    }
	  }
	  break;
	case DLW:		/* delete Word forward */
	  p2 = nextEOW(p);
	  memmove(p, p2, strlen(p2) + 1);
	  size -= p2 - p;
	  out(p, p2 - p);
	  break;
	case BDW:		/* delete Word backward */
	  p2 = prevBOW(p, buffer);
	  memmove(p2, p, strlen(p) + 1);
	  size -= (n = p - p2);
	  advance(-n);
	  p = p2;
	  out(p, n);
	  break;
	case DLE:		/* delete expression forward */
	  p2 = nextEOE(p, buffer);
	  memmove(p, p2, strlen(p2) + 1);
	  size -= p2 - p;
	  out(p, p2 - p); 
	  break;
        case BDE:		/* delete expression backward */
	  p2 = prevBOE(p, buffer);
	  memmove(p2, p, strlen(p) + 1);
	  size -= (n = p - p2);
	  advance(-n);
	  p = p2;
	  out(p, n);
	  break;
	case DLS:		/* delete screen line forward */
	  if (*p) {
	    if (row == size/textWidth) { /* last screen line */
	      n = strlen(p);
	      *p = '\0';
	      size = strlen(buffer);
	      if (show)
		out(p, n);
	    } else {
	      n = textWidth - col;
	      memmove(p, p + n, strlen(p + n) + 1);
	      size -= n;
	      if (show)
		out(p, n);
	    }
	    changed = 1;
	  }
	  break;
	case BDS:		/* delete screen line backward */
	  if (p > buffer) {
	    n = col;
	    memmove(p - n, p, strlen(p) + 1);
	    size -= n;
	    p -= n;
	    changed = 1;
	    if (show) {
	      cursor_goto(0, row);
	      out(p, n);
	    }
	  }
	  break;
	case DLL:		/* delete command line forward */
	  if (*p) {
	    n = strlen(p);
	    *p = '\0';
	    size = strlen(buffer);
	    changed = 1;
	    if (show)
	      out(p, n);
	  }
	  break;
	case BDL:		/* delete command line backward */
	  if (p > buffer) {
	    memmove(buffer, p, strlen(p) + 1);
	    n = p - buffer;
	    p = buffer;
	    size = strlen(buffer);
	    changed = 1;
	    if (show) {
	      cursor_goto(0, 0);
	      out(p, n);
	    }
	  }
	  break;

      /* TRANSPOSITIONS */
	case TRC:		/* transpose chars */
	  if (p > buffer && *p && size > 1) {
	    cc = p[-1];
	    p[-1] = p[0];
	    p[0] = cc;
	    if (show) {
	      advance(-1);
	      outChar(p[-1]);
	      outChar(*p);
	    }
	    changed = 1;
	    p++;
	  }
	  break;
	case TRW:		/* transpose words */
	  b1 = prevBOW(p, buffer);
	  b2 = nextEOW(b1);
	  b4 = nextEOW(b2);
	  b3 = prevBOW(b4, buffer);
	  transpose(b1,b2,b3,b4);
	  if (show) {
	    b2 = p;
	    advance(b1 - p);
	    p = b1;
	    out(p, 0);
	    advance(b4 - p);
	    changed = 1;
	    p = b4;
	  }
	  break;
	case TRE:		/* transpose expressions */
	  b1 = prevBOE(p, buffer);
	  b2 = nextEOE(b1, buffer);
	  b4 = nextEOE(b2, buffer);
	  b3 = prevBOE(b4, buffer);
	  transpose(b1,b2,b3,b4);
	  if (show) {
	    b2 = p;
	    advance(b1 - p);
	    p = b1;
	    out(p, 0);
	    advance(b4 - p);
	    changed = 1;
	    p = b4;
	  }
	  break;

	  /* MOVEMENT */
	case FWC: case RAR:	/* forward char */
	  if (*p) {
	    p++;
	    if (show)
	      advance(1);
	  }
	  break;
	case BKC: case LAR:	/* backward char */
	  if (p > buffer) {
	    p--;
	    if (show)
	      advance(-1);
	  }
	  break;
	case FWW:		/* forward Word */
	  p2 = nextEOW(p);
	  if (show)
	    advance(p2 - p);
	  p = p2;
	  break;
	case BKW:		/* backward Word */
	  p2 = prevBOW(p, buffer);
	  if (show)
	    advance(p2 - p);
	  p = p2;
	  break;
	case FWE:		/* forward expression */
	  p2 = nextEOE(p, buffer);
	  if (show)
	    advance(p2 - p);
	  p = p2;
	  break;
	case BKE:		/* backward expression */
	  p2 = prevBOE(p, buffer);
	  if (show)
	    advance(p2 - p);
	  p = p2;
	  break;
	case UAR:		/* uparrow (one line up) */
	  if (row) {
	    if (show)
	      cursor_goto(col, row - 1);
	    p -= textWidth;
	  } else
	    c = BKL;		/* get previous line from history list */
	  break;
	case DAR:		/* dowarrow (one line down) */
	  if (row < (size - 1)/textWidth) { /* we use (size - 1) because
					       it is OK to be at textWidth
					       on the last line */
	    n = col;
	    if (row == size/textWidth - 1
		&& col > size%textWidth) {
	      n = size%textWidth;
	      p += strlen(p);
	    } else 
	      p += textWidth;
	    if (show)
	      cursor_goto(n, row + 1);
	  }
	  else
	    c = FWL;		/* get next line from history list */
	  break;
	case BGS:		/* beginning of screen line */
	  if (col) {
	    if (show)
	      cursor_goto(0, row);
	    p -= col;
	  } else if (row) {
	    if (show)
	      cursor_goto(0, row - 1);
	    p -= textWidth;
	  }
	  break;
	case ENS:		/* end of screen line */
	  if (col != textWidth - 1) {
	    n = (size/textWidth == row)? size%textWidth: textWidth - 1;
	    if (show)
	      cursor_goto(n, row);
	    p += n - col;
	  } else if (row < size/textWidth) {
	    n = (size/textWidth == row + 1)? size%textWidth: textWidth - 1;
	    if (show)
	      cursor_goto(n, row + 1);
	    p += n - col;
	  }
	  break;
	case BGL:		/* beginning of command line */
	  if (show)
	    cursor_goto(0, 0);
	  p = buffer;
	  break;
	case ENL:		/* end of command line */
	  if (show) {
	    if (size%textWidth == 0)
	      cursor_goto(textWidth, size/textWidth - 1);
	    else
	      cursor_goto(size%textWidth, size/textWidth);
	  }
	  p = buffer + size;
	  break;
	  
      /* RETRIEVAL */
	case UNDO:		/* restore */
	  strcpy(buffer, oldLine);
	  size = strlen(oldLine);
	  printf(cl_eos);
	  cursor_goto(0,0);
	  p = buffer;
	  out(p,0);
	  break;
	case BKL:		/* get previous line from history */
	  if (!historyFlag)	/* no history */
	    break;
	  if (changed)
	    strcpy(historyLine[historyIndex], buffer);
	  if (--indx < 0)	/* wrap-around */
	    indx = HISTORYSIZE - 1;
	  strcpy(buffer + joinSize, historyLine[indx]);
	  size = strlen(buffer);
	  p = buffer + size;
	  changed = 0;
	  if (show) {
	    cursor_goto(joinSize,0);
	    printf(cl_eos);
	    out(buffer + joinSize, 0);
	    if (size && size % textWidth == 0) /* at end of line */
	      cursor_goto(textWidth, (size - 1)/textWidth);
	    else
	      cursor_goto(size % textWidth, size/textWidth);
	  }
	  break;
	case FWL:		/* get next line from history */
	  if (!historyFlag)
	    break;
	  if (changed)
	    strcpy(historyLine[historyIndex], buffer);
	  if (++indx == HISTORYSIZE)
	    indx = 0;
	  strcpy(buffer + joinSize, historyLine[indx]);
	  size = strlen(buffer);
	  p = buffer + size;
	  changed = 0;
	  if (show) {
	    cursor_goto(joinSize,0);
	    printf(cl_eos);
	    out(buffer + joinSize, 0);
	    if (size % textWidth == 0)
	      cursor_goto(textWidth, (size - 1)/textWidth);
	    else
	      cursor_goto(size % textWidth, size/textWidth);
	  }
	  break;
	case FIND:		/* find a particular string in the history */
				/* lines */
	  if (changed)
	    strcpy(historyLine[historyIndex], buffer);
	  if (!finding) {
	    getNewLine(find, "fnd>", 0);
	    thePrompt = prompt;
	    if (show) {
	      cursor_goto(0,0);
	      printf(cl_eos);
	      setPrompt(prompt);
	    }
	    finding = (char) 1;
	    findIndex = indx--;
	  } else
	    indx--;
	  while (findIndex != indx) {
	    if (indx < 0)
	      indx = HISTORYSIZE - 1;
	    if (strstr(historyLine[indx], find))
	      break;
	    indx--;
	  }
	  if (findIndex == indx)
	    strcpy(buffer + joinSize, historyLine[historyIndex]);
	  else
	    strcpy(buffer, historyLine[indx]);
	  p = buffer;
	  size = strlen(buffer);
	  changed = 0;
	  if (show) {
	    cursor_goto(0,0);
	    printf(cl_eos);
	    out(p, 0); 
	    if (joinSize)
	      advance(joinSize);
	    else {
	      advance(strlen(p));
	      p += strlen(p);
	    }
	  }
	  continue;
	  
	  /* MISCELLANEOUS */
	case SUBST:		/* substitute */
	  if (show) {
	    cursor_goto(0,0);
	    printf(cl_eos);
	  }
	  getNewLine(find, "F&R>", 0);
	  thePrompt = prompt;
	  if (show) {
	    cursor_goto(0,0);
	    printf(cl_eos);
	    setPrompt(prompt);
	  }
	  cc = find[0];
	  if (cc)
	    p2 = strchr(find + 1, cc);
	  if (!cc || !p2) {
	    if (show) {
	      out(buffer, 0);
	      advance(p - buffer);
	    }
	    break;
	  }
	  *p2++ = '\0';
	  p = buffer;
	  n2 = strlen(p2);
	  n = n2 + (find - p2) + 2;
	  while ((p = strstr(p, find + 1))) {
	    if (n > 0)
	      memmove(p + n, p, strlen(p) + 1);
	    else if (n < 0)
	      memmove(p, p - n, strlen(p - n) + 1);
	    memcpy(p, p2, n2);
	    p += n2;
	  }
	  p = buffer;
	  find[0] = 0;
	  size += n;
	  if (show)
	    out(p, 0);
	  break;
	case EOF: case RET:	/* end of file or return, enter */
	  if (c == EOF && !*buffer) /* nothing entered */
	    return -1;
	  p += strlen(p);
	  n = size/textWidth - !(size % textWidth);
	  if (show && buffer != find) {
	    while (row++ < n)
	      putchar('\n');
	    putchar('\n');
	  }
#ifdef IGNORETHIS
	  if (p > buffer && p[-1] == '-') { /* continuation */
	    break;
	  }
#endif
	  if (size && historyFlag) /* enter into history list */
	    inHistory(buffer);
	  if (echo)
	    puts(buffer);
	  joinFlag = 0;
	  joinSize = 0;
	  fflush(stdout);
	  return size;
	case INS:		/* insert/overwrite toggle */
	  insertFlag = 1 - insertFlag;
	  if (show)
	    setPrompt(insertFlag? "ins>": "ovw>");
	  break;
	case JOIN:		/* join/replace toggle */
	  if (historyFlag) {
	    joinFlag = 1 - joinFlag;
	    joinSize = joinFlag? size: 0;
	  }
	  break;
	case HELP:
	  if (changed)
	    strcpy(historyLine[historyIndex], buffer);
	  cursor_goto(0,0);
	  printf(cl_eos);
	  setPrompt("hlp>");
	  help();
	  cursor_goto(0,0);
	  setPrompt(prompt);
	  out(buffer, 0);
	  advance(p - buffer);
	  break;
	case QUIT:
	  if (col)
	    putchar('\n');
	  Quit(0);
	  break;
	default:		/* ordinary character */
	  if (!isprint(c))	/* non-printable characters */
	    c = ' ';		/* transform to whitespace */
	  if (!*p) {		/* at end of line */
	    *p++ = c;
	    size++;
	    *p = '\0';
	    if (show)
	      outChar(c);
	  } else if (insertFlag) { /* inserting */
	    memmove(p + 1, p, strlen(p) + 1);
	    *p++ = c;
	    size++;
	    if (show) {
	      out(p - 1, 0);
	      advance(1);
	    }
	  } else {		/* overstrike */
	    *p++ = c;
	    if (show)
	      outChar(c);
	  }
	  changed = 1;		/* the current command line has been changed */

	  /* if we close parentheses, brackets, or braces, then we briefly */
	  /* move the cursor to the matching opening parenthesis, bracket, */
	  /* or brace */
	  if ((p2 = strchr(closers, c))/* group closing */
	      && !charpending()) {/* and no keystroke pending */
	    /* find the matching opener */
	    n2 = p2 - closers;	/* the index */
	    p2 = p - 1;
	    n = 1;
	    do {
	      --p2;
	      if (*p2 == closers[n2])
		n++;
	      else if (*p2 == openers[n2])
		n--;
	    } while (p2 > buffer && n);
	    if (!n) {		/* the corresponding opening parenthesis is */
				/* at p2 */
	      n = p - p2;	/* distance between matching () */
	      advance(-n);	/* go to matching ( */
	      fflush(stdout);	/* or it won't show */
	      usleep(500000);	/* wait half a second */
	      advance(n);	/* go back */
	    }
	  }
	  break;
      }
    } while (c != oldC);
  }
  return size;
}
/*----------------------------------------------------*/
