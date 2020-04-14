/* This is file editorcharclass.hh.

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
// editorcharclass.h

extern char eclass[];

	// character classes
#define charClass(c)		(eclass[(int32_t) c])
#define editorCharClass(c)	(charClass(c) & 0x03)
// distinguishes between WHITESPACE_CHAR, SEPARATOR_CHAR, and ORDINARY_CHAR
#define isFirstChar(c)		(charClass(c) & 0x04)
// may appear at start of identifier
#define isNextChar(c)		(charClass(c) & 0x08)
// may appear later on in identifier
#define isNumberChar(c)		(charClass(c) & 0x10)
// all numbers that may appear in numbers: 0-9, A-F, O, X, W, B, L, period
#define isTwoOperator(c)	(charClass(c) & 0x20)
// may be first char of a two-char operator *= += -= /= ^=
#define isFileNameChar(c)	(charClass(c) & 0x40)
// may appear in a file name: lower case, upper case, _$/.
#define isNotCapital(c)		(charClass(c) != 0x40)
#define isWhiteSpace(c)		(charClass(c) == 0)
#define isSeparatorChar(c)	(charClass(c) & 0x01)
#define isOrdinaryChar(c)	(charClass(c) & 0x02)



