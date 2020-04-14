/* This is file format.hh.

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
// header file for LUX string formatting

// format_check store choices
#define STORE	1
#define NOSTORE	0

// format_check return value macros
#define	isIntegerFormat(A)	((A >= 0 && A < 6)? 1: 0)
#define isFloatFormat(A)	((A > 5 && A < 14)? 1: 0)
#define isStringFormat(A)	((A == 14)? 1: 0)

