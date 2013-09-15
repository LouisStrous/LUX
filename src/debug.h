/* This is file debug.h.

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
/* File debug.h
 "$Id"
 Replaces allocation routines with similar routines that also keep records
 of the transactions so that consistency can be checked.
 LS 31jul98 */

#define malloc	Malloc
#define calloc	Calloc
#define realloc	Realloc
#define free	Free


