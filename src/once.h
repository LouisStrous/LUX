/* This is file once.h.

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
#include "luxdefs.h"

/* editorCharClass table */

/* 0x00 whitespace

   0x01 separator
   0x02 ordinary char
   0x04 may appear at start of identifier A-Z ! $ #
   0x08 may appear in identifier after start A-Z _ 0-9 $
   0x10 may appear in number:  0-9 A-F O W X B . + -
   0x20 first char of a double-char operator
   0x40 may appear in a file name
*/

char	class[] = {
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*                    !     "     #     $     %     &     ' */
 0x00, 0x00, 0x00, 0x06, 0x41, 0x46, 0x4e, 0x41, 0x41, 0x41,
/*  (     )     *     +     ,     -     .     /     0     1 */
 0x41, 0x41, 0x21, 0x71, 0x41, 0x71, 0x51, 0x61, 0x5a, 0x5a,
/*  2     3     4     5     6     7     8     9     :     ; */
 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x41, 0x41,
/*  <     =     >     ?     @     A     B     C     D     E */
 0x41, 0x41, 0x41, 0x41, 0x41, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
/*  F     G     H     I     J     K     L     M     N     O */
 0x5e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x5e, 0x4e, 0x4e, 0x5e,
/*  P     Q     R     S     T     U     V     W     X     Y */
 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x5e, 0x4e,
/*  Z     [     \     ]     ^     _     `     a     b     c */
 0x4e, 0x41, 0x41, 0x41, 0x61, 0x4a, 0x41, 0x5e, 0x5e, 0x5e,
/*  d     e     f     g     h     i     j     k     l     m */
 0x5e, 0x5e, 0x5e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x5e, 0x4e,
/*  n     o     p     q     r     s     t     u     v     w */
 0x4e, 0x5e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e,
/*  x     y     z     {     |     }     ~                   */
 0x5e, 0x4e, 0x4e, 0x41, 0x41, 0x41, 0x41, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* symbol stacks */
#include "install.h"

char		*symbolStack[SYMBOLSTACKSIZE];
hashTableEntry	*varHashTable[HASHSIZE], *subrHashTable[HASHSIZE], 
		*funcHashTable[HASHSIZE], *blockHashTable[HASHSIZE];
symTableEntry	sym[NSYM];
Int		scrat[NSCRAT], curSymbol;
Word		listStack[NLIST];
Word		curContext = 0;
Int		lux_type_size[] =
	{ sizeof(uint8_t), sizeof(Word), sizeof(Int), sizeof(float),
	  sizeof(double), sizeof(char), sizeof(char), sizeof(char *),
	  sizeof(floatComplex), sizeof(doubleComplex) };
char	*curScrat = (char *) scrat, *printString;
