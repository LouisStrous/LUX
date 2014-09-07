/* This is file hershey.hh.

Copyright 2013 Louis Strous, Richard Shine
Copyright 2014 Louis Strous

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
#ifndef HAVE_HERSHEY_H
#define HAVE_HERSHEY_H

enum { HERSHEY_DRAW, HERSHEY_MOVE, HERSHEY_END, HERSHEY_ERR = -1 };

typedef int32_t hershey_handle;

hershey_handle hershey_exists(int32_t hershey_char);
int32_t hershey_coords(hershey_handle *handle, int32_t *x, int32_t *y);
int32_t hershey_max_handle(void);
int32_t hershey_max_char(void);
char *hershey_set_filename(char *filename);

#endif

