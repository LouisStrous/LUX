/* This is file editor.h.

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
#ifdef _ANA_EDITOR_H_
#else
#define _ANA_EDITOR_H_

#include <stdio.h>
#include <string.h>

	/* buffer limits */
#define BUFSIZE		512	/* line input buffer */
#define HISTORYSIZE	100	/* # lines in historybuffer */

#define HISTORYBUFFER	1	/* in calls to getNewLine */

	/* line editor definitions */
/* a "line" is only complete when the RETURN key is pressed.  [L]
   a "sentence" fits in a single line on the screen.  [S]
   an "exp" is a single unambiguous expression.  [E]
   a "Word" is a single Word.  [W]
   a "char" is a single character. [C] */
/* the codes are all the same as in standard Emacs except where noted */

/* general: */
#define ESC	'\033'	/* ESC */
#define TAB	'\011'	/* TAB */
#define RET	'\012'	/* RET */
#define QUIT	0x40a	/* C-x RET (exit) NON-EMACS */
#define PAREN	0x228	/* ESC-(  (insert parentheses) */
#define BRACK   0x25b   /* ESC-[  (insert brackets) NON-EMACS */
#define BRACE   0x27b   /* ESC-{  (insert braces) NON-EMACS */
#define HELP	0x248	/* ESC-h  (help) NON-EMACS */
#define BKS	0x101
#define DEL	0x102
#define INS	0x103
#define RAR	0x104
#define LAR	0x105
#define UAR	0x106
#define DAR	0x107
#define CGR	0x229   /* ESC-)  (complete groups) NON-EMACS */
#define CPW	0x257   /* ESC-w  (complete Word) NON-EMACS */
#define ARG	'\x15'  /* C-u    (argument) */

/* line: */
#define BGL	'\x01'	/* start of line		(C-a) */
#define ENL	'\x05'	/* end of line			(C-e) */
#define FWL	'\x0e'	/* next line from history	(C-n) */
#define BKL	'\x10'	/* previous line from history	(C-p) */
#define DLL	'\x0b'	/* kill line forward		(C-k) */
#define BDL	0x44b	/* kill line back		(C-x k) */

/* sentence: */
#define BGS	0x241	/* start of sentence		(ESC-a) */
#define ENS	0x245	/* end of sentence		(ESC-e) */
#define DLS	0x24b	/* kill sentence forward	(ESC-k) */
#define BDS	(0x400 + DEL)	/* kill sentence back	(C-x del) */

/* expr: */
#define FWE	0x206	/* forward expr			(ESC C-f) */
#define BKE	0x202	/* back expr			(ESC C-b) */
#define DLE	0x20b	/* kill expr forward		(ESC C-k) */
#define BDE	0x301    /* kill expr back		(ESC BKS) NON-EMACS */
#define TRE	0x214	/* transpose exprs		(ESC C-t) */

/* Word: */
#define FWW	0x246	/* forward Word			(ESC-f) */
#define BKW	0x242	/* back Word			(ESC-b) */
#define DLW	0x244	/* delete Word forward		(ESC-d) */
#define BDW	(0x200 + DEL)	/* delete Word back	(ESC-del) */
#define TRW	0x254	/* transpose words		(ESC-t) */

/* char: */
#define FWC	'\x06'	/* forward char			(C-f) */
#define BKC	'\x02'	/* back char			(C-b) */
#define DLC	'\x04'	/* delete char forward		(C-d) */
#define BDC	BKS	/* delete char back		(del) */
#define TRC	'\x14'	/* transpose chars		(C-t) */

/* extra: */
#define FIND	'\x12'	/* find				(C-r) */
#define JOIN	'\x7'	/* join				(C-g) */
#define SUBST	0x225	/* replace			(ESC-%) */
#define UNDO	'\x1f'  /* undo				(C-?) */

#endif
